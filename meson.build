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

add_languages('objc')
nswindow_util_lib = static_library('nswindow_util', ['src/nswindow_util.m'], link_args: ['-framework', 'Foundation'])
nswindow_util_dep = declare_dependency(
  include_directories: include_directories('include'),
  link_args: ['-framework', 'Foundation'],
  link_with: nswindow_util_lib,
)

imgv2_dep = declare_dependency(
  include_directories: include_directories('include'),
  dependencies: [sdlit_dep, pfd_dep, nswindow_util_dep],
  link_with: [sdlit_lib],
)
imgv2_exe = executable('imgv2', ['src/main.cpp'], win_subsystem: 'windows', dependencies: imgv2_dep)
