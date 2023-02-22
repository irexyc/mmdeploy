# Copyright (c) OpenMMLab. All rights reserved.
import os
import argparse
import sys
from urllib import request
import json
import shutil


def parse_args():
    parser = argparse.ArgumentParser(
        description='Installation tool for dependencies')
    parser.add_argument(
        '--device', type=str, help='target device')
    parser.add_argument(
        '--backends', type=str, help='target backends')
    parser.add_argument(
        '--platform', type=str, help='platform')
    parser.add_argument(
        '--cuda', type=str, help='cuda version')
    parser.add_argument(
        '--ubuntu', type=str, default='18.04', help='ubuntu version')
    parser.add_argument(
        '--verinfo', type=str, help='third party version infomation')
    parser.add_argument(
        '--work-dir', type=str, default='.', help='working directory.')
    args = parser.parse_args()
    return args


def remove_file_or_folder(path):
    try:
        if os.path.isdir(path):
            shutil.rmtree(path)
        elif os.path.isfile(path):
            os.remove(path)
        else:
            raise ValueError(f'invalid path: {path}')
    except Exception as e:
        print(f'remove file error: {os.path.abspath(path)}', e)


def safe_call(cmd):
    print(cmd)
    if sys.platform == 'linux':
        code = os.WEXITSTATUS(os.system(cmd))
    elif sys.platform == 'win32':
        code = os.system(cmd)
    else:
        raise NotImplementedError

    if code != 0:
        sys.exit(code)


def remove_noused(dst_folder, used_files):
    os.chdir(dst_folder)
    files = os.listdir('.')
    for f in files:
        if f not in used_files:
            remove_file_or_folder(f)


def extract(src, dst, remove_top_folder=True):
    os.mkdir(dst)
    if sys.platform == 'linux':
        safe_call(f'tar xf {src} -C {dst}')
    elif sys.platform == 'win32':
        safe_call(f'unzip {src} -d {dst}')
    if remove_top_folder:
        os.chdir(dst)
        folder = os.listdir('.')[0]
        files = os.listdir(folder)
        tmp = '_tmp_save'
        os.mkdir(tmp)
        for file in files:
            shutil.move(os.path.join(folder, file), os.path.join(tmp, file))
        os.rmdir(folder)
        for file in files:
            shutil.move(os.path.join(tmp, file), file)
        os.rmdir(tmp)
        os.chdir('..')


def install_opencv(work_dir, platform, version):
    if not os.path.exists(work_dir):
        os.makedirs(work_dir)

    os.chdir(work_dir)
    cmd = f'git clone -b {version} --depth=1 https://github.com/opencv/opencv.git'
    safe_call(cmd)
    os.mkdir('opencv/build')
    os.chdir('opencv/build')
    if sys.platform == 'win32':
        cmd = [
            'cmake .. -A x64 -T v142',
            '-DBUILD_TESTS=OFF',
            '-DBUILD_PERF_TESTS=OFF',
            '-DBUILD_SHARED_LIBS=OFF',
            '-DBUILD_WITH_STATIC_CRT=OFF',
            '-DBUILD_JAVA=OFF',
            '-DBUILD_opencv_python3=OFF',
            '-DBUILD_opencv_python2=OFF',
            '-DWITH_FFMPEG=OFF',
            '-DCMAKE_INSTALL_PREFIX=install'
        ]
        cmd = ' '.join(cmd)
    elif sys.platform == 'linux':
        cmd = [
            'cmake ..',
            '-DOPENCV_FORCE_3RDPARTY_BUILD=ON',
            '-DBUILD_TESTS=OFF',
            '-DBUILD_PERF_TESTS=OFF',
            '-DBUILD_SHARED_LIBS=OFF',
            '-DBUILD_JAVA=OFF',
            '-DBUILD_opencv_python3=OFF',
            '-DBUILD_opencv_python2=OFF',
            '-DWITH_FFMPEG=OFF',
            '-DCMAKE_INSTALL_PREFIX=install'
        ]
        cmd = ' '.join(cmd)
    safe_call(cmd)
    safe_call('cmake --build . --config Release -j2')
    safe_call('cmake --install . --config Release')


def install_pplcv(work_dir, platform, version):
    if not os.path.exists(work_dir):
        os.makedirs(work_dir)

    os.chdir(work_dir)
    cmd = f'git clone -b v{version} --depth=1 https://github.com/openppl-public/ppl.cv.git'
    safe_call(cmd)
    os.chdir('ppl.cv')
    if sys.platform == 'win32':
        os.mkdir('cuda-build')
        os.chdir('cuda-build')
        cmd = [
            'cmake .. -A x64 -T v142,cuda="%CUDA_PATH%"',
            '-DCMAKE_INSTALL_PREFIX=install',
            '-DPPLCV_USE_CUDA=ON',
            '-DPPLCV_USE_MSVC_STATIC_RUNTIME=OFF'
        ]
        cmd = ' '.join(cmd)
        safe_call(cmd)
        safe_call('cmake --build . --config Release -j2')
        safe_call('cmake --install . --config Release')
    elif sys.platform == 'linux':
        cmd = 'bash ./build.sh cuda'
        safe_call(cmd)


def install_openvino(work_dir, platform, version, ubuntu='18.04'):
    URLS = {
        'windows': {
            '2022.2': 'https://storage.openvinotoolkit.org/repositories/openvino/packages/2022.2/windows/w_openvino_toolkit_windows_2022.2.0.7713.af16ea1d79a_x86_64.zip',
        },
        'linux': {
            '2022.2': {
                '18.04': 'https://storage.openvinotoolkit.org/repositories/openvino/packages/2022.2/linux/l_openvino_toolkit_ubuntu18_2022.2.0.7713.af16ea1d79a_x86_64.tgz',
                '20.04': 'https://storage.openvinotoolkit.org/repositories/openvino/packages/2022.2/linux/l_openvino_toolkit_ubuntu20_2022.2.0.7713.af16ea1d79a_x86_64.tgz',
            },
            '2022.3': {
                '18.04': 'https://storage.openvinotoolkit.org/repositories/openvino/packages/2022.3/linux/l_openvino_toolkit_ubuntu18_2022.3.0.9052.9752fafe8eb_x86_64.tgz',
                '20.04': 'https://storage.openvinotoolkit.org/repositories/openvino/packages/2022.3/linux/l_openvino_toolkit_ubuntu20_2022.3.0.9052.9752fafe8eb_x86_64.tgz',
            }
        }
    }

    if not os.path.exists(work_dir):
        os.makedirs(work_dir)

    os.chdir(work_dir)

    if platform == 'windows':
        url = URLS[platform][version]
    elif platform == 'linux':
        url = URLS[platform][version][ubuntu]
    else:
        raise NotImplementedError

    local_file = os.path.basename(url)
    request.urlretrieve(url, local_file)
    dst_folder = 'openvino'
    extract(local_file, dst_folder)
    os.remove(local_file)
    remove_noused(dst_folder, ['runtime'])


def install_onnxruntime(work_dir, platform, version, device):
    URLS = {
        'windows': {
            'cpu': {
                '1.8.1': 'https://github.com/microsoft/onnxruntime/releases/download/v1.8.1/onnxruntime-win-x64-1.8.1.zip',
                '1.14.0': 'https://github.com/microsoft/onnxruntime/releases/download/v1.14.0/onnxruntime-win-x64-1.14.0.zip',
            },
            'cuda': {
                '1.8.1': 'https://github.com/microsoft/onnxruntime/releases/download/v1.8.1/onnxruntime-win-gpu-x64-1.8.1.zip',
                '1.14.0': 'https://github.com/microsoft/onnxruntime/releases/download/v1.14.0/onnxruntime-win-x64-gpu-1.14.0.zip',
            }
        },
        'linux': {
            'cpu': {
                '1.8.1': 'https://github.com/microsoft/onnxruntime/releases/download/v1.8.1/onnxruntime-linux-x64-1.8.1.tgz',
                '1.14.0': 'https://github.com/microsoft/onnxruntime/releases/download/v1.14.0/onnxruntime-linux-x64-1.14.0.tgz',
            },
            'cuda': {
                '1.8.1': 'https://github.com/microsoft/onnxruntime/releases/download/v1.8.1/onnxruntime-linux-x64-gpu-1.8.1.tgz',
                '1.14.0': 'https://github.com/microsoft/onnxruntime/releases/download/v1.14.0/onnxruntime-linux-x64-gpu-1.14.0.tgz',
            }
        }
    }
    if not os.path.exists(work_dir):
        os.makedirs(work_dir)

    os.chdir(work_dir)
    url = URLS[platform][device][version]
    local_file = os.path.basename(url)
    request.urlretrieve(url, local_file)
    dst_folder = 'onnxruntime'
    extract(local_file, dst_folder)
    os.remove(local_file)
    remove_noused(dst_folder, ['lib', 'include', 'VERSION_NUMBER'])


def install_tensorrt(work_dir, platform, version, cuda):
    URLS = {
        'windows': {
            '10.2': {
                '8.2.3.0': 'https://github.com/irexyc/mmdeploy-ci-resource/releases/download/tensorrt/TensorRT-8.2.3.0.Windows10.x86_64.cuda-10.2.cudnn8.2.zip',
            },
            '11.x': {
                '8.2.3.0': 'https://github.com/irexyc/mmdeploy-ci-resource/releases/download/tensorrt/TensorRT-8.2.3.0.Windows10.x86_64.cuda-11.4.cudnn8.2.zip',
            }
        },
        'linux': {
            '10.2': {
                '8.2.3.0': 'https://github.com/irexyc/mmdeploy-ci-resource/releases/download/tensorrt/TensorRT-8.2.3.0.Linux.x86_64-gnu.cuda-10.2.cudnn8.2.tar.gz',
            },
            '11.x': {
                '8.2.3.0': 'https://github.com/irexyc/mmdeploy-ci-resource/releases/download/tensorrt/tensorrt-8.2.3.0.linux.x86_64-gnu.cuda-11.4.cudnn8.2.tar.gz',
            }
        }
    }
    if not os.path.exists(work_dir):
        os.makedirs(work_dir)

    os.chdir(work_dir)
    cuda_ver = '10.2' if cuda[:2] == '10' else '11.x'
    url = URLS[platform][cuda_ver][version]
    local_file = os.path.basename(url)
    request.urlretrieve(url, local_file)
    dst_folder = 'tensorrt'
    extract(local_file, dst_folder)
    os.remove(local_file)
    remove_noused(dst_folder, ['lib', 'include', 'targets'])


def install_cudnn(work_dir, platform, version, cuda):
    URLS = {
        'windows': {
            '10.2': {
                '8.2.1': 'https://github.com/irexyc/mmdeploy-ci-resource/releases/download/cudnn/cudnn-10.2-windows10-x64-v8.2.1.32.zip',
            },
            '11.x': {
                '8.2.1': 'https://github.com/irexyc/mmdeploy-ci-resource/releases/download/cudnn/cudnn-11.3-windows-x64-v8.2.1.32.zip',
            }
        },
        'linux': {
            '10.2': {
                '8.2.1': 'https://github.com/irexyc/mmdeploy-ci-resource/releases/download/cudnn/cudnn-10.2-linux-x64-v8.2.1.32.tgz',
            },
            '11.x': {
                '8.2.1': 'https://github.com/irexyc/mmdeploy-ci-resource/releases/download/cudnn/cudnn-11.3-linux-x64-v8.2.1.32.tgz',
            }
        }
    }

    if not os.path.exists(work_dir):
        os.makedirs(work_dir)

    os.chdir(work_dir)
    cuda_ver = '10.2' if cuda[:2] == '10' else '11.x'
    url = URLS[platform][cuda_ver][version]
    local_file = os.path.basename(url)
    request.urlretrieve(url, local_file)
    dst_folder = 'cudnn'
    extract(local_file, dst_folder)
    os.remove(local_file)
    remove_noused(dst_folder, ['lib', 'lib64', 'include', 'bin'])


def install_cpu_backends(work_dir, platform, backends, ver, ubuntu):
    if backends.find('ort') != -1:
        install_onnxruntime(work_dir, platform, ver['onnxruntime'], 'cpu')
    if backends.find('openvino') != -1:
        install_openvino(work_dir, platform, ver['openvino'], ubuntu)


def install_cuda_backends(work_dir, platform, backends, ver, cuda):
    if backends.find('trt') != -1:
        install_tensorrt(work_dir, platform, ver['tensorrt'], cuda)
        install_cudnn(work_dir, platform, ver['cudnn'], cuda)
    if backends.find('ort') != -1:
        install_onnxruntime(work_dir, platform, ver['onnxruntime'], 'cuda')


def install(args):
    ver = json.loads(args.verinfo)
    work_dir = os.path.abspath(args.work_dir)
    platform = args.platform
    cuda = args.cuda
    backends = args.backends
    ubuntu = args.ubuntu

    install_opencv(work_dir, platform, ver['opencv'])
    if args.device == 'cpu':
        install_cpu_backends(work_dir, platform, backends, ver, ubuntu)
    elif args.device == 'cuda':
        install_pplcv(work_dir, platform, ver['pplcv'])
        install_cuda_backends(work_dir, platform, backends, ver, cuda)


def main():
    args = parse_args()
    print(args)
    install(args)


if __name__ == '__main__':
    main()
