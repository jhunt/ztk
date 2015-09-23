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

typedef int (*main_fn)(int, char**);

static struct {
	const char *prog;
	const char *name;
	main_fn main;

} COMMANDS[] = {
	{ "zpub", "pub", ztk_pub },
	{ "zsub", "sub", ztk_sub },
	{ "zpush", "push", ztk_push },
	{ "zpull", "pull", ztk_pull },
	{ NULL, NULL, NULL },
};

int main(int argc, char **argv)
{
	int i;
	for (i = 0; COMMANDS[i].name; i++) {
		if (strcmp(argv[0], COMMANDS[i].prog) == 0) {
			return (*COMMANDS[i].main)(argc, argv);
		}
	}

	if (argc > 1) {
		for (i = 0; COMMANDS[i].name; i++) {
			if (strcmp(argv[1], COMMANDS[i].name) == 0) {
				argv[1] = argv[0];
				return (*COMMANDS[i].main)(argc - 1, argv + 1);
			}
		}
	}

	fprintf(stderr, "unrecognized command (try `%s (pub|sub|push|pull) [options]')\n", argv[0]);
	return 1;
}
