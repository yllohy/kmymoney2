/***************************************************************************
                          kbanklistitem.h
                             -------------------
    copyright            : (C) 2000 by Michael Edwardes
    email                : mte@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KACCOUNTLISTITEM_H
#define KACCOUNTLISTITEM_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qwidget.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <klistview.h>
#include <kiconview.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "../mymoney/mymoneyobserver.h"
class MyMoneyAccount;
class MyMoneyInstitution;

// #include "../mymoney/mymoneyfile.h"

/**
  *@author Michael Edwardes, Thomas Baumgart
  */

/**
  * This class represents an item in the account list view. It is used
  * by the KAccountsView, the KCategoriesView and the KNewAccountWizard
  * to select between the accounts.
  */
class KAccountListItem : public QListViewItem, MyMoneyObserver  {
public:
  /**
    * Constructor to be used to construct an institution entry
    * object.
    *
    * @param parent pointer to the KListView object this entry should be
    *               added to.
    * @param institution const reference to MyMoneyInstitution for which
    *               the KListView entry is constructed
    */
  KAccountListItem(KListView *parent, const MyMoneyInstitution& institution);

  /**
    * Constructor to be used to construct an account entry
    * object.
    *
    * @param parent pointer to the KListView object this entry should be
    *               added to.
    * @param institution const reference to MyMoneyAccount for which
    *               the KListView entry is constructed
    */
  KAccountListItem(KListView *parent, const MyMoneyAccount& account);

  /**
    * Constructor to be used to construct an account entry
    * object.
    *
    * @param parent pointer to the parent KAccountListView object this entry should be
    *               added to.
    * @param institution const reference to MyMoneyAccount for which
    *               the KListView entry is constructed
    */
  KAccountListItem(KAccountListItem *parent, const MyMoneyAccount& account);

	~KAccountListItem();

  /**
    * This method returns the account's id for this object
    *
    * @return const QCString of the Id
    */
	const QCString accountID(void) const;

  /**
    * This method is re-implemented from QListViewItem::paintCell().
    * Besides the standard implementation, the QPainter is set
    * according to the applications settings.
    */
  void paintCell(QPainter *p, const QColorGroup & cg, int column, int width, int align);

  /**
    * This method is called by the MyMoneyFile object, whenever the
    * account that is represented by this object changes within the
    * MyMoneyFile engine.
    *
    * @param id reference to QCString of the account's id
    */
  void update(const QCString& id);

  /**
    * This function can be called to clean the cache at a defined
    * location during program execution. It will be used by main()
    * to avoid false error reports by the memory leakage checker.
    */
  static void cleanCache(void);

private:
  /**
    * This method is a helper for the constructors that contains
    * the common code.
    *
    * @param account the account data for the object to be created
    */
  void newAccount(const MyMoneyAccount& account);

  /**
    * This function can be called to preload the cache at a defined
    * location during program execution. It will be used by newAccount().
    */
  void loadCache(void);

private:
  QCString m_accountID;
  bool m_bViewNormal;
  int m_nAccountColumn;
  int m_nInstitutionColumn;

  static  QPixmap* accountPixmap;
};


/**
  * This class represents an item in the account icon view. It is used
  * by the KAccountsView to select between the accounts using icons.
  */
class KAccountIconItem : public KIconViewItem, MyMoneyObserver  {
public:
  /**
    * Constructor to be used to construct an account icon object.
    *
    * @param parent pointer to the KIconView object this entry should be
    *               added to.
    * @param account const reference to MyMoneyAccount for which
    *               the KIconView entry is constructed
    * @param pixmap const reference to QPixmap with the icon to be used
    */
  KAccountIconItem(QIconView *parent, const MyMoneyAccount& account, const QPixmap& pixmap);
	~KAccountIconItem();

  /**
    * This method returns the account's id for this object
    *
    * @return const QCString of the Id
    */
	const QCString accountID(void) const { return m_accountID; };

  /**
    * This method is called by the MyMoneyFile object, whenever the
    * account that is represented by this object changes within the
    * MyMoneyFile engine.
    *
    * @param id reference to QCString of the account's id
    */
  void update(const QCString& id);

private:
  QCString m_accountID;
};
#endif
