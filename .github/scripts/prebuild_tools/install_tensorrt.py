import os
import argparse
from urllib import request
import sys
from utils import *

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


def parse_args():
    parser = argparse.ArgumentParser(
        description='TensorRT installation tool.')
    parser.add_argument(
        '--version', type=str, default="8.2.3.0", help='tensorrt version')
    parser.add_argument(
        '--platform', type=str, help='platform')
    parser.add_argument(
        '--cuda', type=str, help='cuda version')
    parser.add_argument(
        '--work-dir', type=str, default='.', help='working directory.')
    args = parser.parse_args()
    return args


def remove_noused(dst_folder):
    os.chdir(dst_folder)
    folder = os.listdir('.')[0]
    os.rename(folder, dst_folder)
    os.chdir(dst_folder)
    used_files = ['lib', 'include', 'targets']
    files = os.listdir('.')
    for f in files:
        if f not in used_files:
            safe_call(f'rm -rf {f}')


def run(args):
    os.chdir(args.work_dir)
    cuda_ver = '10.2' if args.cuda[:2] == '10' else '11.x'
    url = URLS[args.platform][cuda_ver][args.version]
    local_file = os.path.basename(url)
    request.urlretrieve(url, local_file)
    dst_folder = 'tensorrt'
    extract(local_file, dst_folder)
    remove_noused(dst_folder)


def main():
    args = parse_args()
    run(args)


if __name__ == '__main__':
    main()
