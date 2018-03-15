# Makefile for program source directory in GNU NLS utilities package.
# Copyright (C) 1995, 1996, 1997 by Ulrich Drepper <drepper@gnu.ai.mit.edu>
# Copyright (C) 2004-2008 Rodney Dawes <dobey.pwns@gmail.com>
#
# This file may be copied and used freely without restrictions.  It may
# be used in projects which are not available under a GNU Public License,
# but which still want to provide support for the GNU gettext functionality.
#
# - Modified by Owen Taylor <otaylor@redhat.com> to use GETTEXT_PACKAGE
#   instead of PACKAGE and to look for po2tbl in ./ not in intl/
#
# - Modified by jacob berkman <jacob@ximian.com> to install
#   Makefile.in.in and po2tbl.sed.in for use with glib-gettextize
#
# - Modified by Rodney Dawes <dobey.pwns@gmail.com> for use with intltool
#
# We have the following line for use by intltoolize:
# INTLTOOL_MAKEFILE

GETTEXT_PACKAGE = liferea
PACKAGE = liferea
VERSION = 1.12.2

SHELL = /bin/sh

srcdir = .
top_srcdir = ..
top_builddir = ..


prefix = /usr/local
exec_prefix = ${prefix}
datadir = ${datarootdir}
datarootdir = ${prefix}/share
libdir = ${exec_prefix}/lib
localedir = ${datarootdir}/locale
subdir = po
install_sh = ${SHELL} /home/unixjunkie/liferea-1.12.2/install-sh
# Automake >= 1.8 provides $(MKDIR_P).
# Until it can be supposed, use the safe fallback:
mkdir_p = $(install_sh) -d

INSTALL = /usr/bin/install -c
INSTALL_DATA = ${INSTALL} -m 644

GMSGFMT = /usr/bin/gmsgfmt
MSGFMT = /usr/bin/msgfmt
XGETTEXT = /usr/bin/xgettext
INTLTOOL_UPDATE = /usr/bin/intltool-update
INTLTOOL_EXTRACT = /usr/bin/intltool-extract
MSGMERGE = INTLTOOL_EXTRACT="$(INTLTOOL_EXTRACT)" XGETTEXT="$(XGETTEXT)" srcdir=$(srcdir) $(INTLTOOL_UPDATE) --gettext-package $(GETTEXT_PACKAGE) --dist
GENPOT   = INTLTOOL_EXTRACT="$(INTLTOOL_EXTRACT)" XGETTEXT="$(XGETTEXT)" srcdir=$(srcdir) $(INTLTOOL_UPDATE) --gettext-package $(GETTEXT_PACKAGE) --pot

ALL_LINGUAS = 

PO_LINGUAS=$(shell if test -r $(srcdir)/LINGUAS; then grep -v "^\#" $(srcdir)/LINGUAS; else echo "$(ALL_LINGUAS)"; fi)

USER_LINGUAS=$(shell if test -n "$(LINGUAS)"; then LLINGUAS="$(LINGUAS)"; ALINGUAS="$(ALL_LINGUAS)"; for lang in $$LLINGUAS; do if test -n "`grep \^$$lang$$ $(srcdir)/LINGUAS 2>/dev/null`" -o -n "`echo $$ALINGUAS|tr ' ' '\n'|grep \^$$lang$$`"; then printf "$$lang "; fi; done; fi)

USE_LINGUAS=$(shell if test -n "$(USER_LINGUAS)" -o -n "$(LINGUAS)"; then LLINGUAS="$(USER_LINGUAS)"; else if test -n "$(PO_LINGUAS)"; then LLINGUAS="$(PO_LINGUAS)"; else LLINGUAS="$(ALL_LINGUAS)"; fi; fi; for lang in $$LLINGUAS; do printf "$$lang "; done)

POFILES=$(shell LINGUAS="$(PO_LINGUAS)"; for lang in $$LINGUAS; do printf "$$lang.po "; done)

DISTFILES = Makefile.in.in POTFILES.in $(POFILES)
EXTRA_DISTFILES = ChangeLog POTFILES.skip Makevars LINGUAS

POTFILES = \
	../liferea.appdata.xml.in \
	../net.sourceforge.liferea.desktop.in \
	../xslt/feed.xml.in \
	../xslt/source.xml.in \
	../xslt/folder.xml.in \
	../xslt/item.xml.in \
	../xslt/newsbin.xml.in \
	../xslt/vfolder.xml.in \
	../src/browser.c \
	../src/browser_history.c \
	../src/comments.c \
	../src/comments.h \
	../src/common.c \
	../src/common.h \
	../src/conf.c \
	../src/conf.h \
	../src/date.c \
	../src/date.h \
	../src/db.c \
	../src/db.h \
	../src/debug.c \
	../src/debug.h \
	../src/enclosure.c \
	../src/export.c \
	../src/export.h \
	../src/favicon.c \
	../src/favicon.h \
	../src/feed.c \
	../src/feed.h \
	../src/feed_parser.c \
	../src/feed_parser.h \
	../src/feedlist.c \
	../src/feedlist.h \
	../src/folder.c \
	../src/folder.h \
	../src/html.c \
	../src/html.h \
	../src/htmlview.c \
	../src/htmlview.h \
	../src/item.c \
	../src/item.h \
	../src/item_history.c \
	../src/item_history.h \
	../src/item_loader.c \
	../src/item_loader.h \
	../src/itemlist.c \
	../src/itemlist.h \
	../src/itemset.c \
	../src/itemset.h \
	../src/liferea_application.c \
	../src/main.c \
	../src/metadata.c \
	../src/metadata.h \
	../src/migrate.c \
	../src/net.c \
	../src/net.h \
	../src/node.c \
	../src/node.h \
	../src/node_type.c \
	../src/node_type.h \
	../src/render.c \
	../src/render.h \
	../src/rule.c \
	../src/rule.h \
	../src/social.c \
	../src/social.h \
	../src/subscription.c \
	../src/subscription.h \
	../src/subscription_type.h \
	../src/update.c \
	../src/update.h \
	../src/vfolder.c \
	../src/vfolder.h \
	../src/vfolder_loader.c \
	../src/vfolder_loader.h \
	../src/xml.c \
	../src/xml.h \
	../src/ui/enclosure_list_view.c \
	../src/ui/enclosure_list_view.h \
	../src/ui/icons.c \
	../src/ui/icons.h \
	../src/ui/itemview.c \
	../src/ui/itemview.h \
	../src/ui/liferea_shell.c \
	../src/ui/liferea_shell.h \
	../src/ui/rule_editor.c \
	../src/ui/rule_editor.h \
	../src/ui/search_dialog.c \
	../src/ui/search_dialog.h \
	../src/ui/search_folder_dialog.h \
	../src/ui/search_folder_dialog.c \
	../src/ui/auth_dialog.c \
	../src/ui/auth_dialog.h \
	../src/ui/liferea_dialog.c \
	../src/ui/liferea_dialog.h \
	../src/ui/ui_common.c \
	../src/ui/ui_dnd.c \
	../src/ui/ui_dnd.h \
	../src/ui/feed_list_view.c \
	../src/ui/feed_list_view.h \
	../src/ui/ui_folder.c \
	../src/ui/ui_folder.h \
	../src/ui/liferea_htmlview.c \
	../src/ui/liferea_htmlview.h \
	../src/ui/item_list_view.c \
	../src/ui/item_list_view.h \
	../src/ui/liferea_shell.c \
	../src/ui/liferea_shell.h \
	../src/ui/popup_menu.c \
	../src/ui/popup_menu.h \
	../src/ui/preferences_dialog.c \
	../src/ui/preferences_dialog.h \
	../src/ui/feed_list_node.c \
	../src/ui/feed_list_node.h \
	../src/ui/subscription_dialog.c \
	../src/ui/subscription_dialog.h \
	../src/ui/browser_tabs.c \
	../src/ui/browser_tabs.h \
	../src/ui/ui_update.c \
	../src/ui/ui_update.h \
	../src/parsers/atom10.c \
	../src/parsers/atom10.h \
	../src/parsers/cdf_channel.c \
	../src/parsers/cdf_channel.h \
	../src/parsers/cdf_item.c \
	../src/parsers/cdf_item.h \
	../src/parsers/ns_admin.c \
	../src/parsers/ns_admin.h \
	../src/parsers/ns_ag.c \
	../src/parsers/ns_ag.h \
	../src/parsers/ns_blogChannel.c \
	../src/parsers/ns_blogChannel.h \
	../src/parsers/ns_cC.c \
	../src/parsers/ns_cC.h \
	../src/parsers/ns_content.c \
	../src/parsers/ns_content.h \
	../src/parsers/ns_dc.c \
	../src/parsers/ns_dc.h \
	../src/parsers/ns_photo.c \
	../src/parsers/ns_photo.h \
	../src/parsers/ns_slash.c \
	../src/parsers/ns_slash.h \
	../src/parsers/ns_syn.c \
	../src/parsers/ns_syn.h \
	../src/parsers/pie_entry.c \
	../src/parsers/pie_entry.h \
	../src/parsers/pie_feed.c \
	../src/parsers/pie_feed.h \
	../src/parsers/rss_channel.c \
	../src/parsers/rss_channel.h \
	../src/parsers/rss_item.c \
	../src/parsers/rss_item.h \
	../src/fl_sources/default_source.c \
	../src/fl_sources/default_source.h \
	../src/fl_sources/dummy_source.c \
	../src/fl_sources/dummy_source.h \
	../src/fl_sources/inoreader_source.c \
	../src/fl_sources/inoreader_source.h \
	../src/fl_sources/google_source.c \
	../src/fl_sources/google_source.h \
	../src/fl_sources/opml_source.c \
	../src/fl_sources/opml_source.h \
	../src/fl_sources/node_source.c \
	../src/fl_sources/node_source.h \
	../src/fl_sources/reedah_source.c \
	../src/fl_sources/reedah_source.h \
	../src/fl_sources/reedah_source_feed.c \
	../src/fl_sources/reedah_source_feed_list.c \
	../src/fl_sources/reedah_source_feed_list.h \
	../src/fl_sources/ttrss_source.c \
	../src/fl_sources/ttrss_source.h \
	../src/fl_sources/ttrss_source_feed.c \
	../src/fl_sources/ttrss_source_feed_list.c \
	../src/fl_sources/ttrss_source_feed_list.h \
	../src/fl_sources/theoldreader_source.c \
	../src/fl_sources/theoldreader_source.h \
	../src/fl_sources/theoldreader_source_feed.c \
	../src/fl_sources/theoldreader_source_feed_list.c \
	../src/fl_sources/theoldreader_source_feed_list.h \
	../src/webkit/webkit.c \
	../src/webkit/liferea_web_view.c \
	../src/webkit/web_extension/liferea_web_extension.c \
	../src/webkit/web_extension/web_extension_main.c \
	../glade/about.ui \
	../glade/auth.ui \
	../glade/enclosure_handler.ui \
	../glade/google_source.ui \
	../glade/inoreader_source.ui \
	../glade/mainwindow.ui \
	../glade/new_folder.ui \
	../glade/new_newsbin.ui \
	../glade/new_subscription.ui \
	../glade/node_source.ui \
	../glade/opml_source.ui \
	../glade/prefs.ui \
	../glade/properties.ui \
	../glade/reedah_source.ui \
	../glade/rename_node.ui \
	../glade/search_folder.ui \
	../glade/search.ui \
	../glade/simple_search.ui \
	../glade/simple_subscription.ui \
	../glade/theoldreader_source.ui \
	../glade/ttrss_source.ui \
	../glade/update_monitor.ui

CATALOGS=$(shell LINGUAS="$(USE_LINGUAS)"; for lang in $$LINGUAS; do printf "$$lang.gmo "; done)

.SUFFIXES:
.SUFFIXES: .po .pox .gmo .mo .msg .cat

AM_DEFAULT_VERBOSITY = 0
INTLTOOL_V_MSGFMT = $(INTLTOOL__v_MSGFMT_$(V))
INTLTOOL__v_MSGFMT_= $(INTLTOOL__v_MSGFMT_$(AM_DEFAULT_VERBOSITY))
INTLTOOL__v_MSGFMT_0 = @echo "  MSGFMT" $@;

.po.pox:
	$(MAKE) $(GETTEXT_PACKAGE).pot
	$(MSGMERGE) $* $(GETTEXT_PACKAGE).pot -o $*.pox

.po.mo:
	$(INTLTOOL_V_MSGFMT)$(MSGFMT) -o $@ $<

.po.gmo:
	$(INTLTOOL_V_MSGFMT)file=`echo $* | sed 's,.*/,,'`.gmo \
	  && rm -f $$file && $(GMSGFMT) -o $$file $<

.po.cat:
	sed -f ../intl/po2msg.sed < $< > $*.msg \
	  && rm -f $@ && gencat $@ $*.msg


all: all-yes

all-yes: $(CATALOGS)
all-no:

$(GETTEXT_PACKAGE).pot: $(POTFILES)
	$(GENPOT)

install: install-data
install-data: install-data-yes
install-data-no: all
install-data-yes: all
	linguas="$(USE_LINGUAS)"; \
	for lang in $$linguas; do \
	  dir=$(DESTDIR)$(localedir)/$$lang/LC_MESSAGES; \
	  $(mkdir_p) $$dir; \
	  if test -r $$lang.gmo; then \
	    $(INSTALL_DATA) $$lang.gmo $$dir/$(GETTEXT_PACKAGE).mo; \
	    echo "installing $$lang.gmo as $$dir/$(GETTEXT_PACKAGE).mo"; \
	  else \
	    $(INSTALL_DATA) $(srcdir)/$$lang.gmo $$dir/$(GETTEXT_PACKAGE).mo; \
	    echo "installing $(srcdir)/$$lang.gmo as" \
		 "$$dir/$(GETTEXT_PACKAGE).mo"; \
	  fi; \
	  if test -r $$lang.gmo.m; then \
	    $(INSTALL_DATA) $$lang.gmo.m $$dir/$(GETTEXT_PACKAGE).mo.m; \
	    echo "installing $$lang.gmo.m as $$dir/$(GETTEXT_PACKAGE).mo.m"; \
	  else \
	    if test -r $(srcdir)/$$lang.gmo.m ; then \
	      $(INSTALL_DATA) $(srcdir)/$$lang.gmo.m \
		$$dir/$(GETTEXT_PACKAGE).mo.m; \
	      echo "installing $(srcdir)/$$lang.gmo.m as" \
		   "$$dir/$(GETTEXT_PACKAGE).mo.m"; \
	    else \
	      true; \
	    fi; \
	  fi; \
	done

# Empty stubs to satisfy archaic automake needs
dvi info ctags tags CTAGS TAGS ID:

# Define this as empty until I found a useful application.
install-exec installcheck:

uninstall:
	linguas="$(USE_LINGUAS)"; \
	for lang in $$linguas; do \
	  rm -f $(DESTDIR)$(localedir)/$$lang/LC_MESSAGES/$(GETTEXT_PACKAGE).mo; \
	  rm -f $(DESTDIR)$(localedir)/$$lang/LC_MESSAGES/$(GETTEXT_PACKAGE).mo.m; \
	done

check: all $(GETTEXT_PACKAGE).pot
	rm -f missing notexist
	srcdir=$(srcdir) $(INTLTOOL_UPDATE) -m
	if [ -r missing -o -r notexist ]; then \
	  exit 1; \
	fi

mostlyclean:
	rm -f *.pox $(GETTEXT_PACKAGE).pot *.old.po cat-id-tbl.tmp
	rm -f .intltool-merge-cache

clean: mostlyclean

distclean: clean
	rm -f Makefile Makefile.in POTFILES stamp-it
	rm -f *.mo *.msg *.cat *.cat.m *.gmo

maintainer-clean: distclean
	@echo "This command is intended for maintainers to use;"
	@echo "it deletes files that may require special tools to rebuild."
	rm -f Makefile.in.in

distdir = ../$(PACKAGE)-$(VERSION)/$(subdir)
dist distdir: $(DISTFILES)
	dists="$(DISTFILES)"; \
	extra_dists="$(EXTRA_DISTFILES)"; \
	for file in $$extra_dists; do \
	  test -f $(srcdir)/$$file && dists="$$dists $(srcdir)/$$file"; \
	done; \
	for file in $$dists; do \
	  test -f $$file || file="$(srcdir)/$$file"; \
	  ln $$file $(distdir) 2> /dev/null \
	    || cp -p $$file $(distdir); \
	done

update-po: Makefile
	$(MAKE) $(GETTEXT_PACKAGE).pot
	tmpdir=`pwd`; \
	linguas="$(USE_LINGUAS)"; \
	for lang in $$linguas; do \
	  echo "$$lang:"; \
	  result="`$(MSGMERGE) -o $$tmpdir/$$lang.new.po $$lang`"; \
	  if $$result; then \
	    if cmp $(srcdir)/$$lang.po $$tmpdir/$$lang.new.po >/dev/null 2>&1; then \
	      rm -f $$tmpdir/$$lang.new.po; \
            else \
	      if mv -f $$tmpdir/$$lang.new.po $$lang.po; then \
	        :; \
	      else \
	        echo "msgmerge for $$lang.po failed: cannot move $$tmpdir/$$lang.new.po to $$lang.po" 1>&2; \
	        rm -f $$tmpdir/$$lang.new.po; \
	        exit 1; \
	      fi; \
	    fi; \
	  else \
	    echo "msgmerge for $$lang.gmo failed!"; \
	    rm -f $$tmpdir/$$lang.new.po; \
	  fi; \
	done

Makefile POTFILES: stamp-it
	@if test ! -f $@; then \
	  rm -f stamp-it; \
	  $(MAKE) stamp-it; \
	fi

stamp-it: Makefile.in.in $(top_builddir)/config.status POTFILES.in
	cd $(top_builddir) \
	  && CONFIG_FILES=$(subdir)/Makefile.in CONFIG_HEADERS= CONFIG_LINKS= \
	       $(SHELL) ./config.status

# Tell versions [3.59,3.63) of GNU make not to export all variables.
# Otherwise a system limit (for SysV at least) may be exceeded.
.NOEXPORT:
