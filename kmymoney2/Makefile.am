KDE_OPTIONS = noautodist

bin_PROGRAMS = kmymoney2

kmymoney2_SOURCES = kmymoneyutils.cpp kstartuplogo.cpp kmymoney2.cpp main.cpp kmymoney2.stub kmymoney2.skel

kmymoney2_LDADD = $(top_builddir)/kmymoney2/views/libviews.a $(top_builddir)/kmymoney2/reports/libreports.a  $(top_builddir)/kmymoney2/dialogs/libdialogs.a $(top_builddir)/kmymoney2/widgets/libwidgets.a $(top_builddir)/kmymoney2/converter/libconverter.a $(top_builddir)/kmymoney2/mymoney/storage/libstorage.a $(top_builddir)/kmymoney2/mymoney/libkmm_mymoney.la $(top_builddir)/libkgpgfile/libkgpgfile.la $(top_builddir)/kmymoney2/plugins/interfaces/libinterfaces.a $(top_builddir)/kmymoney2/plugins/libkmm_plugin.la dialogs/libdialogs.a $(LIB_KABC) $(LIB_KHTML) $(LIB_KDEUI) $(LIB_KDECORE) $(LIB_QT) $(LIBSOCKET)

SUBDIRS = mymoney widgets dialogs views converter pics icons html templates plugins reports

EXTRA_DIST = kmymoney2ui.rc kmymoney2.desktop lo16-app-kmymoney2.png lo32-app-kmymoney2.png kmymoney_wizard.png mimetype_kmy_16x16.png mimetype_kmy_32x32.png mimetype_kmy_48x48.png mimetype_kmy_64x64.png mimetype_kmy_128x128.png x-kmymoney2.desktop hi16-app-kmymoney2.png hi32-app-kmymoney2.png hi64-app-kmymoney2.png hi128-app-kmymoney2.png tips

instdir=$(includedir)/kmymoney
inst_HEADERS = export.h
noinst_HEADERS = kmymoney2.h kmymoneyutils.h kstartuplogo.h

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(kde_datadir)/kmymoney2/
	$(INSTALL_DATA) $(srcdir)/kmymoney2ui.rc $(DESTDIR)$(kde_datadir)/kmymoney2/kmymoney2ui.rc
	$(mkinstalldirs) $(DESTDIR)$(xdg_appsdir)
	$(INSTALL_DATA) $(srcdir)/kmymoney2.desktop $(DESTDIR)$(xdg_appsdir)/kmymoney2.desktop
	$(mkinstalldirs) $(DESTDIR)$(kde_icondir)/locolor/16x16/apps/
	$(INSTALL_DATA) $(srcdir)/lo16-app-kmymoney2.png $(DESTDIR)$(kde_icondir)/locolor/16x16/apps/kmymoney2.png
	$(mkinstalldirs) $(DESTDIR)$(kde_icondir)/locolor/32x32/apps/
	$(INSTALL_DATA) $(srcdir)/lo32-app-kmymoney2.png $(DESTDIR)$(kde_icondir)/locolor/32x32/apps/kmymoney2.png
	$(mkinstalldirs) $(DESTDIR)$(kde_icondir)/hicolor/16x16/mimetypes/
	$(INSTALL_DATA) $(srcdir)/mimetype_kmy_16x16.png $(DESTDIR)$(kde_icondir)/hicolor/16x16/mimetypes/kmy.png
	$(mkinstalldirs) $(DESTDIR)$(kde_icondir)/hicolor/32x32/mimetypes/
	$(INSTALL_DATA) $(srcdir)/mimetype_kmy_32x32.png $(DESTDIR)$(kde_icondir)/hicolor/32x32/mimetypes/kmy.png
	$(mkinstalldirs) $(DESTDIR)$(kde_icondir)/hicolor/48x48/mimetypes/
	$(INSTALL_DATA) $(srcdir)/mimetype_kmy_48x48.png $(DESTDIR)$(kde_icondir)/hicolor/48x48/mimetypes/kmy.png
	$(mkinstalldirs) $(DESTDIR)$(kde_icondir)/hicolor/64x64/mimetypes/
	$(INSTALL_DATA) $(srcdir)/mimetype_kmy_64x64.png $(DESTDIR)$(kde_icondir)/hicolor/64x64/mimetypes/kmy.png
	$(mkinstalldirs) $(DESTDIR)$(kde_icondir)/hicolor/128x128/mimetypes/
	$(INSTALL_DATA) $(srcdir)/mimetype_kmy_128x128.png $(DESTDIR)$(kde_icondir)/hicolor/128x128/mimetypes/kmy.png
	$(mkinstalldirs) $(kde_mimedir)/application/
	$(INSTALL_DATA) $(srcdir)/x-kmymoney2.desktop $(kde_mimedir)/application/x-kmymoney2.desktop
	$(mkinstalldirs) $(DESTDIR)$(kde_icondir)/hicolor/16x16/apps/
	$(INSTALL_DATA) $(srcdir)/hi16-app-kmymoney2.png $(DESTDIR)$(kde_icondir)/hicolor/16x16/apps/kmymoney2.png
	$(mkinstalldirs) $(DESTDIR)$(kde_icondir)/hicolor/32x32/apps/
	$(INSTALL_DATA) $(srcdir)/hi32-app-kmymoney2.png $(DESTDIR)$(kde_icondir)/hicolor/32x32/apps/kmymoney2.png
	$(mkinstalldirs) $(DESTDIR)$(kde_icondir)/hicolor/64x64/apps/
	$(INSTALL_DATA) $(srcdir)/hi64-app-kmymoney2.png $(DESTDIR)$(kde_icondir)/hicolor/64x64/apps/kmymoney2.png
	$(mkinstalldirs) $(DESTDIR)$(kde_icondir)/hicolor/128x128/apps/
	$(INSTALL_DATA) $(srcdir)/hi128-app-kmymoney2.png $(DESTDIR)$(kde_icondir)/hicolor/128x128/apps/kmymoney2.png
	$(mkinstalldirs) $(DESTDIR)$(kde_datadir)/kmymoney2/
	$(INSTALL_DATA) $(srcdir)/tips $(DESTDIR)$(kde_datadir)/kmymoney2/tips


uninstall-local:
	-rm -f $(DESTDIR)$(kde_datadir)/kmymoney2/kmymoney2ui.rc
	-rm -f $(DESTDIR)$(xdg_appsdir)/kmymoney2.desktop
	-rm -f $(DESTDIR)$(kde_icondir)/locolor/16x16/apps/kmymoney2.png
	-rm -f $(DESTDIR)$(kde_icondir)/locolor/32x32/apps/kmymoney2.png
	-rm -f $(DESTDIR)$(kde_icondir)/hicolor/16x16/mimetypes/kmy.png
	-rm -f $(DESTDIR)$(kde_icondir)/hicolor/32x32/mimetypes/kmy.png
	-rm -f $(DESTDIR)$(kde_icondir)/hicolor/48x48/mimetypes/kmy.png
	-rm -f $(DESTDIR)$(kde_icondir)/hicolor/64x64/mimetypes/kmy.png
	-rm -f $(DESTDIR)$(kde_icondir)/hicolor/128x128/mimetypes/kmy.png
	-rm -f $(DESTDIR)$(kde_mimedir)/application/x-kmymoney2.desktop
	-rm -f $(DESTDIR)$(kde_icondir)/hicolor/16x16/apps/kmymoney2.png
	-rm -f $(DESTDIR)$(kde_icondir)/hicolor/32x32/apps/kmymoney2.png
	-rm -f $(DESTDIR)$(kde_icondir)/hicolor/64x64/apps/kmymoney2.png
	-rm -f $(DESTDIR)$(kde_icondir)/hicolor/128x128/apps/kmymoney2.png
	-rm -f $(DESTDIR)$(kde_datadir)/kmymoney2/tips

####### kdevelop will overwrite this part!!! (end)############
# this 10 paths are KDE specific. Use them:
# kde_htmldir       Where your docs should go to. (contains lang subdirs)
# kde_appsdir       Where your application file (.kdelnk) should go to.
# kde_icondir       Where your icon should go to.
# kde_minidir       Where your mini icon should go to.
# kde_datadir       Where you install application data. (Use a subdir)
# kde_locale        Where translation files should go to.(contains lang subdirs)
# kde_cgidir        Where cgi-bin executables should go to.
# kde_confdir       Where config files should go to.
# kde_mimedir       Where mimetypes should go to.
# kde_toolbardir    Where general toolbar icons should go to.
# kde_wallpaperdir  Where general wallpapers should go to.

DISTCLEANFILES=kmymoney2 *.html *.csv old.asc stripped.txt

# set the include path for X, qt and KDE
INCLUDES= $(all_includes) -I$(top_srcdir) -I. -I$(top_builddir)/kmymoney2/dialogs

METASOURCES = AUTO

# the library search path.
kmymoney2_LDFLAGS = $(all_libraries) $(kde_RPATH) -export-dynamic

# Uncomment the following two lines if you add a ui.rc file for your application to make use of
# KDE
rcdir = $(kde_datadir)/kmymoney2
rc_DATA = kmymoney2ui.rc

#WARNING: if you use a ui.rc file above, use:

# messages: rc.cpp

# instead of

# messages:

# Remove *.moc.* files from the list of files searched for messages.
# Don't know, if that has any negative effect, but it avoids to include
# automatically generated names which usually not need to be translated
# from clobbering the pot file. (ipwizard@user.sourceforge.net, 07/16/2003)
messages: rc.cpp
	if test -f tips; then \
	 $(PREPARETIPS) > _tips.cpp; \
	fi;
	LIST=`find . -name \*.h -o -name \*.hh -o -name \*.H -o -name \*.hxx -o \
	-name \*.hpp -o -name \*.cpp -o -name \*.cc -o -name \*.cxx -o -name \*.ecpp -o \
	-name \*.C | grep -v -E \*\.moc\.\*`; \
	if test -n "$$LIST"; then \
	 $(XGETTEXT) $$LIST -o $(podir)/kmymoney2.pot; \
	fi
	if test -f _tips.cpp; then \
	rm _tips.cpp; \
	fi

#
# since some people might think, the application is called kmymoney, we
# create a link to kmymoney2 after overall installation
#
install-exec-hook:
	@$(NORMAL_INSTALL)
	@list='kmymoney2'; for p in $$list; do \
	p1=`echo $$p|sed 's/$(EXEEXT)$$//'`; \
	if test -f $$p \
	|| test -f $$p1 \
	; then \
	f=`echo "$$p1" | sed 's,^.*/,,;$(transform);s/$$/$(EXEEXT)/'`; \
	echo "Creating symbolic link from kmymoney -> ./kmymoney2"; \
	rm -f $(DESTDIR)$(bindir)/kmymoney; \
	curdir=`pwd`; \
	cd $(DESTDIR)$(bindir); \
	ln -s ./$$f kmymoney; \
	cd $(curdir); \
	else :; fi; \
	done

uninstall-hook:
	-rm -f $(DESTDIR)$(bindir)/kmymoney

if CPPUNIT
TESTS = kmymoneytest 

check_PROGRAMS = kmymoneytest

kmymoneytest_SOURCES = kmymoneytest.cpp kmymoneyutils.cpp
kmymoneytest_LDADD = ./mymoney/storage/libstoragetest.a ./mymoney/libmymoneytest.a ./converter/libconvertertest.a ./reports/libreportstest.a $(kmymoney2_LDADD)

# the library search path.
kmymoneytest_LDFLAGS = $(all_libraries) $(kde_RPATH) $(CPPUNIT_LIBS)
endif

