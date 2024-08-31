# unibar

A pretty, pretty fast and pretty extensible status bar written in Lua.

## Development

<!-- maid-tasks -->

### setup

```sh
export CC=clang
meson setup build/debug -Dbuildtype=debugoptimized
meson setup build/release -Dbuildtype=release
```

### build

```sh
[ -d build ] || maid setup; ninja -C build/debug
```

### release

```sh
[ -d build ] || maid setup; ninja -C build/release
```

### run

```sh
maid build && ./build/debug/unibar
```
