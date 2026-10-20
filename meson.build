project(
  'viw',
  'c',
  license: 'GPLv-3.0-or-later',
  default_options: [
    'c_std=c17',
    'warning_level=2',
    'b_ndebug=if-release'
  ]
)

add_project_arguments(
  [
    '-D_POSIX_C_SOURCE=200809L',
    '-DWLR_USE_UNSTABLE'
  ],
  language: 'c'
)

if get_option('buildtype').startswith('debug')
  add_project_arguments(
    '-DDEBUG',
    '-ggdb3',
    language : 'c'
  )
endif

wayland_server = dependency('wayland-server')
wlroots = dependency('wlroots-0.21')
xkbcommon = dependency('xkbcommon')
luajit = dependency('luajit')

subdir('src')
