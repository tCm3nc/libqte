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
                        help="Architecture for target (default x86)",
                        action='store',
                        choices=['x86_64', 'x86'],
                        default="x86")
    parser.add_argument("--debug",
                        help="Compile in debug mode (default false)",
                        action='store_true')
    parser.add_argument(
        "--bear",
        help=
        "Use the bear interceptor to generate a compile_commands.json file.",
        action='store_true',
        default='true')  # FIXME: this should be false in release

    args = parser.parse_args()
    # get directory of _this_ file.
    current_dir = os.path.dirname(os.path.realpath(__file__))

    # map the arch to something QEMU knows about
    arch = ARCHS[args.arch]
    extra_cflags = ""
    extra_ldflags = ""
    python_dir = sys.executable

    extra_cflags += "-I{} ".format(os.path.join(current_dir, "qqte"))

    if (args.debug):
        extra_cflags += "-DDEBUG"

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
        cmd = 'cd {}; {} make -j `nproc`'.format(
            os.path.join(current_dir, "qemu"), "bear" if args.bear else "")
        print("{}".format(cmd))
        assert (os.system(cmd) == 0)

        # copy the built artefact to our directory
        shutil.copy2(
            os.path.join(current_dir, "qemu", "build",
                         "{}-linux-user".format(arch), "qemu-{}".format(arch)),
            os.path.join(current_dir, "qte-qemu"))

        # Compile the interception library
        target = ""
        if (args.debug):
            target = "debug"
        if (arch == 'i386'):
            target = 'i386'

        extra_cflags = "CFLAGS="
        if (args.debug):
            extra_cflags += "{}".format("-DDEBUG")

        cmd = 'cd {}; make {} {}'.format(os.path.join(current_dir, "libqte"),
                                         target, extra_cflags)
        print(cmd)
        assert (os.system(cmd) == 0)

        # Copy the intercept library to the top level directory
        shutil.copy2(os.path.join(current_dir, "libqte", "libqte.so"),
                     current_dir)

        print("Build succeeded, run with ./qte /bin/ls")
        print("")


if __name__ == '__main__':
    do_main()
