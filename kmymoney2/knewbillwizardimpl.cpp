/***************************************************************************
                          knewbillwizardimpl.cpp
                             -------------------
    copyright            : (C) 2000 by Michael Edwardes
    email                : Michael.Edwardes@students.dmu.ac.uk
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <knuminput.h>
#include <qpushbutton.h>
#include "knewbillwizardimpl.h"

KNewBillWizardImpl::KNewBillWizardImpl(QWidget *parent, const char *name, bool modal, WFlags f )
 : KNewBillWizard(parent,name,modal,f)
{
  page1_bill=true;
  page1_deposit=false;
  page1_transfer=false;
  page2_occurence = MyMoneyScheduled::OCCUR_MONTHLY;
  page3_method = MyMoneyScheduled::PAYMENT_DIRECTDEBIT;

  page2_occurenceCombo->insertItem(i18n("Daily"));
  page2_occurenceCombo->insertItem(i18n("Weekly"));
  page2_occurenceCombo->insertItem(i18n("Fortnightly"));
  page2_occurenceCombo->insertItem(i18n("Monthly"));
  page2_occurenceCombo->insertItem(i18n("Every 3 Months"));
  page2_occurenceCombo->insertItem(i18n("Every 4 Months"));
  page2_occurenceCombo->insertItem(i18n("Yearly"));

  page3_methodCombo->insertItem(i18n("A String to size"));

  page5_numberEdit->setRange(0, 999999999, 1, true);

  connect(page2_occurenceCombo, SIGNAL(activated(int)), this, SLOT(occurenceSelected(int)));
  connect(page3_methodCombo, SIGNAL(activated(int)), this, SLOT(paymentSelected(int)));
  connect(page1_groupBox, SIGNAL(clicked(int)), this, SLOT(page1GroupClicked(int)));
  connect(finishButton(), SIGNAL(clicked()), this, SLOT(slotFinishClicked()));
}

KNewBillWizardImpl::~KNewBillWizardImpl()
{
}

void KNewBillWizardImpl::showPage( QWidget* page )
{
  QWizard::showPage(page);

  if ( (strcmp(page->name(), "widget"))==0 ) {
    if (page1_bill) {
      page1_billRadio->setChecked(true);
      page1_billRadio->setFocus();
    }
    else if (page1_deposit) {
      page1_depositRadio->setChecked(true);
      page1_depositRadio->setFocus();
    }
    else if (page1_transfer) {
      page1_transferRadio->setChecked(true);
      page1_transferRadio->setFocus();
    }
  } else if ( (strcmp(page->name(), "widget_2"))==0 ) {
    page2_occurenceCombo->setCurrentItem(page2_occurence);
  } else if ( (strcmp(page->name(), "widget_3"))==0 ) {
    page3_methodCombo->clear();
    page3_methodCombo->insertItem(i18n("Direct Debit"));
    page3_methodCombo->insertItem(i18n("Other"));

    if (page1_deposit) {
      page3_methodCombo->insertItem(i18n("Direct Deposit"));
      page3_methodCombo->insertItem(i18n("Manual Deposit"));
    }
    else if (page1_bill) {
      page3_methodCombo->insertItem(i18n("Standing Order"));
      page3_methodCombo->insertItem(i18n("Write Cheque"));
    }
    else if (page1_transfer) {
      page3_methodCombo->insertItem(i18n("Standing Order"));
      page3_methodCombo->insertItem(i18n("Write Cheque"));
    }

    switch (page3_method) {
      case MyMoneyScheduled::PAYMENT_DIRECTDEBIT:
        page3_methodCombo->setCurrentItem(0);
        break;
      case MyMoneyScheduled::PAYMENT_OTHER:
        page3_methodCombo->setCurrentItem(1);
        break;
      case MyMoneyScheduled::PAYMENT_DIRECTDEPOSIT:
        page3_methodCombo->setCurrentItem(2);
        break;
      case MyMoneyScheduled::PAYMENT_MANUALDEPOSIT:
        page3_methodCombo->setCurrentItem(3);
        break;
      case MyMoneyScheduled::PAYMENT_STANDINGORDER:
        page3_methodCombo->setCurrentItem(2);
        break;
      case MyMoneyScheduled::PAYMENT_WRITECHEQUE:
        page3_methodCombo->setCurrentItem(3);
        break;
      default:
        page3_methodCombo->setCurrentItem(0);
        break;
    }
  } else if ( (strcmp(page->name(), "widget_4"))==0 ) {
  }
  else if ((strcmp(page->name(), "widget_5"))==0) {
    finishButton()->setEnabled( true );
    finishButton()->setFocus();
  }
}

void KNewBillWizardImpl::page1GroupClicked(int id)
{
  switch (id) {
    case 0:
      page1_bill=true;
      page1_deposit=false;
      page1_transfer=false;
      break;
    case 1:
      page1_bill=false;
      page1_deposit=true;
      page1_transfer=false;
      break;
    case 2:
      page1_bill=false;
      page1_deposit=false;
      page1_transfer=true;
      break;
  }
}

void KNewBillWizardImpl::occurenceSelected(int index)
{
  switch (index) {
    case 0:
      page2_occurence = MyMoneyScheduled::OCCUR_DAILY;
      break;
    case 1:
      page2_occurence = MyMoneyScheduled::OCCUR_WEEKLY;
      break;
    case 2:
      page2_occurence = MyMoneyScheduled::OCCUR_FORTNIGHTLY;
      break;
    case 3:
      page2_occurence = MyMoneyScheduled::OCCUR_MONTHLY;
      break;
    case 4:
      page2_occurence = MyMoneyScheduled::OCCUR_QUARTER;
      break;
    case 5:
      page2_occurence = MyMoneyScheduled::OCCUR_FOURMONTH;
      break;
    case 6:
      page2_occurence = MyMoneyScheduled::OCCUR_YEARLY;
      break;
  }
}

void KNewBillWizardImpl::paymentSelected(int index)
{
  if (index==0) {
    page3_method = MyMoneyScheduled::PAYMENT_DIRECTDEBIT;
    return;
  }
  else if (index==1) {
    page3_method = MyMoneyScheduled::PAYMENT_OTHER;
    return;
  }

  if (page1_deposit) {
    if (index==2) {
      page3_method = MyMoneyScheduled::PAYMENT_DIRECTDEPOSIT;
      return;
    }
    else if (index==3) {
      page3_method = MyMoneyScheduled::PAYMENT_MANUALDEPOSIT;
      return;
    }
  }
  else {
    if (index==2) {
      page3_method = MyMoneyScheduled::PAYMENT_STANDINGORDER;
      return;
    }
    else if (index==3) {
      page3_method = MyMoneyScheduled::PAYMENT_WRITECHEQUE;
      return;
    }
  }
}

void KNewBillWizardImpl::slotFinishClicked()
{
}

#include "knewbillwizardimpl.moc"
