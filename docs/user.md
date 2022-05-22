# Users' Guide

- [Users' Guide](#users-guide)
- [Overview](#overview)
- [Definitions](#definitions)
- [Server](#server)
- [Client](#client)
  - [Log in](#log-in)
  - [Command](#command)
  - [Chat](#chat)

# Overview

This document defines important terms used throughout the project and shows
typical use cases of both `client` and `server` from user perspective.

# Definitions

`user name` is any non-empty case-sensitive sequence of alphanumeric
characters.

`command` is an interpretable sequence of characters issued by the user
while being in `command` state.

`text message` is any sequence of 1-byte long characters, which is not equal
to `end-of-chat` sequence issued by the user while being in `chat` state.

`end-of-chat sequence` is the string `<$>`.

`pending message` is a message sent by **u1** to **u2** while being in
`chat` state. The message had reached the server and had been stored. Target
**u2** has not visited **u2 -> u1** chat yet.

`history message` is a message sent by **u1** to **u2**. The message had
reached target **u2** in `chat` state and had been stored in the archive.

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
program.

## Log in

Freshly started client tries to log in the server with information, provided
in arguments. Malformed arguments or inacessible server are reported and
program terminates. If user is already online, login attempt will be rejected
by the server.

After client has established connection with the server, graphical interface
appear in the terminal with the following layout. It means both client and
server are in `command` state.

```console
> _
cchat as user
```

## Command

`server` is waiting for `client` input and `client` is waiting for user input.
The following set of commands is available.

`help` shows all available commands with short description.

`pend` requests list of users with **pending messages** sent to the current user.

`hist # user` requests up to **10** last **history messages** with particular `user`.

`chat user` initiates chat with a (even non-existent) `user`. All sent messages
are stored in a storage with `pending` messages and stored in a history once
delivered to the target `user`. Once issued, both client and server sessions
proceed to the `chat` state.

`quit` exits the program, connection is closed on both sides and resources are
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

Consider the user **u1** initiated chat **u1 -> u2**. `chat` state is treated
differently than two previous states.

Server sends any message appeared in pending messages from **u2** towards **u1**.
Such messages could appear at any time. Client sends any message comming from
user interface. Asynchronous message passing happens in both directions.

Once the `end-of-chat sequence` detected, both client and server go to `command`
state. Retrieved, but not sent pending messages are stored returned back to
pending storage, so that messages are not lost.
