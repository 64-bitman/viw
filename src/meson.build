executable(
  'viw',
  'api.c',
  'config.c',
  'log.c',
  'main.c',
  'misc.c',
  'mem.c',
  'viw.c',
  'xdg.c',
  include_directories: include_directories('.'),
  dependencies: [wayland_server, wlroots, xkbcommon, luajit]
)
