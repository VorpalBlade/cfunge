#!/usr/bin/env bash

if ! hash gperf; then
	echo "Please install gperf to run this script."
fi
gperf safe_env.gperf > ../safe_env.c

echo "Please do the post processing changes to fix inline statements."
