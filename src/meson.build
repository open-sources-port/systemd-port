# Build linux library
subdir('linux')

# Export linux_port_dep to parent scope
# (this sets the variable so parent meson.build can access it)
meson.override_dependency('linux-port', linux_port_dep)

# Build other libraries/modules
# subdir('other_module')