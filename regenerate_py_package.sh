#!/bin/bash

bazel build -c opt  //tensorflow/tools/pip_package:build_pip_package
rm -rf ~/tmp/tensorflow_pkg/
bazel-bin/tensorflow/tools/pip_package/build_pip_package ~/tmp/tensorflow_pkg
rm -rf ~/tensorflow-1.4-htm
pip install ~/tmp/tensorflow_pkg/tensorflow_lastreleasehtm-1.4.0rc0-cp27-cp27mu-linux_x86_64.whl -t ~/tensorflow-1.4-htm
