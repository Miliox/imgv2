project(
  'imgv2',
  'cpp',
  default_options: [
    'buildtype=release',
    'cpp_std=c++20',
    'default_library=static',
    'optimization=3',
  ],
  license: 'MIT',
  version: '0.0.1')

sdlit_sub = subproject('sdlit')
sdlit_dep = sdlit_sub.get_variable('sdlit_dep')
sdlit_lib = sdlit_sub.get_variable('sdlit_lib')

pfd_dep = subproject('pfd').get_variable('pfd_dep')

imgv2_dep = declare_dependency(
  include_directories: include_directories('include'),
  dependencies: [sdlit_dep, pfd_dep],
  link_with: [sdlit_lib],
)
imgv2_exe = executable('imgv2', ['src/main.cpp'], win_subsystem: 'windows', dependencies: imgv2_dep)
