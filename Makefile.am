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

AM_CFLAGS = -Wall -Wextra @GCOV_CFLAGS@
AM_LDFLAGS = @GCOV_LDFLAGS@

CORE_SRC := src/ztkh. src/ztk.c
CORE_SRC += src/push.c src/pull.c
CORE_SRC += src/pub.c src/sub.c
CORE_SRC += src/req.c src/rep.c
CORE_SRC += src/dealer.c src/router.c

############################################################

bin_PROGRAMS = ztk
ztk_SOURCES = $(CORE_SRC) src/main.c

############################################################

check_SCRIPTS = t/arg-validation t/invoke t/timeout t/delim \
                t/pipeline t/pubsub t/reqrep t/drouter \
                t/debug
TESTS = $(check_SCRIPTS)

############################################################

install-exec-hook:
	$(LN_S) -f $(DESTDIR)$(bindir)/ztk$(EXEEXT) $(DESTDIR)$(bindir)/zpush$(EXEEXT)
	$(LN_S) -f $(DESTDIR)$(bindir)/ztk$(EXEEXT) $(DESTDIR)$(bindir)/zpull$(EXEEXT)
	$(LN_S) -f $(DESTDIR)$(bindir)/ztk$(EXEEXT) $(DESTDIR)$(bindir)/zpub$(EXEEXT)
	$(LN_S) -f $(DESTDIR)$(bindir)/ztk$(EXEEXT) $(DESTDIR)$(bindir)/zsub$(EXEEXT)
	$(LN_S) -f $(DESTDIR)$(bindir)/ztk$(EXEEXT) $(DESTDIR)$(bindir)/zreq$(EXEEXT)
	$(LN_S) -f $(DESTDIR)$(bindir)/ztk$(EXEEXT) $(DESTDIR)$(bindir)/zrep$(EXEEXT)
	$(LN_S) -f $(DESTDIR)$(bindir)/ztk$(EXEEXT) $(DESTDIR)$(bindir)/zdealer$(EXEEXT)
	$(LN_S) -f $(DESTDIR)$(bindir)/ztk$(EXEEXT) $(DESTDIR)$(bindir)/zrouter$(EXEEXT)

############################################################

.PHONY: coverage-clean coverage-report coverage clean-local-cov distclean-local
distclean-local:
	rm -rf t/tmp

clean-local: clean-local-gcda
clean-local-gcda:
	touch sac.gcda
	find . -name '*.gc??' | xargs rm

if GCOV_ENABLED
coverage-clean:
	@rm -fr coverage
	@find . -name "*.gcda" -exec rm {} \;
	@lcov --directory . --zerocounters

coverage-report:
	@mkdir -p coverage
	@lcov --compat-libtool --directory . --base-directory . --capture --output-file coverage/app.info
	@genhtml -o coverage/ coverage/app.info

coverage:
	@make coverage-report

clean-local-cov:
	@make coverage-clean

check:
	@make coverage

else
coverage:
	@echo >&2 "nothin doin"
clean-local-cov:
endif
