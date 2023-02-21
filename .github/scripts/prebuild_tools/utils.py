# Copyright (c) MegFlow. All rights reserved.
import os
import sys


def safe_call(cmd):
    print(cmd)
    code = os.WEXITSTATUS(os.system(cmd))
    if code != 0:
        sys.exit(code)


def extract(src, dst):
    os.mkdir(dst)
    if sys.platform == 'linux':
        safe_call(f'tar xf {src} -C {dst}')
    elif sys.platform == 'win32':
        pass
