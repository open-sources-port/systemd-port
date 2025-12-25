# SPDX-License-Identifier: LGPL-2.1-or-later

linux_port_inc_path = '../../include/linux'

linux_port_includes = [
  incdir,
  systemd_inc,
  shared_inc,
  basic_inc,
  fundamental_inc
]

linux_port_sources = files(
        'mqueue_notify.c',
        join_paths(linux_port_inc_path, 'mqueue_shm.h'),
        'mqueue.c',
        join_paths(linux_port_inc_path, 'mqueue.h'),
        'system_call.c',
)

############################################################

liblinux = static_library(
        'linux_port',
        linux_port_sources,
        include_directories : linux_port_includes,
        dependencies : [],
        c_args : ['-fvisibility=default'],
        build_by_default : false)

############################################################
