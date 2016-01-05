mkdir bin
#mkdir experiments
#mkdir experiments/watershed
#mkdir experiments/linear
#mkdir experiments/square
#mkdir experiments/threshold
#mkdir experiments/felzenszwalb
g++ src/main2.cpp -I. -I./src -O3 -DNDEBUG -std=c++11 -o ./bin/a.out
