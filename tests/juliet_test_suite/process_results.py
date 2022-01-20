#!/usr/bin/env python3

import os
import sqlite3 as sl
import argparse

CWE121 = 'CWE121_Stack_Based_Buffer_Overflow'
CWE122 = 'CWE122_Heap_Based_Buffer_Overflow'
CWE124 = 'CWE124_Buffer_Underwrite'
CWE126 = 'CWE126_Buffer_Overread'
CWE127 = 'CWE127_Buffer_Underread'
CWE415 = 'CWE415_Double_Free'
CWE416 = 'CWE416_Use_After_Free'
CWE590 = 'CWE590_Free_Memory_Not_on_Heap'


def connect_db(db_name):
    # DB layout:
    # id            - A unique ID (sha256(filename||tool||truth))
    # filename      - Full path to the testcase.
    # tool          - The tool that ran this testcase
    # truth         - whether this is a known good or bad case.
    # class         - the CWE class of this test case.
    # ret_code      - the return code from the tool that ran this testcase.
    # ret_output    - the output from the tool that ran this testcase.
    # ret_status    - How did the tool classify this testcase? good or bad?
    # print("Connecting to DB : {}".format(db_name))
    con = sl.connect(db_name)
    return con


def execute_query(db_name, query):
    con = connect_db(db_name)
    if (args.debug == 'yes'):
        print("Executing query : {}".format(query))
    with con:
        cursor = con.execute(query)
    return cursor


def query_total_count_testcases(tool, category, known_status):
    query = "SELECT COUNT() FROM EXPERIMENT WHERE tool == '{}' and class == '{}' and truth  == '{}'  and (ret_status != 'timedout');".format(
        tool, category, known_status)
    cursor = execute_query(args.database, query)
    return cursor


def query_true_positives(tool, category, known_status, ret_status):
    query = "SELECT COUNT() FROM EXPERIMENT WHERE tool = '{}' and class == '{}' and truth == '{}' and (ret_status != 'timedout') and (ret_status == '{}')".format(
        tool, category, known_status, ret_status)
    cursor = execute_query(args.database, query)
    return cursor


def query_true_negatives(tool, category, known_status, ret_status):
    query = "SELECT COUNT() FROM EXPERIMENT WHERE tool = '{}' and class == '{}' and truth == '{}' and (ret_status != 'timedout') and (ret_status == '{}')".format(
        tool, category, known_status, ret_status)
    cursor = execute_query(args.database, query)
    return cursor


def query_false_positives(tool, category, known_status, ret_status):
    query = "SELECT COUNT() FROM EXPERIMENT WHERE tool = '{}' and class == '{}' and truth == '{}' and (ret_status != 'timedout') and (ret_status == '{}')".format(
        tool, category, known_status, ret_status)
    cursor = execute_query(args.database, query)
    return cursor


def query_false_negatives(tool, category, known_status, ret_status):
    query = "SELECT COUNT() FROM EXPERIMENT WHERE tool = '{}' and class == '{}' and truth == '{}' and (ret_status != 'timedout') and (ret_status == '{}')".format(
        tool, category, known_status, ret_status)
    cursor = execute_query(args.database, query)
    return cursor


def process_cwe121(args):
    print("Processing data for : {}".format(CWE121))
    # For each test case we need to process the following:
    # Total number of test cases

    # How many test cases for ASAN?
    tool = 'asan'
    category = CWE121
    total_good_tests = query_total_count_testcases(tool, category, 'good')
    total_bad_tests = query_total_count_testcases(tool, category, 'bad')

    tp_good_tests = query_true_positives(tool, category, 'good', 'good')
    tp_bad_tests = query_true_positives(tool, category, 'bad', 'bad')

    tn_good_tests = query_true_negatives(tool, category, 'good', 'good')
    tn_bad_tests = query_true_negatives(tool, category, 'bad', 'bad')

    fn_good_tests = query_false_negatives(tool, category, 'good', 'bad')
    fn_bad_tests = query_false_negatives(tool, category, 'bad', 'good')

    fp_good_tests = query_false_positives(tool, category, 'good', 'bad')
    fp_bad_tests = query_false_positives(tool, category, 'bad', 'good')

    for row in total_good_tests:
        print("{} : Good : Total : {}".format(tool, row[0]))
    for row in tp_good_tests:
        print("{} : Good : TP : {}".format(tool, row[0]))
    for row in tn_good_tests:
        print("{} : Good : TN : {}".format(tool, row[0]))
    for row in fp_good_tests:
        print("{} : Good : FP : {}".format(tool, row[0]))
    for row in fn_good_tests:
        print("{} : Good : FN : {}".format(tool, row[0]))

    for row in total_bad_tests:
        print("{} : Bad : Total : {}".format(tool, row[0]))
    for row in tp_bad_tests:
        print("{} : Bad : TP : {}".format(tool, row[0]))
    for row in tn_bad_tests:
        print("{} : Bad : TN : {}".format(tool, row[0]))
    for row in fp_bad_tests:
        print("{} : Bad : FP : {}".format(tool, row[0]))
    for row in fn_bad_tests:
        print("{} : Bad : FN : {}".format(tool, row[0]))

    return


def process_cwe122(args):
    print("Processing data for : {}".format(CWE122))
    # How many test cases for ASAN? and good?
    tool = 'asan'
    good_tests_cursor = query_total_count_testcases(tool, CWE122, 'good')
    bad_tests_cursor = query_total_count_testcases(tool, CWE122, 'bad')

    for row in good_tests_cursor:
        print("Total ASAN good cases : {}".format(row[0]))
    for row in bad_tests_cursor:
        print("Total ASAN bad cases : {}".format(row[0]))

    return


def process_cwe124(args):
    print("Processing data for : {}".format(CWE124))
    # How many test cases for ASAN? and good?
    tool = 'asan'
    good_tests_cursor = query_total_count_testcases(tool, CWE124, 'good')
    bad_tests_cursor = query_total_count_testcases(tool, CWE124, 'bad')

    for row in good_tests_cursor:
        print("Total ASAN good cases : {}".format(row[0]))
    for row in bad_tests_cursor:
        print("Total ASAN bad cases : {}".format(row[0]))

    return


def process_cwe126(args):
    print("Processing data for : {}".format(CWE126))
    # How many test cases for ASAN? and good?
    tool = 'asan'
    good_tests_cursor = query_total_count_testcases(tool, CWE126, 'good')
    bad_tests_cursor = query_total_count_testcases(tool, CWE126, 'bad')

    for row in good_tests_cursor:
        print("Total ASAN good cases : {}".format(row[0]))
    for row in bad_tests_cursor:
        print("Total ASAN bad cases : {}".format(row[0]))

    return


def process_cwe127(args):
    print("Processing data for : {}".format(CWE127))

    # How many test cases for ASAN? and good?
    tool = 'asan'
    good_tests_cursor = query_total_count_testcases(tool, CWE127, 'good')
    bad_tests_cursor = query_total_count_testcases(tool, CWE127, 'bad')

    for row in good_tests_cursor:
        print("Total ASAN good cases : {}".format(row[0]))
    for row in bad_tests_cursor:
        print("Total ASAN bad cases : {}".format(row[0]))
    return


def process_cwe415(args):
    print("Processing data for : {}".format(CWE415))
    # How many test cases for ASAN? and good?
    tool = 'asan'
    good_tests_cursor = query_total_count_testcases(tool, CWE415, 'good')
    bad_tests_cursor = query_total_count_testcases(tool, CWE415, 'bad')

    for row in good_tests_cursor:
        print("Total ASAN good cases : {}".format(row[0]))
    for row in bad_tests_cursor:
        print("Total ASAN bad cases : {}".format(row[0]))
    return


def process_cwe416(args):
    print("Processing data for : {}".format(CWE416))
    # How many test cases for ASAN? and good?
    tool = 'asan'
    good_tests_cursor = query_total_count_testcases(tool, CWE416, 'good')
    bad_tests_cursor = query_total_count_testcases(tool, CWE416, 'bad')

    for row in good_tests_cursor:
        print("Total ASAN good cases : {}".format(row[0]))
    for row in bad_tests_cursor:
        print("Total ASAN bad cases : {}".format(row[0]))
    return


def process_cwe590(args):
    print("Processing data for : {}".format(CWE590))
    # How many test cases for ASAN? and good?
    tool = 'asan'
    good_tests_cursor = query_total_count_testcases(tool, CWE590, 'good')
    bad_tests_cursor = query_total_count_testcases(tool, CWE590, 'bad')

    for row in good_tests_cursor:
        print("Total ASAN good cases : {}".format(row[0]))
    for row in bad_tests_cursor:
        print("Total ASAN bad cases : {}".format(row[0]))
    return


def do_processing(args):
    print("Starting processing")
    if (args.category == 'CWE121'):
        process_cwe121(args)
    elif (args.category == 'CWE122'):
        process_cwe122(args)
    elif (args.category == 'CWE124'):
        process_cwe124(args)
    elif (args.category == 'CWE126'):
        process_cwe126(args)
    elif (args.category == 'CWE127'):
        process_cwe127(args)
    elif (args.category == 'CWE415'):
        process_cwe415(args)
    elif (args.category == 'CWE416'):
        process_cwe416(args)
    elif (args.category == 'CWE590'):
        process_cwe590(args)
    elif (args.category == 'all'):
        process_cwe121(args)
        process_cwe122(args)
        process_cwe124(args)
        process_cwe126(args)
        process_cwe127(args)
        process_cwe415(args)
        process_cwe416(args)
        process_cwe590(args)
    else:
        print("Invalid choice specified. Not processing any further.")

    return


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("category",
                        choices=[
                            'CWE121', 'CWE122', 'CWE124', 'CWE126', 'CWE127',
                            'CWE415', 'CWE416', 'CWE590', 'all'
                        ],
                        help="The category of CWE weakness to process",
                        default=None)

    parser.add_argument(
        "--database",
        type=str,
        default='collect.db',
        help="The location of the created database. Defaults to collect.db")
    parser.add_argument("--debug",
                        choices=['yes', 'no'],
                        default='no',
                        help="If set to yes, print out more verbose logs")
    args = parser.parse_args()

    # Check that the database exists on disk.
    if (args.database) and (not (os.path.exists(args.database))):
        print("Database file doesn't exist! : {}".format(args.database))
        exit()
    do_processing(args)
