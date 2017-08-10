#!/bin/bash

HOME=/home/shady/dleoni

# PREREQUISITES: 
# 1 - ./configure --prefix=${HOME}/tensorflow_build
# 2 - modify "tensorflow/workspace.bzl" commenting line 88: replace "patch", with "HOME/patch_build/bin/patch"

export PATH=$HOME/python_build/bin:$HOME/patch_build/bin:$HOME/python_build/include/python2.7:$HOME/python-wheel/bin:$HOME/bazel/output:$HOME/protobuf_build/bin:/usr/local/bin:/usr/bin:/usr/local/sbin:/usr/sbin:/home/shady/.local/bin:/home/shady/bin
export LD_LIBRARY_PATH=$HOME/python_build/lib/engines:$HOME/python_build/lib

rm -rf $HOME/bazel_output_base
rm -rf $HOME/bazel_output_user_root
rm -rf $HOME/tensorflow_pip_package
rm -rf $HOME/tensorflow_build
mkdir $HOME/bazel_output_base
mkdir $HOME/bazel_output_user_root
mkdir $HOME/tensorflow_pip_package
mkdir $HOME/tensorflow_build


bazel --output_base=$HOME/bazel_output_base --output_user_root=$HOME/bazel_output_user_root build -c opt -c dbg --strip=never --verbose_failures --cxxopt="-D_GLIBCXX_USE_CXX11_ABI=0" //tensorflow/tools/pip_package:build_pip_package
cd $HOME/tensorflow
bazel-bin/tensorflow/tools/pip_package/build_pip_package $HOME/tensorflow_pip_package
/usr/bin/yes | pip uninstall tensorflow
project_name=$(sed -n 's/^project_name = //p' $HOME/tensorflow/tensorflow/tools/pip_package/setup.py | tr -d "'")
wheel_name="${HOME}/tensorflow_pip_package/${project_name}-1.1.0rc0-cp27-cp27m-linux_ppc64.whl"
rm -rf ${HOME}/${project_name}
mkdir ${HOME}/${project_name}
pip install --target=${HOME}/${project_name} ${wheel_name}
export PYTHONPATH=${HOME}/${project_name}
