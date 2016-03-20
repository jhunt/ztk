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

#ifndef ZTK_H
#define ZTK_H

#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <sys/types.h>
#include <vigor.h>

#define FORMAT_YAML  1
#define FORMAT_JSON  2
#define FORMAT_DELIM 3

typedef struct {
	void   *zmq;
	list_t  sockopts;
	list_t  binds;
	list_t  connects;
	list_t  peers;

	char   *program;
	int     verbose;

	int     argc;
	char  **argv;

	uint8_t input;
	char    input_delim;

	uint8_t output;
	char    output_delim;
	int     output_npdus;

	struct {
		zmq_pollitem_t *items;
		int             n;
		long            timeout;
	} poll;
} ZTK;

typedef struct {
	int     name;
	void   *value;
	size_t  len;
	list_t  l;
} ztk_sockopt_t;

typedef struct {
	char    *address;
	void    *socket;
	int      type;
	int      bound;
	list_t   l;
	list_t   all;
} ztk_peer_t;

#define for_each_peer(e, ztk)     for_each_object((e), &((ztk)->peers), all)
#define for_each_bind(e, ztk)     for_each_object((e), &((ztk)->binds), l)
#define for_each_connect(e, ztk)  for_each_object((e), &((ztk)->connects), l)

ZTK* ztk_configure(const char *program, int argc, char **argv);
int ztk_shutdown(ZTK *cfg);
int ztk_sockets(ZTK *cfg, int type);
int ztk_bind(ZTK *cfg, ztk_peer_t *e, int type);
int ztk_connect(ZTK *cfg, ztk_peer_t *e, int type);
int ztk_poll(ZTK *cfg);
ztk_peer_t *ztk_next(ZTK *cfg, int events);
pdu_t *ztk_reply(ZTK *cfg, pdu_t *pdu, FILE *io);
pdu_t *ztk_pdu(ZTK *cfg, FILE *io);
void ztk_print_preamble(ZTK *cfg, FILE *io);
void ztk_print(ZTK *cfg, pdu_t *pdu, FILE *io);
void ztk_print_postamble(ZTK *cfg, FILE *io);
void ztk_debugf(ZTK *cfg, const char *fmt, ...);
void ztk_vdebugf(ZTK *cfg, const char *fmt, va_list ap);

int ztk_push(int argc, char **argv);
int ztk_pull(int argc, char **argv);
int ztk_pub(int argc, char **argv);
int ztk_sub(int argc, char **argv);
int ztk_req(int argc, char **argv);
int ztk_rep(int argc, char **argv);
int ztk_dealer(int argc, char **argv);
int ztk_router(int argc, char **argv);
int ztk_tap(int argc, char **argv);

#endif
