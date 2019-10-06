# General instructions

```
git clone --recurse-submodules https://git.chrisoft.org/git/QMidiPlayer.git
mkdir build && cd build && cmake .. && make -j$(nproc)
```

Dependencies:

> cmake (3.10), pkg-config, libfluidsynth 2.x, RtMidi and Qt5.

If you want to build the default visualization plugin, you need some additional libraries:

> glfw3, glew, DevIL, zlib, freetype

C++14 is _required_ to build the project.

Check out `make edit_cache` for more options.

Lite version is currently not built.

# Using Windows?

Since `pkg-config` is barely usable on Windows, the answer is I don't know.
Currently I use [mxe](https://mxe.cc) for Windows builds.

If you are using msvc, I would recommend trying out [this fork](https://github.com/chirs241097/fluidsynth-sans-glib/)
of fluidsynth to reduce your potential pain when building QMidiPlayer.

