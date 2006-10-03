/***************************************************************************
                             transactionform.h
                             ----------
    begin                : Sun May 14 2006
    copyright            : (C) 2006 by Thomas Baumgart
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

#ifndef TRANSACTIONFORM_H
#define TRANSACTIONFORM_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qtable.h>
#include <qvaluelist.h>
#include <qvaluevector.h>
#include <qpalette.h>
#include <qwidgetlist.h>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoney/mymoneyaccount.h>
#include <kmymoney/mymoneyobject.h>
#include <kmymoney/register.h>

#include "../kmymoneysettings.h"

class MyMoneyObjectContainer;

namespace KMyMoneyTransactionForm {

typedef enum {
  LabelColumn1 = 0,
  ValueColumn1,
  LabelColumn2,
  ValueColumn2,
  // insert new values above this line
  MaxColumns
} Column;

/**
  * @author Thomas Baumgart
  */
class TransactionForm : public TransactionEditorContainer
{
  Q_OBJECT
public:
  TransactionForm(QWidget *parent = 0, const char *name = 0);
  virtual ~TransactionForm() {}

  /**
    * Override the QTable member function to avoid display of focus
    */
  void paintFocus(QPainter* /*p*/, const QRect& /*cr*/ ) {}

  QSize tableSize(void) const;
  QSize sizeHint(void) const;
  void adjustColumn(Column col);
  void clear(void);

  void paintCell(QPainter* painter, int row, int col, const QRect& r, bool selected, const QColorGroup& cg);

  void resize(int col);

  void arrangeEditWidgets(QMap<QString, QWidget*>& editWidgets, KMyMoneyRegister::Transaction* t);
  void removeEditWidgets(void);
  void tabOrder(QWidgetList& tabOrderWidgets, KMyMoneyRegister::Transaction* t) const;

  /**
    * reimplemented to prevent normal cell selection behavior
    */
  void setCurrentCell(int, int) {}

protected:
  /**
    * reimplemented to prevent normal mouse press behavior
    */
  void contentsMousePressEvent(QMouseEvent* ev) { ev->ignore(); }

  /**
    * reimplemented to prevent normal mouse move behavior
    */
  void contentsMouseMoveEvent(QMouseEvent* ev) { ev->ignore(); }

  /**
    * reimplemented to prevent normal mouse release behavior
    */
  void contentsMouseReleaseEvent(QMouseEvent* ev) { ev->ignore(); }

  /**
    * reimplemented to prevent normal mouse double click behavior
    */
  void contentsMouseDoubleClickEvent(QMouseEvent* ev) { ev->ignore(); }

  /**
    * reimplemented to prevent normal keyboard behavior
    */
  void keyPressEvent(QKeyEvent* ev) { ev->ignore(); }

  /**
    * Override logic and use standard QFrame behaviour
    */
  bool focusNextPrevChild(bool next);

public slots:
  void slotSetTransaction(KMyMoneyRegister::Transaction* item);

protected slots:
  void resize(void);


protected:
  KMyMoneyRegister::Transaction*       m_transaction;
  int                                  m_rowHeight;
  QColorGroup                          m_cellColorGroup;
};


} // namespace

#endif
// vim:cin:si:ai:et:ts=2:sw=2:
