#!/bin/sh
cd kmymoney2
# First make sure that all the user interfaces
# are built
make kbankviewdecl.cpp
make kcategorydlgdecl.cpp
make kendingbalancedlgdecl.cpp
make klistsettingsdlgdecl.cpp
make knewaccountdlgdecl.cpp
make knewbankdlgdecl.cpp
make knewcategorydlgdecl.cpp
make knewfiledlgdecl.cpp
make kpayeedlgdecl.cpp
make kreconciledlgdecl.cpp
make kscheduleviewdecl.cpp
make ksettingsdlgdecl.cpp
make ktfindresultsdlgdecl.cpp
make kfileinfodlgdecl.cpp
make kfindtransactiondlgdecl.cpp
make knewbillwizard.cpp
make ktransactionviewdecl.cpp

# Make sure that the user has installed designer properly
# Mandrake 7.2 doesn't !!!
for i in *decl*.h knewbillwizard.h; do sed s/"class KComboBox;"/"#include <kcombobox.h>"/ $i > $i.tmp; mv $i.tmp $i; done;
for i in *decl*.h knewbillwizard.h; do sed s/"class KListView;"/"#include <klistview.h>"/ $i > $i.tmp; mv $i.tmp $i; done;
for i in *decl*.h knewbillwizard.h; do sed s/"class KColorButton;"/"#include <kcolorbutton.h>"/ $i > $i.tmp; mv $i.tmp $i; done;
for i in *decl*.h knewbillwizard.h; do sed s/"class KIconView;"/"#include <kiconview.h>"/ $i > $i.tmp; mv $i.tmp $i; done;
for i in *decl*.h knewbillwizard.h; do sed s/"class KIntNumInput;"/"#include <knuminput.h>"/ $i > $i.tmp; mv $i.tmp $i; done;
cd ..
