#!/usr/bin/env bash
rm zwatershed.so
python setup.py build_ext --inplace
printf "SUCCESFUL BUILD\n"