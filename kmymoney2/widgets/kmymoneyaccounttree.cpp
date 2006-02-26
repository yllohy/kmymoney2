/***************************************************************************
                         kmymoneyaccounttree.cpp  -  description
                            -------------------
   begin                : Sat Jan 1 2005
   copyright            : (C) 2005 by Thomas Baumgart
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

// ----------------------------------------------------------------------------
// QT Includes

#include <qpoint.h>
#include <qevent.h>
#include <qdragobject.h>
#include <qtimer.h>
#include <qcursor.h>
#include <qheader.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qstyle.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kmessagebox.h>
#include <klocale.h>
#include <kglobal.h>
#include <kstandarddirs.h>

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoney/mymoneyfile.h>
#include <kmymoney/kmymoneyaccounttree.h>

#include "../kmymoneysettings.h"

/**
  * @todo drag/drop in KMyMoneyAccountTree
  */

KMyMoneyAccountTree::KMyMoneyAccountTree(QWidget* parent, const char* name) :
  KListView(parent, name),
  m_accountConnections(false),
  m_institutionConnections(false)
{
  setRootIsDecorated(true);
  setAllColumnsShowFocus(true);

  addColumn(i18n("Account"));
  addColumn(i18n("Balance"));
  addColumn(i18n("Value"));

  setMultiSelection(false);

  setColumnWidthMode(NameColumn, QListView::Maximum);
  setColumnWidthMode(BalanceColumn, QListView::Maximum);
  setColumnWidthMode(ValueColumn, QListView::Maximum);

  setColumnAlignment(BalanceColumn, Qt::AlignRight);
  setColumnAlignment(ValueColumn, Qt::AlignRight);

  setResizeMode(QListView::AllColumns);
  setSorting(0);

  header()->setResizeEnabled(true);

  setDragEnabled(false);
  setAcceptDrops(false);
  setItemsMovable(false);
  setDropVisualizer(false);
  setDropHighlighter(true);

  // setup a default
  m_baseCurrency.setSmallestAccountFraction(100);
  m_baseCurrency.setSmallestCashFraction(100);

  connect(this, SIGNAL(dropped(QDropEvent*,QListViewItem*,QListViewItem*)), this, SLOT(slotObjectDropped(QDropEvent*,QListViewItem*,QListViewItem*)));
  connect(this, SIGNAL(selectionChanged(QListViewItem*)), this, SLOT(slotSelectObject(QListViewItem*)));
  connect(this, SIGNAL(rightButtonClicked(QListViewItem* , const QPoint&, int)), this, SLOT(slotOpenContextMenu(QListViewItem*)));
  connect(this, SIGNAL(doubleClicked(QListViewItem*,const QPoint&,int)), this, SLOT(slotOpenObject(QListViewItem*)));

  // drag and drop timer connections
  connect( &m_autoopenTimer, SIGNAL( timeout() ), this, SLOT( slotOpenFolder() ) );
  connect( &m_autoscrollTimer, SIGNAL( timeout() ), this, SLOT( slotAutoScroll() ) );

}

void KMyMoneyAccountTree::connectNotify(const char * /* s */)
{
  // update drag and drop settings
  m_accountConnections = (receivers(SIGNAL(reparent(const MyMoneyAccount&, const MyMoneyAccount&))) != 0);
  m_institutionConnections = (receivers(SIGNAL(reparent(const MyMoneyAccount&, const MyMoneyInstitution&))) != 0);
  setDragEnabled(m_accountConnections | m_institutionConnections);
  setAcceptDrops(m_accountConnections | m_institutionConnections);
}

void KMyMoneyAccountTree::disconnectNotify(const char * /* s */)
{
  // update drag and drop settings
  m_accountConnections = (receivers(SIGNAL(reparent(const MyMoneyAccount&, const MyMoneyAccount&))) != 0);
  m_institutionConnections = (receivers(SIGNAL(reparent(const MyMoneyAccount&, const MyMoneyInstitution&))) != 0);
  setDragEnabled(m_accountConnections | m_institutionConnections);
  setAcceptDrops(m_accountConnections | m_institutionConnections);
}

void KMyMoneyAccountTree::setSectionHeader(int sec, const QString& txt)
{
  header()->setLabel(sec, txt);
}

KMyMoneyAccountTreeItem* KMyMoneyAccountTree::selectedItem(void) const
{
  return dynamic_cast<KMyMoneyAccountTreeItem *>(KListView::selectedItem());
}

const KMyMoneyAccountTreeItem* KMyMoneyAccountTree::findItem(const QCString& id) const
{
  // tried to use a  QListViewItemIterator  but that does not fit
  // with the constness of this method. Arghhh.

  QListViewItem* p = firstChild();
  while(p) {
    // item found, check for the id
    KMyMoneyAccountTreeItem* item = dynamic_cast<KMyMoneyAccountTreeItem*>(p);
    if(item && item->id() == id)
      break;

    // item did not match, search the next one
    QListViewItem* next = p->firstChild();
    if(!next) {
      while((next = p->nextSibling()) == 0) {
        p = p->parent();
        if(!p)
          break;
      }
    }
    p = next;
  }

  return dynamic_cast<KMyMoneyAccountTreeItem*>(p);
}

bool KMyMoneyAccountTree::dropAccountOnAccount(const MyMoneyAccount& accFrom, const MyMoneyAccount& accTo) const
{
  bool rc = false;

  // it does not make sense to reparent an account to oneself
  // or to reparent it to it's current parent
  if(accTo.id() != accFrom.id()
  && accFrom.parentAccountId() != accTo.id()) {
    // Moving within a group is generally ok
    rc = accTo.accountGroup() == accFrom.accountGroup();

    // now check for exceptions
    if(rc) {
      if(accTo.accountType() == MyMoneyAccount::Investment
      && accFrom.accountType() != MyMoneyAccount::Stock)
        rc = false;

      else if(accFrom.accountType() == MyMoneyAccount::Stock
      && accTo.accountType() != MyMoneyAccount::Investment)
        rc = false;

    } else {
      if(accFrom.accountGroup() == MyMoneyAccount::Income
      && accTo.accountGroup() == MyMoneyAccount::Expense)
        rc = true;

      if(accFrom.accountGroup() == MyMoneyAccount::Expense
      && accTo.accountGroup() == MyMoneyAccount::Income)
        rc = true;
    }
  }

  return rc;
}

bool KMyMoneyAccountTree::acceptDrag(QDropEvent* event) const
{
  bool rc;

  if(rc = (acceptDrops() && event->source() == viewport())) {
    rc = false;
    KMyMoneyAccountTreeItem* to = dynamic_cast<KMyMoneyAccountTreeItem*>(itemAt( contentsToViewport(event->pos()) ));
    QCString fromId(event->encodedData("text/plain"));
    const KMyMoneyAccountTreeItem* from = findItem(fromId);

    // we can only move accounts around
    if(!from->isAccount())
      from = 0;

    if(to && from && !to->isChildOf(from)) {
      const MyMoneyAccount& accFrom = dynamic_cast<const MyMoneyAccount&>(from->itemObject());

      if(to->isAccount() && m_accountConnections) {
        const MyMoneyAccount& accTo = dynamic_cast<const MyMoneyAccount&>(to->itemObject());
        rc = dropAccountOnAccount(accFrom, accTo);

      } else if(to->isInstitution() && m_institutionConnections) {
        // Moving an account to an institution is ok
        rc = true;
      }
    }
  }

  return rc;
}

void KMyMoneyAccountTree::startDrag(void)
{
  QListViewItem* item = currentItem();
  KMyMoneyAccountTreeItem* p = dynamic_cast<KMyMoneyAccountTreeItem *>(item);
  if(!p)
    return;

  if(p->isAccount()) {
    QTextDrag* drag = new QTextDrag(p->id(), viewport());
    drag->setSubtype("plain");

    // use the icon that is attached to the item to be dragged
    QPixmap pixmap(*p->pixmap(0));
    if(!pixmap.isNull()) {
      QPoint hotspot( pixmap.width() / 2, pixmap.height() / 2 );
      drag->setPixmap(pixmap, hotspot);
    }

    if (drag->dragMove() && drag->target() != viewport())
      emit moved();
  }
  return;
}

void KMyMoneyAccountTree::slotObjectDropped(QDropEvent* event, QListViewItem* parent, QListViewItem* after)
{
  m_autoopenTimer.stop();
  slotStopAutoScroll();
  if(dropHighlighter())
    cleanItemHighlighter();

  KMyMoneyAccountTreeItem* newParent = dynamic_cast<KMyMoneyAccountTreeItem*>(m_dropItem);
  if(newParent) {
    QCString fromId(event->encodedData("text/plain"));
    const KMyMoneyAccountTreeItem* from = findItem(fromId);

    // we can only move accounts around
    if(!from->isAccount())
      from = 0;

    if(from) {
      const MyMoneyAccount& accFrom = dynamic_cast<const MyMoneyAccount&>(from->itemObject());
      if(newParent->isAccount()) {
        const MyMoneyAccount& accTo = dynamic_cast<const MyMoneyAccount&>(newParent->itemObject());
        if(dropAccountOnAccount(accFrom, accTo)) {
          emit reparent(accFrom, accTo);
        }

      } else if(newParent->isInstitution()) {
        const MyMoneyInstitution& institution = dynamic_cast<const MyMoneyInstitution&>(newParent->itemObject());
        emit reparent(accFrom, institution);
      }
    }
  }
}

void KMyMoneyAccountTree::slotSelectObject(QListViewItem* i)
{
  emit selectObject(MyMoneyInstitution());
  emit selectObject(MyMoneyAccount());

  KMyMoneyAccountTreeItem* item = dynamic_cast<KMyMoneyAccountTreeItem*>(i);
  if(item != 0) {
    emit selectObject(item->itemObject());
  }
}

void KMyMoneyAccountTree::slotOpenContextMenu(QListViewItem* i)
{
  KMyMoneyAccountTreeItem* item = dynamic_cast<KMyMoneyAccountTreeItem *>(i);
  if(item) {
    emit selectObject(item->itemObject());

    // Create a copy of the item since the original might be destroyed
    // during processing of this signal.
    if(item->isInstitution()) {
      MyMoneyInstitution institution = dynamic_cast<const MyMoneyInstitution&>(item->itemObject());
      emit openContextMenu(institution);
    } else {
      MyMoneyAccount account = dynamic_cast<const MyMoneyAccount&>(item->itemObject());
      emit openContextMenu(account);
    }
  }
}

void KMyMoneyAccountTree::slotOpenObject(QListViewItem* i)
{
  KMyMoneyAccountTreeItem* item = dynamic_cast<KMyMoneyAccountTreeItem *>(i);
  if(item) {
    emit openObject(item->itemObject());
  }
}

/* drag and drop support inspired partially from KMail */
/* --------------------------------------------------- */
static const int autoscrollMargin = 16;
static const int initialScrollTime = 30;
static const int initialScrollAccel = 5;
static const int autoopenTime = 750;

void KMyMoneyAccountTree::slotOpenFolder(void)
{
  m_autoopenTimer.stop();
  if ( m_dropItem && !m_dropItem->isOpen() ) {
    m_dropItem->setOpen( TRUE );
    m_dropItem->repaint();
  }
}

void KMyMoneyAccountTree::slotStartAutoScroll(void)
{
  if ( !m_autoscrollTimer.isActive() ) {
    m_autoscrollTime = initialScrollTime;
    m_autoscrollAccel = initialScrollAccel;
    m_autoscrollTimer.start( m_autoscrollTime );
  }
}

void KMyMoneyAccountTree::slotStopAutoScroll(void)
{
  m_autoscrollTimer.stop();
}

void KMyMoneyAccountTree::slotAutoScroll(void)
{
  // don't show a highlighter during scrolling
  cleanItemHighlighter();

  QPoint p = viewport()->mapFromGlobal( QCursor::pos() );

  if ( m_autoscrollAccel-- <= 0 && m_autoscrollTime ) {
      m_autoscrollAccel = initialScrollAccel;
      m_autoscrollTime--;
      m_autoscrollTimer.start( m_autoscrollTime );
  }
  int l = QMAX(1,(initialScrollTime-m_autoscrollTime));

  int dx=0,dy=0;
  if ( p.y() < autoscrollMargin ) {
    dy = -l;
  } else if ( p.y() > visibleHeight()-autoscrollMargin ) {
    dy = +l;
  }
  if ( p.x() < autoscrollMargin ) {
    dx = -l;
  } else if ( p.x() > visibleWidth()-autoscrollMargin ) {
    dx = +l;
  }
  if ( dx || dy ) {
    scrollBy(dx, dy);
  } else {
    slotStopAutoScroll();
  }
}

void KMyMoneyAccountTree::contentsDragMoveEvent(QDragMoveEvent* e)
{
  QPoint vp = contentsToViewport(e->pos());
  QRect inside_margin((contentsX() > 0) ? autoscrollMargin : 0,
                      (contentsY() > 0) ? autoscrollMargin : 0,
    visibleWidth() - ((contentsX() + visibleWidth() < contentsWidth())
      ? autoscrollMargin*2 : 0),
    visibleHeight() - ((contentsY() + visibleHeight() < contentsHeight())
      ? autoscrollMargin*2 : 0));

  bool accepted = false;
  QListViewItem *i = itemAt( vp );
  if ( i ) {
    accepted = acceptDrag(e);
    if(accepted && !m_autoscrollTimer.isActive()) {
      if (dropHighlighter()) {
        QRect tmpRect = drawItemHighlighter(0, i);
        if (tmpRect != m_lastDropHighlighter) {
          cleanItemHighlighter();
          m_lastDropHighlighter = tmpRect;
          viewport()->repaint(tmpRect);
        }
      }
    }
    if ( !inside_margin.contains(vp) ) {
      slotStartAutoScroll();
      e->accept(QRect(0,0,0,0)); // Keep sending move events
      m_autoopenTimer.stop();

    } else {
      if(accepted)
        e->accept();
      else
        e->ignore();
      if ( i != m_dropItem ) {
        m_autoopenTimer.stop();
        m_dropItem = i;
        m_autoopenTimer.start( autoopenTime );
      }
    }
    if ( accepted ) {
      switch ( e->action() ) {
        case QDropEvent::Copy:
        case QDropEvent::Link:
          break;
        case QDropEvent::Move:
          e->acceptAction();
          break;
        default:
          break;
      }
    }
  } else {
    e->ignore();
    m_autoopenTimer.stop();
    m_dropItem = 0;
  }

  if(!accepted && dropHighlighter())
    cleanItemHighlighter();
}

void KMyMoneyAccountTree::cleanItemHighlighter(void)
{
  if(m_lastDropHighlighter.isValid()) {
    QRect rect=m_lastDropHighlighter;
    m_lastDropHighlighter = QRect();
    // make sure, we repaint a bit more. that's important during
    // autoscroll. if we don't do that, parts of the highlighter
    // do not get removed
    rect.moveBy(-1, -1);
    rect.setSize(rect.size() + QSize(2,2));
    viewport()->repaint(rect, true);
  }
}

void KMyMoneyAccountTree::viewportPaintEvent(QPaintEvent* e)
{
  QListView::viewportPaintEvent(e);

  if (m_lastDropHighlighter.isValid() && e->rect().intersects(m_lastDropHighlighter)) {
    QPainter painter(viewport());

    // This is where we actually draw the drop-highlighter
    style().drawPrimitive(QStyle::PE_FocusRect, &painter, m_lastDropHighlighter, colorGroup(),
                          QStyle::Style_FocusAtBorder);
  }
}




/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

const MyMoneyObject& KMyMoneyAccountTreeItem::itemObject(void) const
{
  if(m_type == Institution)
    return m_institution;
  return m_account;
}

KMyMoneyAccountTreeItem::KMyMoneyAccountTreeItem(KListView *parent, const QString& txt) :
  KListViewItem(parent),
  m_displayFactor(MyMoneyMoney(1)),
  m_totalValue(MyMoneyMoney(0)),
  m_type(Text)
{
  setText(0, txt);
}

KMyMoneyAccountTreeItem::KMyMoneyAccountTreeItem(KListView *parent, const MyMoneyInstitution& institution) :
  KListViewItem(parent),
  m_displayFactor(MyMoneyMoney(1)),
  m_institution(institution),
  m_totalValue(MyMoneyMoney(0)),
  m_type(Institution)
{
  setText(0, institution.name());
  setPixmap(0, QPixmap(KGlobal::dirs()->findResource("appdata",QString( "icons/hicolor/22x22/actions/%1.png").arg("bank"))));
}

KMyMoneyAccountTreeItem::KMyMoneyAccountTreeItem(KListView *parent, const MyMoneyAccount& account, const MyMoneySecurity& security, const QString& name) :
  KListViewItem(parent),
  m_security(security),
  m_displayFactor(MyMoneyMoney(1)),
  m_account(account),
  m_totalValue(MyMoneyMoney(0)),
  m_type(Account)
{
  MyMoneyAccount acc(account);
  if(!name.isEmpty())
    acc.setName(name);
  updateAccount(acc, true);
}

KMyMoneyAccountTreeItem::KMyMoneyAccountTreeItem(KMyMoneyAccountTreeItem *parent, const MyMoneyAccount& account, const QValueList<MyMoneyPrice>& price, const MyMoneySecurity& security) :
  KListViewItem(parent),
  m_price(price),
  m_security(security),
  m_displayFactor(MyMoneyMoney(1)),
  m_account(account),
  m_totalValue(MyMoneyMoney(0)),
  m_type(Account)
{
  updateAccount(account, true);
}

KMyMoneyAccountTreeItem::~KMyMoneyAccountTreeItem()
{
}

const QCString& KMyMoneyAccountTreeItem::id(void) const
{
  if(m_type == Institution)
    return m_institution.id();
  return m_account.id();
}

bool KMyMoneyAccountTreeItem::isChildOf(const QListViewItem* const item) const
{
  QListViewItem *p = parent();
  while(p && p != item) {
    p = p->parent();
  }
  return (p != 0);
}

void KMyMoneyAccountTreeItem::updatePrice(const MyMoneyPrice& price)
{
  MyMoneyMoney oldValue = m_value;

  // FIXME calculate new value based on price information

  // check if we need to tell upstream account objects in the tree
  // that the value has changed
  if(oldValue != m_value) {
    adjustTotalValue(m_value - oldValue);
    KMyMoneyAccountTree* p = listView();
    if(p) {
      p->emitValueChanged();
    }
  }
}

void KMyMoneyAccountTreeItem::updateAccount(const MyMoneyAccount& account, bool forceTotalUpdate)
{
  // make sure it's for the same object
  if(account.id() != m_account.id())
    return;

  QString icon;
  switch (m_account.accountGroup())
  {
    case MyMoneyAccount::Income:
      icon = "account-types_income";
      break;
    case MyMoneyAccount::Expense:
      icon = "account-types_expense";
      break;
    case MyMoneyAccount::Liability:
      icon = "account-types_liability";
      break;
    case MyMoneyAccount::Asset:
      icon = "account-types_asset";
      break;
    default:
      icon = "account";
  }
  if(m_account.isClosed()) {
    QPixmap pic = QPixmap(KGlobal::dirs()->findResource("appdata",QString( "icons/hicolor/22x22/actions/%1.png").arg(icon)));
    QPixmap closed = QPixmap(KGlobal::dirs()->findResource("appdata",QString( "icons/hicolor/22x22/actions/account-types_closed.png")));
    bitBlt(&pic, 0, 0, &closed, 0, 0, closed.width(), closed.height(), Qt::CopyROP, false);
    setPixmap(0, pic);
  } else
    setPixmap(0, QPixmap(KGlobal::dirs()->findResource("appdata",QString( "icons/hicolor/22x22/actions/%1.png").arg(icon))));

  setText(KMyMoneyAccountTree::NameColumn, account.name());

  // make sure we have the right parent object
  // for the extended features
  KMyMoneyAccountTree* lv = listView();
  if(!lv)
    return;

  MyMoneyMoney oldValue = m_value;
  m_account = account;

  m_balance = balance(account);

  // for income and liability accounts, we reverse the sign
  switch(m_account.accountGroup()) {
    case MyMoneyAccount::Income:
    case MyMoneyAccount::Liability:
      m_balance = -m_balance;
      break;

    default:
      break;
  }

  // calculate the new value by running down the price list
  m_value = m_balance;
  QValueList<MyMoneyPrice>::const_iterator it_p;
  QCString security = m_security.id();
  for(it_p = m_price.begin(); it_p != m_price.end(); ++it_p) {
    m_value = m_value * (MyMoneyMoney(1,1) / (*it_p).rate(security));
    if((*it_p).from() == security)
      security = (*it_p).to();
    else
      security = (*it_p).from();
  }

  // check if we need to update the display of values
  if(parent() && (isOpen() || m_account.accountList().count() == 0)) {
    if(m_security.id() != listView()->baseCurrency().id()) {
      setText(KMyMoneyAccountTree::BalanceColumn, m_balance.formatMoney(m_security.tradingSymbol(), MyMoneyMoney::denomToPrec(m_security.smallestAccountFraction())));
    }
    setText(KMyMoneyAccountTree::ValueColumn, m_value.formatMoney(listView()->baseCurrency().tradingSymbol(), MyMoneyMoney::denomToPrec(listView()->baseCurrency().smallestAccountFraction())) + "  ");
  }

  // check if we need to tell upstream account objects in the tree
  // that the value has changed
  if(oldValue != m_value || forceTotalUpdate) {
    adjustTotalValue(m_value - oldValue);
    lv->emitValueChanged();
  }
}

MyMoneyMoney KMyMoneyAccountTreeItem::balance( const MyMoneyAccount& account ) const
{
  // account.balance() is not compatable with stock accounts
  if ( account.accountType() == MyMoneyAccount::Stock )
    return MyMoneyFile::instance()->balance(account.id());
  else
    return account.balance();
}

void KMyMoneyAccountTreeItem::setOpen(bool open)
{
  // make sure, that we only run through the extened logic if
  // the parent is a KMyMoneyAccountTree object
  if(listView()) {
    // show the top accounts always in total value
    if((open || m_account.accountList().count() == 0) && parent()) {

      // only show the balance, if its a different security/currency
      if(m_security.id() != listView()->baseCurrency().id()) {
        setText(KMyMoneyAccountTree::BalanceColumn, m_balance.formatMoney(m_security.tradingSymbol(), MyMoneyMoney::denomToPrec(m_security.smallestAccountFraction())));
      }
      setText(KMyMoneyAccountTree::ValueColumn, m_value.formatMoney(listView()->baseCurrency().tradingSymbol(), MyMoneyMoney::denomToPrec(listView()->baseCurrency().smallestAccountFraction())) + "  ");

    } else {
      setText(KMyMoneyAccountTree::BalanceColumn, " ");
      if(parent())
        setText(KMyMoneyAccountTree::ValueColumn, m_totalValue.formatMoney(listView()->baseCurrency().tradingSymbol(), MyMoneyMoney::denomToPrec(listView()->baseCurrency().smallestAccountFraction())) + "  ");
      else
        setText(KMyMoneyAccountTree::ValueColumn, m_totalValue.formatMoney(listView()->baseCurrency().tradingSymbol(), MyMoneyMoney::denomToPrec(listView()->baseCurrency().smallestAccountFraction())));
    }
  }
  KListViewItem::setOpen(open);
}

void KMyMoneyAccountTreeItem::adjustTotalValue(const MyMoneyMoney& diff)
{
  m_totalValue += diff;

  // if the entry has no children, or it is currently not open
  // we need to display the value of it
  if(!firstChild() || (!isOpen() && firstChild())) {
    if(firstChild())
      setText(KMyMoneyAccountTree::BalanceColumn, " ");
    if(parent())
      setText(KMyMoneyAccountTree::ValueColumn, m_totalValue.formatMoney(listView()->baseCurrency().tradingSymbol(), MyMoneyMoney::denomToPrec(listView()->baseCurrency().smallestAccountFraction())) + "  ");
    else
      setText(KMyMoneyAccountTree::ValueColumn, m_totalValue.formatMoney(listView()->baseCurrency().tradingSymbol(), MyMoneyMoney::denomToPrec(listView()->baseCurrency().smallestAccountFraction())));
  }

  // now make sure, the upstream accounts also get notified about the value change
  KMyMoneyAccountTreeItem* p = dynamic_cast<KMyMoneyAccountTreeItem*>(parent());
  if(p != 0) {
    p->adjustTotalValue(diff);
  }
}

int KMyMoneyAccountTreeItem::compare(QListViewItem* i, int col, bool ascending) const
{
  KMyMoneyAccountTreeItem* item = dynamic_cast<KMyMoneyAccountTreeItem*>(i);
  // do special sorting only if
  // a) name
  // b) account
  // c) and different group
  // in all other cases use the standard sorting
  if(col != KMyMoneyAccountTree::NameColumn
  || item == 0
  || m_account.accountGroup() == item->m_account.accountGroup())
    return KListViewItem::compare(i, col, ascending);

  return (m_account.accountGroup() - item->m_account.accountGroup());
}

void KMyMoneyAccountTreeItem::paintCell(QPainter *p, const QColorGroup & cg, int column, int width, int align)
{
  QFont font = KMyMoneySettings::listCellFont();

  QColor colour = KMyMoneySettings::listColor();
  QColor bgColour = KMyMoneySettings::listBGColor();

  QColorGroup cg2(cg);

  // display base accounts in bold
  if(!parent())
    font.setBold(true);

  // strike out closed accounts
  if(m_account.value(""))            // FIXME: not defined how closed accounts will be marked in engine
    font.setStrikeOut(true);

  p->setFont(font);

  KListViewItem::paintCell(p, cg2, column, width, align);
}

#include "kmymoneyaccounttree.moc"
// vim:cin:si:ai:et:ts=2:sw=2:
