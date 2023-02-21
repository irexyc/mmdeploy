import os
import argparse
from urllib import request
import sys
from utils import *

URLS = {
    'windows': {
        '10.2': {
            '8.2.3.0': 'https://github.com/irexyc/mmdeploy-ci-resource/releases/download/cudnn/cudnn-10.2-windows10-x64-v8.2.1.32.zip',
        },
        '11.x': {
            '8.2.3.0': 'https://github.com/irexyc/mmdeploy-ci-resource/releases/download/cudnn/cudnn-11.3-windows-x64-v8.2.1.32.zip',
        }
    },
    'linux': {
        '10.2': {
            '8.2.3.0': 'https://github.com/irexyc/mmdeploy-ci-resource/releases/download/cudnn/cudnn-10.2-linux-x64-v8.2.1.32.tgz',
        },
        '11.x': {
            '8.2.3.0': 'https://github.com/irexyc/mmdeploy-ci-resource/releases/download/cudnn/cudnn-11.3-linux-x64-v8.2.1.32.tgz',
        }
    }
}


def parse_args():
    parser = argparse.ArgumentParser(
        description='cuDNN installation tool.')
    parser.add_argument(
        '--version', type=str, default="8.2.3.0", help='cudnn version')
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
    used_files = ['lib', 'lib64', 'include', 'bin']
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
    dst_folder = 'cudnn'
    extract(local_file, dst_folder)
    remove_noused(dst_folder)


def main():
    args = parse_args()
    run(args)


if __name__ == '__main__':
    main()
