/***************************************************************************
                          kgloballedgerview.h  -  description
                             -------------------
    begin                : Sat Jul 13 2002
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

#ifndef KACCOUNTVIEW_H
#define KACCOUNTVIEW_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qwidget.h>
#include <qwidgetstack.h>
#include <qstring.h>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "../mymoney/mymoneyutils.h"
#include "../mymoney/mymoneyaccount.h"
#include "kledgerview.h"

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class KComboBox;
class KTransactionView;

/**
  *@author Thomas Baumgart
  */

class KGlobalLedgerView : public QWidget
{
   Q_OBJECT
public:
	KGlobalLedgerView(QWidget *parent=0, const char *name=0);
	~KGlobalLedgerView();

  void show();

  void refreshView(void);

public slots:
  /**
    * Called when the user changes the visibility
    * setting of the transaction form
    */
  void slotShowTransactionForm(bool show);

protected:
  KComboBox* accountComboBox;
  QVBoxLayout* Form1Layout;
  QHBoxLayout* Layout2;

  void loadAccounts(void);
  void selectAccount(const QCString& id);

protected slots:
  void slotAccountSelected(const QString&);

private:
  void refresh(void);

  /**
    * This member holds the id of the currently selected account
    */
  QCString m_accountId;

  /**
    * m_specificView[] keeps pointers to the specific views for the
    * different accounts.
    * m_accountStack is the widget stack for them
    * m_currentView points to the current active view
    */
  KLedgerView* m_specificView[MyMoneyAccount::MaxAccountTypes];
  QWidgetStack* m_accountStack;
  KLedgerView* m_currentView;

signals:
  void signalViewActivated();

};

#endif
