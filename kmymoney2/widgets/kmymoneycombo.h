/***************************************************************************
                          kmymoneycombo.h  -  description
                             -------------------
    begin                : Mon Mar 12 2007
    copyright            : (C) 2007 by Thomas Baumgart
    email                : ipwizard@users.sourceforge.net
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

#include <qtimer.h>
#include <qmutex.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kcombobox.h>

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoney/mymoneyutils.h>
#include <kmymoney/mymoneysplit.h>
#include <kmymoney/register.h>
#include <kmymoney/mymoneyaccount.h>
#include <kmymoney/transaction.h>
#include <kmymoney/mymoneypayee.h>

class kMyMoneyCompletion;
class KMyMoneySelector;
class kMyMoneyLineEdit;

/**
  * @author Thomas Baumgart
  */
class KMyMoneyCombo : public KComboBox
{
  Q_OBJECT
public:
  KMyMoneyCombo(QWidget *w = 0, const char *name=0);
  KMyMoneyCombo(bool rw, QWidget *w = 0, const char *name=0);

  /**
    * This method is used to turn on/off the hint display and to setup the appropriate text.
    * The hint text is shown in a lighter color if the field is otherwise empty and does
    * not have the keyboard focus.
    *
    * @param hint reference to text. If @a hint is empty, no hint will be shown.
    */
  void setHint(const QString& hint) const;

  /**
    * overridden for internal reasons.
    *
    * @param editable make combo box editable (@a true) or selectable only (@a false).
    */
  void setEditable(bool editable);

  /**
    * This method returns a pointer to the completion object of the combo box.
    *
    * @return pointer to kMyMoneyCompletion or derivative.
    */
  kMyMoneyCompletion* completion(void) const;

  /**
    * This method returns a pointer to the completion object's selector.
    *
    * @return pointer to KMyMoneySelector or derivative.
    */
  KMyMoneySelector* selector(void) const;

  /**
    * This method returns the ids of the currently selected items
    */
  void selectedItems(QCStringList& list) const;

  /**
    * This method returns the id of the first selected item.
    * Usage makes usually only sense when the selection mode
    * of the associated KMyMoneySelector is QListView::Single.
    *
    * @sa KMyMoneySelector::setSelectionMode()
    *
    * @param id reference to QCString containing the id. If no item
    *           is selected id will be empty.
    */
  void selectedItem(QCString& id) const;

  /**
    * This method selects the item with the respective @a id.
    *
    * @param id reference to QCString containing the id
    */
  void setSelectedItem(const QCString& id);

  /**
    * This method checks if the position @a pos is part of the
    * area of the drop down arrow.
    */
  bool isInArrowArea(const QPoint& pos) const;

  void setSuppressObjectCreation(bool suppress) { m_canCreateObjects = !suppress; }

  /**
    * overridden for internal reasons, no API change
    */
  void setCurrentText(const QString& txt = QString()) { KComboBox::setCurrentText(txt); }

  /**
    * overridden to set the background color of the lineedit as well
    */
  void setPaletteBackgroundColor(const QColor& color);

protected slots:
  virtual void slotItemSelected(const QCString& id);

protected:
  /**
    * reimplemented to support our own popup widget
    */
  void mousePressEvent(QMouseEvent *e);

  /**
    * reimplemented to support our own popup widget
    */
  void keyPressEvent(QKeyEvent *e);

  /**
    * reimplemented to support our own popup widget
    */
  void paintEvent(QPaintEvent *);

  /**
    * reimplemented to support detection of new items
    */
  void focusOutEvent(QFocusEvent* );

  /**
    * set the widgets text area based on the item with the given @a id.
    */
  virtual void setCurrentText(const QCString& id);

  /**
    * Overridden for internal reasons, no API change
    */
  void connectNotify(const char* signal);

  /**
    * Overridden for internal reasons, no API change
    */
  void disconnectNotify(const char* signal);

protected:
  /**
    * This member keeps a pointer to the object's completion object
    */
  kMyMoneyCompletion*    m_completion;

  /**
    * Use our own line edit to provide hint functionality
    */
  kMyMoneyLineEdit*      m_edit;

  /**
    * The currently selected item
    */
  QCString               m_id;

signals:
  void itemSelected(const QCString& id);
  void objectCreation(bool);
  void createItem(const QString&, QCString&);

private:
  QTimer                 m_timer;
  QMutex                 m_focusMutex;
  /**
    * Flag to control object creation. Use setSuppressObjectCreation()
    * to modify it's setting. Defaults to @a false.
    */
  bool                   m_canCreateObjects;

};

/**
  * @author Thomas Baumgart
  * This class implements a combo box with the possible states for
  * reconciliation.
  */
class KMyMoneyReconcileCombo : public KMyMoneyCombo
{
  Q_OBJECT
public:
  KMyMoneyReconcileCombo(QWidget *w = 0, const char *name=0);

  void setState(MyMoneySplit::reconcileFlagE state);
  MyMoneySplit::reconcileFlagE state(void) const;
  void removeDontCare(void);

protected slots:
  void slotSetState(const QCString&);
};

/**
  * @author Thomas Baumgart
  * This class implements a combo box with the possible states for
  * actions (Deposit, Withdrawal, etc.).
  *
  * @deprecated
  */
class KMyMoneyComboAction : public KMyMoneyCombo
{
  Q_OBJECT
public:
  KMyMoneyComboAction(QWidget *w = 0, const char *name=0);

  void setAction(int state);
  int action(void) const;
  void protectItem(int id, bool protect);

protected slots:
  void slotSetAction(const QCString&);

signals:
  void actionSelected(int);
};

/**
  * @author Thomas Baumgart
  * This class implements a combo box with the possible states for
  * actions (Deposit, Withdrawal, etc.).
  */
class KMyMoneyCashFlowCombo : public KMyMoneyCombo
{
  Q_OBJECT
public:
  /**
    * Create a combo box that contains the entries "Pay to", "From" and
    * "  " for don't care.
    */
  KMyMoneyCashFlowCombo(QWidget *w = 0, const char *name=0, MyMoneyAccount::accountTypeE type = MyMoneyAccount::Asset);

  void setDirection(KMyMoneyRegister::CashFlowDirection dir);
  KMyMoneyRegister::CashFlowDirection direction(void) const { return m_dir; }
  void removeDontCare(void);

protected slots:
  void slotSetDirection(const QCString& id);

signals:
  void directionSelected(KMyMoneyRegister::CashFlowDirection);

private:
  KMyMoneyRegister::CashFlowDirection   m_dir;
};

/**
  * @author Thomas Baumgart
  * This class implements a combo box with the possible activities
  * for investment transactions (buy, sell, dividend, etc.)
  */
class KMyMoneyActivityCombo : public KMyMoneyCombo
{
  Q_OBJECT
public:
  /**
    * Create a combo box that contains the entries "Buy", "Sell" etc.
    */
  KMyMoneyActivityCombo(QWidget *w = 0, const char *name=0);

  void setActivity(MyMoneySplit::investTransactionTypeE activity);
  MyMoneySplit::investTransactionTypeE activity(void) const { return m_activity; }

protected slots:
  void slotSetActivity(const QCString& id);

signals:
  void activitySelected(MyMoneySplit::investTransactionTypeE);

private:
  MyMoneySplit::investTransactionTypeE  m_activity;
};

/**
  * This class implements a text based payee selector.
  * When initially used, the widget has the functionality of a KComboBox object.
  * Whenever a key is pressed, the set of loaded payees is searched for
  * payees names which match the currently entered text.
  *
  * If any match is found a list selection box is opened and the user can use
  * the up/down, page-up/page-down keys or the mouse to navigate in the list. If
  * a payee is selected, the selection box is closed. Other key-strokes are
  * directed to the parent object to manipulate the text.  The visible contents of
  * the selection box is updated with every key-stroke.
  *
  * This object is a replacement of the kMyMoneyPayee object and should be used
  * for new code.
  *
  * @author Thomas Baumgart
  */
class KMyMoneyPayeeCombo : public KMyMoneyCombo
{
   Q_OBJECT
public:
  KMyMoneyPayeeCombo(QWidget* parent = 0, const char* name = 0);

  void loadPayees(const QValueList<MyMoneyPayee>& list);
};



// -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF --
// -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF --
// -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF --
// -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF --
// -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF --
// -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF --
// -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF --
// -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF --
// -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF --

/***************************************************************************
                          kmymoneycombo.h  -  description
                             -------------------
    begin                : Sat May 5 2001
    copyright            : (C) 2001 by Michael Edwardes
    email                : mte@users.sourceforge.net
                             Javier Campos Morales <javi_c@ctv.es>
                             Felix Rodriguez <frodriguez@mail.wesleyan.edu>
 ***************************************************************************/

/**
  * @author Michael Edwardes
  * @author Thomas Baumgart
  */

class kMyMoneyCombo : public KComboBox
{
  Q_OBJECT
public:
  kMyMoneyCombo(QWidget *w, const char *name=0);
  kMyMoneyCombo(bool rw, QWidget *w, const char *name=0);
  ~kMyMoneyCombo();

  /**
    * return the id of the entry identified by str or -1 if not found
    *
    * @param str reference to the string to be searched for in the list
    *            if the item is not found, the first item of the list will be selected
    */
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
    * @return QCString The account id or QCString() if not found
    **/
  QCString currentAccountId(void);

private:
  /**
    * perform initialization required for all constructors
    */
  void init(void);

protected:
  virtual void focusOutEvent(QFocusEvent *);

protected slots:
  void slotCheckValidSelection(int id);

signals: // Signals
  void selectionChanged(int value);

private:
  /**
    * This member keeps a copy of the original selected item
    */
  int   m_item;

  /**
    * This member keeps a copy of the previously selected item
    */
  int   m_prevItem;

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
