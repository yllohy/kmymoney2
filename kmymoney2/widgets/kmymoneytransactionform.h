/***************************************************************************
                          kmymoneytransactionform.h  -  description
                             -------------------
    begin                : Thu Jul 25 2002
    copyright            : (C) 2002 by Michael Edwardes
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

#ifndef KMYMONEYTRANSACTIONFORM_H
#define KMYMONEYTRANSACTIONFORM_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qwidget.h>
#include <qtabbar.h>
#include <qlabel.h>
#include <qtable.h>
#include <qbitarray.h>
#include <qtoolbutton.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <ktoolbarbutton.h>

// ----------------------------------------------------------------------------
// Project Includes

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class KPushButton;
class QFrame;
class QTable;
class KLedgerView;

/**
  *@author Thomas Baumgart
  */

class kMyMoneyTransactionFormTable : public QTable
{
  Q_OBJECT

public:
  kMyMoneyTransactionFormTable( KLedgerView* view, QWidget* parent = 0, const char* name = 0);
  ~kMyMoneyTransactionFormTable() {};

  void paintFocus(QPainter *p, const QRect &cr);
  QWidget* createEditor(int row, int col, bool initFromCell) const;

  void setEditable(const int row, const int col, const bool editable = true);

  void clearEditable(void);

  bool eventFilter( QObject * o, QEvent * );

  QSize sizeHint() const;

  void adjustColumn(const int col, const int minWidth = 0);

  void setNumRows(const int r, const int rowHeight);

public slots:
  virtual void setNumCols(int c);

protected:
  bool focusNextPrevChild(bool next);

private:
  void resizeEditable(int r, int c);
  QSize tableSize(void) const;

private:
  QBitArray m_editable;
  KLedgerView*  m_view;
};

/**
  * The class kMyMoneyTransactionFormTableItem is a specialized version
  * of the QTableItem base class that allows control of the alignment.
  * Within the base class, all numeric values will be aligned to the left,
  * all other values will be aligned to the right. In order to control
  * the alignment, this class is provided. Use it like the standard QTableItem
  * class and additionally alignment() and setAlignment() to control
  * this attribute.
  */
class kMyMoneyTransactionFormTableItem : public QTableItem
{
public:
  enum alignmentTypeE {
    standard = 0,
    left,
    right
  };

  kMyMoneyTransactionFormTableItem(QTable* tbl, EditType ed, const QString& str);
  ~kMyMoneyTransactionFormTableItem();

  /**
    * Returns the alignment setting of the current object.
    * @return alignmentTypeE of the current setting
    */
  int alignment() const;

  /**
    * This method is used to set the alignment value for this object.
    *
    * @param align alignment type
    */
  void setAlignment(alignmentTypeE align = standard);

private:
  alignmentTypeE m_alignment;
};

class kMyMoneyTransactionForm : public QWidget
{
  Q_OBJECT

public:
  kMyMoneyTransactionForm( KLedgerView* parent = 0, const char* name = 0, WFlags fl = 0, const int rows = 4, const int cols = 4 , const int rowHeight = 25);
  ~kMyMoneyTransactionForm();

  void addTab(QTab *tab) { m_tabBar->addTab(tab); };
  kMyMoneyTransactionFormTable* table(void) { return formTable; };
  QTabBar* tabBar(void) { return m_tabBar; };
  KToolBarButton* newButton(void) const { return buttonNew; };
  KToolBarButton* editButton(void) const { return buttonEdit; };
  KToolBarButton* enterButton(void) const { return buttonEnter; };
  KToolBarButton* cancelButton(void) const { return buttonCancel; };
  KToolBarButton* moreButton(void) const { return buttonMore; };
  KToolBarButton* accountButton(void) const { return buttonAccount; };

protected:
  QTabBar* m_tabBar;
  QFrame* formFrame;
  kMyMoneyTransactionFormTable* formTable;
  KToolBar* m_toolbar;
  KToolBarButton* buttonNew;
  KToolBarButton* buttonEdit;
  KToolBarButton* buttonEnter;
  KToolBarButton* buttonCancel;
  KToolBarButton* buttonMore;
  KToolBarButton* buttonAccount;

  KLedgerView*  m_view;

protected:
  QVBoxLayout* formLayout;
  QHBoxLayout* buttonLayout;
  QGridLayout* formFrameLayout;
};

#endif // KMYMONEYTRANSACTIONFORM_H
