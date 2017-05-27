#!/usr/bin/python3
"""Simple test runner for cfunge"""

import argparse
import os
import os.path
import sys
import subprocess

_SUFFIX_MAP = {
    'b109': '109',
    'b93': '93',
    'b98': '98',
    'bf': '93',
}


def cleanup():
    """Clean up output files from the test"""
    try:
        os.unlink('cfunge_TURT.svg')
    except OSError:
        pass


def compare_contents(name, expected, actual, test_filter):
    """Compare contents and report errors if they differ"""
    filtered_actual = actual
    if test_filter is not None:
        with subprocess.Popen([test_filter],
                              stdin=subprocess.PIPE,
                              stdout=subprocess.PIPE) as process:
            # Some code from subprocess.check_output() in Python 3.4 below
            try:
                filtered_actual, unused_err = process.communicate(actual)
            except:
                process.kill()
                process.wait()
                raise
            ret_code = process.poll()
            if ret_code:
                raise subprocess.CalledProcessError(ret_code, process.args, output=filtered_actual)
    if expected == filtered_actual:
        return True
    else:
        print("Expected %s:" % name, file=sys.stderr)
        print(expected, file=sys.stderr)
        print("Actual %s:" % name, file=sys.stderr)
        print(filtered_actual, file=sys.stderr)
        with open("actual", mode='wb') as f:
            f.write(filtered_actual)
        return False


def main():
    """Main function"""
    parser = argparse.ArgumentParser(description='Test runner for cfunge')
    parser.add_argument('cfunge_path',
                        help='Path to cfunge')
    parser.add_argument('test_file',
                        help='Path to test file')
    parser.add_argument('test_filter',
                        nargs='?',
                        default=None,
                        help='Path to program filtering output.')
    parser.add_argument('--exit-code',
                        default=0,
                        type=int,
                        help='Expected exit code (default: 0)')
    args = parser.parse_args()
    test = args.test_file
    test_extension = test.split('.')[-1]
    expected_file_path_base = '.'.join(test.split('.')[:-1])
    ret_code = 0
    output = b''
    try:
        output = subprocess.check_output([args.cfunge_path,
                                          '-s', _SUFFIX_MAP[test_extension],
                                          test],
                                         env={'TEST_ENV': 'test'})
    except subprocess.CalledProcessError as e:
        ret_code = e.returncode
        output = e.output

    success = True

    if ret_code != args.exit_code:
        print("Incorrect exit code %r (expected %r)" % (ret_code, args.exit_code), file=sys.stderr)

    with open(expected_file_path_base + '.expected', mode='rb') as expected_file:
        success = compare_contents("Output",
                                   expected_file.read(),
                                   output,
                                   args.test_filter) and success
    if os.path.exists('cfunge_TURT.svg'):
        with open(expected_file_path_base + '.TURT.expected', mode='rb') as expected_file, \
             open('cfunge_TURT.svg', mode='rb') as actual_file:
            success = compare_contents("TURT image",
                                       expected_file.read(),
                                       actual_file.read(),
                                       args.test_filter) and success
    cleanup()
    if success:
        sys.exit(0)
    else:
        sys.exit(1)


if __name__ == '__main__':
    main()
