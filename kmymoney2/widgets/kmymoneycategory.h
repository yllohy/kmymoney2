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

#ifndef KMYMONEYCATEGORY_H
#define KMYMONEYCATEGORY_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qwidget.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include "kdecompat.h"
#include <klineedit.h>

// ----------------------------------------------------------------------------
// Project Includes


#include "../mymoney/mymoneyaccount.h"
#include "../kmymoneyutils.h"
class kMyMoneyAccountCompletion;

/**
  * @author Thomas Baumgart
  */

/**
  * This class implements a text based account/category selector. The name
  * is kept for historic reasons. When initially used, the widget has the
  * functionality of a KLineEdit object. Whenever a key is pressed, the set
  * of account is searched for accounts which match the currently entered
  * text.
  *
  * If any match is found a list selection box is opened and the user can use
  * the up/down, page-up/page-down keys or the mouse to navigate in the list. If
  * an account is selected, the selection box is closed. Other key-strokes are
  * directed to the KLineEdit object to manipulate the text. With every key-stroke
  * the selection box is updated.
  */
class kMyMoneyCategory : public KLineEdit
{
   Q_OBJECT
public:
  kMyMoneyCategory(QWidget *parent=0, const char *name=0, const KMyMoneyUtils::categoryTypeE = static_cast<KMyMoneyUtils::categoryTypeE>(KMyMoneyUtils::expense | KMyMoneyUtils::income));
  ~kMyMoneyCategory();

  virtual bool eventFilter(QObject * , QEvent * );

  kMyMoneyAccountCompletion* selector(void) const { return m_accountSelector; };

  QCString selectedAccountId() const { return m_id; }

signals:
  /**
    * This signal is emitted, when a new category name has been
    * entered by the user and this name is not known as account
    * by the MyMoneyFile object.
    */
  // void newCategory(const QString& category);

  /**
    * This signal is emitted when the user selected a different category.
    */
  // void categoryChanged(const QString& category);

  /**
    * This signal is emitted when the user selected a different account/category.
    */
  void categoryChanged(const QCString& id);

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
    * Load the widget with the text given in the parameter @p text.
    * This should not be used to load the name of an account/category
    * but only for fixed text, e.g. 'Split transaction'.
    *
    * @param text text which should be loaded into the widget.
    */
  void loadText(const QString& text);

  /**
    * Load the widget with the account identified by the parameter @p id.
    * This should not be used to load the name of an account/category
    * but only for fixed text, e.g. 'Split transaction'.
    *
    * @param id the id of the category/account that should be loaded
    */
  void loadAccount(const QCString& id);

  /**
    * Load the widget with the name of the account given by
    * the parameter @p id.
    *
    * @param id id of account of which the name should be loaded
    */
  void slotSelectAccount(const QCString& id);

protected:
  virtual void keyPressEvent( QKeyEvent * );
  void focusOutEvent(QFocusEvent *ev);
  void focusInEvent(QFocusEvent *ev);

private:
  void checkForNewCategory(void);

private:
  /**
    * This member keeps the initial value. It is used during
    * resetText() to set the widgets text back to this initial value
    */
  QString                    m_text;

  /**
    * This member keeps the id of the selected category/account.
    */
  QCString                   m_id;

  kMyMoneyAccountCompletion* m_accountSelector;
  bool                       m_inCreation;
  bool                       m_displayOnly;
};

#endif
