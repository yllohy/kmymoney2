/***************************************************************************
                          kbanklistitem.cpp
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

// ----------------------------------------------------------------------------
// QT Includes

#include <qpixmap.h>
#include <qcolor.h>

#if QT_VERSION > 300
#include <qpainter.h>
#endif

// ----------------------------------------------------------------------------
// KDE Includes

#include <kglobal.h>
#include <kconfig.h>
#include <klocale.h>
#if QT_VERSION > 300
#include <kstandarddirs.h>
#else
#include <kstddirs.h>
#endif
#include <kmessagebox.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kbanklistitem.h"
#include "../mymoney/mymoneyfile.h"

QPixmap* KAccountListItem::accountPixmap = 0;

KAccountListItem::KAccountListItem(KListView *parent, const MyMoneyAccount& account)
  : QListViewItem(parent)
{
  newAccount(account);
}

KAccountListItem::KAccountListItem(KAccountListItem *parent, const MyMoneyAccount& account)
  : QListViewItem(parent)
{
  newAccount(account);
}

void KAccountListItem::newAccount(const MyMoneyAccount& account)
{

  if(accountPixmap == 0) {
    accountPixmap = new QPixmap(KGlobal::dirs()->findResource("appdata", "icons/hicolor/22x22/actions/account.png"));
  }

  MyMoneyFile*  file = MyMoneyFile::instance();

  m_accountID = account.id();

  file->attach(m_accountID, this);

  setPixmap(0, *accountPixmap);
  update(m_accountID);
}

KAccountListItem::KAccountListItem(KListView *parent, const MyMoneyInstitution& institution)
  : QListViewItem(parent), m_accountID(institution.id()), m_bViewNormal(true)
{
  setText(0, institution.name());
}

KAccountListItem::~KAccountListItem()
{
  MyMoneyFile::instance()->detach(m_accountID, this);
}

void KAccountListItem::update(const QCString& accountId)
{
  MyMoneyFile*  file = MyMoneyFile::instance();

  try {
    MyMoneyAccount acc = file->account(accountId);

    try {
      MyMoneyMoney balance = file->totalBalance(m_accountID);

      setText(0, acc.name());

      // since income and liabilities are usually negative,
      // we reverse the sign for display purposes
      switch(file->accountGroup(acc.accountType())) {
        case MyMoneyAccount::Income:
        case MyMoneyAccount::Liability:
          balance = -balance;
          break;
        default:
          break;
      }
      setText(2, balance.formatMoney());

    } catch(MyMoneyException *e) {
      KMessageBox::detailedSorry(0, i18n("Unable to retrieve account information"),
          (e->what() + " " + i18n("thrown in") + " " + e->file()+ ":%1").arg(e->line()));
      delete e;
    }

  } catch(MyMoneyException *e) {
    // try to get account info that does not exist anymore
    delete e;
  }
}

const QCString KAccountListItem::accountID(void) const
{
  return m_accountID;
}

void KAccountListItem::paintCell(QPainter *p, const QColorGroup & cg, int column, int width, int align)
{
	KConfig *config = KGlobal::config();
  config->setGroup("List Options");
  QFont defaultFont = QFont("helvetica", 12);
  p->setFont(config->readFontEntry("listCellFont", &defaultFont));

/*
  if (column==2)
  {
    QFont font = p->font();
    if (m_account.balance().amount()<0)
      font.setItalic(true);

    p->setFont(font);
    QListViewItem::paintCell(p, cg, column, width, 2);
  }
  else
*/
  QListViewItem::paintCell(p, cg, column, width, align);
}
