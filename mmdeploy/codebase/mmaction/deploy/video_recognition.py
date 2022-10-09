# Copyright (c) OpenMMLab. All rights reserved.
from typing import Any, Dict, Optional, Sequence, Tuple, Union

import mmcv
import numpy as np
import torch
from mmcv.parallel import DataContainer
from torch.utils.data import Dataset
import os.path as osp

from mmdeploy.codebase.base import BaseTask
from mmdeploy.utils import Task, get_root_logger
from .mmaction import MMACTION_TASK


def process_model_config(model_cfg: mmcv.Config,
                         imgs: Union[Sequence[str], Sequence[np.ndarray]],
                         input_shape: Optional[Sequence[int]] = None):
    """Process the model config.

    Args:
        model_cfg (mmcv.Config): The model config.
        imgs (Sequence[str] | Sequence[np.ndarray]): Input image(s), accepted
            data type are List[str], List[np.ndarray].
        input_shape (list[int]): A list of two integer in (width, height)
            format specifying input shape. Default: None.

    Returns:
        mmcv.Config: the model config after processing.
    """
    cfg = model_cfg.deepcopy()
    test_pipeline = cfg.data.test.pipeline

    if 'Init' not in test_pipeline[0]['type']:
        test_pipeline = [dict(type='OpenCVInit')] + test_pipeline
    else:
        test_pipeline[0] = dict(type='OpenCVInit')
    for i in range(len(test_pipeline)):
        if 'Decode' in test_pipeline[i]['type']:
            test_pipeline[i] = dict(type='OpenCVDecode')
    cfg.data.test.pipeline = test_pipeline
    return cfg


@MMACTION_TASK.register_module(Task.VIDEO_RECOGNITION.value)
class VideoRecognition(BaseTask):
    """Video recognition task class.

    Args:
        model_cfg (mmcv.Config): Original PyTorch model config file.
        deploy_cfg (mmcv.Config): Deployment config file or loaded Config
            object.
        device (str): A string represents device type.
    """

    def init_backend_model(self,
                           model_files: Optional[str] = None,
                           **kwargs) -> torch.nn.Module:
        """Initialize backend model.

        Args:
            model_files (Sequence[str]): Input model files.

        Returns:
            nn.Module: An initialized backend model.
        """
        from .video_recognition_model import build_video_recognition_model
        model = build_video_recognition_model(
            model_files,
            self.model_cfg,
            self.deploy_cfg,
            device=self.device,
            **kwargs)
        return model.eval()

    def init_pytorch_model(self,
                           model_checkpoint: Optional[str] = None,
                           cfg_options: Optional[Dict] = None,
                           **kwargs) -> torch.nn.Module:
        """Initialize torch model.

        Args:
            model_checkpoint (str): The checkpoint file of torch model,
                defaults to `None`.
            cfg_options (dict): Optional config key-pair parameters.

        Returns:
            nn.Module: An initialized torch model generated by OpenMMLab
                codebases.
        """
        from mmcv.cnn.utils import revert_sync_batchnorm
        from mmaction.apis import init_recognizer
        model = init_recognizer(self.model_cfg, model_checkpoint, self.device)
        model = revert_sync_batchnorm(model)
        return model.eval()

    def create_input(self,
                     imgs: Union[str, np.ndarray, Sequence],
                     input_shape: Sequence[int] = None) \
            -> Tuple[Dict, torch.Tensor]:
        """Create input for recognizer.

        Args:
            imgs (Any): Input image(s), accepted data type are `str`,
                `np.ndarray`, `torch.Tensor`.
            input_shape (list[int]): A list of two integer in (width, height)
                format specifying input shape. Defaults to `None`.

        Returns:
            tuple: (data, img), meta information for the input image and input.
        """

        if isinstance(imgs, (list, tuple)):
            if not all(isinstance(img, str) for img in imgs):
                raise AssertionError('imgs must be strings')
        elif isinstance(imgs, str):
            imgs = [imgs]
        else:
            raise AssertionError('imgs must be strings')

        from mmcv.parallel import collate, scatter
        from mmaction.datasets.pipelines import Compose
        if isinstance(imgs, (str, np.ndarray)):
            imgs = [imgs]
        cfg = process_model_config(self.model_cfg, imgs, input_shape)
        test_pipeline = Compose(cfg.data.test.pipeline)

        data_list = []
        for img in imgs:
            # prepare data
            data = dict(filename=img, label=-1,
                        start_index=0, modality='RGB')
            # build the data pipeline
            data = test_pipeline(data)
            data_list.append(data)

        batch_data = collate(data_list, samples_per_gpu=len(imgs))
        if self.device != 'cpu':
            batch_data = scatter(batch_data, [self.device])[0]

        for k, v in batch_data.items():
            # batch_size > 1
            if isinstance(v[0], DataContainer):
                batch_data[k] = v[0].data
        return batch_data, batch_data['imgs']

    def visualize(self,
                  model,
                  image: Union[str, np.ndarray],
                  result: list,
                  output_file: str,
                  window_name: str = '',
                  show_result: bool = False,
                  opacity: float = 0.5):
        """Visualize predictions of a model.

        Args:
            model (nn.Module): Input model.
            image (str | np.ndarray): Input image to draw predictions on.
            result (list): A list of predictions.
            output_file (str): Output file to save drawn image.
            window_name (str): The name of visualization window. Defaults to
                an empty string.
            show_result (bool): Whether to show result in windows, defaults
                to `False`.
            opacity: (float): Opacity of painted segmentation map.
                    Defaults to `0.5`.
        """
        logger = get_root_logger()
        try:
            import decord
            from moviepy.editor import ImageSequenceClip
        except Exception:
            logger.warn('Please install moviepy and decord to '
                        'enable visualize for mmaction')
        # save_dir, save_name = osp.split(output_file)
        # video = decord.VideoReader(image)
        # frames = [x.asnumpy()[..., ::-1] for x in video]

    @staticmethod
    def run_inference(model, model_inputs: Dict[str, torch.Tensor]):
        """Run inference once for a video recognition model of mmaction.

        Args:
            model (nn.Module): Input model.
            model_inputs (dict): A dict containing model inputs tensor and
                meta info.

        Returns:
            list: The predictions of model inference.
        """
        return model(**model_inputs, return_loss=False)

    @staticmethod
    def get_partition_cfg(partition_type: str) -> Dict:
        raise NotImplementedError('Not supported yet.')

    @staticmethod
    def get_tensor_from_input(input_data: Dict[str, Any]) -> torch.Tensor:
        """Get input tensor from input data.

        Args:
            input_data (dict): Input data containing meta info and image
                tensor.
        Returns:
            torch.Tensor: An image in `Tensor`.
        """
        return input_data['imgs'][0]

    @staticmethod
    def evaluate_outputs(model_cfg,
                         outputs: Sequence,
                         dataset: Dataset,
                         metrics: Optional[str] = None,
                         out: Optional[str] = None,
                         metric_options: Optional[dict] = None,
                         format_only: bool = False,
                         log_file: Optional[str] = None):
        """Perform post-processing to predictions of model.

        Args:
            outputs (list): A list of predictions of model inference.
            dataset (Dataset): Input dataset to run test.
            model_cfg (mmcv.Config): The model config.
            metrics (str): Evaluation metrics, which depends on
                the codebase and the dataset.
            out (str): Output result file in pickle format, defaults to `None`.
            metric_options (dict): Custom options for evaluation, will be
                kwargs for dataset.evaluate() function. Defaults to `None`.
            format_only (bool): Format the output results without perform
                evaluation. It is useful when you want to format the result
                to a specific format and submit it to the test server. Defaults
                to `False`.
            log_file (str | None): The file to write the evaluation results.
                Defaults to `None` and the results will only print on stdout.
        """
        from mmcv.utils import get_logger
        logger = get_logger('test', log_file=log_file)

        if out:
            logger.debug(f'writing results to {out}')
            mmcv.dump(outputs, out)
        kwargs = {} if metric_options is None else metric_options
        if format_only:
            dataset.format_results(outputs, **kwargs)
        if metrics:
            dataset.evaluate(outputs, metrics, logger=logger, **kwargs)

    def get_preprocess(self) -> Dict:
        """Get the preprocess information for SDK.

        Return:
            dict: Composed of the preprocess information.
        """
        return {}

    def get_postprocess(self) -> Dict:
        """Get the postprocess information for SDK.

        Return:
            dict: Nonthing for super resolution.
        """
        return {}

    def get_model_name(self) -> str:
        """Get the model name.

        Return:
            str: the name of the model.
        """
        return ''
