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

	uint8_t input;
	char    input_delim;

	uint8_t output;
	char    output_delim;

	struct {
		zmq_pollitem_t *items;
		int             n;
	} poll;
} ztk_config_t;

typedef struct {
	int     name;
	void   *value;
	size_t  len;
	list_t  l;
} ztk_sockopt_t;

typedef struct {
	char    *address;
	void    *socket;
	list_t   l;
	list_t   all;
} ztk_peer_t;

#define for_each_peer(e, ztk)     for_each_object((e), &((ztk)->peers), all)
#define for_each_bind(e, ztk)     for_each_object((e), &((ztk)->binds), l)
#define for_each_connect(e, ztk)  for_each_object((e), &((ztk)->connects), l)

ztk_config_t* ztk_configure(int argc, char **argv);
int ztk_shutdown(ztk_config_t *cfg);
int ztk_sockets(ztk_config_t *cfg, int type);
int ztk_bind(ztk_config_t *cfg, ztk_peer_t *e, int type);
int ztk_connect(ztk_config_t *cfg, ztk_peer_t *e, int type);
int ztk_poll(ztk_config_t *cfg, long timeout);
ztk_peer_t *ztk_next(ztk_config_t *cfg, int events);
pdu_t *ztk_reply(ztk_config_t *cfg, pdu_t *pdu, FILE *io);
pdu_t *ztk_pdu(ztk_config_t *cfg, FILE *io);
void ztk_print(ztk_config_t *cfg, pdu_t *pdu, FILE *io);

int ztk_push(int argc, char **argv);
int ztk_pull(int argc, char **argv);
int ztk_pub(int argc, char **argv);
int ztk_sub(int argc, char **argv);
int ztk_req(int argc, char **argv);
int ztk_rep(int argc, char **argv);
int ztk_dealer(int argc, char **argv);
int ztk_router(int argc, char **argv);

#endif
