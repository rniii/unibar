project('unibar', 'c',
  license: 'Apache-2.0',
  default_options: ['warning_level=3'],
  version: '0.1.0')

lua_dep = dependency('lua')
xcb_dep = dependency('xcb')
xcb_icccm_dep = dependency('xcb-icccm')
xcb_ewmh = dependency('xcb-ewmh')
xcb_util_dep = dependency('xcb-util')
cairo_dep = dependency('cairo')

executable('unibar', 'src/main.c',
  dependencies: [lua_dep, xcb_dep, xcb_icccm_dep, xcb_ewmh, xcb_util_dep, cairo_dep],
  install: true)

install_subdir('core',
  install_dir: get_option('libdir') / 'unibar')
