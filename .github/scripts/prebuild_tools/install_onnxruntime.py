import os
import argparse
import subprocess
import sys

URLS = {
    'linux': {
        'cpu': {
            '1.8.1': ''
        }
    },
    'windows': {
        'cpu': {
            '1.8.1': ''
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


def main():
    args = parse_args()
    build(args)


if __name__ == '__main__':
    main()
