#!/bin/bash

# bash -c "sudo g++ -std=c++17 -pedantic-errors -Wall -Wextra -g -iquote ~/nadav/projects/final_project/include src/nasky.cpp test/nasky_test.cpp framework/src/logger.cpp"

bash -c "sudo kill -9 $(pidof a.out)"
bash -c "sudo rmmod nbd"
bash -c "sudo modprobe nbd"

bash -c "make"

# bash -c "sudo ./a.out"