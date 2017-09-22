#!/bin/bash

bazel build -c opt -c dbg --linkopt="-Wl,-rpath=/home/dleoni/glibc-2.19_install/lib:/usr/lib/x86_64-linux-gnu:/lib/x86_64-linux-gnu" --linkopt="-Wl,--dynamic-linker=/home/dleoni/glibc-2.19_install/lib/ld-linux-x86-64.so.2" --verbose_failures --strip=never  //tensorflow/tools/pip_package:build_pip_package
rm -rf ~/tmp/tensorflow_pkg/
bazel-bin/tensorflow/tools/pip_package/build_pip_package ~/tmp/tensorflow_pkg
/usr/bin/yes | pip uninstall tensorflow
project_name=$(sed -n 's/^project_name = //p' ~/tensorflow/tensorflow/tools/pip_package/setup.py | tr -d "'")
wheel_name="${HOME}/tmp/tensorflow_pkg/${project_name}-1.1.0rc0-cp27-cp27mu-linux_x86_64.whl"
rm -rf ~/${project_name}
mkdir ~/${project_name}
pip install --target=${HOME}/${project_name} ${wheel_name}
export PYTHONPATH=~/${project_name}
