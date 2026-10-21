executable(
  'viw',
  'api.c',
  'config.c',
  'input.c',
  'log.c',
  'main.c',
  'misc.c',
  'mem.c',
  'seat.c',
  'viw.c',
  'xdg.c',
  include_directories: include_directories('.'),
  dependencies: [wayland_server, wlroots, xkbcommon, luajit, libinput, libpcre2_8]
)
