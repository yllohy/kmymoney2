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

#include <klineedit.h>

// ----------------------------------------------------------------------------
// Project Includes


#include "../mymoney/mymoneyaccount.h"
#include "../kmymoneyutils.h"

/**
  *@author Thomas Baumgart
  */

class kMyMoneyCategory : public KLineEdit
{
   Q_OBJECT
public:
  kMyMoneyCategory(QWidget *parent=0, const char *name=0, const KMyMoneyUtils::categoryTypeE = static_cast<KMyMoneyUtils::categoryTypeE>(KMyMoneyUtils::expense | KMyMoneyUtils::income));
  ~kMyMoneyCategory();

  void loadList(const KMyMoneyUtils::categoryTypeE type);

  /**
    */
  void resetText(void);

  virtual bool eventFilter(QObject * , QEvent * );

signals:
  /**
    * This signal is emitted, when a new category name has been
    * entered by the user and this name is not known as account
    * by the MyMoneyFile object.
    */
  void newCategory(const QString& category);

  /**
    * This signal is emitted when the user selected a different category.
    */
  void categoryChanged(const QString& category);

  /**
    * This signal is emitted when the user presses RETURN while editing
    */
  void signalEnter();

  /**
    * This signal is emitted when the user presses ESC while editing
    */
  void signalEsc();

  void signalFocusIn(void);

protected:
  void focusOutEvent(QFocusEvent *ev);
  void focusInEvent(QFocusEvent *ev);

public slots:
  void loadText(const QString& text);

protected:
  virtual void keyPressEvent( QKeyEvent * );

private:
  void addCategories(QStringList& strList, const QCString& id, const QString& leadIn);

private:
  /**
    * This member keeps the initial value. It is used during
    * resetText() to set the widgets text back to this initial value
    */
  QString m_text;

  QValueList<MyMoneyAccount> m_accountList;
  QMap<QString, QCString>  m_categoryConversionList;

};

#endif
