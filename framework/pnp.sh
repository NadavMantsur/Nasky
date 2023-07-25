#!/bin/bash

bash -c "g++ -std=c++17 -pedantic-errors -Wall -Wextra -g -rdynamic -iquote  \
~/nadav/projects/final_project/framework/include \
src/pnp.cpp src/thread_pool.cpp test/pnp_test.cpp"
