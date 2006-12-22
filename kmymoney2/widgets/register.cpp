/***************************************************************************
                             register.cpp  -  description
                             -------------------
    begin                : Fri Mar 10 2006
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

// ----------------------------------------------------------------------------
// QT Includes

#include <qstring.h>
#include <qtimer.h>
#include <qapplication.h>
#include <qeventloop.h>
#include <qtooltip.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <klocale.h>
#include <kglobal.h>
#include <kdebug.h>

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoney/mymoneyobjectcontainer.h>
#include <kmymoney/kmymoneydateinput.h>
#include <kmymoney/kmymoneyedit.h>
#include <kmymoney/kmymoneycategory.h>
#include <kmymoney/register.h>
#include <kmymoney/transactionform.h>

#include "../kmymoneyutils.h"
#include "../kmymoneyglobalsettings.h"

const int LinesPerMemo = 3;

static QString sortOrderText[] = {
  I18N_NOOP("Unknown"),
  I18N_NOOP("Post date"),
  I18N_NOOP("Date entered"),
  I18N_NOOP("Payee"),
  I18N_NOOP("Amount"),
  I18N_NOOP("Number"),
  I18N_NOOP("Entry order"),
  I18N_NOOP("Type"),
  I18N_NOOP("Category")
  };

using namespace KMyMoneyRegister;

static char fancymarker_bg_image[] = {
/* 200x49 */
  0x89,0x50,0x4E,0x47,0x0D,0x0A,0x1A,0x0A,
  0x00,0x00,0x00,0x0D,0x49,0x48,0x44,0x52,
  0x00,0x00,0x00,0xC8,0x00,0x00,0x00,0x31,
  0x08,0x06,0x00,0x00,0x00,0x9F,0xC5,0xE6,
  0x4F,0x00,0x00,0x00,0x06,0x62,0x4B,0x47,
  0x44,0x00,0xFF,0x00,0xFF,0x00,0xFF,0xA0,
  0xBD,0xA7,0x93,0x00,0x00,0x00,0x09,0x70,
  0x48,0x59,0x73,0x00,0x00,0x0B,0x13,0x00,
  0x00,0x0B,0x13,0x01,0x00,0x9A,0x9C,0x18,
  0x00,0x00,0x00,0x86,0x49,0x44,0x41,0x54,
  0x78,0xDA,0xED,0xDD,0x31,0x0A,0x84,0x40,
  0x10,0x44,0xD1,0x1A,0x19,0x10,0xCF,0xE6,
  0xFD,0x4F,0xB2,0x88,0x08,0x22,0x9B,0x18,
  0x4E,0x1B,0x2C,0x1B,0x18,0xBC,0x07,0x7D,
  0x81,0x82,0x1F,0x77,0x4B,0xB2,0x06,0x18,
  0xEA,0x49,0x3E,0x66,0x00,0x81,0x80,0x40,
  0xE0,0xDF,0x81,0x6C,0x66,0x80,0x3A,0x90,
  0xDD,0x0C,0x50,0x07,0x72,0x98,0x01,0xEA,
  0x40,0x4E,0x33,0x40,0x1D,0xC8,0x65,0x06,
  0x18,0x6B,0xF7,0x01,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0xF0,0x16,0x3E,
  0x4C,0xC1,0x83,0x9E,0x64,0x32,0x03,0x08,
  0x04,0x7E,0x0A,0xA4,0x9B,0x01,0xEA,0x40,
  0x66,0x33,0x40,0x1D,0xC8,0x62,0x06,0x18,
  0xFB,0x02,0x05,0x87,0x08,0x55,0xFE,0xDE,
  0xA2,0x9D,0x00,0x00,0x00,0x00,0x49,0x45,
  0x4E,0x44,0xAE,0x42,0x60,0x82
};

bool ItemPtrVector::item_cmp(RegisterItem* i1, RegisterItem* i2)
{
  const QValueList<TransactionSortField>& sortOrder = i1->parent()->sortOrder();
  QValueList<TransactionSortField>::const_iterator it;
  int rc;
  bool ok1, ok2;
  Q_ULLONG n1, n2;

  MyMoneyMoney tmp;

  for(it = sortOrder.begin(); it != sortOrder.end(); ++it) {
    switch(static_cast<TransactionSortField>(abs(*it))) {
      case PostDateSort:
        rc = i2->sortPostDate().daysTo(i1->sortPostDate());
        break;

      case EntryDateSort:
        rc = i2->sortEntryDate().daysTo(i1->sortEntryDate());
        break;

      case PayeeSort:
        rc = QString::localeAwareCompare(i1->sortPayee(), i2->sortPayee());
        break;

      case ValueSort:
        tmp = i1->sortValue() - i2->sortValue();
        if(tmp.isZero())
          rc = 0;
        else if(tmp.isNegative())
          rc = -1;
        else
          rc = 1;
        break;

      case NoSort:
        // convert both values to numbers
        n1 = i1->sortNumber().toULongLong(&ok1);
        n2 = i2->sortNumber().toULongLong(&ok2);
        // the following four cases exist:
        // a) both are converted correct
        //    compare them directly
        // b) n1 is numeric, n2 is not
        //    numbers come first, so return -1
        // c) n1 is not numeric, n2 is
        //    numbers come first, so return 1
        // d) both are non numbers
        //    compare using localeAwareCompare
        if(ok1 && ok2) {  // case a)
          rc = (n1 > n2) ? 1 : ((n1 == n2 ) ? 0 : -1);
        } else if(ok1 && !ok2) {
          rc = -1;
        } else if(!ok1 && ok2) {
          rc = 1;
        } else
          rc = QString::localeAwareCompare(i1->sortNumber(), i2->sortNumber());
        break;

      case EntryOrderSort:
        rc = qstrcmp(i1->sortEntryOrder(), i2->sortEntryOrder());
        break;

      case TypeSort:
        rc = i1->sortType() - i2->sortType();
        break;

      case CategorySort:
        rc = QString::localeAwareCompare(i1->sortCategory(), i2->sortCategory());
        break;

      default:
        qDebug("Invalid sort key %d", *it);
        rc = 0;
        break;
    }

    if(rc == 0) {
      if(dynamic_cast<GroupMarker*>(i1) == 0) {
        if(dynamic_cast<GroupMarker*>(i2) == 0) {
          continue;
        }
        return false;
      }
      return true;
    }

    return (*it < 0) ? rc >= 0 : rc < 0;
  }

  if(rc == 0)
    rc = qstrcmp(i1->sortEntryOrder(), i2->sortEntryOrder());

  return rc < 0;
}

GroupMarker::GroupMarker(Register *parent) :
  RegisterItem(parent),
  m_lastCol(2)
{
  // convert the backgroud once
  QByteArray a;
  a.setRawData(fancymarker_bg_image, sizeof(fancymarker_bg_image));
  m_bg.loadFromData(a);
  a.resetRawData(fancymarker_bg_image, sizeof(fancymarker_bg_image));
}

void GroupMarker::paintRegisterCell(QPainter* painter, int row, int col, const QRect& _r, bool /*selected*/, const QColorGroup& _cg)
{
  if(col == m_lastCol+1) {
    m_lastCol = col;
    return;
  }
  m_lastCol = col;

  QRect r(_r);
  painter->save();
  painter->translate(-r.x(), -r.y());

  // the group marker always uses all cols
  r.setX(m_parent->columnPos(0));
  r.setWidth(m_parent->visibleWidth());
  painter->translate(r.x(), r.y());

  QRect cellRect;
  cellRect.setX(0);
  cellRect.setY(0);
  cellRect.setWidth(m_parent->visibleWidth());
  cellRect.setHeight(m_parent->rowHeight(row + m_startRow));

  // clear out cell rectangle
  QColorGroup cg(_cg);
  if(m_alternate)
    cg.setColor(QColorGroup::Base, KMyMoneySettings::listColor());
  else
    cg.setColor(QColorGroup::Base, KMyMoneySettings::listBGColor());
  QBrush backgroundBrush(cg.base());
  painter->fillRect(cellRect, backgroundBrush);
  painter->setPen(KMyMoneySettings::listGridColor());
  painter->drawLine(cellRect.x(), cellRect.height()-1, cellRect.width(), cellRect.height()-1);

#if 0
  // now draw round rectangle in marker color
  // adjust rectangle a bit before
  cellRect.setX(5);
  cellRect.setY(5);
  cellRect.setWidth(cellRect.width()-5);
  cellRect.setHeight(cellRect.height()-5);

  cg.setColor(QColorGroup::Base, KMyMoneySettings::listMissingConversionRate());
  painter->setBrush(cg.base());
  painter->setPen(Qt::NoPen);
  painter->drawRoundRect(cellRect, 2, 25);

#endif
  // now write the text
  painter->setPen(cg.text());
  QFont font = painter->font();
  int size = font.pointSize();
  if(size == -1) {
    size = font.pixelSize();
    font.setPixelSize(size*2);
  } else {
    font.setPointSize(size*2);
  }
  painter->setFont(font);
  // cellRect.setX(cellRect.x()+5);
  // cellRect.setWidth(cellRect.width()-10);

  painter->drawText(cellRect, Qt::AlignVCenter | Qt::AlignCenter, m_txt);

  // cellRect.setX(cellRect.x()-5);
  // cellRect.setWidth(cellRect.width()+10);

  cellRect.setHeight(m_bg.height());
  painter->drawTiledPixmap(cellRect, m_bg);
  painter->restore();
}

int GroupMarker::rowHeightHint(void) const
{
  return m_bg.height();
}

FancyDateGroupMarker::FancyDateGroupMarker(Register* parent, const QDate& date, const QString& txt) :
  GroupMarker(parent)
{
  m_txt = txt;
  m_date = date;
}

SimpleDateGroupMarker::SimpleDateGroupMarker(Register* parent, const QDate& date, const QString& txt) :
  FancyDateGroupMarker(parent, date, txt)
{
}

int SimpleDateGroupMarker::rowHeightHint(void) const
{
  return RegisterItem::rowHeightHint() / 2;
}

void SimpleDateGroupMarker::paintRegisterCell(QPainter* painter, int row, int /*col*/, const QRect& _r, bool /*selected*/, const QColorGroup& _cg)
{
  QRect r(_r);
  painter->save();
  painter->translate(-r.x(), -r.y());

  // the group marker always uses all cols
  r.setX(m_parent->columnPos(0));
  r.setWidth(m_parent->visibleWidth());
  painter->translate(r.x(), r.y());

  QRect cellRect;
  cellRect.setX(0);
  cellRect.setY(0);
  cellRect.setWidth(m_parent->visibleWidth());
  cellRect.setHeight(m_parent->rowHeight(row + m_startRow));

  // clear out cell rectangle
  QColorGroup cg(_cg);
  if(m_alternate)
    cg.setColor(QColorGroup::Base, KMyMoneySettings::listColor());
  else
    cg.setColor(QColorGroup::Base, KMyMoneySettings::listBGColor());
  QBrush backgroundBrush(cg.base());
  // backgroundBrush.setStyle(Qt::DiagCrossPattern);
  backgroundBrush.setStyle(Qt::Dense5Pattern);
  backgroundBrush.setColor(KMyMoneySettings::listGridColor());
  painter->eraseRect(cellRect);
  painter->fillRect(cellRect, backgroundBrush);
  painter->setPen(KMyMoneySettings::listGridColor());
  painter->drawLine(cellRect.x(), cellRect.height()-1, cellRect.width(), cellRect.height()-1);

  painter->restore();
}

TypeGroupMarker::TypeGroupMarker(Register* parent, CashFlowDirection dir, MyMoneyAccount::accountTypeE accType) :
  GroupMarker(parent),
  m_dir(dir)
{
  switch(dir) {
    case Deposit:
      m_txt = i18n("Deposits onto account", "Deposits");
      if(accType == MyMoneyAccount::CreditCard) {
        m_txt = i18n("Payments towards credit card", "Payments");
      }
      break;
    case Payment:
      m_txt = i18n("Payments made from account", "Payments");
      if(accType == MyMoneyAccount::CreditCard) {
        m_txt = i18n("Payments made with credit card", "Charges");
      }
      break;
    default:
      qDebug("Unknown CashFlowDirection %d for TypeGroupMarker constructor", dir);
      break;
  }
}

PayeeGroupMarker::PayeeGroupMarker(Register* parent, const QString& name) :
  GroupMarker(parent)
{
  m_txt = name;
}

CategoryGroupMarker::CategoryGroupMarker(Register* parent, const QString& category) :
  GroupMarker(parent)
{
  m_txt = category;
}

class RegisterToolTip : public QToolTip
{
public:
  RegisterToolTip(QWidget* parent, Register* reg);
  void maybeTip(const QPoint& pos);
private:
  Register* m_register;
};

RegisterToolTip::RegisterToolTip(QWidget* parent, Register * reg) :
  QToolTip(parent),
  m_register(reg)
{
}

void RegisterToolTip::maybeTip(const QPoint& pos)
{
  QPoint cpos = m_register->viewportToContents(pos);
  // qDebug("RegisterToolTip::mayBeTip(%d,%d)", cpos.x(), cpos.y());
  int row = m_register->rowAt(cpos.y());
  int col = m_register->columnAt(cpos.x());
  RegisterItem* item = m_register->itemAtRow(row);
  if(!item)
    return;

  QPoint relPos(cpos.x() - m_register->columnPos(0), cpos.y() - m_register->rowPos(item->startRow()));
  row = row - item->startRow();

  // qDebug("row = %d, col = %d", row, col);
  // qDebug("relpos = %d,%d", relPos.x(), relPos.y());
  QString msg;
  QRect rect;
  if(!item->maybeTip(cpos, row, col, rect, msg))
    return;

  QPoint tl(rect.topLeft());
  QPoint br(rect.bottomRight());
  QRect r = QRect(m_register->contentsToViewport(tl), m_register->contentsToViewport(br));
  tip(r, msg);
  return;
}

Register::Register(QWidget *parent, const char *name ) :
  TransactionEditorContainer(parent, name),
  m_selectAnchor(0),
  m_focusItem(0),
  m_firstItem(0),
  m_lastItem(0),
  m_firstErronous(0),
  m_lastErronous(0),
  m_markErronousTransactions(0),
  m_rowHeightHint(0),
  m_listsDirty(false),
  m_ignoreNextButtonRelease(false),
  m_buttonState(Qt::ButtonState(0))
{
  m_tooltip = new RegisterToolTip(viewport(), this);

  setNumCols(MaxColumns);
  setCurrentCell(0, 1);

  horizontalHeader()->setLabel(NumberColumn, i18n("No."));
  horizontalHeader()->setLabel(DateColumn, i18n("Date"));
  horizontalHeader()->setLabel(AccountColumn, i18n("Account"));
  horizontalHeader()->setLabel(SecurityColumn, i18n("Security"));
  horizontalHeader()->setLabel(DetailColumn, i18n("Details"));
  horizontalHeader()->setLabel(ReconcileFlagColumn, i18n("C"));
  horizontalHeader()->setLabel(PaymentColumn, i18n("Payment"));
  horizontalHeader()->setLabel(DepositColumn, i18n("Deposit"));
  horizontalHeader()->setLabel(BalanceColumn, i18n("Balance"));
  horizontalHeader()->setLabel(AmountColumn, i18n("Amount"));
  horizontalHeader()->setLabel(PriceColumn, i18n("Price"));
  horizontalHeader()->setLabel(ValueColumn, i18n("Value"));

  setLeftMargin(0);
  verticalHeader()->hide();

  for(int i = 0; i < numCols(); ++i)
    setColumnStretchable(i, false);

  horizontalHeader()->setResizeEnabled(false);
  horizontalHeader()->setMovingEnabled(false);
  horizontalHeader()->setClickEnabled(false);

  horizontalHeader()->installEventFilter(this);

  // never show horizontal scroll bars
  setHScrollBarMode(QScrollView::AlwaysOff);

  connect(this, SIGNAL(clicked(int, int, int, const QPoint&)), this, SLOT(selectItem(int, int, int, const QPoint&)));
  connect(this, SIGNAL(doubleClicked(int, int, int, const QPoint&)), this, SLOT(slotDoubleClicked(int, int, int, const QPoint&)));

  // QTimer::singleShot(500, this, SLOT(slotToggleErronousTransactions()));
}

Register::~Register()
{
  clear();
  delete m_tooltip;
  m_tooltip = 0;
}

bool Register::eventFilter(QObject* o, QEvent* e)
{
  if(o == horizontalHeader() && e->type() == QEvent::MouseButtonPress) {
    emit headerClicked();
    return true;
  }

  return QTable::eventFilter(o, e);
}

void Register::setupRegister(const MyMoneyAccount& account, bool showAccountColumn)
{
  m_account = account;
  bool enabled = isUpdatesEnabled();
  setUpdatesEnabled(false);

  for(int i = 0; i < MaxColumns; ++i)
    hideColumn(i);

  horizontalHeader()->setLabel(PaymentColumn, i18n("Payment made from account", "Payment"));
  horizontalHeader()->setLabel(DepositColumn, i18n("Deposit into account", "Deposit"));

  if(account.id().isEmpty()) {
    setUpdatesEnabled(enabled);
    return;
  }

  // turn on standard columns
  showColumn(DateColumn);
  showColumn(DetailColumn);
  showColumn(ReconcileFlagColumn);

  // balance
  switch(account.accountType()) {
    case MyMoneyAccount::Investment:
    case MyMoneyAccount::Stock:
      break;
    default:
      showColumn(BalanceColumn);
      break;
  }

  // Number column
  switch(account.accountType()) {
    case MyMoneyAccount::Savings:
    case MyMoneyAccount::Cash:
    case MyMoneyAccount::Loan:
    case MyMoneyAccount::AssetLoan:
    case MyMoneyAccount::Asset:
    case MyMoneyAccount::Liability:
    case MyMoneyAccount::Equity:
      if(KMyMoneySettings::alwaysShowNrField())
        showColumn(NumberColumn);
      break;

    case MyMoneyAccount::Checkings:
    case MyMoneyAccount::CreditCard:
      showColumn(NumberColumn);
      break;

    default:
      hideColumn(NumberColumn);
      break;
  }

  if(showAccountColumn)
    showColumn(AccountColumn);

  // Security, activity, payment, deposit, amount, price and value column
  switch(account.accountType()) {
    default:
      showColumn(PaymentColumn);
      showColumn(DepositColumn);
      break;

    case MyMoneyAccount::Investment:
      showColumn(SecurityColumn);
      showColumn(AmountColumn);
      showColumn(PriceColumn);
      showColumn(ValueColumn);
      break;
  }

  // headings
  switch(account.accountType()) {
    case MyMoneyAccount::CreditCard:
      horizontalHeader()->setLabel(PaymentColumn, i18n("Payment made with credit card", "Charge"));
      horizontalHeader()->setLabel(DepositColumn, i18n("Payment towards credit card", "Payment"));
      break;
    case MyMoneyAccount::Asset:
    case MyMoneyAccount::AssetLoan:
      horizontalHeader()->setLabel(PaymentColumn, i18n("Decrease of asset/liability value", "Decrease"));
      horizontalHeader()->setLabel(DepositColumn, i18n("Increase of asset/liability value", "Increase"));
      break;
    case MyMoneyAccount::Liability:
    case MyMoneyAccount::Loan:
      horizontalHeader()->setLabel(PaymentColumn, i18n("Increase of asset/liability value", "Increase"));
      horizontalHeader()->setLabel(DepositColumn, i18n("Decrease of asset/liability value", "Decrease"));
      break;
    default:
      break;
  }

  switch(account.accountType()) {
    case MyMoneyAccount::Investment:
      m_lastCol = ValueColumn;
      break;

    default:
      m_lastCol = BalanceColumn;
      break;
  }

  setUpdatesEnabled(enabled);
}

bool Register::focusNextPrevChild(bool next)
{
  return QFrame::focusNextPrevChild(next);
}

void Register::setSortOrder(const QString& order)
{
  QStringList orderList = QStringList::split(",", order);
  QStringList::const_iterator it;
  m_sortOrder.clear();
  for(it = orderList.begin(); it != orderList.end(); ++it) {
    m_sortOrder << static_cast<TransactionSortField>((*it).toInt());
  }
}

void Register::sortItems(void)
{
  if(m_items.count() == 0)
    return;

  // sort the array of pointers to the transactions
  m_items.sort();

  // update the next/prev item chains
  RegisterItem* prev = 0;
  m_firstItem = m_lastItem = 0;
  for(QValueVector<RegisterItem*>::size_type i = 0; i < m_items.size(); ++i) {
    RegisterItem* item = m_items[i];
    if(!item)
      continue;

    if(!m_firstItem)
      m_firstItem = item;
    m_lastItem = item;
    if(prev)
      prev->setNextItem(item);
    item->setPrevItem(prev);
    item->setNextItem(0);
    prev = item;
  }

  // force update of the item index (row to item array)
  m_listsDirty = true;
}

TransactionSortField Register::primarySortKey(void) const
{
  if(!m_sortOrder.isEmpty())
    return static_cast<KMyMoneyRegister::TransactionSortField>(abs(m_sortOrder.first()));
  return UnknownSort;
}


void Register::clear(void)
{
  m_firstErronous = m_lastErronous = 0;

  RegisterItem* p;
  while((p = firstItem()) != 0) {
    delete p;
  }
  m_items.clear();

  m_firstItem = m_lastItem = 0;

  m_listsDirty = true;
  m_selectAnchor = 0;
  m_focusItem = 0;

  // recalculate row height hint
  QFontMetrics fm( KMyMoneyGlobalSettings::listCellFont() );
  m_rowHeightHint = fm.lineSpacing()+6;
}

void Register::insertItemAfter(RegisterItem*p, RegisterItem* prev)
{
  RegisterItem* next = 0;
  if(!prev)
    prev = lastItem();

  if(prev) {
    next = prev->nextItem();
    prev->setNextItem(p);
  }
  if(next)
    next->setPrevItem(p);

  p->setPrevItem(prev);
  p->setNextItem(next);

  if(!m_firstItem)
    m_firstItem = p;
  if(!m_lastItem)
    m_lastItem = p;

  if(prev == m_lastItem)
    m_lastItem = p;

  m_listsDirty = true;
}

void Register::addItem(RegisterItem* p)
{
  RegisterItem* q = lastItem();
  if(q)
    q->setNextItem(p);
  p->setPrevItem(q);
  p->setNextItem(0);

  m_items.append(p);

  if(!m_firstItem)
    m_firstItem = p;
  m_lastItem = p;
  m_listsDirty = true;
}

void Register::removeItem(RegisterItem* p)
{
  // remove item from list
  if(p->prevItem())
    p->prevItem()->setNextItem(p->nextItem());
  if(p->nextItem())
    p->nextItem()->setPrevItem(p->prevItem());

  // update first and last pointer if required
  if(p == m_firstItem)
    m_firstItem = p->nextItem();
  if(p == m_lastItem)
    m_lastItem = p->prevItem();

  // make sure we don't do it twice
  p->setNextItem(0);
  p->setPrevItem(0);

  // remove it from the m_items array
  for(QValueVector<RegisterItem*>::size_type i = 0; i < m_items.size(); ++i) {
    RegisterItem* item = m_items[i];
    if(!item)
      continue;
    if(item == p) {
      m_items[i] = 0;
      break;
    }
  }
  m_listsDirty = true;
}

RegisterItem* Register::firstItem(void) const
{
  return m_firstItem;
}

RegisterItem* Register::lastItem(void) const
{
  return m_lastItem;
}

void Register::setupItemIndex(int rowCount)
{
  // setup index array
  m_itemIndex.clear();
  m_itemIndex.reserve(rowCount);

  // fill index array
  rowCount = 0;
  RegisterItem* prev = 0;
  m_firstItem = m_lastItem = 0;
  for(QValueVector<RegisterItem*>::size_type i = 0; i < m_items.size(); ++i) {
    RegisterItem* item = m_items[i];
    if(!item)
      continue;
    if(!m_firstItem)
      m_firstItem = item;
    m_lastItem = item;
    if(prev)
      prev->setNextItem(item);
    item->setPrevItem(prev);
    item->setNextItem(0);
    prev = item;
    if(item->isVisible()) {
      for(int j = item->numRowsRegister(); j; --j) {
        m_itemIndex.push_back(item);
      }
    }
  }
}

void Register::drawContents( QPainter *p, int cx, int cy, int cw, int ch )
{
  // the QTable::drawContents() method does not honor the block update flag
  // so we take care of it here
  if ( testWState(WState_Visible|WState_BlockUpdates) != WState_Visible )
    return;

  if(m_listsDirty) {
    updateRegister(KMyMoneySettings::ledgerLens() | !KMyMoneySettings::transactionForm());
  }

  QTable::drawContents(p, cx, cy, cw, ch);
}

void Register::updateRegister(bool forceUpdateRowHeight)
{
  ::timetrace("Update register");
  if(m_listsDirty || forceUpdateRowHeight) {
    // don't get in here recursively
    m_listsDirty = false;

    int rowCount = 0;
    // determine the number of rows we need to display all items
    // while going through the list, check for erronous transactions
    bool alternate = false;
    for(QValueVector<RegisterItem*>::size_type i = 0; i < m_items.size(); ++i) {
      RegisterItem* item = m_items[i];
      if(!item)
        continue;
      if(item->isVisible()) {
        item->setStartRow(rowCount);
        item->setNeedResize();
        item->setAlternate(alternate);
        alternate ^= true;
        rowCount += item->numRowsRegister();

        if(item->isErronous()) {
          if(!m_firstErronous)
            m_firstErronous = item;
          m_lastErronous = item;
        }
      }
    }

    // create item index
    setupItemIndex(rowCount);

    bool needUpdateHeaders = (numRows() != rowCount) | forceUpdateRowHeight;

    // setup QTable.  Make sure to suppress screen updates for now
    bool updatesEnabled = isUpdatesEnabled();
    setUpdatesEnabled(false);
    setNumRows(rowCount);
    setUpdatesEnabled(updatesEnabled);

    // if we need to update the headers, we do it now for all rows
    // again we make sure to suppress screen updates
    if(needUpdateHeaders) {
      // int height = rowHeightHint();

      verticalHeader()->setUpdatesEnabled(false);

      for(int i = 0; i < rowCount; ++i) {
        RegisterItem* item = itemAtRow(i);
        verticalHeader()->resizeSection(i, item->rowHeightHint());
      }
      verticalHeader()->setUpdatesEnabled(true);
    }

    // add or remove scrollbars as required
    updateScrollBars();

    setUpdatesEnabled(updatesEnabled);

    // force resizeing of the columns
    QTimer::singleShot(0, this, SLOT(resize()));
  }
  ::timetrace("Done updateing register");
}

int Register::rowHeightHint(void) const
{
  if(!m_rowHeightHint) {
    qDebug("Register::rowHeightHint(): m_rowHeightHint is zero!!");
  }
  return m_rowHeightHint;
}

void Register::paintCell(QPainter* painter, int row, int col, const QRect& r, bool selected, const QColorGroup& cg)
{
  // determine the item that we need to paint in the row and call it's paintRegisterCell() method
  if(row < 0 || ((unsigned)row) > m_itemIndex.size()) {
    qDebug("Register::paintCell: row %d out of bounds %d", row, m_itemIndex.size());
    return;
  }

  // qDebug("paintCell(%d,%d)", row, col);
  RegisterItem* const item = m_itemIndex[row];
  item->paintRegisterCell(painter, row - item->startRow(), col, r, selected, cg);
}

void Register::focusInEvent(QFocusEvent* ev)
{
  QTable::focusInEvent(ev);
  if(m_focusItem) {
    m_focusItem->setFocus(true, false);
    repaintItems(m_focusItem);
  }
}

void Register::focusOutEvent(QFocusEvent* ev)
{
  if(m_focusItem) {
    m_focusItem->setFocus(false, false);
    repaintItems(m_focusItem);
  }
  QTable::focusOutEvent(ev);
}

void Register::resize(void)
{
  resize(DetailColumn);
}

void Register::resize(int col)
{
  bool enabled = isUpdatesEnabled();
  setUpdatesEnabled(false);

  // resize the register
  int w = visibleWidth();

  // check which space we need
  adjustColumn(NumberColumn);
  if(columnWidth(PaymentColumn))
    adjustColumn(PaymentColumn);
  if(columnWidth(DepositColumn))
    adjustColumn(DepositColumn);
  if(columnWidth(BalanceColumn))
    adjustColumn(BalanceColumn);
  if(columnWidth(PriceColumn))
    adjustColumn(PriceColumn);
  if(columnWidth(ValueColumn))
    adjustColumn(ValueColumn);

  // make amount columns all the same size
  // only extend the entry columns to make sure they fit
  // the widget
  int dwidth = 0;
  int ewidth = 0;
  if(ewidth < columnWidth(PaymentColumn))
    ewidth = columnWidth(PaymentColumn);
  if(ewidth < columnWidth(DepositColumn))
    ewidth = columnWidth(DepositColumn);
  if(dwidth < columnWidth(BalanceColumn))
    dwidth = columnWidth(BalanceColumn);
  if(ewidth < columnWidth(PriceColumn))
    ewidth = columnWidth(PriceColumn);
  if(dwidth < columnWidth(ValueColumn))
    dwidth = columnWidth(ValueColumn);

  // Resize the date and money fields to either
  // a) the size required by the input widget if no transaction form is shown
  // b) the adjusted value for the input widget if the transaction form is visible
  if(!KMyMoneySettings::transactionForm()) {
    kMyMoneyDateInput* dateField = new kMyMoneyDateInput(0);
    kMyMoneyEdit* valField = new kMyMoneyEdit(0);

    dateField->setFont(KMyMoneyGlobalSettings::listCellFont());
    setColumnWidth(DateColumn, dateField->minimumSizeHint().width());
    valField->setMinimumWidth(ewidth);
    ewidth = valField->minimumSizeHint().width();

    delete valField;
    delete dateField;
  } else {
    adjustColumn(DateColumn);
  }

  if(columnWidth(PaymentColumn))
    setColumnWidth(PaymentColumn, ewidth);
  if(columnWidth(DepositColumn))
    setColumnWidth(DepositColumn, ewidth);
  if(columnWidth(BalanceColumn))
    setColumnWidth(BalanceColumn, dwidth);
  if(columnWidth(PriceColumn))
    setColumnWidth(PriceColumn, ewidth);
  if(columnWidth(ValueColumn))
    setColumnWidth(ValueColumn, dwidth);

  if(columnWidth(ReconcileFlagColumn))
    setColumnWidth(ReconcileFlagColumn, 20);

  for(int i = 0; i < numCols(); ++i) {
    if(i == col)
      continue;

    w -= columnWidth(i);
  }
  setColumnWidth(col, w);

  setUpdatesEnabled(enabled);
  updateContents();
}


void Register::adjustColumn(int col)
{
  QString msg = "%1 adjusting column %2";
  ::timetrace((msg.arg("Start").arg(col)).data());
  QHeader *topHeader = horizontalHeader();
  QFontMetrics cellFontMetrics(KMyMoneyGlobalSettings::listCellFont());

  int w = topHeader->fontMetrics().width( topHeader->label( col ) ) + 10;
  if ( topHeader->iconSet( col ) )
    w += topHeader->iconSet( col )->pixmap().width();
  w = QMAX( w, 20 );

  // check for date column
  if(col == DateColumn) {
    QString txt = KGlobal::locale()->formatDate(QDate(6999,12,29), true);
    int nw = cellFontMetrics.width(txt+"  ");
    w = QMAX( w, nw );
  } else {

    // scan through the transactions
    for(unsigned i = 0; i < m_items.size(); ++i) {
      RegisterItem* const item = m_items[i];
      if(!item)
        continue;
      Transaction* t = dynamic_cast<Transaction*>(item);
      if(t) {
        int nw = t->registerColWidth(col, cellFontMetrics);
        w = QMAX( w, nw );
      }
    }
  }
  setColumnWidth( col, w );

  ::timetrace(msg.arg("Done").arg(col));
}

void Register::repaintItems(RegisterItem* first, RegisterItem* last)
{
  if(first == 0 && last == 0) {
    first = firstItem();
    last = lastItem();
  }

  if(first == 0)
    return;

  if(last == 0)
    last = first;

  // qDebug("repaintItems from row %d to row %d", first->startRow(), last->startRow()+last->numRowsRegister()-1);

  // the following code is based on code I found in
  // QTable::cellGeometry() and QTable::updateCell()  (ipwizard)
  QRect cg(0,
           rowPos(first->startRow()),
           visibleWidth(),
           rowPos(last->startRow()+last->numRowsRegister()-1) - rowPos(first->startRow()) + rowHeight(last->startRow()+last->numRowsRegister()-1));

  QRect r(contentsToViewport(QPoint (cg.x() - 2, cg.y() - 2 )), QSize(cg.width() + 4, cg.height() + 4 ));

  QRect tmp = m_lastRepaintRect | r;
  if(abs(tmp.height()) > 3000) {
    // make sure that the previously triggered repaint has been done before we
    // trigger the next. Not having this used to cause some trouble when changing
    // the focus within a 2000 item ledger from the last to the first item.
    QApplication::eventLoop()->processEvents(QEventLoop::ExcludeUserInput, 10);
  }
  m_lastRepaintRect = r;
  QApplication::postEvent( viewport(), new QPaintEvent( r, FALSE ) );

}

void Register::clearSelection(void)
{
  unselectItems();
}

void Register::doSelectItems(int from, int to, bool selected)
{
  int start, end;
  // make sure start is smaller than end
  if(from <= to) {
    start = from;
    end = to;
  } else {
    start = to;
    end = from;
  }
  // make sure we stay in bounds
  if(start < 0)
    start = 0;
  if((end <= -1) || ((unsigned)end > (m_items.size()-1)))
    end = m_items.size()-1;

  RegisterItem* firstItem;
  RegisterItem* lastItem;
  firstItem = lastItem = 0;
  for(int i = start; i <= end; ++i) {
    RegisterItem* const item = m_items[i];
    if(item) {
      if(selected != item->isSelected()) {
        if(!firstItem)
          firstItem = item;
        item->setSelected(selected);
        lastItem = item;
      }
    }
  }

  // anything changed?
  if(firstItem || lastItem)
    repaintItems(firstItem, lastItem);
}

RegisterItem* Register::itemAtRow(int row) const
{
  if(row >= 0 && (unsigned)row < m_itemIndex.size()) {
    return m_itemIndex[row];
  }
  return 0;
}

int Register::rowToIndex(int row) const
{
  for(unsigned i = 0; i < m_items.size(); ++i) {
    RegisterItem* const item = m_items[i];
    if(!item)
      continue;
    if(row >= item->startRow() && row < (item->startRow() + item->numRowsRegister()))
      return i;
  }
  return -1;
}

void Register::selectedTransactions(QValueList<SelectedTransaction>& list) const
{
  for(unsigned i = 0; i < m_items.size(); ++i) {
    RegisterItem* const item = m_items[i];
    if(item && item->isSelected()) {
      Transaction* t = dynamic_cast<Transaction*>(item);
      if(t) {
        SelectedTransaction s(t->transaction(), t->split());
        list << s;
      }
    }
  }
}

QValueList<RegisterItem*> Register::selectedItems(void) const
{
  QValueList<RegisterItem*> list;

  RegisterItem* item = m_firstItem;
  while(item) {
    if(item && item->isSelected()) {
      list << item;
    }
    item = item->nextItem();
  }
  return list;
}

int Register::selectedItemsCount(void) const
{
  int cnt = 0;
  RegisterItem* item = m_firstItem;
  while(item) {
    if(item->isSelected())
      ++cnt;
    item = item->nextItem();
  }
  return cnt;
}

void Register::contentsMouseReleaseEvent( QMouseEvent *e )
{
  if(m_ignoreNextButtonRelease) {
    m_ignoreNextButtonRelease = false;
    return;
  }

  m_buttonState = e->state();
  QTable::contentsMouseReleaseEvent(e);
}

void Register::selectItem(int row, int col, int button, const QPoint& /* mousePos */)
{
  if(row >= 0 && (unsigned)row < m_itemIndex.size()) {
    RegisterItem* item = m_itemIndex[row];
    QCString id = item->id();
    selectItem(item);
    // selectItem() might have changed the pointers, so we
    // need to reconstruct it here
    item = itemById(id);
    Transaction* t = dynamic_cast<Transaction*>(item);
    if(t) {
      if(!id.isEmpty()) {
        switch(button & Qt::MouseButtonMask) {
          case Qt::RightButton:
            emit openContextMenu();
            break;

          case Qt::LeftButton:
            if(t && col == ReconcileFlagColumn && selectedItemsCount() == 1)
              emit reconcileStateColumnClicked(t);
            break;

          default:
            break;
        }
      } else {
        emit emptyItemSelected();
      }
    }
  }
}

void Register::setFocusItem(RegisterItem* focusItem)
{
  if(focusItem->canHaveFocus()) {
    if(m_focusItem) {
      m_focusItem->setFocus(false);
      // issue a repaint here only if we move the focus
      if(m_focusItem != focusItem)
        repaintItems(m_focusItem);
    }
    Transaction* item = dynamic_cast<Transaction*>(focusItem);
    if(m_focusItem != focusItem && item) {
      emit focusChanged(item);
    }

    m_focusItem = focusItem;
    m_focusItem->setFocus(true);
    repaintItems(m_focusItem);
  }
}

void Register::selectItem(RegisterItem* item)
{
  if(!item)
    return;

  kdDebug(2) << "Register::selectItem(" << item << "): type is " << typeid(*item).name() << endl;

  Qt::ButtonState buttonState = m_buttonState;
  m_buttonState = Qt::NoButton;

  if(item->isSelectable()) {
    QCString id = item->id();
    int cnt = selectedItemsCount();
    if(buttonState & Qt::LeftButton) {
      switch(buttonState & (Qt::ShiftButton | Qt::ControlButton)) {
        default:
          if((cnt != 1) || ((cnt == 1) && !item->isSelected())) {
            emit aboutToSelectItem(item);
            // pointer 'item' might have changed. reconstruct it.
            item = itemById(id);
            unselectItems();
            item->setSelected(true);
            setFocusItem(item);
          }
          m_selectAnchor = item;
          break;

        case Qt::ControlButton:
          // toggle selection state of current item
          emit aboutToSelectItem(item);
          // pointer 'item' might have changed. reconstruct it.
          item = itemById(id);
          item->setSelected(!item->isSelected());
          setFocusItem(item);
          break;

        case Qt::ShiftButton:
          emit aboutToSelectItem(item);
          // pointer 'item' might have changed. reconstruct it.
          item = itemById(id);
          unselectItems();
          selectItems(rowToIndex(m_selectAnchor->startRow()), rowToIndex(item->startRow()));
          setFocusItem(item);
          break;
      }
    } else if(buttonState & Qt::RightButton) {
      // if the right button is pressed then only change the
      // selection if none of the Shift/Ctrl button is pressed and
      // one of the following conditions is true:
      //
      // a) single transaction is selected
      // b) multiple transactions are selected and the one to be selected is not
      if(!(buttonState & (Qt::ShiftButton | Qt::ControlButton))) {
        if((cnt > 0) && (!item->isSelected())) {
          emit aboutToSelectItem(item);
          // pointer 'item' might have changed. reconstruct it.
          item = itemById(id);
          unselectItems();
          item->setSelected(true);
          setFocusItem(item);
        }
        m_selectAnchor = item;
      }
    } else {
      // we get here when called by application logic
      emit aboutToSelectItem(item);
      // pointer 'item' might have changed. reconstruct it.
      item = itemById(id);
      unselectItems();
      item->setSelected(true);
      setFocusItem(item);
      m_selectAnchor = item;
    }
    QValueList<SelectedTransaction> list;
    selectedTransactions(list);
    emit selectionChanged(list);
  }
}

void Register::ensureItemVisible(RegisterItem* item)
{
  if(!item)
    return;

  m_ensureVisibleItem = item;
  QTimer::singleShot(0, this, SLOT(slotEnsureItemVisible()));
}

void Register::slotDoubleClicked(int row, int, int, const QPoint&)
{
  if(row >= 0 && (unsigned)row < m_itemIndex.size()) {
    RegisterItem* p = m_itemIndex[row];
    if(p->isSelectable()) {
      m_ignoreNextButtonRelease = true;
      // double click to start editing only works if the focus
      // item is among the selected ones
      if(m_focusItem->isSelected()) {
        // don't emit the signal right away but wait until
        // we come back to the Qt main loop
        QTimer::singleShot(0, this, SIGNAL(editTransaction()));
      }
    }
  }
}

void Register::slotEnsureItemVisible(void)
{
  // make sure to catch latest changes
  bool enabled = isUpdatesEnabled();
  setUpdatesEnabled(false);
  updateRegister();
  setUpdatesEnabled(enabled);

  RegisterItem* item = m_ensureVisibleItem;
  RegisterItem* prev = item->prevItem();
  RegisterItem* next = item->nextItem();

  int rowPrev, rowNext;
  rowPrev = item->startRow();
  rowNext = item->startRow() + item->numRowsRegister() - 1;

  if(prev)
    rowPrev -= prev->numRowsRegister();
  if(next)
    rowNext += next->numRowsRegister();

  if(rowPrev < 0)
    rowPrev = 0;
  if(rowNext >= numRows())
    rowNext = numRows()-1;

  int wt = contentsY();           // window top
  int wh = visibleHeight();       // window height
  int lt = rowPos(rowPrev);       // top of line above lens
  int lb = rowPos(rowNext)+rowHeight(rowNext);       // bottom of line below lens

  // only update widget, if the transaction is not fully visible
  if(lt < wt || lb >= (wt + wh)) {
    if(rowPrev >= 0) {
      ensureCellVisible(rowPrev, 0);
    }

    ensureCellVisible(item->startRow(), 0);

    if(rowNext < numRows()) {
      ensureCellVisible(rowNext, 0);
    }
  }
}

TransactionSortField KMyMoneyRegister::textToSortOrder(const QString& text)
{
  for(int idx = 1; idx < static_cast<int>(MaxSortFields); ++idx) {
    if(text == sortOrderText[idx]) {
      return static_cast<TransactionSortField>(idx);
    }
  }
  return UnknownSort;
}

const QString& KMyMoneyRegister::sortOrderToText(TransactionSortField idx)
{
  if(idx < PostDateSort || idx >= MaxSortFields)
    idx = UnknownSort;
  return sortOrderText[idx];
}

QString Register::text(int /*row*/, int /*col*/) const
{
  return QString("a");
}

QWidget* Register::cellWidget(int row, int col) const
{
  // separeted here in two if()s, because this method is called for each
  // event from QTable::eventFilter and in the most cases it is -1, -1
  if(row < 0 || col < 0)
    return 0;

  if(row > numRows() - 1 || col > numCols() - 1) {
    if(numRows() && numCols())
      qWarning("Register::cellWidget(%d,%d) out of bounds (%d,%d)", row, col, numRows(), numCols());
    return 0;
  }

  if(!m_cellWidgets.count())
    return 0;

  QWidget* w = 0;
  QPair<int, int> idx = qMakePair(row, col);
  QMap<QPair<int, int>, QWidget*>::const_iterator it_w;

  it_w = m_cellWidgets.find(idx);
  if(it_w != m_cellWidgets.end())
    w = *it_w;
  return w;
}

void Register::insertWidget(int row, int col, QWidget* w)
{
  if(row < 0 || col < 0 || row > numRows() - 1 || col > numCols() - 1) {
    qWarning("Register::insertWidget(%d,%d) out of bounds", row, col);
    return;
  }

  QPair<int, int> idx = qMakePair(row, col);
  m_cellWidgets[idx] = w;
}

void Register::clearCellWidget(int row, int col)
{
  if(row < 0 || col < 0 || row > numRows() - 1 || col > numCols() - 1) {
    qWarning("Register::clearCellWidget(%d,%d) out of bounds", row, col);
    return;
  }

  QPair<int, int> idx = qMakePair(row, col);
  QMap<QPair<int, int>, QWidget*>::iterator it_w;

  it_w = m_cellWidgets.find(idx);
  if(it_w != m_cellWidgets.end()) {
    (*it_w)->deleteLater();
    m_cellWidgets.remove(it_w);
  }
}

QWidget* Register::createEditor(int /*row*/, int /*col*/, bool /*initFromCell*/) const
{
  return 0;
}

void Register::setCellContentFromEditor(int /*row*/, int /*col*/)
{
}

void Register::endEdit(int /*row*/, int /*col*/, bool /*accept*/, bool /*replace*/)
{
}

void Register::arrangeEditWidgets(QMap<QString, QWidget*>& editWidgets, KMyMoneyRegister::Transaction* t)
{
  t->arrangeWidgetsInRegister(editWidgets);
  ensureItemVisible(t);
  // updateContents();
}

void Register::tabOrder(QWidgetList& tabOrderWidgets, KMyMoneyRegister::Transaction* t) const
{
  t->tabOrderInRegister(tabOrderWidgets);
}

void Register::removeEditWidgets(QMap<QString, QWidget*>& editWidgets)
{
  // remove pointers from map
  QMap<QString, QWidget*>::iterator it;
  for(it = editWidgets.begin(); it != editWidgets.end(); ) {
    if((*it)->parentWidget() == this) {
      editWidgets.remove(it);
      it = editWidgets.begin();
    } else
      ++it;
  }

  // now delete the widgets
  KMyMoneyRegister::Transaction* t = dynamic_cast<KMyMoneyRegister::Transaction*>(focusItem());
  for(int row = t->startRow(); row < t->startRow() + t->numRowsRegister(true); ++row) {
    for(int col = 0; col < numCols(); ++col) {
      if(cellWidget(row, col))
        clearCellWidget(row, col);
    }
    // make sure to reduce the possibly size to what it was before editing started
    setRowHeight(row, t->rowHeightHint());
  }
}

void Register::slotToggleErronousTransactions(void)
{
  // toggle switch
  m_markErronousTransactions ^= 1;

  // check if anything needs to be redrawn
  KMyMoneyRegister::RegisterItem* p = m_firstErronous;
  while(p && p->prevItem() != m_lastErronous) {
    if(p->isErronous())
      repaintItems(p);
    p = p->nextItem();
  }

  // restart timer
  QTimer::singleShot(500, this, SLOT(slotToggleErronousTransactions()));
}

RegisterItem* Register::itemById(const QCString& id) const
{
  if(id.isEmpty())
    return m_lastItem;

  for(QValueVector<RegisterItem*>::size_type i = 0; i < m_items.size(); ++i) {
    RegisterItem* item = m_items[i];
    if(!item)
      continue;
    if(item->id() == id)
      return item;
  }
  return 0;
}

RegisterItem* Register::scrollPage(int key)
{
  RegisterItem* item = m_focusItem;
  RegisterItem* newItem = item;
  int height = 0;

  switch(key) {
    case Qt::Key_PageUp:
      while(height < visibleHeight() && item->prevItem()) {
        do {
          item = item->prevItem();
          height += item->rowHeightHint();
        } while(!item->isSelectable() && item->prevItem());
        if(item)
          newItem = item;
      }
      break;
    case Qt::Key_PageDown:
      while(height < visibleHeight() && item->nextItem()) {
        do {
          height += item->rowHeightHint();
          item = item->nextItem();
        } while(!item->isSelectable() && item->nextItem());
        if(item)
          newItem = item;
      }
      break;

    case Qt::Key_Up:
      if(item->prevItem()) {
        do {
          item = item->prevItem();
        } while(!item->isSelectable() && item->prevItem());
        if(item)
          newItem = item;
      }
      break;

    case Qt::Key_Down:
      if(item->nextItem()) {
        do {
          item = item->nextItem();
        } while(!item->isSelectable() && item->nextItem());
        if(item)
          newItem = item;
      }
      break;

    case Qt::Key_Home:
      item = m_firstItem;
      while(!item->isSelectable() && item->nextItem())
        item = item->nextItem();
      if(item)
        newItem = item;
      break;

    case Qt::Key_End:
      item = m_lastItem;
      while(!item->isSelectable() && item->prevItem())
        item = item->prevItem();
      if(item)
        newItem = item;
      break;
  }

  // make sure to avoid selecting a possible empty transaction at the end
  Transaction* t = dynamic_cast<Transaction*>(newItem);
  if(t && t->transaction().id().isEmpty()) {
    if(t->prevItem())
      newItem = t->prevItem();
  }
  return newItem;
}

void Register::keyPressEvent(QKeyEvent* ev)
{
  switch(ev->key()) {
    case Qt::Key_Space:
      // get the state out of the event ...
      m_buttonState = ev->state();
      // ... and pretend that we have pressed the left mouse button ;)
      m_buttonState = static_cast<Qt::ButtonState>(m_buttonState | Qt::LeftButton);
      selectItem(m_focusItem);
      break;

    case Qt::Key_PageUp:
    case Qt::Key_PageDown:
    case Qt::Key_Home:
    case Qt::Key_End:
    case Qt::Key_Down:
    case Qt::Key_Up:
      setFocusItem(scrollPage(ev->key()));
      ensureItemVisible(m_focusItem);
      break;

    default:
      QTable::keyPressEvent(ev);
      break;
  }
}

Transaction* Register::transactionFactory(Register *parent, MyMoneyObjectContainer* objects, const MyMoneyTransaction& transaction, const MyMoneySplit& split)
{
  Transaction* t = 0;
  switch(parent->account().accountType()) {
    case MyMoneyAccount::Checkings:
    case MyMoneyAccount::Savings:
    case MyMoneyAccount::Cash:
    case MyMoneyAccount::CreditCard:
    case MyMoneyAccount::Loan:
    case MyMoneyAccount::Asset:
    case MyMoneyAccount::Liability:
    case MyMoneyAccount::Currency:
    case MyMoneyAccount::Income:
    case MyMoneyAccount::Expense:
    case MyMoneyAccount::AssetLoan:
      t = new KMyMoneyRegister::StdTransaction(parent, objects, transaction, split);
      break;

    case MyMoneyAccount::Investment:
      t = new KMyMoneyRegister::InvestTransaction(parent, objects, transaction, split);
      break;

    case MyMoneyAccount::CertificateDep:
    case MyMoneyAccount::MoneyMarket:
    case MyMoneyAccount::Stock:
    case MyMoneyAccount::Equity:
    default:
      qDebug("Register::transactionFactory: invalid accountTypeE %d", parent->account().accountType());
      break;
  }
  return t;
}
#include "register.moc"

// vim:cin:si:ai:et:ts=2:sw=2:
