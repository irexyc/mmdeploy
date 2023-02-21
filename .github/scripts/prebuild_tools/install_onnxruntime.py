import os
import argparse
import subprocess
import sys

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


def safe_call(cmd):
    print(cmd)
    code = os.WEXITSTATUS(os.system(cmd))
    if code != 0:
        sys.exit(code)


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


def build(args):
    os.chdir(args.work_dir)

    url = URLS['abc']

def main():
    args = parse_args()
    build(args)


if __name__ == '__main__':
    main()
