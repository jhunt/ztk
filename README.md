ztk - ZeroMQ Toolkit
====================

**ztk** is a simple toolkit for interfacing with the excellent ZeroMQ
message transport library.  It provides a set of small, sharp and
simple tools to bind and connect to sockets of all types.

(This is very much a work in progress.)

Some examples:

    # run a broadcast server on TCP/7777
    (while true; do echo "PDU|$(date)|.eof."; sleep 1; done) | zpub --bind tcp://*:7777

    # then, run a subscriber
    zsub --connect tcp://127.0.0.1:7777



    # test a simple REP server at ipc:///var/run/rep.socket
    zreq --connect ipc:////var/run/rep.socket
    REQUEST
    ... reply from server ...


    # run some workers in a pipeline:
    for port in $(seq 3001 3004); do
      zpull --bind tcp://*:$port | /some/processing/script &
    done

    # and then distribute some work to them
    /some/input/script | zpush --connect tcp://127.0.0.1:3001 \
                               --connect tcp://127.0.0.1:3002 \
                               --connect tcp://127.0.0.1:3003 \
                               --connect tcp://127.0.0.1:3004

This software is so alpha right now, that other alpha software
gets angry when I call it alpha software.

You've been warned.  :)

Use It For Testing ZeroMQ-based Applications
--------------------------------------------

While the original goal of this software was to enable shell
scripts to implement communication patterns using ZeroMQ (without
having to resort to other languages), it soon found a place as in
running tests for ZeroMQ-based applications.

Here's an example that uses a DEALER socket to communicate to a
black box application:

    #!/bin/bash

    ROOT=$(mktemp -d bbtest.XXXXXX)
    trap "rm -rf ${ROOT}" QUIT TERM INT

    ENDPOINT="ipc://${ROOT}/socket"
    ./blackboxd --foreground --bind ${ENDPOINT} &
    DAEMON_PID=$!

    RESPONSE=$(echo 'QUERY|answer to life/universe/everything?' | \
                 ztk dealer --connect ${ENDPOINT})
    if [[ "${RESPONSE:-(empty)}" != "ANSWER|42" ]]; then
        echo >&2 "failed to answer the question"
        echo >&2 "   expected: 'ANSWER|42'"
        echo >&2 "        got: '${RESPONSE}'"
        exit 1
    fi

    kill -TERM ${DAEMON_PID}

    exit 0

**NOTE:** When using ztk in this manner, it is _vital_ that you
make use of the `--recv-timeout` and `--send-timeout` options,
which set the `ZMQ_RCVTIMEO` and `ZMQ_SNDTIMEO` socket options.
This way, if the application being tested is misbehaving, your ztk
client won't hang indefinitely waiting for a reply that may never
come.

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
    $ make install

