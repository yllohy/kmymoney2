####### kdevelop will overwrite this part!!! (begin)##########

SUBDIRS = libkgpgfile libkdchart kmymoney2 po doc developer-doc

EXTRA_DIST = AUTHORS COPYING ChangeLog INSTALL README TODO ChangeLog.original configure.in.in configure.in.bot README.Fileformats stamp-h1 subdirs inst-apps

noinst_HEADERS = kdecompat.h

####### kdevelop will overwrite this part!!! (end)############
# not a GNU package. You can remove this line, if
# have all needed files, that a GNU package needs
AUTOMAKE_OPTIONS = foreign
KDE_OPTIONS = noautodist

$(top_srcdir)/configure.in: configure.in.in $(top_srcdir)/subdirs
	cd $(top_srcdir) && $(MAKE) -f admin/Makefile.common configure.in ;

$(top_srcdir)/subdirs:
	cd $(top_srcdir) && $(MAKE) -f admin/Makefile.common subdirs

ADDITIONAL_CHECKS=`ls *.m4 | grep -v acinclude\.m4 | grep -v aclocal\.m4`

$(top_srcdir)/acinclude.m4: $(top_srcdir)/admin/acinclude.m4.in $(top_srcdir)/admin/libtool.m4.in
	@cd $(top_srcdir) && cat admin/acinclude.m4.in admin/libtool.m4.in > acinclude.m4
	@for i in $(ADDITIONAL_CHECKS); do \
		cat $$i >> acinclude.m4; \
	done

DISTCLEANFILES=kmymoney2.kdevses kmymoney2-*.tar.gz
MAINTAINERCLEANFILES = subdirs configure.in acinclude.m4 configure.files

.PHONY: check-vpath-environment
check-vpath-environment:
	@if test "x$(top_srcdir)x" != "x$(top_builddir)x"; then \
		echo "Packaging messages does not work in VPATH environment!"; \
		exit 1; \
	fi;

# we use our own version of extractrc if not otherwise specified
export EXTRACTRC ?= $(PWD)/admin/extractrc

generate-messages: check-vpath-environment
	find . -name rc.cpp -exec rm {} \;
	$(MAKE) -C $(top_srcdir) -f admin/Makefile.common package-messages XGETTEXT=$(XGETTEXT)
	find . -name rc.cpp -exec rm {} \;
	@if test `grep '"_: ' po/kmymoney2.pot | grep -v TRANSLATORS | wc -l` -eq 0; then echo -e "It seems that context i18n's are not translated correctly.\n'$(XGETTEXT)' has been used for translation."; exit 1; fi;

package-messages:
	$(MAKE) generate-messages
	$(MAKE) merge-messages

merge-messages: check-vpath-environment
	$(MAKE) -C $(top_srcdir)/po merge

# The following line adds the required admin files
EXTRA_DIST += admin/Makefile.common admin/cvs.sh admin/detect-autoconf.pl admin/acinclude.m4.in admin/conf.change.pl admin/config.pl admin/libtool.m4.in admin/configure.in.min admin/configure.in.bot.end admin/am_edit admin/debianrules admin/fixuifiles admin/extractrc

