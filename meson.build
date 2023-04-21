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
