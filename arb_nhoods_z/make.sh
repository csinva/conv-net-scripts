#!/usr/bin/env bash
rm z_watershed.so
python setup.py build_ext --inplace
printf "BUILD COMPLETE\n"