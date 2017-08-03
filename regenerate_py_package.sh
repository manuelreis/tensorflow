#!/bin/bash

bazel build -c opt -c dbg --verbose_failures --strip=never  //tensorflow/tools/pip_package:build_pip_package
rm -rf ~/tmp/tensorflow_pkg/
bazel-bin/tensorflow/tools/pip_package/build_pip_package ~/tmp/tensorflow_pkg
/usr/bin/yes | pip uninstall tensorflow
project_name=$(sed -n 's/^project_name = //p' ~/tensorflow/tensorflow/tools/pip_package/setup.py | tr -d "'")
echo "PROJECT NAME:$project_name"
wheel_name="${HOME}/tmp/tensorflow_pkg/${project_name}-1.1.0rc0-cp27-cp27mu-linux_x86_64.whl"
echo "WHEEL NAME:$wheel_name"
pip install --target=~/${project_name} ${wheel_name}
