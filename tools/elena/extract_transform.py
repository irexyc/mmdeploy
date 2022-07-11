# Copyright (c) OpenMMLab. All rights reserved.
# flake8: noqa
import argparse
import os
import os.path as osp
import shutil
import subprocess
from glob import glob

import mmcv
import yaml

from mmdeploy.backend.sdk.export_info import (get_preprocess,
                                              get_transform_static)
from mmdeploy.utils import load_config

ELENA_BIN = 'OpFuse'

CODEBASE = [
    'mmclassification',
    'mmdetection',
    # 'mmpose',
    'mmrotate',
    'mmocr',
    'mmsegmentation',
    'mmediting'
]

DEPLOY_CFG = {
    'Image Classification': 'configs/mmcls/classification_tensorrt_dynamic-224x224-224x224.py',
    'Object Detection': 'configs/mmdet/detection/detection_tensorrt_static-800x1344.py',
    'Instance Segmentation': 'configs/mmdet/instance-seg/instance-seg_tensorrt_static-800x1344.py',
    'Semantic Segmentation': 'configs/mmseg/segmentation_tensorrt_static-512x512.py',
    'Oriented Object Detection': 'configs/mmrotate/rotated-detection_tensorrt-fp16_dynamic-320x320-1024x1024.py',
    'Text Recognition': 'configs/mmocr/text-recognition/text-recognition_tensorrt_static-32x32.py',
    'Text Detection': 'configs/mmocr/text-detection/text-detection_tensorrt_static-512x512.py',
    'Restorers': 'configs/mmedit/super-resolution/super-resolution_tensorrt_static-256x256.py'
}  # yapf: disable

INFO = {
    'cpu':
    '''
using std::string;

void FuseFunc(void* stream, uint8_t* data_in, int src_h, int src_w, const char* format,
              int resize_h, int resize_w, const char* interpolation, int crop_top, int crop_left,
              int crop_h, int crop_w, float mean0, float mean1, float mean2, float std0, float std1,
              float std2, int pad_top, int pad_left, int pad_bottom, int pad_right, int pad_h,
              int pad_w, float pad_value, float* data_out, int data_out_num) {
    const char* interpolation_ = "nearest";
    if (strcmp(interpolation, "bilinear") == 0) {
        interpolation_ = "bilinear";
    }
    FuseKernel(resize_h, resize_w, crop_h, crop_w, crop_top, crop_left, mean0, mean1, mean2, std0, std1, std2,
                pad_h, pad_w, pad_top, pad_left, pad_bottom, pad_right, pad_value, data_in, data_out,
                src_h, src_w, format, interpolation_);
}

REGISTER_FUSE_KERNEL(#TAG#_cpu, "#TAG#_cpu",
                     FuseFunc);
''',
    'cuda':
    '''
void FuseFunc(void* stream, uint8_t* data_in, int src_h, int src_w, const char* format,
              int resize_h, int resize_w, const char* interpolation, int crop_top, int crop_left,
              int crop_h, int crop_w, float mean0, float mean1, float mean2, float std0, float std1,
              float std2, int pad_top, int pad_left, int pad_bottom, int pad_right, int pad_h,
              int pad_w, float pad_value, float* data_out, int data_out_num) {
  cudaStream_t stream_ = (cudaStream_t)stream;
  const char* interpolation_ = "nearest";
  if (strcmp(interpolation, "bilinear") == 0) {
    interpolation_ = "bilinear";
  }

  uint8_t* d_in;
  size_t sz = src_h * src_w;
  int data_out_num_ = data_out_num;
  if (strcmp(format, "RGB") == 0 || strcmp(format, "BGR") == 0) {
    sz *= 3;
    data_out_num_ *= 3;
  } else if (strcmp(format, "RGBA") == 0) {
    sz *= 4;
    data_out_num_ *= 4;
  } else if (strcmp(format, "NV12") == 0 || strcmp(format, "NV21") == 0) {
    sz = sz * 3 / 2;
    data_out_num_ = data_out_num_ * 3 / 2;
  }
  cuErrCheck(cudaMalloc(&d_in, sz * sizeof(uint8_t)));
  cuErrCheck(cudaMemcpyAsync(d_in, data_in, sz * sizeof(uint8_t), cudaMemcpyHostToDevice, stream_));

  FuseKernelCU(stream_, resize_h, resize_w, crop_h, crop_w, crop_top, crop_left, mean0, mean1, mean2, std0,
               std1, std2, pad_top, pad_left, pad_bottom, pad_right, pad_h, pad_w, pad_value, d_in,
               data_out, data_out_num_, src_h, src_w, format, interpolation_);

  cuErrCheck(cudaStreamSynchronize(stream_));
  cuErrCheck(cudaFree(d_in));
}

REGISTER_FUSE_KERNEL(#TAG#_cuda, "#TAG#_cuda",
                     FuseFunc);
'''
}


def parse_args():
    parser = argparse.ArgumentParser(description='Extract transform.')
    parser.add_argument('root_path', help='root path to codebase')
    parser.add_argument('mmmdeploy_path', help='mmdeploy path')
    args = parser.parse_args()
    return args


def append_info(device, tag):
    info = INFO[device]
    info = info.replace('#TAG#', tag)
    src_file = 'source.c' if device == 'cpu' else 'source.cu'
    nsp = f'namespace {device}_{tag}' + ' {\n'
    with open(src_file, 'r', encoding='utf-8') as f:
        data = f.readlines()
    for i, line in enumerate(data):
        if '_Kernel' in line:
            data.insert(i, nsp)
            data.insert(i, '#include "elena_registry.h"\n')
            break
    for i, line in enumerate(data):
        data[i] = line.replace('extern "C"', '')
    data.append(info)
    data.append('}')
    with open(src_file, 'w', encoding='utf-8') as f:
        for line in data:
            f.write(line)


def generate_source_code(preprocess, transform_static, tag, args):
    kernel_base_dir = osp.join(args.mmmdeploy_path, 'csrc', 'mmdeploy',
                               'preprocess', 'elena')
    cpu_work_dir = osp.join(kernel_base_dir, 'cpu_kernel')
    cuda_work_dir = osp.join(kernel_base_dir, 'cuda_kernel')
    dst_cpu_kernel_file = osp.join(cpu_work_dir, f'{tag}.cpp')
    dst_cuda_kernel_file = osp.join(cuda_work_dir, f'{tag}.cu')
    json_work_dir = osp.join(kernel_base_dir, 'json')

    preprocess_json_path = osp.join(json_work_dir, f'{tag}_preprocess.json')
    static_json_path = osp.join(json_work_dir, f'{tag}_static.json')
    if osp.exists(preprocess_json_path):
        return
    mmcv.dump(preprocess, preprocess_json_path, sort_keys=False, indent=4)
    mmcv.dump(transform_static, static_json_path, sort_keys=False, indent=4)
    gen_cpu_cmd = f'{ELENA_BIN} {static_json_path} cpu'
    res = subprocess.run(gen_cpu_cmd, shell=True)
    if res.returncode == 0:
        append_info('cpu', tag)
        shutil.copyfile('source.c', dst_cpu_kernel_file)
    os.remove('source.c')
    gen_cuda_cmd = f'{ELENA_BIN} {static_json_path} cuda'
    res = subprocess.run(gen_cuda_cmd, shell=True)
    if res.returncode == 0:
        append_info('cuda', tag)
        shutil.copyfile('source.cu', dst_cuda_kernel_file)
    os.remove('source.cu')
    os.remove('elena_int.h')


def extract_one_model(deploy_cfg_, model_cfg_, args):
    deploy_cfg, model_cfg = load_config(deploy_cfg_, model_cfg_)
    preprocess = get_preprocess(deploy_cfg, model_cfg)
    preprocess['model_cfg'] = model_cfg_
    transform_static, tag = get_transform_static(preprocess['transforms'])
    if tag is not None:
        generate_source_code(preprocess, transform_static, tag, args)


def extract_one_metafile(metafile, codebase, args):
    with open(metafile, encoding='utf-8') as f:
        yaml_info = yaml.load(f, Loader=yaml.FullLoader)
    known_task = list(DEPLOY_CFG.keys())
    for model in yaml_info['Models']:
        try:
            cfg = model['Config']
            task_name = model['Results'][0]['Task']
            if task_name not in known_task:
                continue
            deploy_cfg = osp.join(args.mmmdeploy_path, DEPLOY_CFG[task_name])
            model_cfg = osp.join(args.root_path, codebase, cfg)
            extract_one_model(deploy_cfg, model_cfg, args)
        except Exception:
            pass


def main():
    args = parse_args()
    for cb in CODEBASE:
        metafile_pattern = osp.join(args.root_path, cb, 'configs', '**/*.yml')
        metafiles = glob(metafile_pattern, recursive=True)
        for metafile in metafiles:
            extract_one_metafile(metafile, cb, args)


if __name__ == '__main__':
    main()
