import os
import argparse
import subprocess
import sys


def safe_call(cmd):
    print(cmd)
    code = os.WEXITSTATUS(os.system(cmd))
    if code != 0:
        sys.exit(code)


def parse_args():
    parser = argparse.ArgumentParser(
        description='Opencv installation tool.')
    parser.add_argument(
        '--version', type=str, default="4.5.5", help='opencv version')
    parser.add_argument(
        '--platform', type=str, help='platform')
    parser.add_argument(
        '--work-dir', type=str, default='.', help='working directory.')
    args = parser.parse_args()
    return args


def build(args):
    os.chdir(args.work_dir)

    cmd = f'git clone -b {args.version} --depth=1 https://github.com/opencv/opencv.git'
    safe_call(cmd)
    os.mkdir('opencv/build')
    os.chdir('opencv/build')

    if args.platform == 'win32':
        pass
    elif args.platform == 'linux':
        cmd = '''
            cmake .. \
               -DOPENCV_FORCE_3RDPARTY_BUILD=ON \
               -DBUILD_TESTS=OFF-DBUILD_PERF_TESTS=OFF \
               -DBUILD_SHARED_LIBS=OFF \
               -DBUILD_JAVA=OFF \
               -DBUILD_opencv_python3=OFF \
               -DBUILD_opencv_python2=OFF \
               -DWITH_FFMPEG=OFF \
               -DCMAKE_INSTALL_PREFIX=install
        '''

    safe_call(cmd)
    safe_call('cmake --build . --config Release -j')
    safe_call('cmake --install . --config Release')


def main():
    args = parse_args()
    build(args)


if __name__ == '__main__':
    main()
