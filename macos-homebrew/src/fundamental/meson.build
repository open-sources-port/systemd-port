# SPDX-License-Identifier: LGPL-2.1-or-later

fundamental_path = meson.current_source_dir()
fundamental_inc_path = '../../include/fundamental'

fundamental_headers = files(
        join_paths(fundamental_inc_path, 'bootspec-fundamental.h'),
        join_paths(fundamental_inc_path, 'efivars-fundamental.h'),
        join_paths(fundamental_inc_path, 'macro-fundamental.h'),
        join_paths(fundamental_inc_path, 'sha256.h'),
        join_paths(fundamental_inc_path, 'string-util-fundamental.h'),
        join_paths(fundamental_inc_path, 'tpm-pcr.h')
)

# for sd-boot
fundamental_source_paths = files(
        'bootspec-fundamental.c',
        'efivars-fundamental.c',
        'sha256.c',
        'string-util-fundamental.c',
        'tpm-pcr.c',
)

# for libbasic
fundamental_sources = fundamental_source_paths + fundamental_headers

libfundamental = static_library(
        'fundamental',
        fundamental_sources,
        # include_directories : [include_directories(fundamental_inc_path)],
        include_directories: [
                incdir,
                systemd_inc,
                shared_inc,
                basic_inc,
                fundamental_inc,
                udev_inc,
        ],
        # dependencies : [versiondep,
        #                 threads,
        #                 libcap,
        #                 libm],
        dependencies : [],
        c_args : ['-fvisibility=default'],
        build_by_default : true)
