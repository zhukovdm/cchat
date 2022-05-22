# Programmers' Manual

- [Programmers' Manual](#programmers-manual)
- [Introduction](#introduction)
- [Definitions](#definitions)
- [Architecture](#architecture)
- [Thread-safety](#thread-safety)
- [Network protocol](#network-protocol)
- [User interface](#user-interface)
- [Session state](#session-state)
  - [Log in](#log-in)
  - [Command](#command)
  - [Chat](#chat)
- [Future](#future)
- [References](#references)

# Introduction

`cchat` is a terminal-based network application for instant messaging.
It implements classical client-server architecture. All messages are passed
through the server and later distributed to the target users.

More on architecture and technical details are in the following chapters.

# Definitions

**Network interface** is a point of interconnection between a computer
and a private or public network. Such point could be a network interface
controller (physical card) or could have a purely software nature (loopback).

**IP address** is a network interface identifier, 32-bit long for **IPv4** and
128-bit long for **IPv6**. Currently, `cchat` supports only **IPv4**.

**Port number** is a 16-bit (1...65535) local connection identifier.
0 is a symbol for any currently available port.

**Socket** is a special system entity intended to make possible communication.
Sockets are identified by per-process file descriptors. `cchat` uses reliable
TCP sockets to avoid message losses.

**Network byte order** is a byte order, in which word bytes are transmitted
over network communication. Standard TCP/IP byte order is big-endian. Consider
the 32-bit word `0x12345678` starting at address `a0`. Big-endian means mapping
`{ (12, a0), (34, a1), (56, a2), (78, a3) }` and little-endian means mapping
`{ (78, a0), (56, a1), (34, a2), (12, a3) }`.

**Host byte order** is a byte order, in which word bytes are stored on
a particular computer or server. It could be either little-endian or big-endian.
Therefore, words transmitted over the network shall be decorated by `htons()`,
`htonl()`, `ntohs()`, `ntohl()` function calls from `arpa/inet.h` header.

**Packet** has a header and a body, see [Network protocol](#network-protocol)
for more details.

**End-of-sequence symbol** is the char `$`.

# Architecture

The application is built on top of the following abstractions.

Both `Client` and `Server` are network `Entities`. `Server` could potentially
handle more than one connection. All new connections (sockets) on both client
and server are recognized as a `Session` with internal state.

`Server` is a passive network entity accepting incoming TCP/IP connections
on all interfaces (not safe in general). We understand accepting in terms of
the system call [accept](https://man7.org/linux/man-pages/man2/accept.2.html).
Special worker (thread) is created for each accepted connection to handle it
asynchronously. The server responds on client requests and never initiates
communication.

`Client` is an acitive network entity connecting servers available in the
network. `client` contains `ClientSession` and `Gui`.

`Session`, either `ClientSession` or `ServerSession` is a part of network
entity, maintains proper two-sided connection and interprets text messages
based on the current state (with or without specific semantics). Send and
receive may fail at any time as described in the next paragraph.

Instances of `Connect`, either `RecvConnect` or `SendConnect`, are created
on demand wnenever messages are expected to be sent or received. Communication
may fail and this is indicated by names of the messages, e.g.
`recv_maybe_body()`. Send returns boolean and receive returns `std::optional`.
Upon fail, nothing is sent or received.

Several thread-safe supporting structures were implemented for different
project needs, refer to [Thread safety](#thread-safety).

# Thread-safety

The project implements a bunch of thread-safe containers for generic types.
All containers use `std::mutex` and `std::lock_guard` for synchronization.

`Logger` maintains synchronized access to a general stream dumping received
messages in batches.

`ValueStorage` could carry a value of any copyable and/or movable type with
possibility to `load()` a copy or `store()` new value.

`VectorStorage` is a synchronized `std::vector` with several specific methods.

`DequeStorage` is a synchronized `std::deque` with several specific methods.

`MapStorage` is a synchronized `std::map` with several specific methods.

`User` allows atomically acquire and release user online.

# Network protocol

`cchat` implements simple stateful text-based protocol for message passing.
A packet consists of a header (length of a body in bytes) followed by a textual
body. Body could be interpreted as a plain text or a command with special
semantics depending on a session state. This is a subject of the following
chapters.

POSIX sockets and specific system calls are exclusively used throughout the
project.

# User interface

Only `Client` implements terminal-based user interface. `Gui` runs on a worker
thread and communicate with the main `ClientSession` via thread-safe storages.

# Session state

Session state is a special identifier kept in sync on both client and server
sides. We recognize 3 states: `log in`, `command`, `chat`. All new sessions
start in `log in` state. State prescribes the behavior of both client and
server. `log in` and `command` enforce server to wait for a packet from
a client and generate response. `chat` state is different, both client
and server can send messages asynchronously and simultaneously.

In the following subsections, we discuss each state in greater details.

## Log in

Here we describe what happen, when server is waiting for new connection and
client decides to connect it. It all starts in user's terminal. The user
enters login `name`, `host` address and `port`. Client-side application tries
to connect remote point. We further consider that TCP/IP connection is
established. Both client and server go to `log in` state.

The server waits until accepted user sends a desired **user name**. The client
sends `name` provided in the arguments. The server receives `name`, verifies it,
tries to acquire user entity (make user online). If all checks are passed
successfully, the server respond with the same `name` meaning client and server
can proceed to the `command` state. Otherwise, it sends modified `name` and
both sides terminate session.

## Command

The similar approach as [Log in](#log-in) is employed in the command state.
Server waits for the command, interprets it, maybe generates response and
maybe changes state.

`help` is processed by the client locally, no message passing happens between
client and server.

`quit` received from the client immediately terminates connection and
**no response** is generated.

`chat user` is received by the server. Server extracts `user` from the message
and sends it back to the user. After that, `chat` state is entered on both
sides.

`pend` and `hist # user` enforces server to prepare a sequence of messages
to be sent. Sequence is terminated by the **end-of-sequence symbol**.

## Chat

Both entities have entered this state. Send and receive parts are better
processed asynchronously. Therefore, one more thread is created on both
sides. Communication run until **end-of-chat sequence** is released by the
user. This sets `done` bit to `true` and enforces threads to stop and join.
State is changed back to `command`.

# Future

The following extensions are planned to be done in the future.

- Extensive unit tests with client and server mocking.
- Broken connection detection on both client and server.
- Server stores messages in the database.
- User password authentication.
- Support for cyphered communication.

# References

- [What Is a Network Interface?](https://docs.oracle.com/javase/tutorial/networking/nifs/definition.html)
- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/html/)
- [Concurrent Servers: Parts 1-6](https://eli.thegreenplace.net/2017/concurrent-servers-part-1-introduction/)
- [socket connect() vs bind()](https://stackoverflow.com/questions/27014955/socket-connect-vs-bind)
