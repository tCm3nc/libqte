#!/usr/bin/env python3

from multiprocessing import process
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
            ret_output TEXT,
            ret_status TEXT
        );
        """)
    return


def check_qte():
    cwd = os.getcwd()
    qte_binary = "{}/../../qte-qemu".format(cwd)
    libqte_library = "{}/../../libqte.so".format(cwd)
    # TODO: QASAN
    # TODO: ASAN

    if not (os.path.exists(qte_binary)):
        print(
            "QTE binary doesnt exist. Please build QTE. {}".format(qte_binary))
        exit()
    if not (os.path.exists(libqte_library)):
        print("libqte doesnt exist. Have you compiled QTE?")
        exit()
    qte_binary = os.path.normpath(qte_binary)
    libqte_library = os.path.normpath(libqte_library)
    return (qte_binary, libqte_library)


def run_qte(arg):
    '''
    INPUT:
        testcase_block : 
            [hash_id, testcase, tool, truth, bugclass]
    OUTPUT:
        testcase_block :
            [hash_id, testcase, tool, truth, bugclass, return code, return output, tool classified as X]
    '''
    # Run the configured QEMU binary against the test case
    # Passing in the AFL_USE_QTE=1 environment variable.
    qte_binary, libqte_library = check_qte()
    testcase_block = arg
    hash_id = testcase_block[0]
    testcase = testcase_block[1]
    tool = testcase_block[2]
    truth = testcase_block[3]
    bugclass = testcase_block[4]

    # Enable QTE
    env = os.environ.copy()
    env["AFL_USE_QTE"] = "1"

    file_to_exec = "{} {}".format(qte_binary, testcase)
    # print("Executing : {}".format(file_to_exec))
    output = ''
    try:
        output = subprocess.check_output(file_to_exec,
                                         env=env,
                                         stderr=subprocess.STDOUT,
                                         shell=True,
                                         timeout=5)
    except subprocess.CalledProcessError as cpe:
        # pp.pprint("QTE returned code : {}".format(cpe.returncode))
        # pp.pprint("QTE : {}".format(cpe.output))
        # return (cpe.returncode, cpe.output)
        testcase_block.append(cpe.returncode)
        testcase_block.append(cpe.output)
        testcase_block.append("bad")
        return testcase_block

    except subprocess.TimeoutExpired as te:
        # pp.pprint("QTE timed out on testcase :{}".format(file_to_exec))
        testcase_block.append(1)
        testcase_block.append(output)
        testcase_block.append("timedout")
        return testcase_block

    # otherwise successful run.
    testcase_block.append(0)
    testcase_block.append(output)
    testcase_block.append("good")
    # return (0, output)
    return testcase_block


def add_to_database(result):
    print("Adding result to DB..")
    print("{}".format(result))
    return ""


# this regex matches on:
# tool      - the directory which indiciates the tool under test (qte/qasan, asan)
# class     - the class of the bug being tested
# dir       - the directory of the test (they are grouped under good/bad tests)
# testcase  - the testcase that is being passed to QTE.

classifier_regex = "^.*\/(?P<tool>.*)_.*_tests\/.*(?P<class>CWE.*)\/(?P<dir>.*\/).*(?P<testcase>CWE.*.*__.*_.*.out)$"


def run_tests():
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
                    [hash_id, testcase, tool, truth, bugclass])

    # Now process the testcases
    if (len(built_testcases) > 0):
        print("Total testcases : {}".format(len(built_testcases)))
        # Now we have the list of tests we want to run, we should run it against
        # each tool we know of in a scaled multiprocessed way.
        p = mp.Pool(processes=mp.cpu_count() - 1)
        results = []
        for r in p.imap_unordered(run_qte, built_testcases, chunksize=100):
            add_to_database(r)
            exit()

        # p.close()
        # p.join()


if __name__ == '__main__':
    if (os.path.exists(DATABASE_FILENAME)):
        print("Database already exists, continuing..")
    else:
        print("Setting up the database.")
        setup_db()

    run_tests()
