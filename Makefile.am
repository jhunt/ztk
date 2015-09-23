#
#  Copyright 2015 James Hunt <james@jameshunt.us>
#
#  This file is part of ztk.
#
#  ztk is free software: you can redistribute it and/or modify it under
#  the terms of the GNU General Public License as published by the Free
#  Software Foundation, either version 3 of the License, or (at your
#  option) any later version.
#
#  ztk is distributed in the hope that it will be useful, but WITHOUT ANY
#  WARRANTY; without even the implied warranty of MERCHANTABILITY or
#  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
#  for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with ztk.  If not, see <http://www.gnu.org/licenses/>.
#

AUTOMAKE_OPTIONS = foreign subdir-objects
ACLOCAL_AMFLAGS  = -I build

AM_CFLAGS = -Wall -Wextra

############################################################

bin_PROGRAMS = ztk zpub zsub
ztk_SOURCES = src/ztk.h src/ztk.c src/main.c
zpub_SOURCES = src/ztk.h src/ztk.c src/zpub.c
zsub_SOURCES = src/ztk.h src/ztk.c src/zsub.c

############################################################

check_PROGRAMS = t/zpush
t_zpush_SOURCES = src/ztk.h src/ztk.c src/push.c src/pull.c t/zpush.c
t_zpush_LDADD   = -lctap -lvigor

TESTS = $(check_PROGRAMS)
