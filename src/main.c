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

	ztk_sockopt_t *o;
	for_each_object(o, &ztk->sockopts, l)
		fprintf(stderr, "OPTION: %02x (%lub) = [%s]\n", o->name, o->len, (char *) o->value);

	ztk_peer_t *e;
	for_each_object(e, &ztk->binds, l)
		fprintf(stderr, "BIND '%s'\n", e->address);
	for_each_object(e, &ztk->connects, l)
		fprintf(stderr, "CONNECT '%s'\n", e->address);

	return 0;
}
