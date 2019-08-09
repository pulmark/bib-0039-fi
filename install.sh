#!/bin/sh
###############################################################################
# Installation script using GNU g++ compiler
###############################################################################
g++ -o a.out -no-pie -pthread -pipe -Wall -O2 -march=native -mtune=native \
-fprefetch-loop-arrays --param max-inline-insns-single=2000 -std=c++1y \
main.cc -Wl,-Bdynamic
