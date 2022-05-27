# cchat

`cchat` is a terminal-based network application for instant messaging written
in `C/C++`.

# Dependencies

The project employs `C++` standard library, BSD sockets and `ncurses.h`
library for client GUI implying the source code is intentionally
**platform dependent**. The application has been tested on a machine with:

- `xubuntu-22.04`,
- `linux kernel 5.15.0-27-generic`,
- `g++ 11.2.0`,
- `ncurses 6.3-2`,
- `doxygen 1.9.1`.

# Build and run

Enter `make`, to build both `client` and `server`. Executables with prefix
`cchat-*` are stored in `build/` folder. Installation is not necessary for
running client or server.

```shell
./build/cchat-server --port=12321
```

```shell
./build/cchat-client --name=user --host=127.0.0.1 --port=12321
```

If client starts before server instance, the program fails on unsuccessful
connect and terminates. All argument parameters are verified, malformed
parameters are reported via exception causing program to stop.

Run `make install` to copy `cchat-*` executables into `/usr/bin/` folder.
This makes programs available in the `$PATH`. Note, that copying may require
`root` permissions.

# Documentation

[Users' Guide](./docs/user.md) with typical use cases and
[Programmers' Manual](./docs/prog.md) with technical details can be found
in `docs/` folder.

Run `make docs` to generate doxymentation into `docs/doxygen/` folder,
then consult `docs/doxygen/html/index.html`.
