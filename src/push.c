/*
  Copyright 2015 James Hunt <james@jameshunt.us>

  This file is part of ztk.

  ztk is free software: you can redistribute it and/or modify it under
  the terms of the GNU General Public License as published by the Free
  Software Foundation, either version 3 of the License, or (at your
  option) any later version.

  ztk is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
  for more details.

  You should have received a copy of the GNU General Public License
  along with ztk.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "ztk.h"

int ztk_push(int argc, char **argv)
{
	ztk_config_t *ztk = ztk_configure(argc, argv);

	if (( list_isempty(&ztk->binds) &&  list_isempty(&ztk->connects))
	 || (!list_isempty(&ztk->binds) && !list_isempty(&ztk->connects))) {
		fprintf(stderr, "zpush: you must specify either --bind or --connect option(s)\n");
		exit(1);
	}

	int rc;
	ztk_peer_t *e;

	for_each_object(e, &ztk->binds, l) {
		rc = ztk_bind(ztk, e, ZMQ_PUSH);
		assert(rc == 0);
	}
	for_each_object(e, &ztk->connects, l) {
		rc = ztk_connect(ztk, e, ZMQ_PUSH);
		assert(rc == 0);
	}

	char buf[8192];
	while (fgets(buf, 8192, stdin) != NULL) {
		size_t l;
		int more;
		char *a, *b;
		a = b = buf;

		for_each_peer(e, ztk) {
			zmq_send(e->socket, "", 0, ZMQ_SNDMORE);
		}

		while (*b) {
			a = b;
			while (*b && *b != ztk->input_delim && *b != '\n') b++;
			more = *b == '\n' ? 0 : ZMQ_SNDMORE;
			l = b - a; *b++ = '\0';

			for_each_peer(e, ztk) {
				zmq_send(e->socket, a, l, more);
			}
		}
	}

	for_each_peer(e, ztk) {
		zmq_close(e->socket);
	}
	zmq_ctx_destroy(ztk->zmq);

	return 0;
}
