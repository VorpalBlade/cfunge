#!/usr/bin/python3
"""Simple test runner for cfunge"""

import argparse
import sys
import subprocess

from pathlib import Path


_SUFFIX_MAP = {
    '.b109': '109',
    '.b93': '93',
    '.b98': '98',
    '.bf': '93',
}

def main():
    """Main function"""
    parser = argparse.ArgumentParser(description='Test runner for cfunge')
    parser.add_argument('cfunge_path',
                        type=Path,
                        help='Path to cfunge')
    parser.add_argument('test_file',
                        type=Path,
                        help='Path to test file')
    args = parser.parse_args()
    test = args.test_file  # type: Path
    output = subprocess.check_output([str(args.cfunge_path),
                                      '-s', _SUFFIX_MAP[test.suffix],
                                      str(test)])
    with test.with_suffix('.expected').open(mode='rb') as expected_file:
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
