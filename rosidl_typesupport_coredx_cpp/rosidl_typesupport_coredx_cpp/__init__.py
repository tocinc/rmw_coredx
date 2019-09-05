# Copyright 2014 Open Source Robotics Foundation, Inc.
# Modifications copyright (C) 2017 Twin Oaks Computing, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import os
import subprocess
import sys

from rosidl_cmake import generate_files

def generate_dds_coredx_cpp(
        pkg_name, dds_interface_files, dds_interface_base_path, deps,
        output_basepath, idl_pp):

    include_dirs = [dds_interface_base_path]
    for dep in deps:
        # Only take the first : for separation, as Windows follows with a C:\
        dep_parts = dep.split(':', 1)
        assert len(dep_parts) == 2, "The dependency '%s' must contain a double colon" % dep
        idl_path = dep_parts[1]
        idl_base_path = os.path.dirname(
            os.path.dirname(os.path.dirname(os.path.normpath(idl_path))))
        if idl_base_path not in include_dirs:
            include_dirs.append(idl_base_path)

    for idl_file in dds_interface_files:
        assert os.path.exists(idl_file), 'Could not find IDL file: ' + idl_file

        # get two level of parent folders for idl file
        folder = os.path.dirname(idl_file)
        parent_folder = os.path.dirname(folder)
        output_path = os.path.join(
            output_basepath,
            os.path.basename(parent_folder),
            os.path.basename(folder))
        try:
            os.makedirs(output_path)
        except FileExistsError:
            pass

        cmd = [idl_pp]
        for include_dir in include_dirs:
            cmd += ['-I', include_dir]
        cmd += [
            '-s',       # don't generate code for 'include'd idl files
            # '-i', 'p',  # generate 'print()' routine for data types
            '-d', output_path,
            '-l', 'cpp',
            '-f', idl_file
        ]
        print("Command: " , cmd);

        try:
            subprocess.run(cmd)
        except subprocess.CalledProcessError as e:
            print ("CODEGEN: output:\n", e.stdout)
            print ("CODEGEN: stderr:\n", e.stderr)
            return e.returncode

    return 0


def generate_cpp(arguments_file):
    mapping = {
        'idl__rosidl_typesupport_coredx_cpp.hpp.em':
        '%s__rosidl_typesupport_coredx_cpp.hpp',
        'idl__dds_coredx__type_support.cpp.em':
        'dds_coredx/%s__type_support.cpp'
    }
    generate_files(arguments_file, mapping)
    return 0
