import os
import argparse
from urllib import request
from utils import *

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


def parse_args():
    parser = argparse.ArgumentParser(
        description='ONNX Runtime installation tool.')
    parser.add_argument(
        '--version', type=str, default="1.8.1", help='onnxruntime version')
    parser.add_argument(
        '--device', type=str, default='cpu', help="onnxruntime target device")
    parser.add_argument(
        '--platform', type=str, help='platform')
    parser.add_argument(
        '--work-dir', type=str, default='.', help='working directory.')
    args = parser.parse_args()
    return args


def remove_noused(dst_folder):
    os.chdir(dst_folder)
    folder = os.listdir('.')[0]
    os.rename(folder, dst_folder)
    os.chdir(dst_folder)
    used_files = ['lib', 'include']
    files = os.listdir('.')
    for f in files:
        if f not in used_files:
            safe_call(f'rm -rf {f}')


def run(args):
    os.chdir(args.work_dir)
    url = URLS[args.platform][args.device][args.version]
    local_file = os.path.basename(url)
    request.urlretrieve(url, local_file)
    dst_folder = 'onnxruntime'
    extract(local_file, dst_folder)
    os.remove(local_file)
    remove_noused(dst_folder)


def main():
    args = parse_args()
    run(args)


if __name__ == '__main__':
    main()
