/***************************************************************************
                          kmymoneyregister.h  -  description
                             -------------------
    begin                : Sun Jul 14 2002
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

#ifndef KMYMONEYREGISTER_H
#define KMYMONEYREGISTER_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qwidget.h>
#include <qpainter.h>
#include <qtable.h>

// #include <qfontmetric.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kglobal.h>
#include <kconfig.h>

// ----------------------------------------------------------------------------
// Project Includes

class MyMoneyTransaction;
class KLedgerView;
#include "../mymoney/mymoneymoney.h"
#include "../mymoney/mymoneysplit.h"

/**
  *@author Thomas Baumgart
  */

class KLedgerView;

class kMyMoneyRegister : public QTable
{
  Q_OBJECT

  friend class QHeader;
  friend class QTableHeader;

public:
	kMyMoneyRegister(int maxRpt, QWidget *parent=0, const char *name=0);
	virtual ~kMyMoneyRegister();

  void paintFocus(QPainter* p, const QRect& cr );

  void setView(KLedgerView *view) { m_view = view; };

  /**
    * read setting of configuration from config file
    */
  void readConfig(void);

  /**
    * This method allows a view to turn on or off the ledger lens feature.
    * Per default, the transaction lens is enabled in the constructor of this
    * class.
    *
    * @param enabled If true, the transaction lens feature is turned on, if
    *                false, the transaction lens feature is turned off.
    *
    * @note The widget is not redrawn when the setting is changed. This is left
    *       to the user of this widget.
    */
  void setLedgerLens(const bool enabled);

  void clearCell(int row, int col) {} ;
  void setItem(int row, int col, QTableItem *) {};
  QTableItem* item(int row, int col) const  { return NULL; };

  void clearCellWidget(int row, int col);
  QWidget* cellWidget(int row, int col) const;
  QWidget* createEditor(int row, int col, bool initFromCell) const;

  /**
    * Return the number of table rows displayed per transaction
    *
    * @return int number of rows per transaction
    */
  int rpt(void) const { return m_rpt; };

  /**
    * This method is used to return the index of the currently selected
    * transaction.
    *
    * @return index as int of currently selected transaction.
    */
  int currentTransactionIndex(void) const { return m_currentTransactionIndex; };

  /**
    * This method is used to move the selection bar to a specific
    * transaction in the register.
    *
    * @param idx index into transaction table
    * @return true if a different transaction is selected,
    *         false otherwise.
    */
  bool setCurrentTransactionIndex(int idx);

  /**
    * This method is used to move the selection bar to a specific
    * transaction defined by @p row. Unlike setCurrentTransactionIndex
    * it respects the setting of the transaction lens to select
    * the transaction.
    *
    * @param row The row of the table that should be selected
    * @return true if a different transaction is selected,
    *         false otherwise.
    */
  bool setCurrentTransactionRow(const int row);

  /**
    * This method is used to set the inline editing mode
    *
    * @param editing bool flag. if set, inline editing in the register
    *                is performed, if reset, cells of the register are
    *                read-only. When the object is constructed, the
    *                value of the setting is false.
    *
    * @note If not set, certain events are filtered and not passed
    *       to child widgets. See the source of eventFilter() for details.
    */
  void setInlineEditingMode(const bool editing);

  /**
    * This method is used to make sure that the current selected
    * transaction is completely visible in the register.
    */
  void ensureTransactionVisible(void);

  bool eventFilter(QObject* o, QEvent* e);

  /**
    * This method returns the maximum number a register can provide
    * for a specific register.
    *
    * @return integer containing the maximum number of lines the
    *         register provides for a single transaction
    */
  const int maxRpt(void) const { return m_maxRpt; };

public slots:
  /**
    * This method is used to inform the widget about the number
    * of transactions to be displayed.
    *
    * @param transactions number of transactions in this account
    * @param setCurrentTransaction the last transaction will be selected
    *                       as the current transaction if this flag is
    *                       set to true. Default is true.
    */
  void setTransactionCount(const int transactions, const bool setCurrentTransaction = true);

  virtual void setNumRows(int r);

  /**
    * This method sets the number of columns the register supports.
    * It resizes the widget array used to manage the edit widgets.
    * The old values of m_editWidgets are not saved. Therefor, this
    * method should only be called when not editing a transaction.
    */
  virtual void setNumCols(int r);

protected:
  void paintCell(QPainter *p, int row, int col, const QRect& r, bool selected, const QColorGroup& cg);

  /**
    * resizeData is re-implemented to avoid creation
    * of huge arrays that are not required
    */
  void resizeData(int len) {};

  void insertWidget(int row, int col, QWidget *);

  void contentsMouseDoubleClickEvent( QMouseEvent* e );
  void contentsMouseReleaseEvent( QMouseEvent* e );
/*
  void contentsMousePressEvent( QMouseEvent* e );
  bool eventFilter(QObject *o, QEvent *e);
  void keyPressEvent(QKeyEvent *k);
*/

protected:
  int    m_rpt;     // current rows per transaction
  bool   m_ledgerLens;

  KLedgerView*  m_view;

  QFont  m_cellFont;
  QFont  m_headerFont;

  QColorGroup m_cg;
  QColor m_color;
  QColor m_textColor;
  QColor m_bgColor;
  QColor m_gridColor;

  bool   m_showGrid;
  bool   m_colorPerTransaction;

  QRect  m_cellRect;
  QRect  m_textRect;

  int    m_transactionIndex;
  int    m_currentTransactionIndex;
  int    m_lastTransactionIndex;

  int    m_transactionRow;
  int    m_currentDateRow;

  MyMoneyTransaction const * m_transaction;
  MyMoneyMoney m_balance;
  MyMoneySplit m_split;

  /**
    * This vector keeps pointers to the widgets used to edit the data
    *
    */
  QPtrVector<QWidget>  m_editWidgets;

private:
  /**
    * This method is used to update all horizontal headers to the
    * selected m_headerFont settings
    */
  void updateHeaders(void);

private:
  /**
    * This member is set, if the view currently uses editing inside
    * the register is available. The view must set this using
    * setInlineEditingMode().
    */
  bool m_inlineEditMode;

  /**
    * This member keeps the number of rows this transaction
    * uses when full detail is shown. It can only be set during
    * construction. See kMyMoneyRegister::kMyMoneyRegister.
    */
  int    m_maxRpt;

signals:
  void signalPreviousTransaction();
  void signalNextTransaction();

  /**
    * This signal is emitted when the user presses RETURN
    */
  void signalEnter();

  /**
    * This signal is emitted when the user presses ESC
    */
  void signalEsc();

  /**
    * This signal is emitted when the user presses SPACE
    */
  void signalSpace();

};

#endif
