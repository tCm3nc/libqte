# LibQTE

This is the main runtime library for working with the QTE project.
This library gets `LD_PRELOADED` into the target application that is being 
instrumented by Qemu user mode.

This library is inspired by Andrea Fioraldi's 
[QASAN](https://github.com/andreafioraldi/qasan) project and subsequently uses 
similar structure and code layout to it.

## Compiling

Compiling this library should be as simple as running `make`. 
The dependencies are on having a reasonable version of `clang` installed.
See the `Makefile` for more details.

## Testing

Tests are contained inside the `tests` directory to test simple pieces of 
functionality.

```bash
cd tests
make
LD_PRELOAD=../libqte.so ./double_free
```

## Credits

1 - Andrea Fioraldi's QASAN project.