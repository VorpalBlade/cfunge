#!/usr/bin/python3
"""Simple test runner for cfunge"""

import argparse
import os
import sys
import subprocess

_SUFFIX_MAP = {
    'b109': '109',
    'b93': '93',
    'b98': '98',
    'bf': '93',
}


def main():
    """Main function"""
    parser = argparse.ArgumentParser(description='Test runner for cfunge')
    parser.add_argument('cfunge_path',
                        help='Path to cfunge')
    parser.add_argument('test_file',
                        help='Path to test file')
    args = parser.parse_args()
    test = args.test_file
    test_extension = test.split('.')[-1]
    expected_file_path = '.'.join(test.split('.')[:-1]) + '.expected'
    output = subprocess.check_output([args.cfunge_path,
                                      '-s', _SUFFIX_MAP[test_extension],
                                      test])

    with open(expected_file_path, mode='rb') as expected_file:
        expected = expected_file.read()
        if expected == output:
            sys.exit(0)
        else:
            print("Expected:")
            print(expected)
            print("Actual:")
            print(output)
            sys.exit(1)


if __name__ == '__main__':
    main()
