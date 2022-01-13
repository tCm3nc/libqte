#!/usr/bin/env python3

import os
import pprint as pp
import subprocess
import re


def run_qte(qte_binary, testcase):
    # Enable QTE
    env = os.environ.copy()
    env["AFL_USE_QTE"] = "1"

    file_to_exec = "{} {}".format(qte_binary, testcase)
    output = ''
    try:
        output = subprocess.check_output(file_to_exec,
                                         env=env,
                                         shell=True,
                                         stderr=subprocess.STDOUT,
                                         timeout=5)
    except subprocess.CalledProcessError as cpe:
        pp.pprint("QTE returned code : {}".format(cpe.returncode))
        pp.pprint("QTE : {}".format(cpe.output))
        output = cpe.output
        return (cpe.returncode, output)
    except subprocess.TimeoutExpired as te:
        pp.pprint("QTE timed out on testcase :{}".format(file_to_exec))
        output = te.output
        return (2, output)

    # otherwise successful run.
    return (0, output)


def run_tests():
    # Run the configured QEMU binary against the test case
    # Passing in the AFL_USE_QTE=1 environment variable.
    qte_binary = "../../qte-qemu"
    libqte_library = "../../libqte.so"

    if not (os.path.exists(qte_binary)):
        print(
            "QTE binary doesnt exist. Please build QTE. {}".format(qte_binary))
        exit()
    if not (os.path.exists(libqte_library)):
        print("libqte doesnt exist. Have you compiled QTE?")
        exit()

    for root, dirs, files in os.walk(os.getcwd(), topdown=True):
        if not ('good_tests' in root):
            for name in files:
                if (name.endswith(".out")):
                    # match = re.search("^(?P<root>CWE(\d+).*__.*_(\d+).out)$",
                    #                   name)
                    print("name is : {}".format(name))
                    # print("root is : {}".format(match.group("root")))
                    testcase = ('{}/{}'.format(root, name))
                    print("Running QTE on : {}".format(testcase))
                    # QTE is present, we can run test cases.

                    ret, output = run_qte(qte_binary, testcase)
                    # if (ret != 0):
                    #     # Failing or timed out test case!
                    #     with open("report.txt", "a") as f:
                    #         f.write("Failure testcase : {}\n".format(testcase))
                    # Exit early as a test condition.
                    exit()


if __name__ == '__main__':
    run_tests()
