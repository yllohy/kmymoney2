####### kdevelop will overwrite this part!!! (begin)##########

SUBDIRS = libkgpgfile kmymoney2 po doc developer-doc

EXTRA_DIST = kmymoney2.kdevprj AUTHORS COPYING ChangeLog INSTALL README TODO ChangeLog.original configure.in.in configure.in.bot README.Fileformats stamp-h1 subdirs
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

package-messages:
	$(MAKE) -C $(top_srcdir) -f admin/Makefile.common package-messages

merge-messages:
	$(MAKE) -C $(top_srcdir)/po merge

# The following line adds the required admin files
EXTRA_DIST += admin/Makefile.common admin/cvs.sh admin/detect-autoconf.sh admin/acinclude.m4.in admin/conf.change.pl admin/config.pl admin/libtool.m4.in admin/configure.in.min admin/configure.in.bot.end admin/am_edit admin/debianrules

