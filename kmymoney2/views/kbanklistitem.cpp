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
  : KListViewItem(parent)
{
  newAccount(account);
}

KAccountListItem::KAccountListItem(KAccountListItem *parent, const MyMoneyAccount& account)
  : KListViewItem(parent)
{
  newAccount(account);
}

void KAccountListItem::loadCache(void)
{
  if(accountPixmap == 0) {
    accountPixmap = new QPixmap(KGlobal::dirs()->findResource("appdata", "icons/hicolor/22x22/actions/account.png"));
  }
}

void KAccountListItem::cleanCache(void)
{
  if(accountPixmap != 0) {
    delete accountPixmap;
    accountPixmap = 0;
  }
}

void KAccountListItem::newAccount(const MyMoneyAccount& account)
{
  loadCache();
  MyMoneyFile*  file = MyMoneyFile::instance();

  m_accountID = account.id();

  file->attach(m_accountID, this);

  setPixmap(0, *accountPixmap);
  setText(0, account.name());

  MyMoneyMoney balance = file->totalBalance(m_accountID);
  
  // since income and liabilities are usually negative,
  // we reverse the sign for display purposes
  switch(account.accountGroup()) {
    case MyMoneyAccount::Income:
    case MyMoneyAccount::Liability:
      balance = -balance;
      break;
    default:
      break;
  }
  setText(2, balance.formatMoney());
}

KAccountListItem::KAccountListItem(KListView *parent, const QString& txt)
  : KListViewItem(parent), m_accountID(QCString()), m_bViewNormal(true)
{
  setText(0, txt);  
}

KAccountListItem::KAccountListItem(KListView *parent, const MyMoneyInstitution& institution)
  : KListViewItem(parent), m_accountID(institution.id()), m_bViewNormal(true)
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
      setText(1, QString::number(file->transactionCount(m_accountID)));
      
      // since income and liabilities are usually negative,
      // we reverse the sign for display purposes
      switch(acc.accountGroup()) {
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
    qDebug("Trying to get account info that does not exist anymore");
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

  QColor colour = Qt::white;
  QColor bgColour = QColor(224, 253, 182); // Same as for home view
  QColor textColour;
  
  bgColour = config->readColorEntry("listBGColor", &bgColour);
  colour = config->readColorEntry("listColor", &colour);
  textColour = config->readColorEntry("listGridColor", &textColour);

  QColorGroup cg2(cg);
  cg2.setColor(QColorGroup::Text, textColour);

  if (isAlternate())
  {
    cg2.setColor(QColorGroup::Base, bgColour);
  }
  else
  {
    cg2.setColor(QColorGroup::Base, colour);
  }

  int indent = 0;
  if (column == 0)
  {
    indent = -20 * (depth()+1);
    p->save();
    p->translate(indent, 0);
    if (isAlternate())
      p->fillRect( 0, 0, width+indent, height(), bgColour );
    else
      p->fillRect( 0, 0, width+indent, height(), colour );

    if (childCount() > 0)
    {
      p->save();
      p->translate((20 * (depth()+1))-20, 0);
      
      p->setPen( cg.foreground() );
      p->setBrush( cg.base() );
      p->drawRect( 5, height() / 2 - 4, 9, 9 );
      p->drawLine( 7, height() / 2, 11, height() / 2 );
      if ( !isOpen() )
          p->drawLine( 9, height() / 2 - 2, 9, height() / 2 + 2 );
      p->restore();
    }

    p->restore();
  }

  QListViewItem::paintCell(p, cg2, column, width, align);
}



KAccountIconItem::KAccountIconItem(QIconView* parent, const MyMoneyAccount& account, const QPixmap& pixmap )
  : KIconViewItem(parent, account.name(), pixmap)
{
  MyMoneyFile*  file = MyMoneyFile::instance();

  m_accountID = account.id();

  file->attach(m_accountID, this);
}

KAccountIconItem::~KAccountIconItem()
{
  MyMoneyFile::instance()->detach(m_accountID, this);
}

void KAccountIconItem::update(const QCString& /* id */)
{
}

KTransactionListItem::KTransactionListItem(KListView* view, KTransactionListItem* parent, const QCString& accountId, const QCString& transactionId)
  : KListViewItem(view, parent)
{
  m_accountId = accountId;
  m_transactionId = transactionId;
}

KTransactionListItem::~KTransactionListItem()
{
}

void KAccountListItem::paintBranches(QPainter* p, const QColorGroup&/* cg*/, int w, int/* y*/, int h)
{
  KConfig *config = KGlobal::config();
  config->setGroup("List Options");

  QColor colour = Qt::white;
  QColor bgColour = QColor(224, 253, 182); // Same as for home view

  bgColour = config->readColorEntry("listBGColor", &bgColour);
  colour = config->readColorEntry("listColor", &colour);

  if (isAlternate())
    p->fillRect( 0, 0, w, h, bgColour );
  else
    p->fillRect( 0, 0, w, h, colour );
}
