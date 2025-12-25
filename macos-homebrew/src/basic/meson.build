# SPDX-License-Identifier: LGPL-2.1-or-later

basic_inc_path = '../../include/basic'
sys_compat_inc_path = '../../include/sys_compat'
# basic_includes = include_directories(basic_inc_path)
basic_includes = [
  incdir,
  systemd_inc,
  shared_inc,
  basic_inc,
  fundamental_inc,
  udev_inc,
]

basic_sources = files(
        'bpf_socket.c',
        join_paths(basic_inc_path, 'bpf_socket.h'),
        'MurmurHash2.c',
        join_paths(basic_inc_path, 'MurmurHash2.h'),
        'af-list.c',
        join_paths(basic_inc_path, 'af-list.h'),
        'alloc-util.c',
        join_paths(basic_inc_path, 'alloc-util.h'),
        'architecture.c',
        join_paths(basic_inc_path, 'architecture.h'),
        'arphrd-util.c',
        join_paths(basic_inc_path, 'arphrd-util.h'),
        'async.c',
        join_paths(basic_inc_path, 'async.h'),
        'audit-util.c',
        join_paths(basic_inc_path, 'audit-util.h'),
        'build.c',
        join_paths(basic_inc_path, 'build.h'),
        'bus-label.c',
        join_paths(basic_inc_path, 'bus-label.h'),
        'cap-list.c',
        join_paths(basic_inc_path, 'cap-list.h'),
        'capability-util.c',
        join_paths(basic_inc_path, 'capability-util.h'),
        'cgroup-util.c',
        join_paths(basic_inc_path, 'cgroup-util.h'),
        'chase-symlinks.c',
        join_paths(basic_inc_path, 'chase-symlinks.h'),
        'chattr-util.c',
        join_paths(basic_inc_path, 'chattr-util.h'),
        'conf-files.c',
        join_paths(basic_inc_path, 'conf-files.h'),
        join_paths(basic_inc_path, 'def.h'),
        'devnum-util.c',
        join_paths(basic_inc_path, 'devnum-util.h'),
        'dirent-util.c',
        join_paths(basic_inc_path, 'dirent-util.h'),
        join_paths(basic_inc_path, 'dns-def.h'),
        'efivars.c',
        join_paths(basic_inc_path, 'efivars.h'),
        'env-file.c',
        join_paths(basic_inc_path, 'env-file.h'),
        'env-util.c',
        join_paths(basic_inc_path, 'env-util.h'),
        'errno-list.c',
        join_paths(basic_inc_path, 'errno-list.h'),
        join_paths(basic_inc_path, 'errno-util.h'),
        'escape.c',
        join_paths(basic_inc_path, 'escape.h'),
        'ether-addr-util.c',
        join_paths(basic_inc_path, 'ether-addr-util.h'),
        'extract-word.c',
        join_paths(basic_inc_path, 'extract-word.h'),
        'fd-util.c',
        join_paths(basic_inc_path, 'fd-util.h'),
        'fileio.c',
        join_paths(basic_inc_path, 'fileio.h'),
        'filesystems.c',
        join_paths(basic_inc_path, 'filesystems.h'),
        'format-util.c',
        join_paths(basic_inc_path, 'format-util.h'),
        'fs-util.c',
        join_paths(basic_inc_path, 'fs-util.h'),
        'glob-util.c',
        join_paths(basic_inc_path, 'glob-util.h'),
        'glyph-util.c',
        join_paths(basic_inc_path, 'glyph-util.h'),
        'gunicode.c',
        join_paths(basic_inc_path, 'gunicode.h'),
        'hash-funcs.c',
        join_paths(basic_inc_path, 'hash-funcs.h'),
        'hashmap.c',
        join_paths(basic_inc_path, 'hashmap.h'),
        'hexdecoct.c',
        join_paths(basic_inc_path, 'hexdecoct.h'),
        'hmac.c',
        join_paths(basic_inc_path, 'hmac.h'),
        'hostname-util.c',
        join_paths(basic_inc_path, 'hostname-util.h'),
        'in-addr-util.c',
        join_paths(basic_inc_path, 'in-addr-util.h'),
        'inotify-util.c',
        join_paths(basic_inc_path, 'inotify-util.h'),
        'io-util.c',
        join_paths(basic_inc_path, 'io-util.h'),
        'ioprio-util.c',
        join_paths(basic_inc_path, 'ioprio-util.h'),
        'limits-util.c',
        join_paths(basic_inc_path, 'limits-util.h'),
        join_paths(meson.project_source_root(), 'include', 'linux/btrfs.h'),
        join_paths(meson.project_source_root(), 'include', 'linux/btrfs_tree.h'),
        join_paths(meson.project_source_root(), 'include', 'linux/can/netlink.h'),
        join_paths(meson.project_source_root(), 'include', 'linux/can/vxcan.h'),
        join_paths(meson.project_source_root(), 'include', 'linux/cfm_bridge.h'),
        join_paths(meson.project_source_root(), 'include', 'linux/fib_rules.h'),
        join_paths(meson.project_source_root(), 'include', 'linux/fou.h'),
        join_paths(meson.project_source_root(), 'include', 'linux/genetlink.h'),
        join_paths(meson.project_source_root(), 'include', 'linux/hdlc/ioctl.h'),
        join_paths(meson.project_source_root(), 'include', 'linux/if.h'),
        join_paths(meson.project_source_root(), 'include', 'linux/if_addr.h'),
        join_paths(meson.project_source_root(), 'include', 'linux/if_bonding.h'),
        join_paths(meson.project_source_root(), 'include', 'linux/if_bridge.h'),
        join_paths(meson.project_source_root(), 'include', 'linux/if_ether.h'),
        join_paths(meson.project_source_root(), 'include', 'linux/if_link.h'),
        join_paths(meson.project_source_root(), 'include', 'linux/if_macsec.h'),
        join_paths(meson.project_source_root(), 'include', 'linux/if_tun.h'),
        join_paths(meson.project_source_root(), 'include', 'linux/if_tunnel.h'),
        join_paths(meson.project_source_root(), 'include', 'linux/in.h'),
        join_paths(meson.project_source_root(), 'include', 'linux/in6.h'),
        join_paths(meson.project_source_root(), 'include', 'linux/ipv6_route.h'),
        join_paths(meson.project_source_root(), 'include', 'linux/l2tp.h'),
        join_paths(meson.project_source_root(), 'include', 'linux/libc-compat.h'),
        join_paths(meson.project_source_root(), 'include', 'linux/mrp_bridge.h'),
        join_paths(meson.project_source_root(), 'include', 'linux/netdevice.h'),
        join_paths(meson.project_source_root(), 'include', 'linux/netfilter/nf_tables.h'),
        join_paths(meson.project_source_root(), 'include', 'linux/netfilter/nfnetlink.h'),
        join_paths(meson.project_source_root(), 'include', 'linux/netlink.h'),
        join_paths(meson.project_source_root(), 'include', 'linux/nexthop.h'),
        join_paths(meson.project_source_root(), 'include', 'linux/nl80211.h'),
        join_paths(meson.project_source_root(), 'include', 'linux/pkt_sched.h'),
        join_paths(meson.project_source_root(), 'include', 'linux/rtnetlink.h'),
        join_paths(meson.project_source_root(), 'include', 'linux/wireguard.h'),
        join_paths(basic_inc_path, 'list.h'),
        'locale-util.c',
        join_paths(basic_inc_path, 'locale-util.h'),
        'log.c',
        join_paths(basic_inc_path, 'log.h'),
        'login-util.c',
        join_paths(basic_inc_path, 'login-util.h'),
        join_paths(basic_inc_path, 'macro.h'),
        join_paths(basic_inc_path, 'math-util.h'),
        'memfd-util.c',
        join_paths(basic_inc_path, 'memfd-util.h'),
        'memory-util.c',
        join_paths(basic_inc_path, 'memory-util.h'),
        'mempool.c',
        join_paths(basic_inc_path, 'mempool.h'),
        join_paths(basic_inc_path, 'missing_audit.h'),
        join_paths(basic_inc_path, 'missing_capability.h'),
        join_paths(basic_inc_path, 'missing_drm.h'),
        join_paths(basic_inc_path, 'missing_fcntl.h'),
        join_paths(basic_inc_path, 'missing_fs.h'),
        join_paths(basic_inc_path, 'missing_input.h'),
        join_paths(basic_inc_path, 'missing_ioprio.h'),
        join_paths(basic_inc_path, 'missing_keyctl.h'),
        join_paths('../../include/linux', 'missing_magic.h'),
        join_paths(basic_inc_path, 'missing_mman.h'),
        join_paths(basic_inc_path, 'missing_mount.h'),
        join_paths(basic_inc_path, 'missing_network.h'),
        join_paths(basic_inc_path, 'missing_prctl.h'),
        join_paths(basic_inc_path, 'missing_random.h'),
        join_paths(basic_inc_path, 'missing_resource.h'),
        join_paths(basic_inc_path, 'missing_sched.h'),
        join_paths(basic_inc_path, 'missing_securebits.h'),
        join_paths('../../include/linux', 'socket.h'),
        join_paths(basic_inc_path, 'missing_stat.h'),
        join_paths(basic_inc_path, 'missing_stdlib.h'),
        join_paths(sys_compat_inc_path, 'missing_syscall.h'),
        join_paths(basic_inc_path, 'missing_timerfd.h'),
        join_paths(basic_inc_path, 'missing_type.h'),
        'mkdir.c',
        join_paths(basic_inc_path, 'mkdir.h'),
        'mountpoint-util.c',
        join_paths(basic_inc_path, 'mountpoint-util.h'),
        'namespace-util.c',
        join_paths(basic_inc_path, 'namespace-util.h'),
        join_paths(basic_inc_path, 'nss-util.h'),
        'nulstr-util.c',
        join_paths(basic_inc_path, 'nulstr-util.h'),
        'ordered-set.c',
        join_paths(basic_inc_path, 'ordered-set.h'),
        'os-util.c',
        join_paths(basic_inc_path, 'os-util.h'),
        'parse-util.c',
        join_paths(basic_inc_path, 'parse-util.h'),
        'path-lookup.c',
        join_paths(basic_inc_path, 'path-lookup.h'),
        'path-util.c',
        join_paths(basic_inc_path, 'path-util.h'),
        'percent-util.c',
        join_paths(basic_inc_path, 'percent-util.h'),
        'prioq.c',
        join_paths(basic_inc_path, 'prioq.h'),
        'proc-cmdline.c',
        join_paths(basic_inc_path, 'proc-cmdline.h'),
        'process-util.c',
        join_paths(basic_inc_path, 'process-util.h'),
        'procfs-util.c',
        join_paths(basic_inc_path, 'procfs-util.h'),
        join_paths(basic_inc_path, 'pthread-util.h'),
        'random-util.c',
        join_paths(basic_inc_path, 'random-util.h'),
        'ratelimit.c',
        join_paths(basic_inc_path, 'ratelimit.h'),
        join_paths(basic_inc_path, 'raw-clone.h'),
        join_paths(basic_inc_path, 'raw-reboot.h'),
        'recurse-dir.c',
        join_paths(basic_inc_path, 'recurse-dir.h'),
        'replace-var.c',
        join_paths(basic_inc_path, 'replace-var.h'),
        'rlimit-util.c',
        join_paths(basic_inc_path, 'rlimit-util.h'),
        join_paths(basic_inc_path, 'set.h'),
        'sigbus.c',
        join_paths(basic_inc_path, 'sigbus.h'),
        'signal-util.c',
        join_paths(basic_inc_path, 'signal-util.h'),
        'siphash24.c',
        join_paths(basic_inc_path, 'siphash24.h'),
        'socket-util.c',
        join_paths(basic_inc_path, 'socket-util.h'),
        'sort-util.c',
        join_paths(basic_inc_path, 'sort-util.h'),
        join_paths(basic_inc_path, 'sparse-endian.h'),
        join_paths(basic_inc_path, 'special.h'),
        'stat-util.c',
        join_paths(basic_inc_path, 'stat-util.h'),
        join_paths(basic_inc_path, 'static-destruct.h'),
        join_paths(basic_inc_path, 'stdio-util.h'),
        'strbuf.c',
        join_paths(basic_inc_path, 'strbuf.h'),
        'string-table.c',
        join_paths(basic_inc_path, 'string-table.h'),
        'string-util.c',
        join_paths(basic_inc_path, 'string-util.h'),
        'strv.c',
        join_paths(basic_inc_path, 'strv.h'),
        'strxcpyx.c',
        join_paths(basic_inc_path, 'strxcpyx.h'),
        'sync-util.c',
        join_paths(basic_inc_path, 'sync-util.h'),
        'sysctl-util.c',
        join_paths(basic_inc_path, 'sysctl-util.h'),
        'syslog-util.c',
        join_paths(basic_inc_path, 'syslog-util.h'),
        'terminal-util.c',
        join_paths(basic_inc_path, 'terminal-util.h'),
        'time-util.c',
        join_paths(basic_inc_path, 'time-util.h'), 
        'tmpfile-util.c',
        join_paths(basic_inc_path, 'tmpfile-util.h'),
        'uid-range.c',
        join_paths(basic_inc_path, 'uid-range.h'),
        join_paths(basic_inc_path, 'umask-util.h'),
        join_paths(basic_inc_path, 'unaligned.h'),
        'unit-def.c',
        join_paths(basic_inc_path, 'unit-def.h'),
        'unit-file.c',
        join_paths(basic_inc_path, 'unit-file.h'),
        'unit-name.c',
        join_paths(basic_inc_path, 'unit-name.h'),
        'user-util.c',
        join_paths(basic_inc_path, 'user-util.h'),
        'utf8.c',
        join_paths(basic_inc_path, 'utf8.h'),
        'util.c',
        join_paths(basic_inc_path, 'util.h'),
        'virt.c',
        join_paths(basic_inc_path, 'virt.h'),
        'xattr-util.c',
        join_paths(basic_inc_path, 'xattr-util.h'),
)

missing_audit_h = files(join_paths(basic_inc_path, 'missing_audit.h'))
missing_capability_h = files(join_paths(basic_inc_path, 'missing_capability.h'))
missing_socket_h = files(join_paths('../../include/linux', 'socket.h'))

missing_syscall_def_h = files(join_paths(sys_compat_inc_path, 'missing_syscall_def.h'))
basic_sources += missing_syscall_def_h

generate_af_list = find_program('generate-af-list.sh')
af_list_txt = custom_target(
        'af-list.txt',
        output : 'af-list.txt',
        command : [generate_af_list, cpp, config_h, missing_socket_h],
        capture : true)

generate_arphrd_list = find_program('generate-arphrd-list.sh')
arphrd_list_txt = custom_target(
        'arphrd-list.txt',
        output : 'arphrd-list.txt',
        command : [generate_arphrd_list, cpp, config_h],
        capture : true)

generate_cap_list = find_program('generate-cap-list.sh')
cap_list_txt = custom_target(
        'cap-list.txt',
        output : 'cap-list.txt',
        command : [generate_cap_list, cpp, config_h, missing_capability_h],
        capture : true)

generate_errno_list = find_program('generate-errno-list.sh')
errno_list_txt = custom_target(
        'errno-list.txt',
        output : 'errno-list.txt',
        command : [generate_errno_list, cpp],
        capture : true)

generated_gperf_headers = []
foreach item : [['af',     af_list_txt,     'af',         ''],
                ['arphrd', arphrd_list_txt, 'arphrd',     'ARPHRD_'],
                ['cap',    cap_list_txt,    'capability', ''],
                ['errno',  errno_list_txt,  'errno',      '']]

        fname = '@0@-from-name.gperf'.format(item[0])
        gperf_file = custom_target(
                fname,
                input : item[1],
                output : fname,
                command : [generate_gperfs, item[2], item[3], '@INPUT@'],
                capture : true)

        fname = '@0@-from-name.h'.format(item[0])
        target1 = custom_target(
                fname,
                input : gperf_file,
                output : fname,
                command : [gperf,
                           '-L', 'ANSI-C', '-t', '--ignore-case',
                           '-N', 'lookup_@0@'.format(item[2]),
                           '-H', 'hash_@0@_name'.format(item[2]),
                           '-p', '-C',
                           '@INPUT@'],
                capture : true)

        fname = '@0@-to-name.h'.format(item[0])
        awkscript = '@0@-to-name.awk'.format(item[0])
        target2 = custom_target(
                fname,
                input : [awkscript, item[1]],
                output : fname,
                command : [awk, '-f', '@INPUT0@', '@INPUT1@'],
                capture : true)

        generated_gperf_headers += [target1, target2]
endforeach

basic_sources += generated_gperf_headers

############################################################

arch_list = [
        'alpha',
        'arc',
        'arm',
        'arm64',
        'i386',
        'ia64',
        'loongarch64',
        'm68k',
        'mips64',
        'mips64n32',
        'mipso32',
        'parisc',
        'powerpc',
        'powerpc64',
        'riscv32',
        'riscv64',
        's390',
        's390x',
        'sparc',
        'x86_64'
]

run_target(
        'update-syscall-tables',
        command : [update_syscall_tables_sh, meson.current_source_dir()] + arch_list)

syscall_list_txt = files('syscall-list.txt')

syscall_lists = []
foreach arch: arch_list
        syscall_lists += files('syscalls-@0@.txt'.format(arch))
endforeach

missing_syscalls_py = find_program('missing_syscalls.py')

run_target(
        'update-syscall-header',
        command : [missing_syscalls_py,
                   missing_syscall_def_h,
                   syscall_lists])

############################################################

filesystem_includes = ['linux/magic.h',
                       'linux/gfs2_ondisk.h']

check_filesystems = find_program('check-filesystems.sh')
r = run_command([check_filesystems, cpp, files('filesystems-gperf.gperf')] + filesystem_includes, check: false)
if r.returncode() != 0
        warning('Unknown filesystems defined in kernel headers:\n\n' + r.stdout())
endif

filesystems_gperf_h = custom_target(
        'filesystems-gperf.h',
        input : 'filesystems-gperf.gperf',
        output : 'filesystems-gperf.h',
        command : [gperf, '@INPUT@', '--output-file', '@OUTPUT@'])

generate_filesystem_list = find_program('generate-filesystem-list.py')
fname = 'filesystem-list.h'
filesystem_list_h = custom_target(
        fname,
        input : 'filesystems-gperf.gperf',
        output : fname,
        command : [generate_filesystem_list,
                   '@INPUT@'],
        capture : true)

generate_filesystem_switch_case_h = find_program('generate-filesystem-switch-case.py')
fname = 'filesystem-switch-case.h'
filesystem_switch_case_h = custom_target(
        fname,
        input : 'filesystems-gperf.gperf',
        output : 'filesystem-switch-case.h',
        command : [generate_filesystem_switch_case_h,
                   '@INPUT@'],
        capture : true)

basic_sources += [filesystem_list_h, filesystem_switch_case_h, filesystems_gperf_h]

libbasic = static_library(
        'basic',
        basic_sources,
        fundamental_sources,
        include_directories : basic_includes,
        dependencies : [versiondep,
                        threads,
                        libcap,
                        libm],
        c_args : ['-fvisibility=default'],
        build_by_default : false)

############################################################

basic_gcrypt_sources = files(
        'gcrypt-util.c',
        join_paths(basic_inc_path, 'gcrypt-util.h'))

# A convenience library that is separate from libbasic to avoid
# unnecessary linking to libgcrypt.
libbasic_gcrypt = static_library(
        'basic-gcrypt',
        basic_gcrypt_sources,
        include_directories : basic_includes,
        dependencies : [libgcrypt],
        c_args : ['-fvisibility=default'],
        build_by_default : false)

############################################################

basic_compress_sources = files(
        'compress.c',
        join_paths(basic_inc_path, 'compress.h'))

# A convenience library that is separate from libbasic to avoid unnecessary
# linking to the compression libraries.
libbasic_compress = static_library(
        'basic-compress',
        basic_compress_sources,
        include_directories : basic_includes,
        dependencies : [libxz,
                        libzstd,
                        liblz4],
        c_args : ['-fvisibility=default'],
        build_by_default : false)
