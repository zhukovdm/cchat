# cchat

`cchat` is a terminal-based network application for instant messaging written
in `C/C++`.

# Dependencies

The project employs `C++` standard library, POSIX sockets and `ncurses.h`
library for GUI running on a client implying the source code is intentionally
**platform dependent**.

Application has been tested on a machine with installed `linux kernel 5.15.0-27-generic`,
`g++ 11.2.0`, `ncurses 6.3-2` and `doxygen 1.9.1`.

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

The previous commands should be entered in order. If client starts first,
it fails on unsuccessful **connect** and terminates. All argument parameters
are verified, malformed parameters are reported causing program to stop.

`cchat-*` executables are copied to the `/usr/bin/` folder upon
`make install`, but `root` permissions are required. Once installed,
programs are available in the `$PATH`.

# Documentation

[Users' Guide](./docs/USER.md) with typical use cases and
[Programmers' Manual](./docs/PROG.md) with technical details can be found
in `docs/` folder. **Doxymentation** is generated upon running `make docs`,
then see `docs/doxygen/html/index.html`.
