subdir('script')
subdir('input')

executable(
  'viw',
  'log.c',
  'main.c',
  'misc.c',
  'mem.c',
  'output.c',
  'seat.c',
  'viw.c',
  'xdg.c',
  script_sources,
  input_sources,
  include_directories: include_directories('.'),
  dependencies: [wayland_server, wlroots, xkbcommon, luajit, libinput, libpcre2_8]
)
