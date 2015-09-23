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

int ztk_pub(int argc, char **argv)
{
	ztk_config_t *ztk = ztk_configure(argc, argv);

	if (( list_isempty(&ztk->binds) &&  list_isempty(&ztk->connects))
	 || (!list_isempty(&ztk->binds) && !list_isempty(&ztk->connects))) {
		fprintf(stderr, "%s: you must specify either --bind or --connect option(s)\n", argv[0]);
		return 1;
	}

	int rc;
	ztk_peer_t *e;
	for_each_bind(e, ztk) {
		rc = ztk_bind(ztk, e, ZMQ_PUB);
		if (rc != 0) {
			fprintf(stderr, "%s: bind to %s failed: %s\n", argv[0], e->address, zmq_strerror(errno));
			return 2;
		}
	}
	for_each_connect(e, ztk) {
		rc = ztk_connect(ztk, e, ZMQ_PUB);
		if (rc != 0) {
			fprintf(stderr, "%s: connect to %s failed: %s\n", argv[0], e->address, zmq_strerror(errno));
			return 2;
		}
	}

	char buf[8192];
	while (fgets(buf, 8192, stdin) != NULL) {
		size_t l;
		char *a, *b, c;
		a = b = buf;

		while (*b) {
			a = b;
			while (*b && *b != ztk->input_delim && *b != '\n') b++;
			l = b - a; c = *b; *b++ = '\0';

			for_each_peer(e, ztk) {
				zmq_send(e->socket, a, l, c == '\n' ? 0 : ZMQ_SNDMORE);
			}
		}
	}

	for_each_peer(e, ztk) {
		zmq_close(e->socket);
	}
	zmq_ctx_destroy(ztk->zmq);

	return 0;
}
