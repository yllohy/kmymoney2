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

#include <qwidget.h>
#include <klineedit.h>

#include "../mymoney/mymoneyaccount.h"
/**
  *@author Thomas Baumgart
  */

class kMyMoneyCategory : public KLineEdit
{
   Q_OBJECT
public:
  enum categoryTypeE {
    liability =  0x01,
    asset =      0x02,
    expense =    0x04,
    income =     0x08
  };

	kMyMoneyCategory(QWidget *parent=0, const char *name=0, const categoryTypeE type = static_cast<categoryTypeE>(expense | income));
	~kMyMoneyCategory();

  void loadList(const categoryTypeE type);

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

protected:
  void focusOutEvent(QFocusEvent *ev);

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
