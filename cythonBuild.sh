#!/usr/bin/env bash
echo "STARTING BUILD"
# cd src_cython
printf "\n${PWD##*/}\n"
# python setup.py build_ext --inplace
# warn
#gcc -I. -I./src -pthread -fno-strict-aliasing -g -O2 -DNDEBUG -std=c++11 -g -fwrapv -O3 -Wall -Wstrict-prototypes -fPIC -I/groups/turaga/home/singhc/anaconda/include/python2.7 -c src_cython/main2.cpp -o build/temp.linux-x86_64-2.7/main2.o

cd src_cython
python setup.py build_ext --inplace
# p -I. -I./src -O3 -DNDEBUG -std=c++11 -o ./bin/a.out

cd ..
printf "\nFINISHED BUILD\n"