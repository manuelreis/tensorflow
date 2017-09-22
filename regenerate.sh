#!/bin/bash

bazel build -c opt -c dbg --verbose_failures --strip=never --cxxopt="-fgnu-tm" --copt="-Dalways_inline=deprecated" --copt="-L/home/mreis/tinySTM-1.0.5/abi/gcc" --copt="-litm" --copt="-Wl,-rpath=/home/mreis/tinySTM-1.0.5/abi/gcc"  //tensorflow/tools/pip_package:build_pip_package
rm -rf ~/tmp/tensorflow_pkg/
bazel-bin/tensorflow/tools/pip_package/build_pip_package ~/tmp/tensorflow_pkg
/usr/bin/yes | pip uninstall tensorflow
#pip install --user ~/tmp/tensorflow_pkg/tensorflow-1.1.0rc0-cp27-cp27mu-linux_x86_64.whl
rm -rf ~/tensorflow-stm-and-htm
pip install ~/tmp/tensorflow_pkg/tensorflow_gccstm-1.1.0rc0-cp27-cp27mu-linux_x86_64.whl -t ~/tensorflow-stm-and-htm

