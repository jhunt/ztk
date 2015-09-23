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
#include <string.h>
#include <getopt.h>

static void s_ztk_configure_bind(ztk_config_t *ztk, const char *peer)
{
	ztk_peer_t *z = vmalloc(sizeof(ztk_peer_t));
	z->address = strdup(peer);
	list_push(&ztk->peers, &z->all);
	list_push(&ztk->binds, &z->l);
}

static void s_ztk_configure_connect(ztk_config_t *ztk, const char *peer)
{
	ztk_peer_t *z = vmalloc(sizeof(ztk_peer_t));
	z->address = strdup(peer);
	list_push(&ztk->peers, &z->all);
	list_push(&ztk->connects, &z->l);
}

static void s_ztk_configure_sockopt(ztk_config_t *ztk, int name, const void *data, size_t len)
{
	ztk_sockopt_t *o = vmalloc(sizeof(ztk_sockopt_t));
	o->name  = name;
	o->value = vmalloc(len + 1);
	o->len   = len;
	memcpy(o->value, data, len);
	list_push(&ztk->sockopts, &o->l);
}

ztk_config_t* ztk_configure(int argc, char **argv)
{
	ztk_config_t *ztk = vmalloc(sizeof(ztk_config_t));

	list_init(&ztk->peers);
	list_init(&ztk->binds);
	list_init(&ztk->connects);
	list_init(&ztk->sockopts);

	ztk->zmq = zmq_ctx_new();

	ztk->input_delim = ztk->output_delim = '|';
	ztk->input = ztk->output = FORMAT_DELIM;

	struct option long_opts[] = {
		{ "help",                   no_argument, NULL,    'h' },
		{ "verbose",                no_argument, NULL,    'v' },
		{ "quiet",                  no_argument, NULL,    'q' },

		{ "input",            required_argument, NULL,    'i' },
		{ "output",           required_argument, NULL,    'o' },
		{ "delimiter",        required_argument, NULL,    'd' },

		{ "bind",             required_argument, NULL,    'l' },
		{ "connect",          required_argument, NULL,    'c' },
		{ "subscribe",        required_argument, NULL,    'S' },
		{ "identity",         required_argument, NULL,    'I' },
		{ "rate",             required_argument, NULL,    'r' },
		{ "ipv6",                   no_argument, NULL,    '6' },
		{ "ipv4",                   no_argument, NULL,    '4' },

		{ "send-hwm",         required_argument, NULL, '\001' },
		{ "recv-hwm",         required_argument, NULL, '\002' },
		{ "recovery",         required_argument, NULL, '\003' },
		{ "send-buffer",      required_argument, NULL, '\004' },
		{ "recv-buffer",      required_argument, NULL, '\005' },
		{ "linger",           required_argument, NULL, '\006' },
		{ "reconnect",        required_argument, NULL, '\007' },

		{ "backlog",          required_argument, NULL, '\010' },
		{ "max-message-size", required_argument, NULL, '\011' },
		{ "multicast-hops",   required_argument, NULL, '\012' },
		{ "send-timeout",     required_argument, NULL, '\013' },
		{ "recv-timeout",     required_argument, NULL, '\014' },
		{ "tcp-keepalive",    required_argument, NULL, '\015' },

		{ "input-delimiter",  required_argument, NULL, '\020' },
		{ "output-delimiter", required_argument, NULL, '\021' },
		{ 0, 0, 0, 0 },
	};
	const char *short_opts = "hvqi:o:d:l:c:S:I:r:64";

	union {
		int     i;
		int64_t i64;
	} v;

	for (;;) {
		int idx = 1;
		int c = getopt_long(argc, argv, short_opts, long_opts, &idx);
		if (c == -1) break;

		switch (c) {
		case 'h':
		case '?':
			break;

		case 'v': /* --verbose */
			break;

		case 'q': /* --quiet */
			break;

		case 'i': /* --input */
			if (strcasecmp(optarg, "yaml") == 0 || strcasecmp(optarg, "yml") == 0)
				ztk->input = FORMAT_YAML;
			else if (strcasecmp(optarg, "json") == 0)
				ztk->input = FORMAT_JSON;
			else
				ztk->input = FORMAT_DELIM;
			break;

		case 'o': /* --output */
			if (strcasecmp(optarg, "yaml") == 0 || strcasecmp(optarg, "yml") == 0)
				ztk->input = FORMAT_YAML;
			else if (strcasecmp(optarg, "json") == 0)
				ztk->input = FORMAT_JSON;
			else
				ztk->input = FORMAT_DELIM;
			break;

		case 'd': /* --delimiter */
			ztk->input_delim = ztk->output_delim = optarg[0];
			break;

		case 'l': /* --bind */
			s_ztk_configure_bind(ztk, optarg);
			break;

		case 'c': /* --connect */
			s_ztk_configure_connect(ztk, optarg);
			break;

		case 'S': /* --subscribe */
			s_ztk_configure_sockopt(ztk, ZMQ_SUBSCRIBE, optarg, strlen(optarg));
			break;

		case 'I': /* --identity */
			s_ztk_configure_sockopt(ztk, ZMQ_IDENTITY, optarg, strlen(optarg));
			break;

		case 'r': /* --rate */
			v.i = atoi(optarg);
			s_ztk_configure_sockopt(ztk, ZMQ_RATE, &v.i, sizeof(v.i));
			break;

		case '6': /* --ipv6 */
			v.i = 1;
			s_ztk_configure_sockopt(ztk, ZMQ_IPV6, &v.i, sizeof(v.i));
			break;

		case '4': /* --ipv4 */
			v.i = 0;
			s_ztk_configure_sockopt(ztk, ZMQ_IPV6, &v.i, sizeof(v.i));
			break;

		case '\001': /* --send-hwm */
			v.i = atoi(optarg);
			s_ztk_configure_sockopt(ztk, ZMQ_SNDHWM, &v.i, sizeof(v.i));
			break;

		case '\002': /* --recv-hwm */
			v.i = atoi(optarg);
			s_ztk_configure_sockopt(ztk, ZMQ_RCVHWM, &v.i, sizeof(v.i));
			break;

		case '\003': /* --recovery */
			v.i = atoi(optarg);
			s_ztk_configure_sockopt(ztk, ZMQ_RECOVERY_IVL, &v.i, sizeof(v.i));
			break;

		case '\004': /* --send-buffer */
			v.i = atoi(optarg);
			s_ztk_configure_sockopt(ztk, ZMQ_SNDBUF, &v.i, sizeof(v.i));
			break;

		case '\005': /* --recv-buffer */
			v.i = atoi(optarg);
			s_ztk_configure_sockopt(ztk, ZMQ_RCVBUF, &v.i, sizeof(v.i));
			break;

		case '\006': /* --linger */
			v.i = atoi(optarg);
			s_ztk_configure_sockopt(ztk, ZMQ_LINGER, &v.i, sizeof(v.i));
			break;

		case '\007': /* --reconnect */
			/* FIXME: handle range x:y */
			v.i = atoi(optarg);
			s_ztk_configure_sockopt(ztk, ZMQ_RECONNECT_IVL, &v.i, sizeof(v.i));
			break;

		case '\010': /* --backlog */
			v.i = atoi(optarg);
			s_ztk_configure_sockopt(ztk, ZMQ_BACKLOG, &v.i, sizeof(v.i));
			break;

		case '\011': /* --max-message-size */
			v.i64 = strtoul(optarg, NULL, 0);
			s_ztk_configure_sockopt(ztk, ZMQ_MAXMSGSIZE, &v.i64, sizeof(v.i64));
			break;

		case '\012': /* --multicast-hops */
			v.i = atoi(optarg);
			s_ztk_configure_sockopt(ztk, ZMQ_MULTICAST_HOPS, &v.i, sizeof(v.i));
			break;

		case '\013': /* --send-timeout */
			v.i = atoi(optarg);
			s_ztk_configure_sockopt(ztk, ZMQ_SNDTIMEO, &v.i, sizeof(v.i));
			break;

		case '\014': /* --recv-timeout */
			v.i = atoi(optarg);
			s_ztk_configure_sockopt(ztk, ZMQ_RCVTIMEO, &v.i, sizeof(v.i));
			break;

		case '\015': /* --tcp-keepalive */
			v.i = atoi(optarg);
			s_ztk_configure_sockopt(ztk, ZMQ_TCP_KEEPALIVE, &v.i, sizeof(v.i));
			break;

		case '\020': /* --input-delimiter */
			ztk->input_delim = optarg[0];
			break;

		case '\021': /* --output-delimiter */
			ztk->output_delim = optarg[0];
			break;
		}
	}

	return ztk;
}

void s_set_sockopts(ztk_config_t *ztk, ztk_peer_t *e, int type, int after)
{
	int subd = 0;
	ztk_sockopt_t *opt;
	for_each_object(opt, &ztk->sockopts, l) {

		if (type == ZMQ_SUB) {
			if (opt->name == ZMQ_SUBSCRIBE && !after)
				continue; /* too soon */
			if (opt->name != ZMQ_SUBSCRIBE && after)
				continue; /* too late */
		}

		if (opt->name == ZMQ_SUBSCRIBE)
			subd = 1;

		fprintf(stderr, ">> setting %02x option\n", opt->name);
		zmq_setsockopt(e->socket, opt->name, opt->value, opt->len);
	}

	if (type == ZMQ_SUB && !subd && after) {
		fprintf(stderr, ">> setting catch-all ZMQ_SUBSCRIBE ('')\n");
		zmq_setsockopt(e->socket, ZMQ_SUBSCRIBE, "", 0);
	}
}

int ztk_bind(ztk_config_t *ztk, ztk_peer_t *e, int type)
{
	e->socket = zmq_socket(ztk->zmq, type);
	s_set_sockopts(ztk, e, type, 0);
	return zmq_bind(e->socket, e->address);
}

int ztk_connect(ztk_config_t *ztk, ztk_peer_t *e, int type)
{
	e->socket = zmq_socket(ztk->zmq, type);
	s_set_sockopts(ztk, e, type, 0);

	int rc = zmq_connect(e->socket, e->address);
	if (rc != 0) return rc;

	s_set_sockopts(ztk, e, type, 1);
	return 0;
}

int ztk_poll(ztk_config_t *ztk, long timeout)
{
	if (!ztk->poll.items) {
		int n = list_len(&ztk->peers);
		ztk->poll.items = vcalloc(n, sizeof(zmq_pollitem_t));
		ztk->poll.n     = n;

		ztk_peer_t *e;
		for_each_peer(e, ztk) {
			n--;
			if (n < 0)
				return -1;
			ztk->poll.items[n].socket  = e->socket;
			ztk->poll.items[n].fd      = -1;
			ztk->poll.items[n].events  = ZMQ_POLLIN;
			ztk->poll.items[n].revents = 0;
		}
	}

	return zmq_poll(ztk->poll.items, ztk->poll.n, timeout);
}

ztk_peer_t *ztk_next(ztk_config_t *ztk, int events)
{
	if (!ztk->poll.items)
		return NULL;

	int i;
	for (i = 0; i < ztk->poll.n; i++) {
		if ((ztk->poll.items[i].revents & events) == events) {
			ztk->poll.items[i].revents = 0;

			ztk_peer_t *e;
			for_each_peer(e, ztk) {
				if (e->socket == ztk->poll.items[i].socket)
					return e;
			}

			return NULL;
		}
	}

	return NULL;
}
