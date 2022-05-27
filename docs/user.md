# Users' Guide

- [Introduction](#introduction)
- [Definitions](#definitions)
- [Server](#server)
- [Client](#client)
  - [Log in](#log-in)
  - [Command](#command)
  - [Chat](#chat)

# Introduction

This document defines important terms used throughout the project and shows
typical use cases of both `client` and `server` from user perspective.

# Definitions

`user name` is any non-empty case-sensitive sequence of alphanumeric
characters.

`command` is an interpretable sequence of characters issued by the user
while being in `command` state.

`end-of-chat sequence` is the string `<$>`.

`text message` is any sequence of 1-byte long characters, which is not equal
to `end-of-chat` sequence issued by the user while being in `chat` state.

`pending message` is a message sent by **u1** to **u2** while being in
`chat` state. The message had reached the server and had been stored. Target
**u2** has not visited chat with **u1** yet.

`history message` is a message sent by **u1** to **u2**. The message had
reached target **u2** in `chat` state and had been stored in the server
archive structure.

# Server

The `cchat-server` program accepts incoming client connections and create
specific worker to handle it asynchronously. Important events, such as open
and closed connections or chats, are reported to the `stdout`. User cannot
control `cchat-server` from outside.

Possible server output:

```console
Server listens at socket 3, port 12321.
New connection from peer 2.0.165.78 port 42318.
Chat u1 -> u2 started.
Chat u1 -> u2 ended.
Socket 4 done in ClientMode 1.
Closing connection with peer 2.0.165.84 port 42324.
```

# Client

In this chapter, we discuss interaction between the user and `cchat-client`
program. If the server stops, all `history` and `pending` messages are lost.

## Log in

Freshly started client tries to log in the server with information, provided
in arguments. Malformed arguments or inacessible server are reported and
program terminates. If user is already online, login attempt will be rejected
by the server.

After the client has established connection with a server, graphical interface
appears in the terminal with the following layout.

```console
> _
cchat as user
```

## Command

`server` is waiting for `client` input and `client` is waiting for user input.
The following set of commands is available.

- `help` shows all available commands with short description.
- `pend` requests list of users with **pending messages** sent to the current user.
- `hist # user` requests up to **10** last **history messages** with particular `user`.
- `chat user` initiates chat with a (even non-existent) `user`. All sent messages
  are stored in a storage with `pending` messages and stored in a history once
  delivered to the target `user`. Once issued, both client and server sessions
  proceed to the `chat` state.
- `quit` exits the program, connection is closed on both sides and resources are
  released.

The example of communication could look as follows. Consider **u1** sends
`text message` "Hi!" to **u2** and **u2** reads them later. Messages without
`[user]` decoration shall be understood as commands with special semantics.

```console
> 
cchat as u1
<$>
[u1] Hi!
chat u2
```

```console
> 
cchat as u2
[u1] Hi!
hist 5 u1
<$>
[u1] Hi!
chat u1
hist 5 u1
u1
pend
```

## Chat

`chat` mode is treated differently than two previous modes. Consider a chat
from **u1** towards **u2**.

Server sends any message appeared in `pending messages` from **u2** towards **u1**.
Client sends any message coming from user interface to the server. Messages could
appear at any time, asynchronous message passing happens in both directions.

Once the `end-of-chat sequence` detected, chat mode is exited and the user
could issue commands again.
