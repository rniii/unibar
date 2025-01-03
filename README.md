# unibar

A pretty, pretty fast and pretty extensible status bar written in Lua.

## Development

<!-- maid-tasks -->

### build

```sh
mkdir -p build
cd build; zig build-exe --name unibar \
  $(pkg-config --cflags-only-I --libs-only-l cairo lua xcb{,-icccm,-ewmh,-util}) \
  ../src/main.zig
```

### run

```sh
maid build && ./build/unibar
```
