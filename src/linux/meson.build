linux_port_sources = [
  'init.c',
  'netfilter.c'
]

linux_port_inc = include_directories('../../include')

linux_port = static_library(
  'linux-port',
  linux_port_sources,
  include_directories: linux_port_inc,
  install: false
)

# The library for use by tests or other modules
linux_port_dep  = declare_dependency(link_with: linux_port, include_directories: linux_port_inc)

meson.override_dependency('linux-port', linux_port_dep)
