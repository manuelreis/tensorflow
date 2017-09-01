#!/bin/bash

bazel build -c opt -c dbg --verbose_failures --strip=never  //tensorflow/tools/pip_package:build_pip_package
rm -rf ~/tmp/tensorflow_pkg/
bazel-bin/tensorflow/tools/pip_package/build_pip_package ~/tmp/tensorflow_pkg
/usr/bin/yes | pip uninstall tensorflow
project_name=$(sed -n 's/^project_name = //p' $HOME/tensorflow/tensorflow/tools/pip_package/setup.py | tr -d "'")
wheel_name="${HOME}/tensorflow_pip_package/${project_name}-1.1.0rc0-cp27-cp27m-linux_ppc64.whl"
rm -rf ${HOME}/${project_name}
mkdir ${HOME}/${project_name}
pip install --target=${HOME}/${project_name} ${wheel_name}
export PYTHONPATH=${HOME}/${project_name}
