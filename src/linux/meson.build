linux_port_sources = [
  'init.c',
  # add other source files here
]

linux_port_inc = include_directories('../../include')

linux_port = static_library(
  'linux-port',
  linux_port_sources,
  include_directories: linux_port_inc,
  install: false
)

# Export the library for use by tests or other modules
declare_dependency(link_with: linux_port, include_directories: linux_port_inc)
