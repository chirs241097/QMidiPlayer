# General instructions
Use qmake or Qt Creator.

Dependencies:

> libfluidsynth 2.x, Qt5, Qt quick controls(lite version) and RtMidi.

C++14 is _required_ to build the project.

To build the default visualization plugin, you need the latest SMELT library
(along with all its dependencies), which can be found
[here](https://github.com/BearKidsTeam/SMELT).

Some dependencies in the project file are hard-coded paths. You may
have to modify them first. Alternatively, you can set the environmental
variable `SMELT_DIR` to where your SMELT build is.

If you don't want to build the default visualization plugin, just remove
the line containing "visualization" in qmidiplayer.pro.

# Instruction for dumbs

1. Get SMELT [here](https://github.com/BearKidsTeam/SMELT).
2. Get the source code [here](https://github.com/chirs241097/QMidiPlayer).
3. The following steps are done in an interactive shell.
4. Change directory (`cd`) to the folder with the source code of SMELT.
   type `make` to build it.
5. Type `export QMP_BUILD_MODE=1` to allow QMP to scan plugins in
   /usr/lib/qmidiplayer.
6. Type `export SMELT_DIR=<path to the folder with SMELT in it>` in order
   to tell qmake where SMELT is.
7. Change directory to the folder with the source code of QMidiPlayer.
   type `qmake` and then `make` to build it. Appending `PREFIX=/usr` to `qmake`
   is highly recommended because QMidiPlayer only scans for plugins in 
   working directory and /usr/lib/qmidiplayer at this moment.
8. Use `sudo make install` to install QMidiPlayer.
