/***************************************************************************
                          stdtransactiondownloaded.h
                             -------------------
    begin                : Sun May 11 2008
    copyright            : (C) 2008 by Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef STDTRANSACTIONDOWNLOADED_H
#define STDTRANSACTIONDOWNLOADED_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoney/transaction.h>

namespace KMyMoneyTransactionForm {
  class TransactionForm;
}; // namespace

namespace KMyMoneyRegister {

class StdTransactionDownloaded : public StdTransaction
{
public:
  StdTransactionDownloaded(Register* parent, const MyMoneyTransaction& transaction, const MyMoneySplit& split, int uniqueId);
  virtual ~StdTransactionDownloaded() {};

  virtual const char* className(void) { return "StdTransactionDownloaded"; }
#if 0
  virtual void paintRegisterCell(QPainter* painter, int row, int col, const QRect& r, bool selected, const QColorGroup& cg);

  bool formCellText(QString& txt, int& align, int row, int col, QPainter* painter = 0);
  void registerCellText(QString& txt, int& align, int row, int col, QPainter* painter = 0);

  int numColsForm(void) const { return 4; }

  void arrangeWidgetsInForm(QMap<QString, QWidget*>& editWidgets);
  void arrangeWidgetsInRegister(QMap<QString, QWidget*>& editWidgets);
  void tabOrderInForm(QWidgetList& tabOrderWidgets) const;
  void tabOrderInRegister(QWidgetList& tabOrderWidgets) const;

  int numRowsRegister(bool expanded) const;
#endif

  /**
    * Provided for internal reasons. No API change. See RegisterItem::numRowsRegister()
    */
  int numRowsRegister(void) const { return RegisterItem::numRowsRegister(); }
};

}; // namespace

#endif
// vim:cin:si:ai:et:ts=2:sw=2:

