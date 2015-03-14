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

int main(int argc, char **argv)
{
	ztk_config_t *ztk = ztk_configure(argc, argv);

	if (list_isempty(&ztk->binds)) {
		fprintf(stderr, "zpub: you must specify at least one --bind option\n");
		exit(1);
	}

	if (!list_isempty(&ztk->connects)) {
		fprintf(stderr, "zpub: --connect option makes no sense\n");
		exit(1);
	}

	int rc;
	ztk_endpoint_t *e;
	for_each_object(e, &ztk->binds, l) {
		rc = ztk_bind(ztk, e, ZMQ_PUB);
		assert(rc == 0);
		fprintf(stderr, "bound to %s\n", e->address);
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

			for_each_object(e, &ztk->binds, l) {
				zmq_send(e->socket, a, l, c == '\n' ? 0 : ZMQ_SNDMORE);
			}
		}
	}

	for_each_object(e, &ztk->binds, l) {
		vzmq_shutdown(e->socket, 0);
	}
	zmq_ctx_destroy(ztk->zmq);

	return 0;
}
