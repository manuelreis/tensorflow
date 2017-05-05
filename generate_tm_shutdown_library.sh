#!/bin/bash

cd tensorflow/core/kernels
TF_INC=$(python -c 'import tensorflow as tf; print(tf.sysconfig.get_include())')
gcc -std=c++11 -mrtm -shared tm_shutdown_op.cc resource_variable_ops.cc -o tm_shutdown.so -fPIC -I $TF_INC -O2
rm -rf ~/tm_shutdown_lib/
mkdir  ~/tm_shutdown_lib/
mv tm_shutdown.so ~/tm_shutdown_lib
export LD_LIBRARY_PATH=/home/mreis/tm_shutdown_lib
cd -
