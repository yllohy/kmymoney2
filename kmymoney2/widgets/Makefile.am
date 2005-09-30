KDE_OPTIONS = noautodist

INCLUDES = $(all_includes) -I$(top_srcdir) -I.

noinst_LIBRARIES = libwidgets.a
libwidgets_a_METASOURCES = AUTO

libwidgets_a_SOURCES = kmymoneyregisterinvestment.cpp kmymoneyequity.cpp kmymoneyequitycompletion.cpp kmymoneycompletion.cpp kmymoneyequityselector.cpp kmymoneyaccountcombo.cpp kmymoneyaccountcompletion.cpp kmymoneycurrencyselector.cpp kmymoneypriceview.cpp kmymoneypriceviewdecl.ui kschedulebriefwidget.ui kmymoneyaccountselector.cpp kmymoneyregisterloan.cpp kmymoneyregistersearch.cpp kmymoneybriefschedule.cpp kmymoneyscheduleddatetbl.cpp kmymoneydatetbl.cpp kmymoneyscheduledcalendar.cpp kmymoneycalendar.cpp kmymoneycalculator.cpp kmymoneycategory.cpp kmymoneypayee.cpp kmymoneytransactionform.cpp kmymoneyregistercheckings.cpp kmymoneyregister.cpp kmymoneysplittable.cpp kmymoneycombo.cpp kmymoneyhlayout.cpp kmymoneylineedit.cpp kmymoneyedit.cpp kmymoneydateinput.cpp kmymoneyreportcontroldecl.ui kmymoneyreportconfigtab1decl.ui kmymoneyreportconfigtab2decl.ui kmymoneyreportconfigtab3decl.ui kmymoneyreportconfigtabchartdecl.ui kmymoneyonlinequoteconfig.cpp kmymoneyonlinequoteconfigdecl.ui kmymoneyaccounttree.cpp kmymoneygpgconfigdecl.ui kmymoneygpgconfig.cpp kmymoneytitlelabel.cpp

EXTRA_DIST = kmymoneygpgconfigdecl.ui kmymoneyonlinequoteconfigdecl.ui kmymoneypriceviewdecl.ui kmymoneyreportconfigtab1decl.ui kmymoneyreportconfigtab2decl.ui kmymoneyreportconfigtab3decl.ui kmymoneyreportcontroldecl.ui kschedulebriefwidget.ui kmymoneyreportconfigtabchartdecl.ui

# include the widgets that are available to 3rd party sw (e.g. plugins or
# Qt designer) here
instdir=$(includedir)/kmymoney
inst_HEADERS = kmymoneydateinput.h kmymoneyedit.h kmymoneytitlelabel.h kmymoneypayee.h

# include the widgets that are only available to KMyMoney here
noinst_HEADERS = kmymoneyregisterinvestment.h kmymoneyequity.h kmymoneyequitycompletion.h kmymoneycompletion.h kmymoneyequityselector.h kmymoneyaccountcombo.h kmymoneyaccountcompletion.h kmymoneycurrencyselector.h kmymoneypriceview.h kmymoneyaccountselector.h kmymoneyregisterloan.h kmymoneyregistersearch.h kmymoneybriefschedule.h kmymoneyscheduleddatetbl.h kmymoneydatetbl.h kmymoneyscheduledcalendar.h kmymoneycalendar.h kmymoneycalculator.h kmymoneycategory.h kmymoneytransactionform.h kmymoneyregistercheckings.h kmymoneyregister.h kmymoneysplittable.h kmymoneycombo.h kmymoneyhlayout.h kmymoneylineedit.h kmymoneyonlinequoteconfig.h kmymoneyaccounttree.h kmymoneygpgconfig.h 

# make sure, automatically generated files exist
BUILT_SOURCES = kupdatestockpricedlgdecl.h knewaccountdlgdecl.h knewequityentrydecl.h kcurrencycalculatordecl.h kequitypriceupdatedlgdecl.h $(WIDGET_MOCS)

.PHONY: kupdatestockpricedlgdecl.h knewaccountdlgdecl.h knewequityentrydecl.h kcurrencycalculatordecl.h kequitypriceupdatedlgdecl.h

kupdatestockpricedlgdecl.h:
	$(MAKE) -C ../dialogs kupdatestockpricedlgdecl.h

knewaccountdlgdecl.h:
	$(MAKE) -C ../dialogs knewaccountdlgdecl.h

knewequityentrydecl.h:
	$(MAKE) -C ../dialogs knewequityentrydecl.h

kcurrencycalculatordecl.h:
	$(MAKE) -C ../dialogs kcurrencycalculatordecl.h

kequitypriceupdatedlgdecl.h:
	$(MAKE) -C ../dialogs kequitypriceupdatedlgdecl.h

PICS_DIR=kmymoney2/pics
WIDGET_PNGS = kmymoneytitlelabel.png

WIDGET_CPPS=kmymoneytitlelabel.cpp kmymoneyedit.cpp kmymoneycalculator.cpp kmymoneydateinput.cpp kmymoneypayee.cpp
WIDGET_MOCS=kmymoneytitlelabel.moc kmymoneyedit.moc kmymoneycalculator.moc kmymoneydateinput.moc kmymoneypayee.moc
libkmymoney_la_SOURCES = kmmwidgets.cpp $(WIDGET_CPPS)

noinst_LTLIBRARIES = libkmymoney.la

kmmwidgets.cpp:	kmymoney.widgets
	$(srcdir)/makekdewidgets -g KMyMoney -n CustomWidgetPlugin -o kmmwidgets.cpp -i kmymoney $(srcdir)/kmymoney.widgets

# make sure to compile the Qt designer version w/o the memory leak checker 
# also we need a copy of the mymoneymoney.lo file which has been compiled
# already. This is a quick hack around a bootstrap problem. We could link
# the libkmymoney.so against libkmm_mymoney.so but this is not yet installed
# but required to let UIC generate the right code for the dialogs and views.
# Hence the quick hack to include the required objects from the
# libkmm_mymoney.so directly into libkmymoney.so
libkmymoney_la_CXXFLAGS = -U_CHECK_MEMORY
libkmymoney_la_LDFLAGS = `ls ../mymoney/*.lo` $(KDE_LDFLAGS) -rpath $(DESTDIR)$(qt_libraries)/../plugins/designer $(LIB_KDEUI) $(LIB_KDECORE) $(LIB_QT)

if INSTALL_QTDESIGNER_SUPPORT
install-exec-local: libkmymoney.la
	$(mkinstalldirs) $(DESTDIR)$(qt_libraries)/../plugins/designer
	$(INSTALL_PROGRAM) .libs/libkmymoney.so $(DESTDIR)$(qt_libraries)/../plugins/designer 
	$(mkinstalldirs) $(DESTDIR)$(kde_datadir)/$(PICS_DIR)
	for file in $(WIDGET_PNGS); do \
	  $(INSTALL_DATA) $(srcdir)/$$file $(DESTDIR)$(kde_datadir)/$(PICS_DIR); \
	done

uninstall-local:
	-rm $(DESTDIR)$(qt_libraries)/../plugins/designer/libkmymoney.so
	for file in $(WIDGET_PNGS); do \
	  rm -rf $(DESTDIR)$(kde_datadir)/$(PICS_DIR)/$$file; \
	done
endif
