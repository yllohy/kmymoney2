/***************************************************************************
                          kmergetransactionsdlg.h
                             -------------------
    begin                : Sun Aug 20 2006
    copyright            : (C) 2006 by Ace Jones
    email                : <acejones@users.sf.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KMERGETRANSACTIONSDLG_H
#define KMERGETRANSACTIONSDLG_H

// ----------------------------------------------------------------------------
// QT Includes

class QResizeEvent;

// ----------------------------------------------------------------------------
// KDE Includes

#include <kdebug.h>

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoney/register.h>
#include <kmymoney/mymoneyaccount.h>
#include <kmymoney/mymoneyobjectcontainer.h>

#include "../dialogs/kselecttransactionsdlgdecl.h"

class KMergeTransactionsDlg: public KSelectTransactionsDlgDecl
{
  Q_OBJECT
public:
  KMergeTransactionsDlg(const MyMoneyAccount& account, QWidget* parent = 0, const char* name = 0);

  /**
   * Adds the transaction @a t to the dialog
   */
  void addTransaction(const MyMoneyTransaction& t);
#if 0
  /**
    * This method is used by the IMyMoneyRegisterParent interface.  It should
    * accept a CONST pointer as a return value, if the function itself is
    * going to be CONST!!!
    *
    * It returns a pointer to the transaction data
    * in the ledger of this account. The transaction is identified
    * by the parameter @p idx.
    *
    * @param idx index into ledger starting at 0
    * @return pointer to MyMoneyTransaction object representing the
    *         selected transaction. If idx is out of bounds,
    *         0 will be returned.
    */
  virtual KMyMoneyTransaction* transaction(const int idx) const
  {
    unsigned uidx = static_cast<unsigned>(idx);
    if ( uidx >= m_transactionList.size() )
      return NULL;
    else
      return &(m_transactionList[uidx]);
  }
#endif
  int exec(void);
  void show(void);

public slots:
  void slotHelp();

protected:
  void resizeEvent(QResizeEvent* ev);
  // void resizeRegister(void);

private:
  MyMoneyObjectContainer    m_objects;
  /**
   * The list of transactions which are to be displayed by the dialog
   */
  // mutable QValueList<KMyMoneyTransaction> m_transactionList;

  /**
    * The account in which the transactions are displayed
    */
  MyMoneyAccount m_account;
};

#endif // KMERGETRANSACTIONSDLG_H
// vim:cin:si:ai:et:ts=2:sw=2:
