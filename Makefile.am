####### kdevelop will overwrite this part!!! (begin)##########

SUBDIRS = kmymoney2 po doc developer-doc 

EXTRA_DIST = kmymoney2.kdevprj AUTHORS COPYING ChangeLog INSTALL README TODO ChangeLog.original configure.in.in 

####### kdevelop will overwrite this part!!! (end)############
# not a GNU package. You can remove this line, if 
# have all needed files, that a GNU package needs
AUTOMAKE_OPTIONS = foreign 

$(top_srcdir)/configure.in: configure.in.in $(top_srcdir)/subdirs
	cd $(top_srcdir) && $(MAKE) -f admin/Makefile.common configure.in ;

$(top_srcdir)/subdirs:
	cd $(top_srcdir) && $(MAKE) -f admin/Makefile.common subdirs

$(top_srcdir)/acinclude.m4: $(top_srcdir)/admin/acinclude.m4.in $(top_srcdir)/admin/libtool.m4.in
	@cd $(top_srcdir) && cat admin/acinclude.m4.in admin/libtool.m4.in > acinclude.m4

DISTCLEANFILES=kmymoney2.kdevses kmymoney2-*.tar.gz
MAINTAINERCLEANFILES = subdirs configure.in acinclude.m4 configure.files 

package-messages:
	$(MAKE) -f admin/Makefile.common package-messages
	$(MAKE) -C po merge

EXTRA_DIST += admin/cppunit.patch

# The following line adds some files that are required to build an RPM
EXTRA_DIST += admin/acinclude.m4.in admin/libtool.m4.in admin/conf.change.pl admin/config.pl

dist-hook:
	cd $(top_distdir) && perl admin/am_edit -padmin
	cd $(top_distdir) && $(MAKE) -f admin/Makefile.common subdirs
