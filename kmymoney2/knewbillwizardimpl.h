/***************************************************************************
                          knewbillwizardimpl.h
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

#ifndef KNEWBILLWIZARDIMPL_H
#define KNEWBILLWIZARDIMPL_H

#include <klocale.h>

#include <qwidget.h>
#include <qradiobutton.h>
#include <kcombobox.h>
#include <qbuttongroup.h>

#include "./mymoney/mymoneyscheduled.h"
#include "knewbillwizard.h"

/**
  *@author Michael Edwardes
  */

class KNewBillWizardImpl : public KNewBillWizard  {
   Q_OBJECT
private:
  bool page1_bill;
  bool page1_deposit;
  bool page1_transfer;

  MyMoneyScheduled::occurenceE page2_occurence;
  MyMoneyScheduled::paymentTypeE page3_method;

protected slots:
  void occurenceSelected(int index);
  void paymentSelected(int index);
  void page1GroupClicked(int id);
  void slotFinishClicked();

public:
	KNewBillWizardImpl(QWidget *parent=0, const char *name=0, bool modal=true, WFlags f=0);
	~KNewBillWizardImpl();
  void showPage(QWidget* page);
};

#endif
