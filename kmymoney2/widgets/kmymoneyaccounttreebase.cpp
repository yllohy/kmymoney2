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
#include <kiconloader.h>
#include <kstandarddirs.h>

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoney/mymoneyfile.h>
#include <kmymoney/kmymoneyaccounttree.h>
#include <kmymoney/kmymoneyglobalsettings.h>

#include <kmymoney/kmymoneyutils.h>

KMyMoneyAccountTreeBase::KMyMoneyAccountTreeBase(QWidget* parent, const char* name) :
  KListView(parent, name),
  m_accountConnections(false),
  m_institutionConnections(false),
  m_queuedSort(0)
{
  setRootIsDecorated(true);
  setAllColumnsShowFocus(true);

  m_nameColumn = addColumn(i18n("Account"));
  setColumnWidthMode(m_nameColumn, QListView::Manual);

  m_typeColumn = -1;
  m_balanceColumn = -1;
  m_valueColumn = -1;

  setMultiSelection(false);

  setResizeMode(QListView::LastColumn);
  setShowSortIndicator(true);
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
  connect(this, SIGNAL(contextMenu(KListView*, QListViewItem* , const QPoint&)), this, SLOT(slotOpenContextMenu(KListView*, QListViewItem*, const QPoint&)));
  connect(this, SIGNAL(doubleClicked(QListViewItem*,const QPoint&,int)), this, SLOT(slotOpenObject(QListViewItem*)));

  // drag and drop timer connections
  connect( &m_autoopenTimer, SIGNAL( timeout() ), this, SLOT( slotOpenFolder() ) );
  connect( &m_autoscrollTimer, SIGNAL( timeout() ), this, SLOT( slotAutoScroll() ) );

}

KMyMoneyAccountTreeBase::~KMyMoneyAccountTreeBase()
{
  if (!m_configGroup.isEmpty())
    saveLayout(KGlobal::config(), m_configGroup);
}

void KMyMoneyAccountTreeBase::restoreLayout(const QString& group)
{
  Q_ASSERT(!group.isEmpty());
  m_configGroup = group;
  // make sure to use the previous settings. If no settings are found
  // we use equal distribution of all fields as an initial setting
  // For some reason, if the view is never selected with this code, it
  // stores a value of 32 for the columns. We have to detect that as well.
//  setColumnWidth(0, 0);
  KListView::restoreLayout(KGlobal::config(), m_configGroup);
#if 0
  if(columnWidth(0) < 60) {
    setResizeMode(QListView::AllColumns);
  }
#endif
}

void KMyMoneyAccountTreeBase::showType(void)
{
  m_typeColumn = addColumn(i18n("Type"));
  setColumnWidthMode(m_typeColumn, QListView::Manual);
  setColumnAlignment(m_typeColumn, Qt::AlignLeft);
}

void KMyMoneyAccountTreeBase::showValue(void)
{
  m_balanceColumn = addColumn(i18n("Total Balance"));
  setColumnWidthMode(m_balanceColumn, QListView::Manual);
  setColumnAlignment(m_balanceColumn, Qt::AlignRight);

  m_valueColumn = addColumn(i18n("Total Value"));
  setColumnWidthMode(m_valueColumn, QListView::Manual);
  setColumnAlignment(m_valueColumn, Qt::AlignRight);
}

void KMyMoneyAccountTreeBase::connectNotify(const char * /* s */)
{
  // update drag and drop settings
  m_accountConnections = (receivers(SIGNAL(reparent(const MyMoneyAccount&, const MyMoneyAccount&))) != 0);
  m_institutionConnections = (receivers(SIGNAL(reparent(const MyMoneyAccount&, const MyMoneyInstitution&))) != 0);
  setDragEnabled(m_accountConnections | m_institutionConnections);
  setAcceptDrops(m_accountConnections | m_institutionConnections);
}

void KMyMoneyAccountTreeBase::disconnectNotify(const char * /* s */)
{
  // update drag and drop settings
  m_accountConnections = (receivers(SIGNAL(reparent(const MyMoneyAccount&, const MyMoneyAccount&))) != 0);
  m_institutionConnections = (receivers(SIGNAL(reparent(const MyMoneyAccount&, const MyMoneyInstitution&))) != 0);
  setDragEnabled(m_accountConnections | m_institutionConnections);
  setAcceptDrops(m_accountConnections | m_institutionConnections);
}

void KMyMoneyAccountTreeBase::setSectionHeader(const QString& txt)
{
  header()->setLabel(nameColumn(), txt);
}

KMyMoneyAccountTreeBaseItem* KMyMoneyAccountTreeBase::selectedItem(void) const
{
  return dynamic_cast<KMyMoneyAccountTreeBaseItem *>(KListView::selectedItem());
}

const KMyMoneyAccountTreeBaseItem* KMyMoneyAccountTreeBase::findItem(const QCString& id) const
{
  // tried to use a  QListViewItemIterator  but that does not fit
  // with the constness of this method. Arghhh.

  QListViewItem* p = firstChild();
  while(p) {
    // item found, check for the id
    KMyMoneyAccountTreeBaseItem* item = dynamic_cast<KMyMoneyAccountTreeBaseItem*>(p);
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

  return dynamic_cast<KMyMoneyAccountTreeBaseItem*>(p);
}

bool KMyMoneyAccountTreeBase::dropAccountOnAccount(const MyMoneyAccount& accFrom, const MyMoneyAccount& accTo) const
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
      && !accFrom.isInvest())
        rc = false;

      else if(accFrom.isInvest()
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

    // if it's generally ok to drop here, make sure that
    // the accTo does not have a child with the same name
    const KMyMoneyAccountTreeBaseItem* to = findItem(accTo.id());
    if(to) {
      to = dynamic_cast<KMyMoneyAccountTreeBaseItem*> (to->firstChild());
      while(to && rc) {
        if(to->isAccount()) {
          const MyMoneyAccount& acc = dynamic_cast<const MyMoneyAccount&>(to->itemObject());
          if(acc.name() == accFrom.name())
            rc = false;
        }
        to = dynamic_cast<KMyMoneyAccountTreeBaseItem*> (to->nextSibling());
      }
    }
  }

  return rc;
}

bool KMyMoneyAccountTreeBase::acceptDrag(QDropEvent* event) const
{
  bool rc;

  if(rc = (acceptDrops() && event->source() == viewport())) {
    rc = false;
    KMyMoneyAccountTreeBaseItem* to = dynamic_cast<KMyMoneyAccountTreeBaseItem*>(itemAt( contentsToViewport(event->pos()) ));
    QCString fromId(event->encodedData("text/plain"));
    const KMyMoneyAccountTreeBaseItem* from = findItem(fromId);

    // we can only move accounts around
    if(!from->isAccount())
      from = 0;

    if(to && from && !to->isChildOf(from)) {
      const MyMoneyAccount& accFrom = dynamic_cast<const MyMoneyAccount&>(from->itemObject());

      if(to->isAccount() && m_accountConnections) {
        const MyMoneyAccount& accTo = dynamic_cast<const MyMoneyAccount&>(to->itemObject());
        rc = dropAccountOnAccount(accFrom, accTo);

      } else if(to->isInstitution() && m_institutionConnections) {
        // Moving a non-stock account to an institution is ok
        if(!accFrom.isInvest())
          rc = true;
      }
    }
  }

  return rc;
}

void KMyMoneyAccountTreeBase::startDrag(void)
{
  QListViewItem* item = currentItem();
  KMyMoneyAccountTreeBaseItem* p = dynamic_cast<KMyMoneyAccountTreeBaseItem *>(item);
  if(!p)
    return;

  if(p->isAccount()) {
    QTextDrag* drag = new QTextDrag(p->id(), viewport());
    drag->setSubtype("plain");

    // use the icon that is attached to the item to be dragged
    if (p->pixmap(0)) {
      QPixmap pixmap(*p->pixmap(0));
      QPoint hotspot( pixmap.width() / 2, pixmap.height() / 2 );
      drag->setPixmap(pixmap, hotspot);
    }

    if (drag->dragMove() && drag->target() != viewport())
      emit moved();
  }
  return;
}

void KMyMoneyAccountTreeBase::slotObjectDropped(QDropEvent* event, QListViewItem* /* parent */, QListViewItem* /* after */)
{
  m_autoopenTimer.stop();
  slotStopAutoScroll();
  if(dropHighlighter())
    cleanItemHighlighter();

  KMyMoneyAccountTreeBaseItem* newParent = dynamic_cast<KMyMoneyAccountTreeBaseItem*>(m_dropItem);
  if(newParent) {
    QCString fromId(event->encodedData("text/plain"));
    const KMyMoneyAccountTreeBaseItem* from = findItem(fromId);

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
        if(!accFrom.isInvest()) {
          const MyMoneyInstitution& institution = dynamic_cast<const MyMoneyInstitution&>(newParent->itemObject());
          emit reparent(accFrom, institution);
        }
      }
    }
  }
}

void KMyMoneyAccountTreeBase::slotSelectObject(QListViewItem* i)
{
  emit selectObject(MyMoneyInstitution());
  emit selectObject(MyMoneyAccount());

  KMyMoneyAccountTreeBaseItem* item = dynamic_cast<KMyMoneyAccountTreeBaseItem*>(i);
  if(item != 0) {
    emit selectObject(item->itemObject());
  }
}

void KMyMoneyAccountTreeBase::slotOpenContextMenu(KListView* lv, QListViewItem* i, const QPoint&)
{
  Q_UNUSED(lv);

  KMyMoneyAccountTreeBaseItem* item = dynamic_cast<KMyMoneyAccountTreeBaseItem *>(i);
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

void KMyMoneyAccountTreeBase::slotOpenObject(QListViewItem* i)
{
  KMyMoneyAccountTreeBaseItem* item = dynamic_cast<KMyMoneyAccountTreeBaseItem *>(i);
  if(item) {
    // Create a copy of the item since the original might be destroyed
    // during processing of this signal.
    if(item->isAccount()) {
      MyMoneyAccount acc = dynamic_cast<const MyMoneyAccount&>(item->itemObject());
      emit openObject(acc);
    } else if(item->isInstitution()) {
      MyMoneyInstitution inst = dynamic_cast<const MyMoneyInstitution&>(item->itemObject());
      emit openObject(inst);
    }
  }
}

/* drag and drop support inspired partially from KMail */
/* --------------------------------------------------- */
static const int autoscrollMargin = 16;
static const int initialScrollTime = 30;
static const int initialScrollAccel = 5;
static const int autoopenTime = 750;

void KMyMoneyAccountTreeBase::slotOpenFolder(void)
{
  m_autoopenTimer.stop();
  if ( m_dropItem && !m_dropItem->isOpen() ) {
    m_dropItem->setOpen( TRUE );
    m_dropItem->repaint();
  }
}

void KMyMoneyAccountTreeBase::slotStartAutoScroll(void)
{
  if ( !m_autoscrollTimer.isActive() ) {
    m_autoscrollTime = initialScrollTime;
    m_autoscrollAccel = initialScrollAccel;
    m_autoscrollTimer.start( m_autoscrollTime );
  }
}

void KMyMoneyAccountTreeBase::slotStopAutoScroll(void)
{
  m_autoscrollTimer.stop();
}

void KMyMoneyAccountTreeBase::slotAutoScroll(void)
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

void KMyMoneyAccountTreeBase::contentsDragMoveEvent(QDragMoveEvent* e)
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

void KMyMoneyAccountTreeBase::cleanItemHighlighter(void)
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

void KMyMoneyAccountTreeBase::viewportPaintEvent(QPaintEvent* e)
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

const MyMoneyObject& KMyMoneyAccountTreeBaseItem::itemObject(void) const
{
  if(m_type == Institution)
    return m_institution;
  return m_account;
}

KMyMoneyAccountTreeBaseItem::KMyMoneyAccountTreeBaseItem(KListView *parent, const MyMoneyInstitution& institution) :
  KListViewItem(parent),
  m_totalValue(MyMoneyMoney(0)),
  m_institution(institution),
  m_type(Institution)
{
  setName();
}

KMyMoneyAccountTreeBaseItem::KMyMoneyAccountTreeBaseItem(KListView *parent, const MyMoneyAccount& account, const MyMoneySecurity& security, const QString& name) :
  KListViewItem(parent),
  m_security(security),
  m_totalValue(MyMoneyMoney(0)),
  m_account(account),
  m_type(Account)
{
  if(!name.isEmpty()) {
    // we do not want to modify the original account
    MyMoneyAccount acc(account);
    acc.setName(name);
    m_account = acc;
  }
  setName();
}

KMyMoneyAccountTreeBaseItem::KMyMoneyAccountTreeBaseItem(KMyMoneyAccountTreeBaseItem *parent, const MyMoneyAccount& account, const QValueList<MyMoneyPrice>& price, const MyMoneySecurity& security) :
  KListViewItem(parent),
  m_price(price),
  m_security(security),
  m_totalValue(MyMoneyMoney(0)),
  m_account(account),
  m_type(Account)
{
  setName();
}

KMyMoneyAccountTreeBaseItem::~KMyMoneyAccountTreeBaseItem()
{
}

const QCString& KMyMoneyAccountTreeBaseItem::id(void) const
{
  if(m_type == Institution)
    return m_institution.id();
  return m_account.id();
}

bool KMyMoneyAccountTreeBaseItem::isChildOf(const QListViewItem* const item) const
{
  QListViewItem *p = parent();
  while(p && p != item) {
    p = p->parent();
  }
  return (p != 0);
}

MyMoneyMoney KMyMoneyAccountTreeBaseItem::value() const
{
  // calculate the new value by running down the price list
  MyMoneyMoney result = balance();
  QValueList<MyMoneyPrice>::const_iterator it_p;
  QCString security = m_security.id();
  for(it_p = m_price.begin(); it_p != m_price.end(); ++it_p) {
    result = result * (MyMoneyMoney(1,1) / (*it_p).rate(security));
    if((*it_p).from() == security)
      security = (*it_p).to();
    else
      security = (*it_p).from();
  }
  if (listView())
    result = result.convert(listView()->baseCurrency().smallestAccountFraction());
  return result;
}

void KMyMoneyAccountTreeBaseItem::setName()
{
  KMyMoneyAccountTreeBase* lv = dynamic_cast<KMyMoneyAccountTreeBase*>(listView());
  if (!lv)
    return;
  if (isInstitution()) {
    setPixmap(lv->nameColumn(), m_institution.pixmap());
    setText(lv->nameColumn(), m_institution.name());
  } else {
    setPixmap(lv->nameColumn(), m_account.accountGroupPixmap());
    setText(lv->nameColumn(), m_account.name());
#ifndef KMM_DESIGNER
    if(lv->typeColumn()>=0 && !MyMoneyFile::instance()->isStandardAccount(m_account.id()))
      setText(lv->typeColumn(), KMyMoneyUtils::accountTypeToString(m_account.accountType()));
#endif
  }
}

void KMyMoneyAccountTreeBaseItem::fillColumns()
{
  KMyMoneyAccountTreeBase* lv = dynamic_cast<KMyMoneyAccountTreeBase*>(listView());
  if (!lv)
    return;
  if (lv->valueColumn()<0)
    return;
  // show the top accounts always in total value
  if((isOpen() || m_account.accountList().count() == 0) && parent()) {

      // only show the balance, if its a different security/currency
    if(m_security.id() != listView()->baseCurrency().id()) {
      setText(lv->balanceColumn(), balance().formatMoney(m_security));
    }
    setText(lv->valueColumn(), m_value.formatMoney(listView()->baseCurrency()) + "  ");

  } else {
    setText(lv->balanceColumn(), " ");
    if(parent())
      setText(lv->valueColumn(), m_totalValue.formatMoney(listView()->baseCurrency()) + "  ");
    else
      setText(lv->valueColumn(), m_totalValue.formatMoney(listView()->baseCurrency()));
  }
}

void KMyMoneyAccountTreeBaseItem::updateAccount(bool forceTotalUpdate)
{
  MyMoneyMoney oldValue = m_value;
  m_value = value();

  fillColumns();

  // check if we need to tell upstream account objects in the tree
  // that the value has changed
  if(oldValue != m_value || forceTotalUpdate) {
    adjustTotalValue(m_value - oldValue);
    if (listView())
      listView()->emitValueChanged();
  }
}

void KMyMoneyAccountTreeBaseItem::setOpen(bool open)
{
  if (open == isOpen())
    return;
  KListViewItem::setOpen(open);
  fillColumns();

  if(listView())
    listView()->queueSort();
}

void KMyMoneyAccountTreeBaseItem::adjustTotalValue(const MyMoneyMoney& diff)
{
  m_totalValue += diff;

  // if the entry has no children,
  // or it is the top entry
  // or it is currently not open
  // we need to display the value of it
  KMyMoneyAccountTreeBase* lv = dynamic_cast<KMyMoneyAccountTreeBase*>(listView());
  if(!lv)
    return;
  if(!firstChild() || !parent() || (!isOpen() && firstChild())) {
    if(firstChild())
      setText(lv->balanceColumn(), " ");
    if(parent())
      setText(lv->valueColumn(), m_totalValue.formatMoney(listView()->baseCurrency()) + "  ");
    else
      setText(lv->valueColumn(), m_totalValue.formatMoney(listView()->baseCurrency()));
  }

  // now make sure, the upstream accounts also get notified about the value change
  KMyMoneyAccountTreeBaseItem* p = dynamic_cast<KMyMoneyAccountTreeBaseItem*>(parent());
  if(p != 0) {
    p->adjustTotalValue(diff);
  }
}

int KMyMoneyAccountTreeBaseItem::compare(QListViewItem* i, int col, bool ascending) const
{
  KMyMoneyAccountTreeBaseItem* item = dynamic_cast<KMyMoneyAccountTreeBaseItem*>(i);
  // do special sorting only if
  // a) name
  // b) account
  // c) and different group
  // d) value column
  // in all other cases use the standard sorting
  KMyMoneyAccountTreeBase* lv = dynamic_cast<KMyMoneyAccountTreeBase*>(listView());
  if(lv && item) {
    if (col == lv->nameColumn()) {
      if(m_account.accountGroup() != item->m_account.accountGroup())
        return (m_account.accountGroup() - item->m_account.accountGroup());
    } else if (col == lv->balanceColumn() || col == lv->valueColumn()) {
      MyMoneyMoney result = MyMoneyMoney(text(col)) - MyMoneyMoney(item->text(col));
      if(result.isNegative())
        return -1;
      if(result.isZero())
        return 0;
      return 1;
      }
  }
  // do standard sorting here
  return KListViewItem::compare(i, col, ascending);
}

void KMyMoneyAccountTreeBaseItem::paintCell(QPainter *p, const QColorGroup & cg, int column, int width, int align)
{
  QColorGroup cg2(cg);

  if(isAlternate())
    cg2.setColor(QColorGroup::Base, KMyMoneyGlobalSettings::listColor());
  else
    cg2.setColor(QColorGroup::Base, KMyMoneyGlobalSettings::listBGColor());

#ifndef KMM_DESIGNER
  // display base accounts in bold
  QFont font = KMyMoneyGlobalSettings::listCellFont();
  if(!parent())
    font.setBold(true);

  // strike out closed accounts
  if(m_account.isClosed())
    font.setStrikeOut(true);

  p->setFont(font);
#endif

  QListViewItem::paintCell(p, cg2, column, width, align);
}

void KMyMoneyAccountTreeBase::expandCollapseAll(bool expand)
{
  QListViewItemIterator it(this);
  QListViewItem* p;
  while((p = it.current()) != 0) {
    p->setOpen(expand);
    ++it;
  }
}

void KMyMoneyAccountTreeBase::slotExpandAll(void)
{
  expandCollapseAll(true);
}

void KMyMoneyAccountTreeBase::slotCollapseAll(void)
{
  expandCollapseAll(false);
}

void KMyMoneyAccountTreeBase::queueSort(void)
{
  if (sortColumn() == balanceColumn() || sortColumn() == valueColumn()) {
    ++m_queuedSort;
    QTimer::singleShot(100, this, SLOT(slotActivateSort()));
  }
}

void KMyMoneyAccountTreeBase::slotActivateSort(void)
{
  --m_queuedSort;
  if(!m_queuedSort)
    KListView::sort();
}

#include "kmymoneyaccounttreebase.moc"
// vim:cin:si:ai:et:ts=2:sw=2:
