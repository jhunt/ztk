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

static void s_ztk_configure_bind(ZTK *ztk, const char *peer)
{
	ztk_peer_t *z = vmalloc(sizeof(ztk_peer_t));
	z->address = strdup(peer);
	list_push(&ztk->peers, &z->all);
	list_push(&ztk->binds, &z->l);
}

static void s_ztk_configure_connect(ZTK *ztk, const char *peer)
{
	ztk_peer_t *z = vmalloc(sizeof(ztk_peer_t));
	z->address = strdup(peer);
	list_push(&ztk->peers, &z->all);
	list_push(&ztk->connects, &z->l);
}

static void s_ztk_configure_sockopt(ZTK *ztk, int name, const void *data, size_t len)
{
	ztk_sockopt_t *o = vmalloc(sizeof(ztk_sockopt_t));
	o->name  = name;
	o->value = vmalloc(len + 1);
	o->len   = len;
	memcpy(o->value, data, len);
	list_push(&ztk->sockopts, &o->l);
}

ZTK* ztk_configure(const char *program, int argc, char **argv)
{
	ZTK *ztk = vmalloc(sizeof(ZTK));
	ztk->program = strdup(program);

	list_init(&ztk->peers);
	list_init(&ztk->binds);
	list_init(&ztk->connects);
	list_init(&ztk->sockopts);

	ztk->zmq = zmq_ctx_new();

	ztk->input_delim = ztk->output_delim = '|';
	ztk->input = ztk->output = FORMAT_DELIM;
	ztk->poll.timeout = -1;

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
		{ "poll-timeout",     required_argument, NULL, '\016' },
		{ "timeout",          required_argument, NULL,    't' },

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
			ztk->verbose = 1;
			break;

		case 'q': /* --quiet */
			ztk->verbose = 0;
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

		case '\016': /* --poll-timeout */
			v.i = atoi(optarg);
			ztk->poll.timeout = v.i;
			break;

		case 't': /* --timeout */
			v.i = atoi(optarg);
			s_ztk_configure_sockopt(ztk, ZMQ_LINGER,   &v.i, sizeof(v.i));
			s_ztk_configure_sockopt(ztk, ZMQ_SNDTIMEO, &v.i, sizeof(v.i));
			s_ztk_configure_sockopt(ztk, ZMQ_RCVTIMEO, &v.i, sizeof(v.i));
			ztk->poll.timeout = v.i;
			break;

		case '\020': /* --input-delimiter */
			ztk->input_delim = optarg[0];
			break;

		case '\021': /* --output-delimiter */
			ztk->output_delim = optarg[0];
			break;
		}
	}

	ztk->argc = argc - optind;
	ztk->argv = argv + optind;

	return ztk;
}

static const char *s_socktype(int type)
{
	switch (type) {
	case ZMQ_PUB:    return "PUB";
	case ZMQ_SUB:    return "SUB";
	case ZMQ_PUSH:   return "PUSH";
	case ZMQ_PULL:   return "PULL";
	case ZMQ_REQ:    return "REQ";
	case ZMQ_REP:    return "REP";
	case ZMQ_DEALER: return "DEALER";
	case ZMQ_ROUTER: return "ROUTER";
	default:         return "(UNKNOWN)";
	}
}

static const char *s_sockoptname(int opt)
{
	switch (opt) {
	case  ZMQ_SNDHWM:               return  "ZMQ_SNDHWM";
	case  ZMQ_RCVHWM:               return  "ZMQ_RCVHWM";
	case  ZMQ_AFFINITY:             return  "ZMQ_AFFINITY";
	case  ZMQ_SUBSCRIBE:            return  "ZMQ_SUBSCRIBE";
	case  ZMQ_UNSUBSCRIBE:          return  "ZMQ_UNSUBSCRIBE";
	case  ZMQ_IDENTITY:             return  "ZMQ_IDENTITY";
	case  ZMQ_RATE:                 return  "ZMQ_RATE";
	case  ZMQ_RECOVERY_IVL:         return  "ZMQ_RECOVERY_IVL";
	case  ZMQ_SNDBUF:               return  "ZMQ_SNDBUF";
	case  ZMQ_RCVBUF:               return  "ZMQ_RCVBUF";
	case  ZMQ_LINGER:               return  "ZMQ_LINGER";
	case  ZMQ_RECONNECT_IVL:        return  "ZMQ_RECONNECT_IVL";
	case  ZMQ_RECONNECT_IVL_MAX:    return  "ZMQ_RECONNECT_IVL_MAX";
	case  ZMQ_BACKLOG:              return  "ZMQ_BACKLOG";
	case  ZMQ_MAXMSGSIZE:           return  "ZMQ_MAXMSGSIZE";
	case  ZMQ_MULTICAST_HOPS:       return  "ZMQ_MULTICAST_HOPS";
	case  ZMQ_RCVTIMEO:             return  "ZMQ_RCVTIMEO";
	case  ZMQ_SNDTIMEO:             return  "ZMQ_SNDTIMEO";
	case  ZMQ_IPV6:                 return  "ZMQ_IPV6";
	case  ZMQ_IPV4ONLY:             return  "ZMQ_IPV4ONLY";
	case  ZMQ_IMMEDIATE:            return  "ZMQ_IMMEDIATE";
	case  ZMQ_ROUTER_MANDATORY:     return  "ZMQ_ROUTER_MANDATORY";
	case  ZMQ_ROUTER_RAW:           return  "ZMQ_ROUTER_RAW";
	case  ZMQ_PROBE_ROUTER:         return  "ZMQ_PROBE_ROUTER";
	case  ZMQ_XPUB_VERBOSE:         return  "ZMQ_XPUB_VERBOSE";
	case  ZMQ_REQ_CORRELATE:        return  "ZMQ_REQ_CORRELATE";
	case  ZMQ_REQ_RELAXED:          return  "ZMQ_REQ_RELAXED";
	case  ZMQ_TCP_KEEPALIVE:        return  "ZMQ_TCP_KEEPALIVE";
	case  ZMQ_TCP_KEEPALIVE_IDLE:   return  "ZMQ_TCP_KEEPALIVE_IDLE";
	case  ZMQ_TCP_KEEPALIVE_CNT:    return  "ZMQ_TCP_KEEPALIVE_CNT";
	case  ZMQ_TCP_KEEPALIVE_INTVL:  return  "ZMQ_TCP_KEEPALIVE_INTVL";
	case  ZMQ_TCP_ACCEPT_FILTER:    return  "ZMQ_TCP_ACCEPT_FILTER";
	case  ZMQ_PLAIN_SERVER:         return  "ZMQ_PLAIN_SERVER";
	case  ZMQ_PLAIN_USERNAME:       return  "ZMQ_PLAIN_USERNAME";
	case  ZMQ_PLAIN_PASSWORD:       return  "ZMQ_PLAIN_PASSWORD";
	case  ZMQ_CURVE_SERVER:         return  "ZMQ_CURVE_SERVER";
	case  ZMQ_CURVE_PUBLICKEY:      return  "ZMQ_CURVE_PUBLICKEY";
	case  ZMQ_CURVE_SECRETKEY:      return  "ZMQ_CURVE_SECRETKEY";
	case  ZMQ_CURVE_SERVERKEY:      return  "ZMQ_CURVE_SERVERKEY";
	case  ZMQ_ZAP_DOMAIN:           return  "ZMQ_ZAP_DOMAIN";
	case  ZMQ_CONFLATE:             return  "ZMQ_CONFLATE";
	default: return "(UNKNOWN)";
	}
}

int ztk_shutdown(ZTK *ztk)
{
	ztk_debugf(ztk, "shutting down\n");

	ztk_peer_t *e;
	for_each_peer(e, ztk) {
		ztk_debugf(ztk, "  closing %s socket %s to %s\n", s_socktype(e->type),
				(e->bound ? "bound" : "connected"), e->address);
		zmq_close(e->socket);
	}

	ztk_debugf(ztk, "  destroying 0MQ context\n");
	zmq_ctx_destroy(ztk->zmq);
	return 0;
}

int ztk_sockets(ZTK *ztk, int type)
{
	ztk_peer_t *e;
	for_each_bind(e, ztk) {
		if (ztk_bind(ztk, e, type) != 0) {
			fprintf(stderr, "bind of %s failed: %s\n", e->address, zmq_strerror(errno));
			return -1;
		}
	}
	for_each_connect(e, ztk) {
		if (ztk_connect(ztk, e, type) != 0) {
			fprintf(stderr, "connect of %s failed: %s\n", e->address, zmq_strerror(errno));
			return -1;
		}
	}
	return 0;
}

void s_set_sockopts(ZTK *ztk, ztk_peer_t *e, int type, int after)
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

		ztk_debugf(ztk, "setting option %s (%#02x)\n", s_sockoptname(opt->name), opt->name);
		zmq_setsockopt(e->socket, opt->name, opt->value, opt->len);
	}

	if (type == ZMQ_SUB && !subd && after) {
		ztk_debugf(ztk, "setting option %s (%#02x)\n", s_sockoptname(ZMQ_SUBSCRIBE), ZMQ_SUBSCRIBE);
		zmq_setsockopt(e->socket, ZMQ_SUBSCRIBE, "", 0);
	}
}

int ztk_bind(ZTK *ztk, ztk_peer_t *e, int type)
{
	e->type = type;
	e->bound = 1;

	ztk_debugf(ztk, "binding a %s socket to %s\n", s_socktype(type), e->address);

	e->socket = zmq_socket(ztk->zmq, type);
	s_set_sockopts(ztk, e, type, 0);

	int rc = zmq_bind(e->socket, e->address);
	if (rc != 0) return rc;

	s_set_sockopts(ztk, e, type, 1);
	return 0;
}

int ztk_connect(ZTK *ztk, ztk_peer_t *e, int type)
{
	e->type = type;
	e->bound = 0;

	ztk_debugf(ztk, "connecting a %s socket to %s\n", s_socktype(type), e->address);

	e->socket = zmq_socket(ztk->zmq, type);
	s_set_sockopts(ztk, e, type, 0);

	int rc = zmq_connect(e->socket, e->address);
	if (rc != 0) return rc;

	s_set_sockopts(ztk, e, type, 1);
	return 0;
}

int ztk_poll(ZTK *ztk)
{
	if (!ztk->poll.items) {
		ztk_debugf(ztk, "first call to ztk_poll() - setting up 0MQ poller items\n");

		int n = list_len(&ztk->peers);
		ztk_debugf(ztk, "  found %i sockets to poll\n", n);
		ztk->poll.items   = vcalloc(n, sizeof(zmq_pollitem_t));
		ztk->poll.n       = n;

		ztk_peer_t *e;
		for_each_peer(e, ztk) {
			ztk_debugf(ztk, "  registering peer %s socket %s to %s\n", s_socktype(e->type),
					e->bound ? "bound" : "connected", e->address);
			n--;
			if (n < 0)
				return -1;
			ztk->poll.items[n].socket  = e->socket;
			ztk->poll.items[n].fd      = -1;
			ztk->poll.items[n].events  = ZMQ_POLLIN;
			ztk->poll.items[n].revents = 0;
		}
	}

	ztk_debugf(ztk, "polling %i sockets for %is%s\n", ztk->poll.n, ztk->poll.timeout,
			ztk->poll.timeout < 0 ? " (indefinitely)" : "");
	return zmq_poll(ztk->poll.items, ztk->poll.n, ztk->poll.timeout);
}

ztk_peer_t *ztk_next(ZTK *ztk, int events)
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

pdu_t *ztk_reply(ZTK *ztk, pdu_t *orig, FILE *io)
{
	char s[8192];
	if (fgets(s, 8192, io) == NULL)
		return NULL;

	char *a, *b;
	pdu_t *pdu = NULL;
	a = b = s;

	while (*b) {
		a = b;
		while (*b && *b != ztk->input_delim && *b != '\n') b++;
		*b++ = '\0';

		if (!pdu) {
			pdu = orig ? pdu_reply(orig, a, 0) : pdu_make(a, 0);
		} else {
			pdu_extendf(pdu, "%s", a);
		}
	}

	return pdu;
}

pdu_t *ztk_pdu(ZTK *ztk, FILE *io)
{
	return ztk_reply(ztk, NULL, io);
}

void ztk_print(ZTK *ztk, pdu_t *pdu, FILE *io)
{
	char *frame;
	int i = 1;

	fprintf(io, "%s", pdu_type(pdu));
	while ((frame = pdu_string(pdu, i++)) != NULL) {
		fprintf(io, "%c%s", ztk->output_delim, frame);
		free(frame);
	}
	fprintf(io, "\n");
	fflush(io);
}

void ztk_debugf(ZTK *cfg, const char *fmt, ...)
{
	if (!cfg->verbose)
		return;

	va_list ap;
	va_start(ap, fmt);
	ztk_vdebugf(cfg, fmt, ap);
	va_end(ap);
}

void ztk_vdebugf(ZTK *cfg, const char *fmt, va_list ap)
{
	if (!cfg->verbose)
		return;

	fprintf(stderr, "%s> ", cfg->program);
	vfprintf(stderr, fmt, ap);
}
