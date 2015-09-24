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

	if (ztk_sockets(ztk, ZMQ_PUB) != 0)
		return 2;

	ztk_peer_t *e;
	pdu_t *pdu;
	while ((pdu = ztk_pdu(ztk, stdin)))
		for_each_peer(e, ztk)
			pdu_send_and_free(pdu_dup(pdu, NULL), e->socket);

	return ztk_shutdown(ztk);
}
