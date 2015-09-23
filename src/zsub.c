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

	if (list_isempty(&ztk->connects)) {
		fprintf(stderr, "zsub: you must specify at least one --connect option\n");
		exit(1);
	}

	if (!list_isempty(&ztk->binds)) {
		fprintf(stderr, "zsub: --bind option makes no sense\n");
		exit(1);
	}

	int rc;
	int n = 0;
	ztk_peer_t *e;
	for_each_object(e, &ztk->connects, l) {
		rc = ztk_connect(ztk, e, ZMQ_SUB);
		assert(rc == 0);

		fprintf(stderr, "connect to %s\n", e->address);
		n++;
	}

	zmq_pollitem_t *zpoll = vcalloc(n, sizeof(zmq_pollitem_t));
	assert(zpoll);

	n = 0;
	for_each_object(e, &ztk->connects, l) {
		zpoll[n].socket = e->socket;
		zpoll[n].events = ZMQ_POLLIN;
		n++;
	}

	char buf[8192];
	while ((rc = zmq_poll(zpoll, n, 200)) >= 0) {
		int i;
		for (i = 0; i < n; i++) {
			if (zpoll[i].revents == ZMQ_POLLIN) {
				int bytes = zmq_recv(zpoll[i].socket, buf, 8192, 0);
				if (bytes > 8191) bytes = 8191;
				buf[bytes] = '\0';

				int more;
				size_t len = sizeof(more);
				zmq_getsockopt(zpoll[i].socket, ZMQ_RCVMORE, &more, &len);

				printf("%s%c", buf, more ? '|' : '\n');
			}
			zpoll[i].revents = 0;
		}
	}

	for_each_object(e, &ztk->binds, l) {
		vzmq_shutdown(e->socket, 500);
	}
	zmq_ctx_destroy(ztk->zmq);

	return 0;
}
