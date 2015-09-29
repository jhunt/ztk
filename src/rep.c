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

int ztk_rep(int argc, char **argv)
{
	ZTK *ztk = ztk_configure("rep", argc, argv);

	if (( list_isempty(&ztk->binds) &&  list_isempty(&ztk->connects))
	 || (!list_isempty(&ztk->binds) && !list_isempty(&ztk->connects))) {
		fprintf(stderr, "%s: you must specify either --bind or --connect option(s)\n", argv[0]);
		return 1;
	}

	if (ztk_sockets(ztk, ZMQ_REP) != 0)
		return 2;

	/* recv */
	signal_handlers();
	while (!signalled()) {
		if (ztk_poll(ztk) < 0)
			continue;

		ztk_peer_t *e;
		while ((e = ztk_next(ztk, ZMQ_POLLIN)) != NULL) {
			pdu_t *q = pdu_recv(e->socket);
			ztk_print(ztk, q, stdout);

			pdu_t *a = ztk_pdu(ztk, stdin);
			if (a)
				pdu_send_and_free(a, e->socket);
		}
	}

	return ztk_shutdown(ztk);
}
