/***************************************************************************
                          kscheduleview.cpp
                             -------------------
    copyright            : (C) 2000 by Michael Edwardes
    email                : mte@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <kglobal.h>
#include <klocale.h>

#include "knewbillwizardimpl.h"

#include "kscheduleview.h"
#include "kmymoneysettings.h"
#include "mymoney/mymoneyscheduled.h"
#include "kschedulelistitem.h"

KScheduleView::KScheduleView(QWidget *parent, const char *name)
 : KScheduleViewDecl(parent,name)
{
	scheduledList->setRootIsDecorated(false);
	scheduledList->addColumn(i18n("Date"));
	scheduledList->addColumn(i18n("Bank"));
	scheduledList->addColumn(i18n("Account"));
	scheduledList->addColumn(i18n("Type"));
	scheduledList->addColumn(i18n("Occurence"));
	scheduledList->addColumn(i18n("Amount"));
	scheduledList->addColumn(i18n("Status"));
	scheduledList->setMultiSelection(false);
	
	connect(newBtn, SIGNAL(clicked()), this, SLOT(newBtnClicked()));
}

KScheduleView::~KScheduleView()
{
}

void KScheduleView::refresh(MyMoneyFile *file)
{
  scheduledList->clear();

//  QListIterator<MyMoneyBank> bankIt = file->bankIterator();
  MyMoneyBank *bank;
  for ( bank=file->bankFirst(); bank; bank=file->bankNext() ) {

//    QListIterator<MyMoneyAccount> accountIt = bank->accountIterator();
    MyMoneyAccount *account;
    for ( account=bank->accountFirst(); account; account=bank->accountNext() ) {

      QList<MyMoneyScheduled::s_scheduleData> list = account->scheduled().getScheduled();
      MyMoneyScheduled::s_scheduleData *scheduleData;
      for (scheduleData=list.first(); scheduleData!=0; scheduleData=list.next()) {
        (void) new KScheduleListItem(scheduledList, *scheduleData, bank->name(), account->name());
      }
    }
  }
}

void KScheduleView::newBtnClicked()
{
  KNewBillWizardImpl wizard(this, "NewBillWizard");
  wizard.setCaption(i18n("New Bill Wizard"));
  wizard.exec();
}
