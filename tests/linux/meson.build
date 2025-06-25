# linux_port_dep = dependency('', fallback : ['linux-port', 'linux_port_dep'])
linux_port_dep = dependency('linux-port')

test_sources = [
  'test_init.c',
  # other test sources if any
]

# Use dependency defined in root scope
test_exe = executable('test_init', test_sources,
  dependencies: linux_port_dep,
  include_directories: include_directories('../../include')
)

test('linux-port-init', test_exe)

