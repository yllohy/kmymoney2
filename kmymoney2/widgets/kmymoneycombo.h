/***************************************************************************
                          kmymoneycombo.h  -  description
                             -------------------
    begin                : Sat May 5 2001
    copyright            : (C) 2001 by Michael Edwardes
    email                : mte@users.sourceforge.net
                             Javier Campos Morales <javi_c@ctv.es>
                             Felix Rodriguez <frodriguez@mail.wesleyan.edu>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KMYMONEYCOMBO_H
#define KMYMONEYCOMBO_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

#include <kcombobox.h>

// ----------------------------------------------------------------------------
// Project Includes


/**
  *@author Michael Edwardes
  */

class kMyMoneyCombo : public KComboBox
{
	Q_OBJECT
public:
  kMyMoneyCombo(QWidget *w, const char *name=0);
  kMyMoneyCombo(bool rw, QWidget *w, const char *name=0);
  ~kMyMoneyCombo();

  /** No descriptions */
  virtual bool eventFilter(QObject *, QEvent *);

  // return the id of the entry identified by str or -1 if not found
  // @param str reference to the string to be searched for in the list
  // if the item is not found, the first item of the list will be selected
  void setCurrentItem(const QString& str);

  // override the base class variant
  void setCurrentItem(int i) { KComboBox::setCurrentItem(i); }

  void loadCurrentItem(int i);

  void resetCurrentItem(void);

  /**
    * Loads the combo box with accounts for convenience.
    *
    * @see currentAccountId()
    *
    * @param asset  Load the asset accounts if true
    * @param liability Load the liablity accounts if true
    * @return none
  **/
  void loadAccounts(bool asset=true, bool liability=false);

  /**
    * Convenience function to return the account id of the
    * currently selected item
    *
    * @return QCString The account id or "" if not found
  **/
  QCString currentAccountId(void);

private:
  /// perform initialization required for all constructors
  void init(void);

protected:
  virtual void focusOutEvent(QFocusEvent *);

protected slots:
  void slotCheckValidSelection(int id);

signals: // Signals
/*
  // No descriptions
  void signalFocusOut();
  void signalNextTransaction();
*/
  void selectionChanged(int value);
  void signalEnter();
  void signalEsc();

  /** signal is sent, when the tab key is pressed */
  void signalTab();
  /** signal is sent, when the Back-tab (Shift-Tab) key is pressed */
  void signalBackTab();

private:
  /**
    * This member keeps a copy of the original selected item
    */
  int   m_item;

  /**
    * The current type of the combo box
    *
    * Only supports 'account' type for now.  Can easily
    * be extended to cater for categories or whatever.
  **/
  enum combo_type { NONE, ACCOUNT };
  combo_type m_type;
};

#endif
