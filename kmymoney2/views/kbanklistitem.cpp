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
#include <qstyle.h>
#include <qglobal.h>

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
#include "../kmymoneyutils.h"

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
  m_suspendUpdate = false;
  
  loadCache();
  MyMoneyFile*  file = MyMoneyFile::instance();

  setAccountID(account.id());

  file->attach(account.id(), this);

  setPixmap(0, *accountPixmap);
  
  // make sure, we translate the standard account names
  if(file->isStandardAccount(account.id())) {
    if(account.id() == file->asset().id()) {
      setText(0, i18n("Asset"));
    } else if(account.id() == file->liability().id()) {
      setText(0, i18n("Liability"));
    } else if(account.id() == file->income().id()) {
      setText(0, i18n("Income"));
    } else if(account.id() == file->expense().id()) {
      setText(0, i18n("Expense"));
    }
  } else
    setText(0, account.name());

  MyMoneyMoney balance = file->totalBalance(account.id());
  
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
  : KListViewItem(parent), m_bViewNormal(true), m_suspendUpdate(false)
{
  setText(0, txt);  
}

KAccountListItem::KAccountListItem(KListView *parent, const MyMoneyInstitution& institution)
  : KListViewItem(parent), m_bViewNormal(true), m_suspendUpdate(false)
{
  setAccountID(institution.id());
  setText(0, institution.name());
}

KAccountListItem::~KAccountListItem()
{
  MyMoneyFile::instance()->detach(accountID(), this);
}

void KAccountListItem::update(const QCString& accountId)
{
  if(m_suspendUpdate == true)
    return;
    
  MyMoneyFile*  file = MyMoneyFile::instance();
    
  try {
    MyMoneyAccount acc = file->account(accountId);

    try {
      MyMoneyMoney balance = file->totalBalance(accountId);

      setText(0, acc.name());
      if(!acc.parentAccountId().isEmpty())
        setText(1, QString::number(file->transactionCount(accountId)));
      else
        setText(1, QString(" "));
      
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

void KAccountListItem::paintCell(QPainter *p, const QColorGroup & cg, int column, int width, int align)
{
  p->setFont(KMyMoneyUtils::cellFont());

  QColor colour = KMyMoneyUtils::listColour();
  QColor bgColour = KMyMoneyUtils::backgroundColour();
  
  QColorGroup cg2(cg);

  if (isAlternate())
    cg2.setColor(QColorGroup::Base, colour);
  else
    cg2.setColor(QColorGroup::Base, bgColour);

  QListViewItem::paintCell(p, cg2, column, width, align);
  
  int indent = 0;
  if (column == 0)
  {
    int ts = listView()->treeStepSize();
    int ofs;
    indent = ts * (depth()+1);
    p->save();
    p->translate(-indent, 0);

    if ( isSelected()) {
      p->fillRect( 0, 0, indent, height(), cg2.brush( QColorGroup::Highlight ) );
      if ( isEnabled() || !listView() )
        p->setPen( cg2.highlightedText() );
      else if ( !isEnabled() && listView())
        p->setPen( listView()->palette().disabled().highlightedText() );
        
    } else
      p->fillRect( 0, 0, indent, height(), cg2.base() );

    // draw dotted lines in upper levels to the left of us
    QListViewItem *parent = this;
    for(int j = depth()-1; j >= 0; --j) {
      if(!parent)
        break;
      parent = parent->parent();
      if(parent->nextSibling()) {
        ofs = (j * ts) + ts/2 - 1;
        for(int j = 0; j < height(); j += 2)
          p->drawPoint(ofs, j);
      }
    }

    if(childCount() == 0) {
      // if we have no children, the we need to draw a vertical line
      // which length depends if we have a sibling or not.
      // also a horizontal line to the right is required.
      ofs = depth()*ts + ts/2 - 1;
      int end = nextSibling() ? height() : height()/2;
      for(int i = 0; i < end; i += 2)
        p->drawPoint(ofs, i);

      for(int i = ofs; i < (depth()+1)*ts; i += 2)
        p->drawPoint(i, height()/2);

    } else {
      // draw upper part of vertical line      
      ofs = depth()*ts + ts/2 - 1;
      for(int i = 0; i < height()/2-(ts-2)/4; i += 2)
        p->drawPoint(ofs, i);

      // draw horizontal part        
      for(int i = ofs + ts/4 ; i < (depth()+1)*ts; i += 2)
        p->drawPoint(i, height()/2);
        
      // need to draw box with +/- in it
      ofs = depth() * ts;
      p->drawRect( ofs + ts/4, height() / 2 - (ts-2)/4, (ts-2)/2, (ts-2)/2 );
      p->drawLine( ofs + ts/2-3, height() / 2, ofs + ts/2+1, height() / 2 );
      if ( !isOpen() )
          p->drawLine( ofs + ts/2-1, height() / 2 - 2, ofs + ts/2-1, height() / 2 + 2 );

      // if there are more siblings, we need to draw
      // the remainder of the vertical line
      if(nextSibling()) {
        ofs = depth()*ts + ts/2 - 1;
        for(int i = height() / 2 + (ts-2)/4; i < height(); i += 2)
          p->drawPoint(ofs, i);
      }
    }

    p->restore();
  }
}



KAccountIconItem::KAccountIconItem(QIconView* parent, const MyMoneyAccount& account, const QPixmap& pixmap )
  : KIconViewItem(parent, account.name(), pixmap)
{
  MyMoneyFile*  file = MyMoneyFile::instance();

  setAccountID(account.id());

  file->attach(account.id(), this);
}

KAccountIconItem::~KAccountIconItem()
{
  MyMoneyFile::instance()->detach(accountID(), this);
}

void KAccountIconItem::update(const QCString& id)
{
  QString name;
  
  try {
    MyMoneyAccount acc = MyMoneyFile::instance()->account(id);
    name = acc.name();
  } catch(MyMoneyException *e) {
    delete e;
  }
  setText(name, true);
}

KTransactionListItem::KTransactionListItem(KListView* view, KTransactionListItem* parent, const QCString& accountId, const QCString& transactionId)
  : KListViewItem(view, parent)
{
  setAccountID(accountId);
  m_transactionId = transactionId;
}

KTransactionListItem::~KTransactionListItem()
{
}

void KTransactionListItem::paintCell(QPainter *p, const QColorGroup &cg, int column, int width, int alignment)
{
  QColorGroup _cg = cg;
  _cg.setColor(QColorGroup::Base, backgroundColor());
  QListViewItem::paintCell(p, _cg, column, width, alignment);
}

const QColor KTransactionListItem::backgroundColor()
{
  QColor bgColour = KMyMoneyUtils::backgroundColour();
  QColor listColour = KMyMoneyUtils::listColour();
  return isAlternate() ? bgColour : listColour;
}

void KAccountListItem::paintBranches(QPainter* /* p */, const QColorGroup& /* cg */, int /* w */, int /* y */, int /* h */)
{
}

void KAccountListItem::paintFocus(QPainter* p, const QColorGroup& cg, const QRect& r)
{
  int indent = listView()->treeStepSize() * (depth()+1);

  QRect r2(r);
  r2.setLeft(r2.left() + -indent);
  
  if (isSelected())
    p->fillRect(  r2.left(),
                  r2.top(),
                  -indent,
                  r2.height(),
                  cg.highlight());

  listView()->style().drawPrimitive(
                QStyle::PE_FocusRect, p, r2, cg,
                (isSelected() ? QStyle::Style_FocusAtBorder : QStyle::Style_Default),
                QStyleOption(isSelected() ? cg.highlight() : cg.base()));
}

