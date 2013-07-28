#!/usr/bin/env bash
# This is used to format files in cfunge as needed. You need to install astyle to use this:
#   http://astyle.sourceforge.net/
#   I use astyle 2.01. No idea if it will work with previous versions.
astyle --indent-preprocessor --indent-namespaces --indent-labels --keep-one-line-statements --keep-one-line-blocks --indent=tab=4 --max-instatement-indent=40 --brackets=linux --min-conditional-indent=1 --unpad-paren --indent-switches --pad-header --pad-oper "$@"
