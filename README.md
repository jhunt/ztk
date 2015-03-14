ztk - ZeroMQ Toolkit
====================

**ztk** is a simple toolkit for interfacing with the excellent 0MQ
message transport library.  It provides a set of small, sharp and
simple tools to bind and connect to sockets of all types.

(This is very much a work in progress.)

Some examples:

    # run a broadcast server on TCP/7777
    (while true; do echo "PDU|$(date)|.eof."; sleep 1; done) | ./zpub --bind tcp://*:7777

    # then, run a subscriber
    ./zsub --connect tcp://127.0.0.1:7777

I have plans for a utility binary for each of the socket types
(DEALER, ROUTER, PUSH, PULL, etc.) and hope to support
input/output formats like YAML, JSON etc.

This software is so alpha right now, that other alpha software
gets angry when I call it alpha software.

You've been warned.  :)

Copyright
---------

ztk is Copyright 2015 James Hunt <james@jameshunt.us>

Licensing
---------

ztk is free software: you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation, either version 3 of the License, or (at your
option) any later version.

ztk is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

Building ztk
------------

    $ ./bootstrap
    $ ./configure
    $ make

