/***************************************************************************
                          kmymoneysplittable.h  -  description
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

#ifndef KMYMONEYSPLITTABLE_H
#define KMYMONEYSPLITTABLE_H

#include <qwidget.h>
#include <qtable.h>

/**
  *@author Thomas Baumgart
  */

class kMyMoneySplitTable : public QTable  {
   Q_OBJECT
public: 
	kMyMoneySplitTable(QWidget *parent=0, const char *name=0);
	virtual ~kMyMoneySplitTable();

  void paintCell(QPainter *p, int row, int col, const QRect& r, bool /*selected*/);
  void paintFocus(QPainter *p, const QRect &cr);
  void setCurrentRow(int row);
  int currentRow(void) { return m_currentRow; }
  void setMaxRows(int row);

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
    * This method returns the inline editing mode
    *
    * @return true if inline edit mode is on, false otherwise
    */
  const bool inlineEditingMode(void) const { return m_inlineEditMode; };

protected:
  void contentsMousePressEvent( QMouseEvent* e );
  void contentsMouseReleaseEvent( QMouseEvent* e );
  void contentsMouseDoubleClickEvent( QMouseEvent* e );
  bool eventFilter(QObject *o, QEvent *e);
  void endEdit(int row, int col, bool accept, bool replace );

public slots:
  /** No descriptions */
  virtual void setCurrentCell(int row, int col);

  virtual void setNumRows(int r);

private:
  int m_key;

  /// array to be used to access the input widgets
  QWidget* m_colWidget[3];

  /// the currently selected row (will be printed as selected)
  int m_currentRow;

  /// the number of rows filled with data
  int m_maxRows;

  bool m_inlineEditMode;

protected slots:
	virtual void columnWidthChanged(int col);

signals: // Signals
  /// signalNavigationKey is sent, when the Up- or Down-Key is pressed
  void signalNavigationKey(int key);

  // signalTab is sent, when the Tab-Key is pressed
  void signalTab(void);

  // signalEsc is sent, when the Esc-Key is pressed
  void signalEsc(void);

  void signalEnter(void);

  // signalDelete is sent, when the Del-Key is pressed
  void signalDelete(void);

  void signalCancelEdit(int key);
};

#endif
