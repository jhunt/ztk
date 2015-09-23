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

int ztk_pull(int argc, char **argv)
{
	ztk_config_t *ztk = ztk_configure(argc, argv);

	if (( list_isempty(&ztk->binds) &&  list_isempty(&ztk->connects))
	 || (!list_isempty(&ztk->binds) && !list_isempty(&ztk->connects))) {
		fprintf(stderr, "zpull: you must specify either --bind or --connect option(s)\n");
		exit(1);
	}

	int rc;
	ztk_peer_t *e;

	int n;
	for_each_object(e, &ztk->binds, l) {
		n++;
		rc = ztk_bind(ztk, e, ZMQ_PULL);
		assert(rc == 0);
	}
	for_each_object(e, &ztk->connects, l) {
		n++;
		rc = ztk_connect(ztk, e, ZMQ_PULL);
		assert(rc == 0);
	}

	/* recv */
	signal_handlers();
	while (!signalled()) {
		if (ztk_poll(ztk, -1) < 0)
			continue;

		ztk_peer_t *e;
		while ((e = ztk_next(ztk, ZMQ_POLLIN)) != NULL) {
			pdu_t *pdu = pdu_recv(e->socket);
			fprintf(stdout, "%s", pdu_type(pdu));
			char *frame;
			int i = 1;
			while ((frame = pdu_string(pdu, i++)) != NULL) {
				fprintf(stdout, "|%s", frame);
				free(frame);
			}
			fprintf(stdout, "\n");
			fflush(stdout);
		}
	}

	for_each_peer(e, ztk) {
		zmq_close(e->socket);
	}
	zmq_ctx_destroy(ztk->zmq);

	return 0;
}
