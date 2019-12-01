The original project doesn't come with a readme.

The code has been slightly tweaked to add support for amd64 and to obtain a
more readable output format.

Tested with mxe (gcc 5.5.0, binutils 2.28). Obviously you need a shared build.
mxe disables shared versions of bfd and iberty, but actually they work just
fine. Simply remove this line
```
$(PKG)_BUILD_SHARED =
```
from their makefiles and change the options for `./configure`
(`--enable-static` to `--disable-static`, `--disable-shared` to
`--enable-shared`).

[Source project](https://github.com/cloudwu/backtrace-mingw)

References

[Printing a Stack Trace with MinGW](http://theorangeduck.com/page/printing-stack-trace-mingw)
