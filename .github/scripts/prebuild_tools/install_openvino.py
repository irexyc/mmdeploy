import os
import argparse
from urllib import request
import sys
from utils import *

URLS = {
    'windows': {
        '2022.2': 'https://storage.openvinotoolkit.org/repositories/openvino/packages/2022.2/windows/w_openvino_toolkit_windows_2022.2.0.7713.af16ea1d79a_x86_64.zip',
    },
    'linux': {
        '2022.2': {
            '18': 'https://storage.openvinotoolkit.org/repositories/openvino/packages/2022.2/linux/l_openvino_toolkit_ubuntu18_2022.2.0.7713.af16ea1d79a_x86_64.tgz',
            '20': 'https://storage.openvinotoolkit.org/repositories/openvino/packages/2022.2/linux/l_openvino_toolkit_ubuntu20_2022.2.0.7713.af16ea1d79a_x86_64.tgz',
        },
        '2022.3': {
            '18': 'https://storage.openvinotoolkit.org/repositories/openvino/packages/2022.3/linux/l_openvino_toolkit_ubuntu18_2022.3.0.9052.9752fafe8eb_x86_64.tgz',
            '20': 'https://storage.openvinotoolkit.org/repositories/openvino/packages/2022.3/linux/l_openvino_toolkit_ubuntu20_2022.3.0.9052.9752fafe8eb_x86_64.tgz',
        }
    }
}


def parse_args():
    parser = argparse.ArgumentParser(
        description='OpenVINO installation tool.')
    parser.add_argument(
        '--version', type=str, default="2022.2", help='onnxruntime version')
    parser.add_argument(
        '--platform', type=str, help='platform')
    parser.add_argument(
        '--system', type=str, help='ubuntu system version')
    parser.add_argument(
        '--work-dir', type=str, default='.', help='working directory.')
    args = parser.parse_args()
    return args


def remove_noused(dst_folder):
    os.chdir(dst_folder)
    onnx_folder = os.listdir('.')[0]
    os.rename(onnx_folder, dst_folder)
    os.chdir(dst_folder)
    used_files = ['runtime']
    files = os.listdir('.')
    for f in files:
        if f not in used_files:
            safe_call(f'rm -rf {f}')


def run(args):
    os.chdir(args.work_dir)
    if args.platform == 'windows':
        url = URLS[args.platform][args.version]
    elif args.platform == 'linux':
        url = URLS[args.platform][args.version][args.system]
    else:
        raise NotImplementedError

    local_file = os.path.basename(url)
    request.urlretrieve(url, local_file)
    dst_folder = 'openvino'
    extract(local_file, dst_folder)
    remove_noused(dst_folder)


def main():
    args = parse_args()
    run(args)


if __name__ == '__main__':
    main()
