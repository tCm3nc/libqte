#!/usr/bin/env python3

import chunk
from multiprocessing import process
import os
import pprint as pp
import subprocess
import re
import sqlite3 as sl
import hashlib
import multiprocessing as mp
from timeit import default_timer as timer
import argparse

DATABASE_FILENAME = 'collect.db'

qte_path = ''
qte_lib_path = ''
qasan_path = ''
qasan_lib_path = ''
testsuite_choice = ''


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


def add_to_database(result):
    print("Adding result to DB..")
    # print("{}".format(result))
    sql_query = 'INSERT INTO EXPERIMENT ' + \
    '(id, filename, tool, truth, class, ret_code, ret_output, ret_status)' + \
    ' values(?, ?, ?, ?, ?, ?, ?, ?)'

    # Update the DB
    con = sl.connect(DATABASE_FILENAME)
    with con:
        try:
            con.executemany(sql_query, (result,))
        except sl.IntegrityError as ie:
            print("Possible duplicate entry in DB.. skipping..")
            print("{}".format(ie))
            pass
    return ""


def check_qte():
    if not (os.path.exists(qte_path)):
        print("QTE binary doesnt exist. Please build QTE. {}".format(qte_path))
        exit()
    if not (os.path.exists(qte_lib_path)):
        print("libqte doesnt exist in {}. Have you compiled QTE?".format(
            qte_lib_path))
        exit()
    qte_binary = os.path.normpath(qte_path)
    libqte_library = os.path.normpath(qte_lib_path)
    return (qte_binary, libqte_library)


def check_qasan():
    if not (os.path.exists(qasan_path)):
        print("QASAN binary doesn't exist. Please build QASAN. {}".format(
            qasan_path))
        exit()
    if not (os.path.exists(qasan_lib_path)):
        print("libqasan doesn't exist in : {}, please build QASAN.".format(
            qasan_lib_path))
        exit()
    qasan_binary = os.path.normpath(qasan_path)
    qasan_library = os.path.normpath(qasan_lib_path)
    return (qasan_binary, qasan_library)


def run_qte(arg):
    '''
    Have to provide AFL_USE_QTE=1 in environment
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


def run_qasan(arg):
    '''
    Have to provide AFL_USE_QASAN=1 in environment
    INPUT:
        testcase_block : 
            [hash_id, testcase, tool, truth, bugclass]
    OUTPUT:
        testcase_block :
            [hash_id, testcase, tool, truth, bugclass, return code, return output, tool classified as X]
    '''
    # Run the configured QEMU binary against the test case
    # Passing in the AFL_USE_QTE=1 environment variable.
    qasan_binary, qasan_lib_path = check_qasan()
    testcase_block = arg
    hash_id = testcase_block[0]
    testcase = testcase_block[1]
    tool = testcase_block[2]
    truth = testcase_block[3]
    bugclass = testcase_block[4]

    # Enable QASAN
    env = os.environ.copy()
    env["AFL_USE_QASAN"] = "1"

    file_to_exec = "{} {}".format(qasan_binary, testcase)
    #print("Executing : {}".format(file_to_exec))
    output = ''
    try:
        output = subprocess.check_output(file_to_exec,
                                         env=env,
                                         stderr=subprocess.STDOUT,
                                         shell=True,
                                         timeout=5)
    except subprocess.CalledProcessError as cpe:
        testcase_block.append(cpe.returncode)
        testcase_block.append(cpe.output)
        testcase_block.append("bad")
        return testcase_block

    except subprocess.TimeoutExpired as te:
        testcase_block.append(1)
        testcase_block.append(output)
        testcase_block.append("timedout")
        return testcase_block

    # otherwise successful run.
    testcase_block.append(0)
    testcase_block.append(output)
    testcase_block.append("good")
    return testcase_block


def run_asan(arg):
    '''
    Have to provide ASAN_OPTIONS=detect_leaks=0 in environment
    INPUT:
        testcase_block : 
            [hash_id, testcase, tool, truth, bugclass]
    OUTPUT:
        testcase_block :
            [hash_id, testcase, tool, truth, bugclass, return code, return output, tool classified as X]
    '''
    # Run the ASAN compiled test case
    # Passing in the ASAN_OPTIONS=detect_leaks=0 environment variable.
    testcase_block = arg
    hash_id = testcase_block[0]
    testcase = testcase_block[1]
    tool = testcase_block[2]
    truth = testcase_block[3]
    bugclass = testcase_block[4]

    # Enable ASAN
    env = os.environ.copy()
    env["ASAN_OPTIONS"] = "detect_leaks=0"

    file_to_exec = "{}".format(testcase)
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
        pp.pprint("QTE timed out on testcase :{}".format(file_to_exec))
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

                # Check if the test suite chosen requires this testcase
                if (testsuite_choice != 'all'):
                    if (testsuite_choice == 'qte' and tool != 'qte'):
                        continue
                    if (testsuite_choice == 'qasan' and tool != 'qasan'):
                        continue
                    if (testsuite_choice == 'asan' and tool != 'asan'):
                        continue

                if ('bad' in truth):
                    truth = 'bad'
                elif ('good' in truth):
                    truth = 'good'

                hash_mat = testcase + tool + truth
                hash_id = hashlib.sha256(hash_mat.encode()).hexdigest()

                # print("ID : {}, Tool under test : {} Class : {}, truth : {},\
                #  testcase : {}".format(hash_id, tool, bugclass, truth,
                #                        testcase))

                built_testcases.append(
                    [hash_id, testcase, tool, truth, bugclass])

    # Now process the testcases
    if (len(built_testcases) > 0):
        print("Total testcases : {}".format(len(built_testcases)))
        # Now we have the list of tests we want to run, we should run it against
        # each tool we know of in a scaled multiprocessed way.
        start = timer()
        p = mp.Pool(processes=mp.cpu_count() - 1)
        # FIXME: poor mans stopper.
        i = 0
        qte_results = []
        qasan_results = []
        asan_results = []
        # Split out the test suites in case they're requested individually.
        print("Test suite choice : {}".format(testsuite_choice))
        if (testsuite_choice != 'all'):
            if (testsuite_choice == 'qte'):
                qte_results = p.imap_unordered(run_qte,
                                               built_testcases,
                                               chunksize=1000)
            if (testsuite_choice == 'qasan'):
                qasan_results = p.imap_unordered(run_qasan,
                                                 built_testcases,
                                                 chunksize=1000)
            if (testsuite_choice == 'asan'):
                asan_results = p.imap_unordered(run_asan,
                                                built_testcases,
                                                chunksize=1000)
        else:
            # Run all the test suites
            qte_results = p.imap_unordered(run_qte,
                                           built_testcases,
                                           chunksize=1000)
            qasan_results = p.imap_unordered(run_qasan,
                                             built_testcases,
                                             chunksize=1000)
            asan_results = p.imap_unordered(run_asan,
                                            built_testcases,
                                            chunksize=1000)

        # Process the outputs
        print("Processing output of returned results now.")
        for r in qte_results:
            add_to_database(r)
            i += 1
            if (i == 2):
                break
        for r in qasan_results:
            add_to_database(r)

        for r in asan_results:
            add_to_database(r)

        end = timer()
        print("Elapsed time : {} seconds".format(end - start))


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("qte_binary", help="Path to the QTE binary")
    parser.add_argument("qte_library", help="Path to libqte.so")
    parser.add_argument("qasan_binary", help="Path to QASAN binary")
    parser.add_argument("qasan_library", help="Path to libqasan.so")
    parser.add_argument(
        "--testsuite",
        help="Defines the test suites that are run.(Default all)",
        choices=["all", "qte", "qasan", "asan"],
        default=all)

    args = parser.parse_args()

    # Check if DB already exists.
    # Don't need to create one each time the script runs.
    if (os.path.exists(DATABASE_FILENAME)):
        print("Database already exists, continuing..")
    else:
        print("Setting up the database.")
        setup_db()

    # Update the globals with the path and library information.
    qte_path = args.qte_binary
    qte_lib_path = args.qte_library
    qasan_path = args.qasan_binary
    qasan_lib_path = args.qasan_library
    testsuite_choice = args.testsuite

    run_tests()
