#!/usr/bin/env python3

import os
import sqlite3 as sl
import argparse

CWE121 = 'CWE121_Stack_Based_Buffer_Overflow'
CWE122 = 'CWE122_Heap_Based_Buffer_Overflow'
CWE124 = ''
CWE126 = ''
CWE127 = ''
CWE415 = ''
CWE416 = ''
CWE590 = ''


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


def query_total_count_testcases(tool, category, known_status):
    query = "SELECT COUNT() FROM EXPERIMENT WHERE tool == '{}' and class == '{}' and truth  == '{}'  and (ret_status != 'timedout');".format(
        tool, category, known_status)
    print("Executing query : {}".format(query))
    con = connect_db(args.database)
    with con:
        cursor = con.execute(query)

    return cursor


def process_cwe121(args):
    # For each test case we need to process the following:
    # Total number of test cases
    print("Processing data for CWE121 : {}".format(CWE121))

    # How many test cases for ASAN? and good?
    tool = 'asan'
    good_tests_cursor = query_total_count_testcases(tool, CWE121, 'good')
    for row in good_tests_cursor:
        print("ASAN(Good) CWE 121 cases : {}".format(row[0]))
    bad_tests_cursor = query_total_count_testcases(tool, CWE121, 'bad')
    for row in bad_tests_cursor:
        print("ASAN(Bad) CWE 121 cases : {}".format(row[0]))
    return


def process_cwe122(args):
    # How many test cases for ASAN? and good?
    tool = 'asan'
    good_tests_cursor = query_total_count_testcases(tool, CWE121, 'good')
    for row in good_tests_cursor:
        print("ASAN(Good) CWE 122 cases : {}".format(row[0]))
    bad_tests_cursor = query_total_count_testcases(tool, CWE121, 'bad')
    for row in bad_tests_cursor:
        print("ASAN(Bad) CWE 122 cases : {}".format(row[0]))

    return


def process_cwe124(args):
    return


def process_cwe126(args):
    return


def process_cwe127(args):
    return


def process_cwe415(args):
    return


def process_cwe416(args):
    return


def process_cwe590(args):
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
    else:
        print("Invalid choice specified. Not processing any further.")

    return


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("category",
                        choices=[
                            'CWE121', 'CWE122', 'CWE124', 'CWE126', 'CWE127',
                            'CWE415', 'CWE416', 'CWE590'
                        ],
                        help="The category of CWE weakness to process",
                        default=None)
    parser.add_argument("known_status", choices=['good', 'bad'])
    parser.add_argument(
        "--database",
        type=str,
        default='collect.db',
        help="The location of the created database. Defaults to collect.db")
    args = parser.parse_args()

    # Check that the database exists on disk.
    if (args.database) and (not (os.path.exists(args.database))):
        print("Database file doesn't exist! : {}".format(args.database))
        exit()
    do_processing(args)
