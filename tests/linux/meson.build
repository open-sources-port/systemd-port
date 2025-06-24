linux_port_dep = dependency('', fallback : ['linux-port', 'linux_port_dep'])

test_sources = [
  'test_init.c',
  # other test sources if any
]

executable('test_init', test_sources,
  include_directories: include_directories('../../include'),
  link_with: linux_port_dep)

test('linux-port-init', executable('test_init'))
