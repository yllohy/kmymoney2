/***************************************************************************
                          ksplittransactiondlg.h  -  description
                             -------------------
    begin                : Thu Jan 10 2002
    copyright            : (C) 2000-2002 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KSPLITTRANSACTIONDLG_H
#define KSPLITTRANSACTIONDLG_H

#include "ksplittransactiondlgdecl.h"
#include "../mymoney/mymoneymoney.h"
#include "../widgets/kmymoneycombo.h"
#include "../widgets/kmymoneyedit.h"
#include "../widgets/kmymoneylineedit.h"

/**
  *@author Thomas Baumgart
  */

class KSplitTransactionDlg : public kSplitTransactionDlgDecl  {
  Q_OBJECT

public: 
	KSplitTransactionDlg( QWidget* parent,  const char* name, MyMoneyMoney* amount, const bool amountValid = false);
	~KSplitTransactionDlg();


protected:
  void resizeEvent(QResizeEvent*);

private:
  // Setup initial width for the amount fields
  void initAmountWidth(void);

  // Update the display of the sums
  void updateSums(void);

  // create input widgets
  void createInputWidgets(void);

  // called upon mouse click, to see where to set the focus
  void slotFocusChange(int row, int col, int button, const QPoint & mousePos);

protected slots:
  void slotFinishClicked();
  void slotCancelClicked();
  void slotClearAllClicked();

private slots:
  // used internally to setup the initial size of all widgets
  void initSize(void);

private:

  // keeps the actual width required for the amount field
  int   m_amountWidth;

  // the sum of all splits
  MyMoneyMoney  m_amountSplits;

  // the initial amount entered into the transaction register
  MyMoneyMoney* m_amountTransaction;

  // flag if an amount for the transaction was specified
  bool          m_amountValid;

  // the number of table rows that are filled with real data
  int   m_numSplits;

  // the number of table rows that are required to fill
  // the widget if less splits are entered. This number is adjusted
  // during resize.
  int   m_numExtraLines;

  // pointer input widget for category. the widget will be
  // created in createInputWidgets()
  kMyMoneyCombo* m_category;

  // pointer to input widget for memo. the widget will be
  // created in createInputWidgets()
  kMyMoneyLineEdit* m_memo;

  // pointer to input widget for amount. the widget will be
  // created in createInputWidgets()
  kMyMoneyEdit*  m_amount;

};

#endif
