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

// ----------------------------------------------------------------------------
// KDE Includes

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

  void setEditable(int row, int col, bool editable = true);
  void clearEditable(void);

  bool eventFilter( QObject * o, QEvent * );

public slots:
  virtual void setNumCols(int c);
  virtual void setNumRows(int r);

protected:
  bool focusNextPrevChild(bool next);

private:
  void resizeEditable(int r, int c);

private:
  QBitArray m_editable;
  KLedgerView*  m_view;
};

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
  int alignment() const;
  void setAlignment(alignmentTypeE align = standard);

private:
  alignmentTypeE m_alignment;
};

class kMyMoneyTransactionForm : public QWidget
{ 
  Q_OBJECT

public:
  kMyMoneyTransactionForm( KLedgerView* parent = 0, const char* name = 0, WFlags fl = 0, const int rows = 4, const int cols = 4 );
  ~kMyMoneyTransactionForm();

  void addTab(QTab *tab) { m_tabBar->addTab(tab); };
  kMyMoneyTransactionFormTable* table(void) { return formTable; };
  QTabBar* tabBar(void) { return m_tabBar; };
  KPushButton* newButton(void) const { return buttonNew; };
  KPushButton* editButton(void) const { return buttonEdit; };
  KPushButton* enterButton(void) const { return buttonEnter; };
  KPushButton* cancelButton(void) const { return buttonCancel; };
  KPushButton* moreButton(void) const { return buttonMore; };

protected:
  QTabBar* m_tabBar;
  QFrame* formFrame;
  kMyMoneyTransactionFormTable* formTable;

  KPushButton* buttonNew;
  KPushButton* buttonEdit;
  KPushButton* buttonEnter;
  KPushButton* buttonCancel;
  KPushButton* buttonMore;

  KLedgerView*  m_view;

protected:
  QVBoxLayout* formLayout;
  QHBoxLayout* buttonLayout;
  QGridLayout* formFrameLayout;
};

#endif // KMYMONEYTRANSACTIONFORM_H
