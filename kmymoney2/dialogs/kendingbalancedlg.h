/***************************************************************************
                          kendingbalancedlg.h
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

#ifndef KENDINGBALANCEDLG_H
#define KENDINGBALANCEDLG_H

//Generated area. DO NOT EDIT!!!(begin)
#include <qwidget.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
//Generated area. DO NOT EDIT!!!(end)

#include <klocale.h>
#include <qdialog.h>
#include <qdatetime.h>
#include "../widgets/kmymoneyedit.h"
#include "../mymoney/mymoneymoney.h"
#include "../widgets/kmymoneydateinput.h"

#include "kendingbalancedlgdecl.h"

// This dialog lets the user selected an ending balance.
// It is designed to be used in conjunction with KReconcileDlg.
class KEndingBalanceDlg : public KEndingBalanceDlgDecl  {
   Q_OBJECT

public:
  // Read from these to get the value when the dialog has finished.
  MyMoneyMoney endingBalance;
  MyMoneyMoney previousBalance;
  QDate endingDate;

public:
	KEndingBalanceDlg(MyMoneyMoney& prevBal, MyMoneyMoney& endingGuess, QWidget *parent=0, const char *name=0);
	~KEndingBalanceDlg();

protected: 
/*
	void initDialog();
	//Generated area. DO NOT EDIT!!!(begin)
	QGroupBox *reconcileFrame;
	QLabel *previousLabel;
	QLabel *previousEdit;
	QLabel *endingLabel;
	QLabel *dateLabel;
	QPushButton *okBtn;
	QPushButton *cancelBtn;
	//Generated area. DO NOT EDIT!!!(end)
*/
protected slots:
  void okClicked();

private:
//  kMyMoneyDateInput *endingDateEdit;
//  kMyMoneyEdit *endingEdit;
};

#endif
