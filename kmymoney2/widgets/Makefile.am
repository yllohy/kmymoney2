KDE_OPTIONS = noautodist

INCLUDES = $(all_includes) -I$(top_srcdir) -I.

noinst_LIBRARIES = libwidgets.a
libwidgets_a_METASOURCES = AUTO

libwidgets_a_SOURCES = kmymoneyregisterinvestment.cpp kmymoneyequity.cpp kmymoneyequitycompletion.cpp kmymoneycompletion.cpp kmymoneyequityselector.cpp kmymoneyaccountcombo.cpp kmymoneyaccountcompletion.cpp kmymoneycurrencyselector.cpp kmymoneypriceview.cpp kmymoneypriceviewdecl.ui kschedulebriefwidget.ui kmymoneyaccountselector.cpp kmymoneyregisterloan.cpp kmymoneyregistersearch.cpp kmymoneybriefschedule.cpp kmymoneyscheduleddatetbl.cpp kmymoneydatetbl.cpp kmymoneyscheduledcalendar.cpp kmymoneycalendar.cpp kmymoneycalculator.cpp kmymoneycategory.cpp kmymoneypayee.cpp kmymoneytransactionform.cpp kmymoneyregistercheckings.cpp kmymoneyregister.cpp kmymoneysplittable.cpp kmymoneycombo.cpp kmymoneyhlayout.cpp kmymoneylineedit.cpp kmymoneyedit.cpp kmymoneydateinput.cpp kmymoneyreportcontroldecl.ui kmymoneyreportconfigtab1decl.ui kmymoneyreportconfigtab2decl.ui kmymoneyreportconfigtab3decl.ui kmymoneyonlinequoteconfig.cpp kmymoneyonlinequoteconfigdecl.ui kmymoneyaccounttree.cpp kmymoneygpgconfigdecl.ui kmymoneygpgconfig.cpp kmymoneytitlelabel.cpp

EXTRA_DIST = kmymoneygpgconfigdecl.ui kmymoneyonlinequoteconfigdecl.ui kmymoneypriceviewdecl.ui kmymoneyreportconfigtab1decl.ui kmymoneyreportconfigtab2decl.ui kmymoneyreportconfigtab3decl.ui kmymoneyreportcontroldecl.ui kschedulebriefwidget.ui

noinst_HEADERS = kmymoneyregisterinvestment.h kmymoneyequity.h kmymoneyequitycompletion.h kmymoneycompletion.h kmymoneyequityselector.h kmymoneyaccountcombo.h kmymoneyaccountcompletion.h kmymoneycurrencyselector.h kmymoneypriceview.h kmymoneyaccountselector.h kmymoneyregisterloan.h kmymoneyregistersearch.h kmymoneybriefschedule.h kmymoneyscheduleddatetbl.h kmymoneydatetbl.h kmymoneyscheduledcalendar.h kmymoneycalendar.h kmymoneycalculator.h kmymoneycategory.h kmymoneypayee.h kmymoneytransactionform.h kmymoneyregistercheckings.h kmymoneyregister.h kmymoneysplittable.h kmymoneycombo.h kmymoneyhlayout.h kmymoneylineedit.h kmymoneyedit.h kmymoneydateinput.h kmymoneyonlinequoteconfig.h kmymoneyaccounttree.h kmymoneygpgconfig.h kmymoneytitlelabel.h

# make sure, automatically generated files exist
BUILT_SOURCES = kupdatestockpricedlgdecl.h knewaccountdlgdecl.h knewequityentrydecl.h kcurrencycalculatordecl.h kequitypriceupdatedlgdecl.h

.PHONY: kupdatestockpricedlgdecl.h knewaccountdlgdecl.h knewequityentrydecl.h kcurrencycalculatordecl.h kequitypriceupdatedlgdecl.h

kupdatestockpricedlgdecl.h:
	make -C ../dialogs kupdatestockpricedlgdecl.h

knewaccountdlgdecl.h:
	make -C ../dialogs knewaccountdlgdecl.h

knewequityentrydecl.h:
	make -C ../dialogs knewequityentrydecl.h

kcurrencycalculatordecl.h:
	make -C ../dialogs kcurrencycalculatordecl.h

kequitypriceupdatedlgdecl.h:
	make -C ../dialogs kequitypriceupdatedlgdecl.h


# The below has been commented out for now. Once we add
# generation of the Qt designer library plugin for all
# our widgets, this stuff might become handy. I keep
# those referenced files on my local box for now
# (ipwizard@users.sourceforge.net - 2005/03/19)

# the Qt designer widget plugin
#all-local: libkmymoney.so

#Makefile.widgets:	kmymoneywidgets.pro
#	$(qt_libraries)/../bin/qmake -o Makefile.widgets kmymoneywidgets.pro

#kmymoneywidgets.cpp:	kmymoney.widgets makekdewidgets
#	./makekdewidgets -g KMyMoney -n CustomWidgetPlugin -o kmymoneywidgets.cpp kmymoney.widgets

#libkmymoney.so:	Makefile.widgets kmymoneywidgets.cpp $(libwidgets_a_OBJECTS)
#	make -f Makefile.widgets

#clean-local:
#	make -f Makefile.widgets clean

#install-exec-local: Makefile.widgets libkmymoney.so
#	make -f Makefile.widgets install

#uninstall-local: Makefile.widgets
#	make -f Makefile.widgets uninstall
