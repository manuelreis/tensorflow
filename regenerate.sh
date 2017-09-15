#!/bin/bash

bazel build -c opt -c dbg --verbose_failures --strip=never --cxxopt="-fgnu-tm" //tensorflow/tools/pip_package:build_pip_package
rm -rf ~/tmp/tensorflow_pkg/
bazel-bin/tensorflow/tools/pip_package/build_pip_package ~/tmp/tensorflow_pkg
/usr/bin/yes | pip uninstall tensorflow
#pip install --user ~/tmp/tensorflow_pkg/tensorflow-1.1.0rc0-cp27-cp27mu-linux_x86_64.whl
rm -rf ~/tensorflow-gccstm
pip install ~/tmp/tensorflow_pkg/tensorflow_gccstm-1.1.0rc0-cp27-cp27mu-linux_x86_64.whl -t ~/tensorflow-gccstm

