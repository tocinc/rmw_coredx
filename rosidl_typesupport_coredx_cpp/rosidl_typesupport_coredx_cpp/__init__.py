# Copyright 2014 Open Source Robotics Foundation, Inc.
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

from rosidl_cmake import convert_camel_case_to_lower_case_underscore
from rosidl_cmake import expand_template
from rosidl_cmake import get_newest_modification_time
from rosidl_parser import parse_message_file
from rosidl_parser import parse_service_file
from rosidl_parser import validate_field_types

def parse_ros_interface_files(pkg_name, ros_interface_files):
    message_specs = []
    service_specs = []
    for idl_file in ros_interface_files:
        extension = os.path.splitext(idl_file)[1]
        if extension == '.msg':
            message_spec = parse_message_file(pkg_name, idl_file)
            message_specs.append((idl_file, message_spec))
        elif extension == '.srv':
            service_spec = parse_service_file(pkg_name, idl_file)
            service_specs.append(service_spec)
    return (message_specs, service_specs)


def generate_cpp(args, message_specs, service_specs, known_msg_types):
    template_dir = args['template_dir']
    mapping_msgs = {
        os.path.join(template_dir, 'msg__rosidl_typesupport_coredx_cpp.hpp.em'):
        '%s__rosidl_typesupport_coredx_cpp.hpp',
        os.path.join(template_dir, 'msg__type_support.cpp.em'):
        '%s__type_support.cpp',
    }
    mapping_srvs = {
        os.path.join(template_dir, 'srv__rosidl_typesupport_coredx_cpp.hpp.em'):
        '%s__rosidl_typesupport_coredx_cpp.hpp',
        os.path.join(template_dir, 'srv__type_support.cpp.em'):
        '%s__type_support.cpp',
    }

    for template_file in mapping_msgs.keys():
        assert os.path.exists(template_file), '****  Could not find template: ' + template_file
    for template_file in mapping_srvs.keys():
        assert os.path.exists(template_file), '****  Could not find template: ' + template_file

    functions = {
        'get_header_filename_from_msg_name': convert_camel_case_to_lower_case_underscore,
    }
    # generate_dds_coredx_cpp() and therefore the make target depend on the additional files
    # therefore they must be listed here even if the generated type support files are independent
    latest_target_timestamp = get_newest_modification_time(
        args['target_dependencies'] + args.get('additional_files', []))

    for idl_file, spec in message_specs:
        validate_field_types(spec, known_msg_types)
        subfolder = os.path.basename(os.path.dirname(idl_file))
        for template_file, generated_filename in mapping_msgs.items():
            generated_file = os.path.join(args['output_dir'], subfolder)
            if generated_filename.endswith('.cpp'):
                generated_file = os.path.join(generated_file, 'dds_coredx')
            generated_file = os.path.join(
                generated_file, generated_filename %
                convert_camel_case_to_lower_case_underscore(spec.base_type.type))

            data = {'spec': spec, 'subfolder': subfolder}
            data.update(functions)
            # print(" ... expanding msg " + template_file + " for idl_file "
            #     + idl_file + " to " + generated_file)
            expand_template(
                template_file, data, generated_file,
                minimum_timestamp=latest_target_timestamp)

    for spec in service_specs:
        validate_field_types(spec, known_msg_types)
        for template_file, generated_filename in mapping_srvs.items():
            generated_file = os.path.join(args['output_dir'], 'srv')
            if generated_filename.endswith('.cpp'):
                generated_file = os.path.join(generated_file, 'dds_coredx')
            generated_file = os.path.join(
                generated_file, generated_filename %
                convert_camel_case_to_lower_case_underscore(spec.srv_name))

            data = {'spec': spec}
            data.update(functions)
            # print(" ... expanding srv " + template_file + " for idl_file " +
            #   idl_file + " to " + generated_file)
            expand_template(
                template_file, data, generated_file,
                minimum_timestamp=latest_target_timestamp)

    return 0


def generate_dds_coredx_cpp(
        pkg_name, dds_interface_files, dds_interface_base_path, deps,
        output_basepath, idl_pp, message_specs, service_specs):

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

    for index, idl_file in enumerate(dds_interface_files):
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
        # print("Command: " , cmd);

        subprocess.check_call(cmd)

        # if os.name == 'nt':
        #     # modify generated code to use declspec(dllimport)
        #     msg_name = os.path.splitext(os.path.basename(idl_file))[0]
        #     msg_filename = os.path.join(output_path, msg_name + '.h')
        #     _modify(msg_filename, pkg_name, msg_name, _inject_dllimport)
        #     msg_plugin_filename = os.path.join(output_path, msg_name + 'Plugin.h')
        #     _modify(msg_plugin_filename, pkg_name, msg_name, _rework_declspec)
        #     msg_support_filename = os.path.join(output_path, msg_name + 'Support.h')
        #     _modify(msg_support_filename, pkg_name, msg_name, _inject_dllimport)

    return 0
