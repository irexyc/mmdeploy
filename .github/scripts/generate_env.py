# Copyright (c) OpenMMLab. All rights reserved.
import os
import argparse
from glob import glob

'''
tensorrt
cudnn
opencv
ppl.cv
openvino
onnxruntime
'''


def parse_args():
    parser = argparse.ArgumentParser(
        description='Generate path')
    parser.add_argument(
        '--third-party-dir', type=str, help='third parth directory')
    parser.add_argument(
        '--platform', type=str, help='platform')
    parser.add_argument(
        '--out', type=str, help='out file')
    args = parser.parse_args()
    return args


class EnvGetFunc:
    """Trace Transform."""

    def __init__(self):
        self.module_dict = dict()

    def register_module(self, name):
        if name in self.module_dict:
            raise KeyError(f'{name} is already registered')

        def _register(func):
            self.module_dict[name] = func
            return func

        return _register

    def get(self, name):
        return self.module_dict[name]


_ENVGET_WRAPPER = EnvGetFunc()


@_ENVGET_WRAPPER.register_module(name='tensorrt')
def get_env_tensorrt(work_dir, env_data, platform):
    env_data['TENSORRT_DIR'] = 'tensorrt'
    env_data['NAME'].append('TENSORRT_DIR')


@_ENVGET_WRAPPER.register_module(name='cudnn')
def get_env_cudnn(work_dir, env_data, platform):
    env_data['CUDNN_DIR'] = 'cudnn'
    env_data['NAME'].append('CUDNN_DIR')


@_ENVGET_WRAPPER.register_module(name='ppl.cv')
def get_env_pplcv(work_dir, env_data, platform):
    env_data['pplcv_DIR'] = os.path.join(
        'ppl.cv', 'cuda-build', 'install', 'lib', 'cmake', 'ppl')
    env_data['NAME'].append('pplcv_DIR')


@_ENVGET_WRAPPER.register_module(name='openvino')
def get_env_openvino(work_dir, env_data, platform):
    env_data['InferenceEngine_DIR'] = os.path.join(
        'openvino', 'runtime', 'cmake')
    env_data['NAME'].append('InferenceEngine_DIR')
    os.chdir('openvino')
    files = glob('**/*.so', recursive=True)
    dll_dir = [os.path.dirname(file) for file in files]
    dll_dir = set(dll_dir)
    for path in dll_dir:
        env_data['LD_LIBRARY_PATH'].append(path)


@_ENVGET_WRAPPER.register_module(name='onnxruntime')
def get_env_onnxruntime(work_dir, env_data, platform):
    env_data['ONNXRUNTIME_DIR'] = os.path.join(
        'onnxruntime')
    env_data['NAME'].append('ONNXRUNTIME_DIR')


@_ENVGET_WRAPPER.register_module(name='opencv')
def get_env_opencv(work_dir, env_data, platform):
    if platform == 'linux':
        env_data['OpenCV_DIR'] = os.path.join(
            'opencv', 'build', 'install', 'lib', 'cmake', 'opencv4')
    elif platform == 'windows':
        env_data['OpenCV_DIR'] = os.path.join(
            'opencv', 'build', 'install', 'x64', 'vc16', 'staticlib')
    env_data['NAME'].append('OpenCV_DIR')


def generate_env(args):
    platform = args.platform
    work_dir = os.path.abspath(args.third_party_dir)
    out = os.path.abspath(args.out)
    os.chdir(work_dir)

    files = os.listdir('.')
    env_data = {
        'NAME': [],
        'PATH': [],
        'LD_LIBRARY_PATH': [],
    }
    for file in files:
        os.chdir(work_dir)
        func = _ENVGET_WRAPPER.get(file)
        func(work_dir, env_data, platform)

    # write output
    print(env_data)
    if platform == 'linux':
        with open(out, 'w') as f:
            # named env variables
            base = '${MMDEPLOY_THIRD_PARTY_ROOT}'
            for key in env_data['NAME']:
                value = os.path.join(
                    base, str(env_data[key]))
                info = f'export {key}={value}\n'
                f.write(info)
            # runtime
            ld_path = map(lambda p: os.path.join(base, p),
                          env_data['LD_LIBRARY_PATH'])
            ld_path = ':'.join(ld_path)
            info = f'export LD_LIBRARY_PATH={ld_path}:$LD_LIBRARY_PATH'
            f.write(info)
    if platform == 'windows':
        with open(out, 'w') as f:
            base = '${env:MMDEPLOY_THIRD_PARTY_ROOT}'
            for key in env_data['NAME']:
                value = os.path.join(
                    base, str(env_data[key]))
                info = f'$env:{key}="{value}"\n'
                f.write(info)


def main():
    args = parse_args()
    print(args)
    generate_env(args)


if __name__ == '__main__':
    main()
