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


def query_true_positives(tool, category):
    # TP - When the testcase is good, and the output is also classified as good
    query = "SELECT COUNT() FROM EXPERIMENT WHERE tool = '{}' and class == '{}' and truth == 'good' and (ret_status != 'timedout') and (ret_status == 'good')".format(
        tool, category)
    cursor = execute_query(args.database, query)
    return cursor


def query_true_negatives(tool, category):
    # TN - When the testcase is bad, and the output is also classified as bad
    query = "SELECT COUNT() FROM EXPERIMENT WHERE tool = '{}' and class == '{}' and truth == 'bad' and (ret_status != 'timedout') and (ret_status == 'bad')".format(
        tool, category)
    cursor = execute_query(args.database, query)
    return cursor


def query_false_positives(tool, category):
    # FP - When the testcase is bad, and the output is classified as good
    query = "SELECT COUNT() FROM EXPERIMENT WHERE tool = '{}' and class == '{}' and truth == 'bad' and (ret_status != 'timedout') and (ret_status == 'good')".format(
        tool, category)
    cursor = execute_query(args.database, query)
    return cursor


def query_false_negatives(tool, category):
    # FN - When the testcase is good, but the output is classified as bad.
    query = "SELECT COUNT() FROM EXPERIMENT WHERE tool = '{}' and class == '{}' and truth == 'good' and (ret_status != 'timedout') and (ret_status == 'bad')".format(
        tool, category)
    cursor = execute_query(args.database, query)
    return cursor


def print_stats(tool, category):
    total_good_tests = query_total_count_testcases(tool, category, 'good')

    tp_tests = query_true_positives(tool, category)
    tn_tests = query_true_negatives(tool, category)
    fp_tests = query_false_positives(tool, category)
    fn_tests = query_false_negatives(tool, category)

    total_tests = 0
    tp = 0
    fp = 0
    tn = 0
    fn = 0
    print("-" * 80)
    for row in total_good_tests:
        total_tests = int(row[0]) * 2
        print("{} : Total : {}".format(tool, total_tests))
    if (total_tests == 0):
        print("No tests have been run for this tool {}".format(tool))
        return
    for row in tp_tests:
        entry = row[0]
        tp = int(entry)
        percent = (tp / total_tests) * 100
        print("{} : TP : {} | {:.2f}%".format(tool, entry, percent))
    for row in tn_tests:
        entry = row[0]
        tn = int(entry)
        percent = (tn / total_tests) * 100
        print("{} : TN : {} | {:.2f}%".format(tool, entry, percent))
    for row in fp_tests:
        entry = row[0]
        fp = int(entry)
        percent = (fp / total_tests) * 100
        print("{} : FP : {} | {:.2f}%".format(tool, entry, percent))
    for row in fn_tests:
        entry = row[0]
        fn = int(entry)
        percent = (fn / total_tests) * 100
        print("{} : FN : {} | {:.2f}%".format(tool, entry, percent))

    # positive predictive value - TP / (TP + FP)
    # negative predicitive value - TN / (TN + FN)
    sensitivity = (tp / (tp + fn))
    specificity = (tn / (tn + fp))
    prevalence = (tp + fn) / total_tests
    ppv = tp / (tp + fp)
    npv = tn / (tn + fn)
    try:
        odds_ratio = (tp * tn) / (tp * fn)
    except ZeroDivisionError:
        odds_ratio = 0
        pass
    try:
        relative_risk = (ppv) / (npv)
    except ZeroDivisionError:
        relative_risk = 0
        pass

    print(
        "Sensitivity : {:.2f}, Specificity : {:.2f}, Prevalence : {:.2f}, PPV : {:.2f}, NPV : {:.2f}, OR : {:.2f}, RR : {:.2f}"
        .format(sensitivity, specificity, prevalence, ppv, npv, odds_ratio,
                relative_risk))

    print("-" * 80)


def process_cwe121(args):
    print("Processing data for : {}".format(CWE121))
    tool = 'qte'
    category = CWE121
    print_stats(tool, category)

    tool = 'qasan'
    print_stats(tool, category)

    tool = 'asan'
    print_stats(tool, category)
    return


def process_cwe122(args):
    print("Processing data for : {}".format(CWE122))
    tool = 'qte'
    category = CWE122
    print_stats(tool, category)

    tool = 'qasan'
    print_stats(tool, category)

    tool = 'asan'
    print_stats(tool, category)
    return


def process_cwe124(args):
    print("Processing data for : {}".format(CWE124))
    tool = 'qte'
    category = CWE124
    print_stats(tool, category)

    tool = 'qasan'
    print_stats(tool, category)

    tool = 'asan'
    print_stats(tool, category)

    return


def process_cwe126(args):
    print("Processing data for : {}".format(CWE126))
    tool = 'qte'
    category = CWE126
    print_stats(tool, category)

    tool = 'qasan'
    print_stats(tool, category)

    tool = 'asan'
    print_stats(tool, category)

    return


def process_cwe127(args):
    print("Processing data for : {}".format(CWE127))
    tool = 'qte'
    category = CWE127
    print_stats(tool, category)

    tool = 'qasan'
    print_stats(tool, category)

    tool = 'asan'
    print_stats(tool, category)

    return


def process_cwe415(args):
    print("Processing data for : {}".format(CWE415))
    tool = 'qte'
    category = CWE415
    print_stats(tool, category)

    tool = 'qasan'
    print_stats(tool, category)

    tool = 'asan'
    print_stats(tool, category)

    return


def process_cwe416(args):
    print("Processing data for : {}".format(CWE416))
    tool = 'qte'
    category = CWE416
    print_stats(tool, category)

    tool = 'qasan'
    print_stats(tool, category)

    tool = 'asan'
    print_stats(tool, category)

    return


def process_cwe590(args):
    print("Processing data for : {}".format(CWE590))
    tool = 'qte'
    category = CWE590
    print_stats(tool, category)

    tool = 'qasan'
    print_stats(tool, category)

    tool = 'asan'
    print_stats(tool, category)

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
