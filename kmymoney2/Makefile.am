####### kdevelop will overwrite this part!!! (begin)##########
bin_PROGRAMS = kmymoney2
kmymoney2_SOURCES = kexportdlg.cpp kimportdlg.cpp ktransactiontableitem.cpp kexportdlgdecl.ui kimportdlgdecl.ui ktransactionviewdecl.ui kfindtransactiondlgdecl.ui kfileinfodlgdecl.ui knewfiledlgdecl.ui knewbankdlgdecl.ui knewaccountdlgdecl.ui knewcategorydlgdecl.ui kendingbalancedlgdecl.ui kscheduleviewdecl.ui kreconciledlgdecl.ui kpayeedlgdecl.ui knewbillwizard.ui kcategorydlgdecl.ui kbankviewdecl.ui ktransactionview.cpp ktfindresultsdlgdecl.ui ktfindresultsdlg.cpp kstartuplogo.cpp kstartdlg.cpp ksettingsdlg.cpp kscheduleview.cpp kschedulelistitem.cpp kreconcilelistitem.cpp kreconciledlg.cpp kpayeedlg.cpp knewfiledlg.cpp knewcategorydlg.cpp knewbillwizardimpl.cpp knewbankdlg.cpp knewaccountdlg.cpp kmymoneyview.cpp kmymoneysettings.cpp kmainview.cpp kfindtransactiondlg.cpp kfileinfodlg.cpp kendingbalancedlg.cpp kcategorylistitem.cpp kcategoriesdlg.cpp kbanksview.cpp kbanklistitem.cpp kmymoney2.cpp main.cpp 
kmymoney2_LDADD   = ./widgets/libwidgets.a ./mymoney/libmymoney.a ./dialogs/libdialogs.a  -lkfile -lkdeui -lkdecore $(LIB_QT) $(LIBSOCKET)

SUBDIRS = widgets mymoney pics dialogs icons 

EXTRA_DIST = main.cpp kmymoney2.cpp kmymoney2.h kmymoney2ui.rc kmymoney2.desktop kbanklistitem.cpp kbanklistitem.h kbanksview.cpp kbanksview.h kcategoriesdlg.cpp kcategoriesdlg.h kcategorylistitem.cpp kcategorylistitem.h kendingbalancedlg.cpp kendingbalancedlg.h kfileinfodlg.cpp kfileinfodlg.h kfindtransactiondlg.cpp kfindtransactiondlg.h kmainview.cpp kmainview.h kmymoneysettings.cpp kmymoneysettings.h kmymoneyview.cpp kmymoneyview.h knewaccountdlg.cpp knewaccountdlg.h knewbankdlg.cpp knewbankdlg.h knewbillwizardimpl.cpp knewbillwizardimpl.h knewcategorydlg.cpp knewcategorydlg.h knewfiledlg.cpp knewfiledlg.h kpayeedlg.cpp kreconciledlg.cpp kreconciledlg.h kreconcilelistitem.cpp kreconcilelistitem.h kschedulelistitem.cpp kschedulelistitem.h kscheduleview.cpp kscheduleview.h ksettingsdlg.cpp ksettingsdlg.h kstartdlg.cpp kstartdlg.h kstartuplogo.cpp kstartuplogo.h ktfindresultsdlg.cpp ktfindresultsdlg.h ktfindresultsdlgdecl.ui ktransactionview.cpp ktransactionview.h kbankviewdecl.ui kcategorydlgdecl.ui knewbillwizard.ui kpayeedlgdecl.ui kreconciledlgdecl.ui kscheduleviewdecl.ui default_categories.dat kendingbalancedlgdecl.ui knewcategorydlgdecl.ui knewaccountdlgdecl.ui knewbankdlgdecl.ui knewfiledlgdecl.ui kfileinfodlgdecl.ui kfindtransactiondlgdecl.ui ktransactionviewdecl.ui kimportdlgdecl.ui kexportdlgdecl.ui lo16-app-kmymoney2.png lo32-app-kmymoney2.png kmymoney_wizard.png mimetype_kmy_16x16.png mimetype_kmy_32x32.png mimetype_kmy_48x48.png x-kmymoney2.desktop kpayeedlg.h ktransactiontableitem.cpp ktransactiontableitem.h kimportdlg.cpp kimportdlg.h kexportdlg.cpp kexportdlg.h 

install-data-local:
	$(mkinstalldirs) $(kde_datadir)/kmymoney2/
	$(INSTALL_DATA) $(srcdir)/kmymoney2ui.rc $(kde_datadir)/kmymoney2/kmymoney2ui.rc
	$(mkinstalldirs) $(kde_appsdir)/Applications/
	$(INSTALL_DATA) $(srcdir)/kmymoney2.desktop $(kde_appsdir)/Applications/kmymoney2.desktop
	$(mkinstalldirs) $(kde_datadir)/kmymoney2/
	$(INSTALL_DATA) $(srcdir)/default_categories.dat $(kde_datadir)/kmymoney2/default_categories.dat
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

uninstall-local:
	-rm -f $(kde_datadir)/kmymoney2/kmymoney2ui.rc
	-rm -f $(kde_appsdir)/Applications/kmymoney2.desktop
	-rm -f $(kde_datadir)/kmymoney2/default_categories.dat
	-rm -f $(kde_icondir)/locolor/16x16/apps/kmymoney2.png
	-rm -f $(kde_icondir)/locolor/32x32/apps/kmymoney2.png
	-rm -f $(kde_icondir)/hicolor/16x16/mimetypes/kmy.png
	-rm -f $(kde_icondir)/hicolor/32x32/mimetypes/kmy.png
	-rm -f $(kde_icondir)/hicolor/48x48/mimetypes/kmy.png

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

# set the include path for X, qt and KDE
INCLUDES= $(all_includes)

METASOURCES = AUTO

# the library search path. 
kmymoney2_LDFLAGS = $(all_libraries) $(KDE_RPATH)

# Uncomment the following two lines if you add a ui.rc file for your application to make use of
# KDE
#rcdir = $(kde_datadir)/kmymoney2
#rc_DATA = kmymoney2ui.rc

#WARNING: if you use a ui.rc file above, use:

# messages: rc.cpp

# instead of 

# messages:

messages: 
	LIST=`find . -name \*.h -o -name \*.hh -o -name \*.H -o -name \*.hxx -o -name \*.hpp -o -name \*.cpp -o -name \*.cc -o -name \*.cxx -o -name \*.ecpp -o -name \*.C`; \
	if test -n "$$LIST"; then \
	 $(XGETTEXT) $$LIST -o $(podir)/kmymoney2.pot; \
	fi

