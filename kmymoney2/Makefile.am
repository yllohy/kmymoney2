# SEE BELOW BECAUSE I OVERRIDE LDADD because of the bug

####### kdevelop will overwrite this part!!! (begin)##########
bin_PROGRAMS = kmymoney2

## INCLUDES were found outside kdevelop specific part

kmymoney2_SOURCES = kmymoneyutils.cpp kstartuplogo.cpp kmymoney2.cpp main.cpp 
kmymoney2_LDADD   = ./views/libviews.a ./converter/libconverter.a ./dialogs/libdialogs.a ./widgets/libwidgets.a ./mymoney/storage/libstorage.a ./mymoney/libmymoney.a  $(LIB_KHTML) $(LIB_KDEUI) $(LIB_KDECORE) $(LIB_QT) $(LIBSOCKET)

SUBDIRS = widgets dialogs views converter mymoney pics icons html 

EXTRA_DIST = main.cpp kmymoney2.cpp kmymoney2.h kmymoney2ui.rc kmymoney2.desktop kstartuplogo.cpp kstartuplogo.h lo16-app-kmymoney2.png lo32-app-kmymoney2.png kmymoney_wizard.png mimetype_kmy_16x16.png mimetype_kmy_32x32.png mimetype_kmy_48x48.png x-kmymoney2.desktop hi16-app-kmymoney2.png hi32-app-kmymoney2.png tips default_accounts_de.dat kmymoneyutils.cpp kmymoneyutils.h default_accounts_enGB.dat comptes_par_defaut_fr.dat default_accounts_enC.dat 

install-data-local:
	$(mkinstalldirs) $(kde_datadir)/kmymoney2/
	$(INSTALL_DATA) $(srcdir)/kmymoney2ui.rc $(kde_datadir)/kmymoney2/kmymoney2ui.rc
	$(mkinstalldirs) $(kde_appsdir)/Applications/
	$(INSTALL_DATA) $(srcdir)/kmymoney2.desktop $(kde_appsdir)/Applications/kmymoney2.desktop
	$(mkinstalldirs) $(kde_icondir)/locolor/16x16/apps/
	$(INSTALL_DATA) $(srcdir)/lo16-app-kmymoney2.png $(kde_icondir)/locolor/16x16/apps/kmymoney2.png
	$(mkinstalldirs) $(kde_icondir)/locolor/32x32/apps/
	$(INSTALL_DATA) $(srcdir)/lo32-app-kmymoney2.png $(kde_icondir)/locolor/32x32/apps/kmymoney2.png
	$(mkinstalldirs) $(kde_icondir)/hicolor/16x16/mimetypes/
	$(INSTALL_DATA) $(srcdir)/mimetype_kmy_16x16.png $(kde_icondir)/hicolor/16x16/mimetypes/kmy.png
	$(mkinstalldirs) $(kde_icondir)/hicolor/32x32/mimetypes/
	$(INSTALL_DATA) $(srcdir)/mimetype_kmy_32x32.png $(kde_icondir)/hicolor/32x32/mimetypes/kmy.png
	$(mkinstalldirs) $(kde_icondir)/hicolor/48x48/mimetypes/
	$(INSTALL_DATA) $(srcdir)/mimetype_kmy_48x48.png $(kde_icondir)/hicolor/48x48/mimetypes/kmy.png
	$(mkinstalldirs) $(kde_icondir)/hicolor/16x16/apps/
	$(INSTALL_DATA) $(srcdir)/hi16-app-kmymoney2.png $(kde_icondir)/hicolor/16x16/apps/kmymoney2.png
	$(mkinstalldirs) $(kde_icondir)/hicolor/32x32/apps/
	$(INSTALL_DATA) $(srcdir)/hi32-app-kmymoney2.png $(kde_icondir)/hicolor/32x32/apps/kmymoney2.png
	$(mkinstalldirs) $(kde_datadir)/kmymoney2/
	$(INSTALL_DATA) $(srcdir)/tips $(kde_datadir)/kmymoney2/tips
	$(mkinstalldirs) $(kde_datadir)/kmymoney2/
	$(INSTALL_DATA) $(srcdir)/default_accounts_de.dat $(kde_datadir)/kmymoney2/default_accounts_de.dat
	$(mkinstalldirs) $(kde_datadir)/kmymoney2/
	$(INSTALL_DATA) $(srcdir)/default_accounts_enGB.dat $(kde_datadir)/kmymoney2/default_accounts_enGB.dat
	$(mkinstalldirs) $(kde_datadir)/kmymoney2/
	$(INSTALL_DATA) $(srcdir)/comptes_par_defaut_fr.dat $(kde_datadir)/kmymoney2/comptes_par_defaut_fr.dat
	$(mkinstalldirs) $(kde_datadir)/kmymoney2/
	$(INSTALL_DATA) $(srcdir)/default_accounts_enC.dat $(kde_datadir)/kmymoney2/default_accounts_enC.dat

uninstall-local:
	-rm -f $(kde_datadir)/kmymoney2/kmymoney2ui.rc
	-rm -f $(kde_appsdir)/Applications/kmymoney2.desktop
	-rm -f $(kde_icondir)/locolor/16x16/apps/kmymoney2.png
	-rm -f $(kde_icondir)/locolor/32x32/apps/kmymoney2.png
	-rm -f $(kde_icondir)/hicolor/16x16/mimetypes/kmy.png
	-rm -f $(kde_icondir)/hicolor/32x32/mimetypes/kmy.png
	-rm -f $(kde_icondir)/hicolor/48x48/mimetypes/kmy.png
	-rm -f $(kde_icondir)/hicolor/16x16/apps/kmymoney2.png
	-rm -f $(kde_icondir)/hicolor/32x32/apps/kmymoney2.png
	-rm -f $(kde_datadir)/kmymoney2/tips
	-rm -f $(kde_datadir)/kmymoney2/default_accounts_de.dat
	-rm -f $(kde_datadir)/kmymoney2/default_accounts_enGB.dat
	-rm -f $(kde_datadir)/kmymoney2/comptes_par_defaut_fr.dat
	-rm -f $(kde_datadir)/kmymoney2/default_accounts_enC.dat

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

DISTCLEANFILES=kmymoney2

# set the include path for X, qt and KDE
INCLUDES= $(all_includes)

METASOURCES = AUTO

# the library search path.
kmymoney2_LDFLAGS = $(all_libraries) $(kde_RPATH)

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
	LIST=`find . -name \*.h -o -name \*.hh -o -name \*.H -o -name \*.hxx -o \
	-name \*.hpp -o -name \*.cpp -o -name \*.cc -o -name \*.cxx -o -name \*.ecpp -o \
	-name \*.C | grep -v -E \*\.moc\.\*`; \
	if test -n "$$LIST"; then \
	 $(XGETTEXT) $$LIST -o $(podir)/kmymoney2.pot; \
	fi
