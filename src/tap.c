/*
  Copyright 2015 James Hunt <james@jameshunt.us>

  This file is part of ztk->

  ztk is free software: you can redistribute it and/or modify it under
  the terms of the GNU General Public License as published by the Free
  Software Foundation, either version 3 of the License, or (at your
  option) any later version.

  ztk is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
  for more details.

  You should have received a copy of the GNU General Public License
  along with ztk->  If not, see <http://www.gnu.org/licenses/>.
 */

#include "ztk.h"

#define INGRESS 1
#define EGRESS  2

static uint64_t T0 = 0;

static void s_ztk_debugf(pdu_t *pdu, FILE *io, uint64_t ts, int dir)
{
	char *frame;
	int i = 1;

	if (T0 > 0) {
		fprintf(io, "%c(%+8.3lf)%c %i:[ ",
				(dir == INGRESS ? '<' : ' '),
				(ts - T0) / 1000.0,
				(dir ==  EGRESS ? '>' : ' '),
				pdu_size(pdu));
	} else {
		fprintf(io, "%c(% 12.3lf)%c %i:[ ",
				(dir == INGRESS ? '<' : ' '),
				ts / 1000.0,
				(dir ==  EGRESS ? '>' : ' '),
				pdu_size(pdu));
	}

	fprintf(io, "%s", pdu_type(pdu));
	while ((frame = pdu_string(pdu, i++)) != NULL) {
		fprintf(io, " | %s", frame);
		free(frame);
	}
	fprintf(io, " ]\n");
	fflush(io);
}

int ztk_tap(int argc, char **argv)
{
	ZTK *ztk = ztk_configure("tap", argc, argv);

	if (ztk->argc <= 0) {
		fprintf(stderr, "%s: you must specify an architecture (pub-sub, pull-push, etc.)\n", argv[0]);
		return 1;
	}

	if (ztk->relative_ts) {
		T0 = time_ms();
	}

	int bind_sock;
	int conn_sock;
	list_t *egress;

	#define disposition(b,c,e) do { \
		bind_sock = (b);     \
		conn_sock = (c);     \
		egress    = &ztk->e; \
	} while (0)

	if (strcmp(ztk->argv[0], "push-pull") == 0) {
		disposition(ZMQ_PUSH, ZMQ_PULL, binds);

	} else if (strcmp(ztk->argv[0], "pull-push") == 0) {
		disposition(ZMQ_PULL, ZMQ_PUSH, connects);

	} else if (strcmp(ztk->argv[0], "pub-sub") == 0) {
		disposition(ZMQ_PUB, ZMQ_SUB, binds);

	} else if (strcmp(ztk->argv[0], "sub-pub") == 0) {
		disposition(ZMQ_SUB, ZMQ_PUB, connects);

	} else {
		fprintf(stderr, "%s: invalid communication architecture '%s'\n", argv[0], ztk->argv[0]);
		return 1;
	}

	if (list_isempty(&ztk->binds) && list_isempty(&ztk->connects)) {
		fprintf(stderr, "%s: you must specify both --bind and --connect option(s)\n", argv[0]);
		return 1;
	}

	ztk_peer_t *e;
	for_each_bind(e, ztk) {
		if (ztk_bind(ztk, e, bind_sock) != 0) {
			fprintf(stderr, "bind of %s failed: %s\n", e->address, zmq_strerror(errno));
			return 2;
		}
	}
	for_each_connect(e, ztk) {
		if (ztk_connect(ztk, e, conn_sock) != 0) {
			fprintf(stderr, "connect of %s failed: %s\n", e->address, zmq_strerror(errno));
			return 2;
		}
	}

	signal_handlers();
	while (!signalled()) {
		if (ztk_poll(ztk) < 0)
			continue;

		while ((e = ztk_next(ztk, ZMQ_POLLIN)) != NULL) {
			pdu_t *pdu = pdu_recv(e->socket);
			s_ztk_debugf(pdu, stdout, time_ms(), INGRESS);

			for_each_object(e, egress, l) {
				pdu_send_and_free(pdu_dup(pdu, NULL), e->socket);
				s_ztk_debugf(pdu, stdout, time_ms(), EGRESS);
			}

			pdu_free(pdu);
		}
	}

	return ztk_shutdown(ztk);
}
