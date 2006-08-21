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

class QResizeEvent;

#include "../dialogs/kmergetransactionsdlgdecl.h"
#include "../widgets/kmymoneyregistersearch.h"

class KMergeTransactionsDlg: public KMergeTransactionsDlgDecl, public IMyMoneyRegisterParent
{
  Q_OBJECT
public:
  KMergeTransactionsDlg(QCString _accountid);

  /**
   * Adds this transaction to the dialog
   */
  void addTransaction(const QCString& id);

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
  virtual KMyMoneyTransaction* transaction(const int idx) const { return &(m_transactionList[idx]); }

  /**
    * This method is used by the IMyMoneyRegisterParent interface.  It is not
    * really needed, and should be removed as a required member of the
    * interface.
    *
    * It returns the balance of any visible transaction
    * in the ledger of this account. The balance depends on filters
    * and is automatically calculated when any view option is changed
    * (e.g. filters, sort order, etc.)
    *
    * @param idx index into the ledger starting at 0
    * @return Value of the balance for the account after the selected
    *         transaction has been taken into account. If idx is out
    *         of bounds, 0 will be returned as value. For liability type
    *         accounts, the sign will be inverted for display purposes.
    */
  virtual const MyMoneyMoney balance(const int) const { return 0; }

  /**
    * This method is used by the IMyMoneyRegisterParent interface.
    */
  virtual bool focusNextPrevChild(bool next) { return KMergeTransactionsDlgDecl::focusNextPrevChild(next); }

  void show(void);

public slots:
  void slotHelp();

protected:
  void resizeEvent(QResizeEvent* ev);
  void resizeRegister(void);

private:
  /**
   * The list of transactions which are to be displayed by the dialog
   */
  mutable QValueList<KMyMoneyTransaction> m_transactionList;

  /**
   * The ID of the account in which the transactions are displayed
   */
  QCString m_displayaccountid;
};

#endif // KMERGETRANSACTIONSDLG_H
// vim:cin:si:ai:et:ts=2:sw=2:
