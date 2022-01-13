#!/usr/bin/env python3

import os
import pprint as pp
import subprocess
import re
import sqlite3 as sl
import hashlib
import multiprocessing as mp

DATABASE_FILENAME = 'collect.db'


def setup_db():
    con = sl.connect(DATABASE_FILENAME)
    with con:
        # id            - A unique ID (sha256(filename||tool||truth))
        # filename      - Full path to the testcase.
        # tool          - The tool that ran this testcase
        # truth         - whether this is a known good or bad case.
        # class         - the CWE class of this test case.
        # ret_code      - the return code from the tool that ran this testcase.
        # ret_output    - the output from the tool that ran this testcase.
        # ret_status    - How did the tool classify this testcase? good or bad?
        con.execute("""
        CREATE TABLE EXPERIMENT (
            id TEXT PRIMARY KEY,
            filename TEXT,
            tool TEXT,
            truth TEXT,            
            class TEXT,
            ret_code INTEGER,
            ret_output TEXT
        );
        """)
    return


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
        # pp.pprint("QTE returned code : {}".format(cpe.returncode))
        # pp.pprint("QTE : {}".format(cpe.output))
        output = cpe.output
        return (cpe.returncode, output)
    except subprocess.TimeoutExpired as te:
        pp.pprint("QTE timed out on testcase :{}".format(file_to_exec))
        output = te.output
        return (2, output)

    # otherwise successful run.
    return (0, output)


# this regex matches on:
# tool      - the directory which indiciates the tool under test (qte/qasan, asan)
# class     - the class of the bug being tested
# dir       - the directory of the test (they are grouped under good/bad tests)
# testcase  - the testcase that is being passed to QTE.

classifier_regex = "^.*\/(?P<tool>.*)_.*_tests\/.*(?P<class>CWE.*)\/(?P<dir>.*\/).*(?P<testcase>CWE.*.*__.*_.*.out)$"


def run_tests():
    # Run the configured QEMU binary against the test case
    # Passing in the AFL_USE_QTE=1 environment variable.
    qte_binary = "../../qte-qemu"
    libqte_library = "../../libqte.so"
    # TODO: QASAN
    # TODO: ASAN

    if not (os.path.exists(qte_binary)):
        print(
            "QTE binary doesnt exist. Please build QTE. {}".format(qte_binary))
        exit()
    if not (os.path.exists(libqte_library)):
        print("libqte doesnt exist. Have you compiled QTE?")
        exit()

    built_testcases = []
    for root, dirs, files in os.walk(os.getcwd(),
                                     topdown=True,
                                     followlinks=True):
        for name in files:
            if (name.endswith(".out")):
                testcase = ('{}/{}'.format(root, name))
                match = re.search(classifier_regex, testcase)

                tool = match.group("tool")
                bugclass = match.group("class")
                truth = match.group("dir")

                if ('bad' in truth):
                    truth = 'bad'
                elif ('good' in truth):
                    truth = 'good'

                hash_mat = testcase + tool + truth
                hash_id = hashlib.sha256(hash_mat.encode()).hexdigest()

                # print(
                #     "ID : {}, Tool under test : {} Class : {}, truth : {}, testcase : {}"
                #     .format(hash_id, tool, bugclass, truth, testcase))

                built_testcases.append(
                    (hash_id, testcase, tool, truth, bugclass))

    if (len(built_testcases) > 0):
        print("Total testcases : {}".format(len(built_testcases)))
    # Now we have the list of tests we want to run, we should run it against
    # each tool we know of in a scaled multiprocessed way.


if __name__ == '__main__':
    if (os.path.exists(DATABASE_FILENAME)):
        print("Database already exists, continuing..")
    else:
        print("Setting up the database.")
        setup_db()

    run_tests()
