#!/usr/bin/env python3

import os
import shutil
import sys
import argparse

ARCHS = {"x86_64": "x86_64", "amd64": "x86_64", "x86": "i386", "i386": "i386"}


def do_main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--user",
                        help="Compile usermode instrumentation only",
                        action='store_true',
                        default="true")
    parser.add_argument("--cc",
                        help="C compiler (default clang)",
                        action='store',
                        default="clang")
    parser.add_argument("--cxx",
                        help="C++ compiler (default clang++)",
                        action='store',
                        default="clang++")
    parser.add_argument("--arch",
                        help="Architecture for target (default x86_64)",
                        action='store',
                        default="x86_64")
    parser.add_argument("--debug",
                        help="Compile in debug mode (default false)",
                        action='store',
                        default='true')

    args = parser.parse_args()
    # get directory of _this_ file.
    current_dir = os.path.dirname(os.path.realpath(__file__))

    # map the arch to something QEMU knows about
    arch = ARCHS[args.arch]
    extra_cflags = ""
    extra_ldflags = ""
    python_dir = sys.executable
    if (args.user):
        # Configure QEMU in user mode
        cmd = 'cd {}; ./configure --target-list="{}-linux-user" --disable-system \
        --enable-pie --cc="{}" --cxx="{}" --extra-cflags="-O3 -ggdb {}" \
        --extra-ldflags="{}" --enable-linux-user --disable-gtk --disable-sdl\
        --disable-vnc --disable-strip --python="{}"'.format(
            os.path.join(current_dir, "qemu"), arch, args.cc, args.cxx,
            extra_cflags, extra_ldflags, python_dir)
        print("{}".format(cmd))
        assert (os.system(cmd) == 0)

        # compile the project
        cmd = 'cd {}; make -j `nproc`'.format(os.path.join(current_dir, "qemu"))
        print("{}".format(cmd))
        assert (os.system(cmd) == 0)

        # copy the built artefact to our directory
        shutil.copy2(
            os.path.join(current_dir, "qemu", "build",
                         "{}-linux-user".format(arch), "qemu-{}".format(arch)),
            os.path.join(current_dir, "qte-qemu"))

        # Compile the interception library
        extra_arg = ""
        if (args.debug):
            extra_arg = "debug"

        cmd = 'cd {}; make {}'.format(os.path.join(current_dir, "libqte"),
                                      extra_arg)
        assert (os.system(cmd) == 0)

        # Copy the intercept library to the top level directory
        shutil.copy2(os.path.join(current_dir, "libqte", "libqte.so"),
                     current_dir)

        print("Build succeeded, run with ./qte /bin/ls")
        print("")


if __name__ == '__main__':
    do_main()
