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

## Building QMidiPlayer with mxe in Windows Subsystem for Linux

First get mxe working using their [instructions](https://mxe.cc/#tutorial), then:
```
make cmake qt5 glfw3 glew devil zlib freetype fluidsynth
```

Proceed with normal instructions for Linux with tools replaced by those
provided by mxe. You will have to build and install RtMidi yourself.

At the moment there are some issues with mxe that may cause a
build failure:
  - A cmake file installed by glfw3 refers to a wrong file. Open
  mxe/usr/x86_64-w64-mingw32.shared/lib/cmake/glfw3/glfw3Targets-release.cmake
  ane add the missing dots for a temporary workaround. Also glfw3.dll
  should be in bin, not in lib.
  - It seems that GLEW can't tell Windows shared libraries from static
  libraries. Check the function __glew_set_find_library_suffix in
  FindGLEW.cmake and make sure the section for WIN32 makes sense.

This list may change whenever mxe updates. You are on your own to figure them out.

Another option is to use msys2. The build steps should resemble building on Linux a lot, just
remember to install dependencies for the correct toolchain. It is however not recommended to
use msys2 builds as release builds because the libraries provided by msys2 come with a sh*t
load of unnecessary features enabled for QMidiPlayer.
