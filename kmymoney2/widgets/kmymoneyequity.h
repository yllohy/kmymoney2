/***************************************************************************
                          kmymoneycategory.h  -  description
                             -------------------
    begin                : Sun Aug 11 2002
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

#ifndef KMYMONEYEQUITY_H
#define KMYMONEYEQUITY_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qwidget.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include "kdecompat.h"
#include <klineedit.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "../kmymoneyutils.h"
class kMyMoneyEquityCompletion;

/**
  * @author Thomas Baumgart
  */

/**
  * This class implements a text based equity selector.
  * When initially used, the widget has the
  * functionality of a KLineEdit object. Whenever a key is pressed, the set
  * of equities is searched for equities which match the currently entered
  * text.
  *
  * If any match is found a list selection box is opened and the user can use
  * the up/down, page-up/page-down keys or the mouse to navigate in the list. If
  * an account is selected, the selection box is closed. Other key-strokes are
  * directed to the KLineEdit object to manipulate the text. With every key-stroke
  * the selection box is updated.
  */
class kMyMoneyEquity : public KLineEdit
{
   Q_OBJECT
public:
  kMyMoneyEquity(QWidget *parent=0, const char *name=0);
  ~kMyMoneyEquity();

  virtual bool eventFilter(QObject * , QEvent * );

signals:
  /**
    * This signal is emitted when the user selected a different equity.
    */
  void equityChanged(const QCString& id);

  /**
    * This signal is emitted when the user presses RETURN while editing
    */
  void signalEnter();

  /**
    * This signal is emitted when the user presses ESC while editing
    */
  void signalEsc();

  void signalFocusIn(void);

public slots:
  /**
    * Load the widget with the equity identified by the parameter @p id.
    *
    * @param id the id of the equity that should be loaded
    */
  void loadEquity(const QCString& id);

  /**
    * Load the widget with the name of the equity given by
    * the parameter @p id.
    *
    * @param id id of equity of which the name should be loaded
    */
  void slotSelectEquity(const QCString& id);

protected:
  virtual void keyPressEvent( QKeyEvent * );
  void focusOutEvent(QFocusEvent *ev);
  void focusInEvent(QFocusEvent *ev);

private:
  /**
    * This member keeps the initial value. It is used during
    * resetText() to set the widgets text back to this initial value
    */
  QString                    m_text;

  /**
    * This member keeps the id of the selected equity
    */
  QCString                   m_id;

  kMyMoneyEquityCompletion*  m_equitySelector;
  bool                       m_inCreation;
};

#endif
