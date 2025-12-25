# SPDX-License-Identifier: LGPL-2.1-or-later

udev_inc_path = '../../include/udev'
udev_includes = include_directories(udev_inc_path)

libudev_sources = files(
        'libudev-device.c',
        join_paths(udev_inc_path, 'libudev-device-internal.h'),
        'libudev-enumerate.c',
        'libudev-hwdb.c',
        'libudev-list.c',
        join_paths(udev_inc_path, 'libudev-list-internal.h'),
        'libudev-monitor.c',
        'libudev-queue.c',
        'libudev-util.c',
        join_paths(udev_inc_path, 'libudev-util.h'),
        'libudev.c',
        join_paths(udev_inc_path, 'libudev.h'))

############################################################

libudev_includes = [incdir, systemd_inc, shared_inc, include_directories('.')]

libudev_sym = files('libudev.sym')
libudev_sym_path = meson.current_source_dir() / 'libudev.sym'

install_headers(join_paths(udev_inc_path, 'libudev.h'), subdir: 'udev')
# libudev_h_path = meson.current_source_dir() / 'libudev.h'
libudev_h_path = join_paths(udev_inc_path, 'libudev.h')

libudev_basic = shared_library(
        'udev-basic',
        libudev_sources,
        include_directories : libudev_includes,
        c_args : ['-fvisibility=default'],
        build_by_default : false)

# static_libudev = get_option('static-libudev')
# static_libudev_pic = static_libudev == 'true' or static_libudev == 'pic'

libudev_pc = custom_target(
        'libudev.pc',
        input : 'libudev.pc.in',
        output : 'libudev.pc',
        command : [jinja2_cmdline, '@INPUT@', '-o', '@OUTPUT@'],
        install : pkgconfiglibdir != 'no',
        install_tag : 'devel',
        install_dir : pkgconfiglibdir)

############################################################

tests += [
        [files('test-libudev.c'),
        #  [libshared,
        #   libudev_basic]],
         [libudev_basic]],
]
