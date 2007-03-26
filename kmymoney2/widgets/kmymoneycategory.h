/***************************************************************************
                          kmymoneycategory.h
                             -------------------
    begin                : Mon Jul 10 2006
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

#ifndef KMYMONEYCATEGORY_H
#define KMYMONEYCATEGORY_H

// ----------------------------------------------------------------------------
// QT Includes

class QWidget;
class QFrame;

// ----------------------------------------------------------------------------
// KDE Includes

#include "kdecompat.h"
#include <kcombobox.h>
class KPushButton;

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoney/mymoneyaccount.h>
#include "../widgets/kmymoneycombo.h"

class kMyMoneyAccountSelector;

/**
  * This class implements a text based account/category selector.
  * When initially used, the widget has the functionality of a KComboBox object.
  * Whenever a key is pressed, the set of loaded accounts is searched for
  * accounts which match the currently entered text.
  *
  * If any match is found a list selection box is opened and the user can use
  * the up/down, page-up/page-down keys or the mouse to navigate in the list. If
  * an account is selected, the selection box is closed. Other key-strokes are
  * directed to the parent object to manipulate the text.  The visible contents of
  * the selection box is updated with every key-stroke.
  *
  * This object is a replacement of the kMyMoneyCategory object and should be used
  * for new code.
  *
  * @author Thomas Baumgart
  */
class KMyMoneyCategory : public KMyMoneyCombo
{
  Q_OBJECT
public:
  /**
    * Standard constructor for the account selection object.
    *
    * If parameter @a splitButton is @a true, the widget
    * will construct a surrounding QFrame and reparent itself to be a child of this
    * QFrame. It also adds a KPushButton with the "Split" icon to the right of the
    * input field. In this case it is important not to use the pointer to this widget
    * but it's parent when placing the object in a QLayout, QTable or some such. The
    * parent widget (the QFrame in this case) can be extracted with the parentWidget()
    * method.
    *
    * Reparenting is handled by the object transparently for both cases.
    *
    * Standard usage example (no split button):
    *
    * @code
    * KMyMoneyCategory* category = new KMyMoneyCategory;
    * category->reparent(newParent);
    * layout->addWidget(category);
    * table->setCellWidget(category);
    * @endcode
    *
    * Enhanced usage example (with split button):
    *
    * @code
    * KMyMoneyCategory* category = new KMyMoneyCategory(0, 0, true);
    * category->reparent(newParent);
    * layout->addWidget(category->parentWidget());
    * table->setCellWidget(category->parentWidget());
    * @endcode
    */
  KMyMoneyCategory(QWidget* parent = 0, const char* name = 0, bool splitButton = false);

  virtual ~KMyMoneyCategory();

  /**
    * This member returns a pointer to the completion object.
    *
    * @return pointer to completion's selector object
    */
  kMyMoneyAccountSelector* selector(void) const;

  /**
    * This member returns a pointer to the split button. In case the @a splitButton parameter
    * of the constructor was @a false, this method prints a warning to stderr and returns 0.
    */
  KPushButton* splitButton(void) const;

  /**
    * Reimplemented for internal reasons. No API change
    */
  virtual void reparent( QWidget *parent, WFlags, const QPoint &, bool showIt = FALSE );

  /**
    * Reimplemented for internal reasons. No API change.
    */
  virtual void setPalette(const QPalette& palette);

  /**
    * Force the text field to show the text for split transaction.
    */
  void setSplitTransaction(void);

  /**
    * Check if the text field contains the text for a split transaction
    */
  bool isSplitTransaction(void) const;

  /**
    * overridden for internal reasons, no API change
    */
  void setCurrentText(const QString& txt = QString()) { KMyMoneyCombo::setCurrentText(txt); }

protected:
  /**
    * Reimplemented to support protected category text ("split transactions")
    *
    * @sa focusIn()
    */
  virtual void focusInEvent(QFocusEvent* ev);

  /**
    * set the widgets text area based on the item with the given @a id.
    */
  virtual void setCurrentText(const QCString& id);

public slots:
  virtual void slotItemSelected(const QCString& id);

signals:
  /**
    * Signal to inform other objects that this object has reached focus.
    * Used for e.g. to open the split dialog when the focus reaches this
    * object and it contains the text 'Split transaction'.
    *
    * @sa focusInEvent()
    */
  void focusIn(void);

private:
  KPushButton*      m_splitButton;
  QFrame*           m_frame;
};


class KMyMoneySecurity : public KMyMoneyCategory
{
  Q_OBJECT
public:
  KMyMoneySecurity(QWidget* parent = 0, const char* name = 0);
  virtual ~KMyMoneySecurity();

  /**
    * overridden for internal reasons, no API change
    */
  void setCurrentText(const QString& txt = QString()) { KMyMoneyCategory::setCurrentText(txt); }

protected:
  /**
    * set the widgets text area based on the item with the given @a id.
    */
  virtual void setCurrentText(const QCString& id);
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



#include "../kmymoneyutils.h"
#include <klineedit.h>
class kMyMoneyAccountCompletion;
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

  kMyMoneyAccountCompletion* completion(void) const { return m_completion; };

  QCString selectedAccountId() const { return m_id; }

  void removeAccount(const QCString& id);

  /**
   * This method is used to turn on/off the hint display
   */
  void setHint(const QString& hint) { m_hint = hint; };

signals:
  /**
    * This signal is emitted, when a new category name has been
    * entered by the user and this name is not known as account
    * by the MyMoneyFile object.
    * Before the signal is emitted, a MyMoneyAccount is constructed
    * by this object and filled with the desired name. All other members
    * of MyMoneyAccount will remain in their default state. Upon return,
    * the connected slot should have created the object in the MyMoneyFile
    * engine and filled the member @p id.
    *
    * @param acc reference to MyMoneyAccount object that caries the name
    *            and will return information about the created category.
    */
  void newCategory(MyMoneyAccount& acc);

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
  void connectNotify(const char * signal);

  /** reimplemented */
  void drawContents( QPainter *);

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

  kMyMoneyAccountCompletion* m_completion;
  bool                       m_inCreation;
  bool                       m_displayOnly;

  /**
   * This member tells what to display as hint as long as the field is empty
   */
  QString m_hint;

};

#endif
