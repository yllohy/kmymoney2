/***************************************************************************
                          kmymoneycombo.cpp  -  description
                             -------------------
    begin                : Mon Mar 12 2007
    copyright            : (C) 2007 by Thomas Baumgart
    email                : ipwizard@users.sourceforge.net
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

#include <qrect.h>
#include <qstyle.h>
#include <qpainter.h>
#include <qapplication.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <klocale.h>
#include <klistview.h>
#include <kdebug.h>
#include <kconfig.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneycombo.h"
#include <kmymoney/kmymoneycompletion.h>
#include <kmymoney/kmymoneylineedit.h>
#include <kmymoney/mymoneysplit.h>
#include <kmymoney/registeritem.h>
#include <kmymoney/mymoneyscheduled.h>

#include "kmymoneyselector.h"

KMyMoneyCombo::KMyMoneyCombo(QWidget *w, const char *name) :
  KComboBox(w, name),
  m_completion(0),
  m_edit(0),
  m_canCreateObjects(false)
{
}

KMyMoneyCombo::KMyMoneyCombo(bool rw, QWidget *w, const char *name) :
  KComboBox(rw, w, name),
  m_completion(0),
  m_edit(0),
  m_canCreateObjects(false)
{
  if(rw) {
    m_edit = new kMyMoneyLineEdit(this, "combo edit");
    setLineEdit(m_edit);
  }
}

void KMyMoneyCombo::setCurrentText(const QCString& id)
{
    setCurrentText();
    if(!id.isEmpty()) {
      QListViewItem* item = selector()->item(id);
      if(item)
        setCurrentText(item->text(0));
    }
}

void KMyMoneyCombo::slotItemSelected(const QCString& id)
{
  if(editable()) {
    bool blocked = signalsBlocked();
    blockSignals(true);
    setCurrentText(id);
    blockSignals(blocked);
  }

  m_completion->hide();

  if(m_id != id) {
    m_id = id;
    emit itemSelected(id);
  }
}

void KMyMoneyCombo::setEditable(bool y)
{
  if(y == editable())
    return;

  KComboBox::setEditable(y);

  // make sure we use our own line edit style
  if(y) {
    m_edit = new kMyMoneyLineEdit(this, "combo edit");
    setLineEdit(m_edit);
    m_edit->setPaletteBackgroundColor(paletteBackgroundColor());

  } else {
    m_edit = 0;
  }
}

void KMyMoneyCombo::setHint(const QString& hint) const
{
  if(m_edit)
    m_edit->setHint(hint);
}

void KMyMoneyCombo::paintEvent(QPaintEvent* ev)
{
  KComboBox::paintEvent(ev);

  // if we don't have an edit field, we need to paint the text onto the button
  if(!m_edit) {
    if(m_completion) {
      QCStringList list;
      selector()->selectedItems(list);
      if(!list.isEmpty()) {
        QString str = selector()->item(list[0])->text(0);
        // we only paint, if the text is longer than 1 char. Assumption
        // is that length 1 is the blank case so no need to do painting
        if(str.length() > 1) {
          QPainter p( this );
          const QColorGroup & g = colorGroup();
          p.setPen(g.text());

          QRect re = style().querySubControlMetrics( QStyle::CC_ComboBox, this,
                                                    QStyle::SC_ComboBoxEditField );
          re = QStyle::visualRect(re, this);
          p.setClipRect( re );
          p.save();
          p.setFont(font());
          QFontMetrics fm(font());
          int x = re.x(), y = re.y() + fm.ascent();
          p.drawText( x, y, str );
          p.restore();
        }
      }
    }
  }
}

void KMyMoneyCombo::setPaletteBackgroundColor(const QColor& color)
{
  KComboBox::setPaletteBackgroundColor(color);
  if(m_edit) {
    m_edit->setPaletteBackgroundColor(color);
  }
}

void KMyMoneyCombo::mousePressEvent(QMouseEvent *e)
{
  // mostly copied from QCombo::mousePressEvent() and adjusted for our needs
  if(e->button() != LeftButton)
    return;

  if(((!editable() || isInArrowArea(mapToGlobal(e->pos()))) && selector()->itemList().count()) && !m_completion->isVisible()) {
    m_completion->show();
  }

  if(m_timer.isActive()) {
    m_timer.stop();
    m_completion->slotMakeCompletion("");
  } else {
    KConfig config( "kcminputrc", true );
    config.setGroup("KDE");
    m_timer.start(config.readNumEntry("DoubleClickInterval", 400), true);
  }
}

bool KMyMoneyCombo::isInArrowArea(const QPoint& pos) const
{
  QRect arrowRect = style().querySubControlMetrics( QStyle::CC_ComboBox, this,
                                                    QStyle::SC_ComboBoxArrow);
  arrowRect = QStyle::visualRect(arrowRect, this);

  // Correction for motif style, where arrow is smaller
  // and thus has a rect that doesn't fit the button.
  arrowRect.setHeight( QMAX(  height() - (2 * arrowRect.y()), arrowRect.height() ) );

  // if the button is not editable, it covers the whole widget
  if(!editable())
    arrowRect = rect();

  return arrowRect.contains(mapFromGlobal(pos));
}

void KMyMoneyCombo::keyPressEvent(QKeyEvent* e)
{
  if((e->key() == Key_F4 && e->state() == 0 ) ||
     (e->key() == Key_Down && (e->state() & AltButton)) ||
     (!editable() && e->key() == Key_Space)) {
    // if we have at least one item in the list, we open the dropdown
    if(selector()->listView()->firstChild())
      m_completion->show();
    e->ignore();
    return;
  }
  KComboBox::keyPressEvent(e);
}

void KMyMoneyCombo::connectNotify(const char* signal)
{
  if(signal && !strcmp(signal, SIGNAL(createItem(const QString&,QCString&)))) {
    m_canCreateObjects = true;
  }
}

void KMyMoneyCombo::disconnectNotify(const char* signal)
{
  if(signal && !strcmp(signal, SIGNAL(createItem(const QString&,QCString&)))) {
    m_canCreateObjects = false;
  }
}

void KMyMoneyCombo::focusOutEvent(QFocusEvent* e)
{
  if(editable() && !currentText().isEmpty()) {
    if(m_canCreateObjects) {
      if(!m_completion->selector()->contains(currentText())) {
        QCString id;
        // annouce that we go into a possible dialog to create an object
        // This can be used by upstream widgets to disable filters etc.
        emit objectCreation(true);

        emit createItem(currentText(), id);

        // Announce that we return from object creation
        emit objectCreation(false);

        // update the field to a possibly created object
        m_id = id;
        setCurrentText(id);

        // make sure the completion does not show through
        m_completion->hide();
      }

    // else if we cannot create objects, and the current text is not
    // in the list, then we clear the text and the selection.
    } else if(!m_completion->selector()->contains(currentText())) {
      setCurrentText(QString());
    }
  }

  KComboBox::focusOutEvent(e);

  // force update of hint and id if there is no text in the widget
  if(editable() && currentText().isEmpty()) {
    QCString id = m_id;
    m_id = QCString();
    if(!id.isEmpty())
      emit itemSelected(m_id);
    repaint();
  }
}

KMyMoneySelector* KMyMoneyCombo::selector(void) const
{
  return m_completion->selector();
}

kMyMoneyCompletion* KMyMoneyCombo::completion(void) const
{
  return m_completion;
}

void KMyMoneyCombo::selectedItem(QCString& id) const
{
  id = m_id;
}

void KMyMoneyCombo::selectedItems(QCStringList& list) const
{
  if(lineEdit() && lineEdit()->text().length() == 0) {
    list.clear();
  } else {
    m_completion->selector()->selectedItems(list);
  }
}

void KMyMoneyCombo::setSelectedItem(const QCString& id)
{
  m_completion->selector()->setSelected(id, true);
  blockSignals(true);
  slotItemSelected(id);
  blockSignals(false);
  update();
}

QSize KMyMoneyCombo::sizeHint() const
{
  return KComboBox::sizeHint();

  // I wanted to use the code below to adjust the size of the combo box
  // according to the largest item in the selector list. Apparently that
  // does not work too well in the enter and edit schedule dialog for
  // the category combo box. So we just use the standard implementation for now.
#if 0
  constPolish();
  int i, w;
  QFontMetrics fm = fontMetrics();

  int maxW = count() ? 18 : 7 * fm.width(QChar('x')) + 18;
  int maxH = QMAX( fm.lineSpacing(), 14 ) + 2;

  w = selector()->optimizedWidth();
  if ( w > maxW )
    maxW = w;

  QSize sizeHint = (style().sizeFromContents(QStyle::CT_ComboBox, this,
                 QSize(maxW, maxH)).
      expandedTo(QApplication::globalStrut()));

  return sizeHint;
#endif
}



KMyMoneyReconcileCombo::KMyMoneyReconcileCombo(QWidget* w, const char* name) :
  KMyMoneyCombo(false, w, name)
{
  m_completion = new kMyMoneyCompletion(this, 0);
  // connect(m_completion, SIGNAL(itemSelected(const QCString&)), this, SIGNAL(itemSelected(const QCString&)));

  // add the items in reverse order of appearance (see KMyMoneySelector::newItem() for details)
  // selector()->newTopItem(i18n("Frozen"), QString(), "F");
  selector()->newTopItem(i18n("Reconciled"), QString(), "R");
  selector()->newTopItem(i18n("Cleared"), QString(), "C");
  selector()->newTopItem(i18n("Not reconciled"), QString(), " ");
  selector()->newTopItem(" ", QString(), "U");

  connect(m_completion, SIGNAL(itemSelected(const QCString&)), this, SLOT(slotItemSelected(const QCString&)));
  connect(this, SIGNAL(itemSelected(const QCString&)), this, SLOT(slotSetState(const QCString&)));
}

void KMyMoneyReconcileCombo::slotSetState(const QCString& state)
{
  setSelectedItem(state);
}

void KMyMoneyReconcileCombo::removeDontCare(void)
{
  selector()->removeItem("U");
}

void KMyMoneyReconcileCombo::setState(MyMoneySplit::reconcileFlagE state)
{
  QCString id;
  switch(state) {
    case MyMoneySplit::NotReconciled:
      id = " ";
      break;
    case MyMoneySplit::Cleared:
      id = "C";
      break;
    case MyMoneySplit::Reconciled:
      id = "R";
      break;
    case MyMoneySplit::Frozen:
      id = "F";
      break;
    case MyMoneySplit::Unknown:
      id = "U";
      break;
    default:
      kdDebug(2) << "Unknown reconcile state '" << state << "' in KMyMoneyComboReconcile::setState()\n";
      break;
  }
  setSelectedItem(id);
}

MyMoneySplit::reconcileFlagE KMyMoneyReconcileCombo::state(void) const
{
  MyMoneySplit::reconcileFlagE state = MyMoneySplit::NotReconciled;

  QCStringList list;
  selector()->selectedItems(list);
  if(!list.isEmpty()) {
    if(list[0] == "C")
      state = MyMoneySplit::Cleared;
    if(list[0] == "R")
      state = MyMoneySplit::Reconciled;
    if(list[0] == "F")
      state = MyMoneySplit::Frozen;
    if(list[0] == "U")
      state = MyMoneySplit::Unknown;
  }
  return state;
}


KMyMoneyComboAction::KMyMoneyComboAction(QWidget* w, const char* name) :
  KMyMoneyCombo(false, w, name)
{
  m_completion = new kMyMoneyCompletion(this, 0);
  QCString num;
  // add the items in reverse order of appearance (see KMyMoneySelector::newItem() for details)
  selector()->newTopItem(i18n("ATM"), QString(), num.setNum(KMyMoneyRegister::ActionAtm));
  selector()->newTopItem(i18n("Withdrawal"), QString(), num.setNum(KMyMoneyRegister::ActionWithdrawal));
  selector()->newTopItem(i18n("Transfer"), QString(), num.setNum(KMyMoneyRegister::ActionTransfer));
  selector()->newTopItem(i18n("Deposit"), QString(), num.setNum(KMyMoneyRegister::ActionDeposit));
  selector()->newTopItem(i18n("Cheque"), QString(), num.setNum(KMyMoneyRegister::ActionCheck));
  connect(m_completion, SIGNAL(itemSelected(const QCString&)), this, SLOT(slotItemSelected(const QCString&)));
  connect(this, SIGNAL(itemSelected(const QCString&)), this, SLOT(slotSetAction(const QCString&)));
}

void KMyMoneyComboAction::protectItem(int id, bool protect)
{
  QCString num;
  selector()->protectItem(num.setNum(id), protect);
}

void KMyMoneyComboAction::slotSetAction(const QCString& act)
{
  setSelectedItem(act);
  update();
  emit actionSelected(action());
}

void KMyMoneyComboAction::setAction(int action)
{
  if(action < 0 || action > 5) {
    kdDebug(2) << "KMyMoneyComboAction::slotSetAction(" << action << ") invalid. Replaced with 2\n";
    action = 2;
  }
  QCString act;
  act.setNum(action);
  setSelectedItem(act);
}

int KMyMoneyComboAction::action(void) const
{
  QCStringList list;
  selector()->selectedItems(list);
  if(!list.isEmpty()) {
    return list[0].toInt();
  }
  kdDebug(2) << "KMyMoneyComboAction::action(void): unknown selection\n";
  return 0;
}

KMyMoneyCashFlowCombo::KMyMoneyCashFlowCombo(QWidget* w, const char* name, MyMoneyAccount::accountTypeE accountType) :
  KMyMoneyCombo(false, w, name)
{
  m_completion = new kMyMoneyCompletion(this, 0);
  QCString num;
  // add the items in reverse order of appearance (see KMyMoneySelector::newItem() for details)
  if(accountType == MyMoneyAccount::Income || accountType == MyMoneyAccount::Expense) {
    // this is used for income/expense accounts to just show the reverse sense
    selector()->newTopItem(i18n("Activity for expense categories", "Paid"), QString(), num.setNum(KMyMoneyRegister::Deposit));
    selector()->newTopItem(i18n("Activity for income categories", "Received"), QString(), num.setNum(KMyMoneyRegister::Payment));
  } else {
    selector()->newTopItem(i18n("From"), QString(), num.setNum(KMyMoneyRegister::Deposit));
    selector()->newTopItem(i18n("Pay to"), QString(), num.setNum(KMyMoneyRegister::Payment));
  }
  selector()->newTopItem(" ", QString(), num.setNum(KMyMoneyRegister::Unknown));
  connect(m_completion, SIGNAL(itemSelected(const QCString&)), this, SLOT(slotItemSelected(const QCString&)));
  connect(this, SIGNAL(itemSelected(const QCString&)), this, SLOT(slotSetDirection(const QCString&)));
}

void KMyMoneyCashFlowCombo::setDirection(KMyMoneyRegister::CashFlowDirection dir)
{
  m_dir = dir;
  QCString num;
  setSelectedItem(num.setNum(dir));
}

void KMyMoneyCashFlowCombo::slotSetDirection(const QCString& id)
{
  QCString num;
  for(int i = KMyMoneyRegister::Deposit; i <= KMyMoneyRegister::Unknown; ++i) {
    num.setNum(i);
    if(num == id) {
      m_dir = static_cast<KMyMoneyRegister::CashFlowDirection>(i);
      break;
    }
  }
  emit directionSelected(m_dir);
  update();
}

void KMyMoneyCashFlowCombo::removeDontCare(void)
{
  QCString num;
  selector()->removeItem(num.setNum(KMyMoneyRegister::Unknown));
}


KMyMoneyActivityCombo::KMyMoneyActivityCombo(QWidget* w, const char* name) :
  KMyMoneyCombo(false, w, name)
{
  m_completion = new kMyMoneyCompletion(this, 0);
  QCString num;
  // add the items in reverse order of appearance (see KMyMoneySelector::newItem() for details)
  selector()->newTopItem(i18n("Split shares"), QString(), num.setNum(MyMoneySplit::SplitShares));
  selector()->newTopItem(i18n("Remove shares"), QString(), num.setNum(MyMoneySplit::RemoveShares));
  selector()->newTopItem(i18n("Add shares"), QString(), num.setNum(MyMoneySplit::AddShares));
  selector()->newTopItem(i18n("Yield"), QString(), num.setNum(MyMoneySplit::Yield));
  selector()->newTopItem(i18n("Reinvest dividend"), QString(), num.setNum(MyMoneySplit::ReinvestDividend));
  selector()->newTopItem(i18n("Dividend"), QString(), num.setNum(MyMoneySplit::Dividend));
  selector()->newTopItem(i18n("Sell shares"), QString(), num.setNum(MyMoneySplit::SellShares));
  selector()->newTopItem(i18n("Buy shares"), QString(), num.setNum(MyMoneySplit::BuyShares));

  connect(m_completion, SIGNAL(itemSelected(const QCString&)), this, SLOT(slotItemSelected(const QCString&)));
  connect(this, SIGNAL(itemSelected(const QCString&)), this, SLOT(slotSetActivity(const QCString&)));
}

void KMyMoneyActivityCombo::setActivity(MyMoneySplit::investTransactionTypeE activity)
{
  m_activity = activity;
  QCString num;
  setSelectedItem(num.setNum(activity));
}

void KMyMoneyActivityCombo::slotSetActivity(const QCString& id)
{
  QCString num;
  for(int i = MyMoneySplit::BuyShares; i <= MyMoneySplit::SplitShares; ++i) {
    num.setNum(i);
    if(num == id) {
      m_activity = static_cast<MyMoneySplit::investTransactionTypeE>(i);
      break;
    }
  }
  emit activitySelected(m_activity);
  update();
}

KMyMoneyPayeeCombo::KMyMoneyPayeeCombo(QWidget* parent, const char * name) :
  KMyMoneyCombo(true, parent, name)
{
  m_completion = new kMyMoneyCompletion(this);

  // set to ascending sort
  selector()->listView()->setSorting(0);

  connect(m_completion, SIGNAL(itemSelected(const QCString&)), this, SLOT(slotItemSelected(const QCString&)));
  connect(this, SIGNAL(textChanged(const QString&)), m_completion, SLOT(slotMakeCompletion(const QString&)));
}

void KMyMoneyPayeeCombo::loadPayees(const QValueList<MyMoneyPayee>& list)
{
  selector()->listView()->clear();
  QValueList<MyMoneyPayee>::const_iterator it;
  for(it = list.begin(); it != list.end(); ++it) {
    selector()->newTopItem((*it).name(), QString(), (*it).id());
  }
}


class KMyMoneyGeneralComboPrivate {
public:
  QMap<QString, int> m_strings;
  void insertItem(const QString& s, int id) { m_strings[s] = id; }

  int itemId(const QString& s) const {
    QMap<QString, int>::const_iterator it;
    it = m_strings.find(s);
    if(it != m_strings.end())
      return *it;
    return -1;
  }

  const QString& itemText(int id) {
    QMap<QString, int>::const_iterator it;
    for(it = m_strings.begin(); it != m_strings.end(); ++it) {
      if(*it == id) {
        return it.key();
      }
    }
    return QString::null;
  }
};

KMyMoneyGeneralCombo::KMyMoneyGeneralCombo(QWidget* w, const char* name) :
  KComboBox(w, name),
  d(new KMyMoneyGeneralComboPrivate)
{
  connect(this, SIGNAL(highlighted(int)), this, SLOT(slotChangeItem(int)));
}

KMyMoneyGeneralCombo::~KMyMoneyGeneralCombo()
{
  delete d;
}

void KMyMoneyGeneralCombo::setItem(int id)
{
  setCurrentItem(id);
}

int KMyMoneyGeneralCombo::item(void) const
{
  return currentItem();
}

void KMyMoneyGeneralCombo::setCurrentItem(int id)
{
  const QString& txt = d->itemText(id);
  for(int idx = 0; idx < count(); ++idx) {
    if(txt == text(idx)) {
      KComboBox::setCurrentItem(idx);
      break;
    }
  }
}

int KMyMoneyGeneralCombo::currentItem(void) const
{
  return d->itemId(currentText());
}

void KMyMoneyGeneralCombo::clear(void)
{
  d->m_strings.clear();
  KComboBox::clear();
}

void KMyMoneyGeneralCombo::insertItem(const QString& txt, int id, int idx)
{
  d->insertItem(txt, id);
  KComboBox::insertItem(txt, idx);
}

void KMyMoneyGeneralCombo::removeItem(int id)
{
  const QString& txt = d->itemText(id);
  for(int idx = 0; idx < count(); ++idx) {
    if(txt == text(idx)) {
      KComboBox::removeItem(idx);
      break;
    }
  }
}

void KMyMoneyGeneralCombo::slotChangeItem(int idx)
{
  emit itemSelected(d->itemId(text(idx)));
}

KMyMoneyPeriodCombo::KMyMoneyPeriodCombo(QWidget* parent, const char* name) :
  KMyMoneyGeneralCombo(parent, name)
{
  insertItem(i18n("All dates"), MyMoneyTransactionFilter::allDates);
  insertItem(i18n("Until today"), MyMoneyTransactionFilter::untilToday);
  insertItem(i18n("Today"), MyMoneyTransactionFilter::today);
  insertItem(i18n("Current month"), MyMoneyTransactionFilter::currentMonth);
  insertItem(i18n("Current quarter"), MyMoneyTransactionFilter::currentQuarter);
  insertItem(i18n("Current year"), MyMoneyTransactionFilter::currentYear);
  insertItem(i18n("Current fiscal year"), MyMoneyTransactionFilter::currentFiscalYear);
  insertItem(i18n("Month to date"), MyMoneyTransactionFilter::monthToDate);
  insertItem(i18n("Year to date"), MyMoneyTransactionFilter::yearToDate);
  insertItem(i18n("Year to month"), MyMoneyTransactionFilter::yearToMonth);
  insertItem(i18n("Last month"), MyMoneyTransactionFilter::lastMonth);
  insertItem(i18n("Last year"), MyMoneyTransactionFilter::lastYear);
  insertItem(i18n("Last fiscal year"), MyMoneyTransactionFilter::lastFiscalYear);
  insertItem(i18n("Last 7 days"), MyMoneyTransactionFilter::last7Days);
  insertItem(i18n("Last 30 days"), MyMoneyTransactionFilter::last30Days);
  insertItem(i18n("Last 3 months"), MyMoneyTransactionFilter::last3Months);
  insertItem(i18n("Last quarter"), MyMoneyTransactionFilter::lastQuarter);
  insertItem(i18n("Last 6 months"), MyMoneyTransactionFilter::last6Months);
  insertItem(i18n("Last 11 months"), MyMoneyTransactionFilter::last11Months);
  insertItem(i18n("Last 12 months"), MyMoneyTransactionFilter::last12Months);
  insertItem(i18n("Next 7 days"), MyMoneyTransactionFilter::next7Days);
  insertItem(i18n("Next 30 days"), MyMoneyTransactionFilter::next30Days);
  insertItem(i18n("Next 3 months"), MyMoneyTransactionFilter::next3Months);
  insertItem(i18n("Next quarter"), MyMoneyTransactionFilter::lastQuarter);
  insertItem(i18n("Next 6 months"), MyMoneyTransactionFilter::next6Months);
  insertItem(i18n("Next 12 months"), MyMoneyTransactionFilter::next12Months);
  insertItem(i18n("Last 3 months to next 3 months"), MyMoneyTransactionFilter::last3ToNext3Months);
  insertItem(i18n("User defined"), MyMoneyTransactionFilter::userDefined);
}

void KMyMoneyPeriodCombo::setCurrentItem(MyMoneyTransactionFilter::dateOptionE id)
{
  if(id >= MyMoneyTransactionFilter::dateOptionCount)
    id = MyMoneyTransactionFilter::userDefined;

  KMyMoneyGeneralCombo::setCurrentItem(id);
}

MyMoneyTransactionFilter::dateOptionE KMyMoneyPeriodCombo::currentItem(void) const
{
  return static_cast<MyMoneyTransactionFilter::dateOptionE>(KMyMoneyGeneralCombo::currentItem());
}

QDate KMyMoneyPeriodCombo::start(MyMoneyTransactionFilter::dateOptionE id)
{
  QDate start, end;
  MyMoneyTransactionFilter::translateDateRange(id, start, end);
  return start;
}

QDate KMyMoneyPeriodCombo::end(MyMoneyTransactionFilter::dateOptionE id)
{
  QDate start, end;
  MyMoneyTransactionFilter::translateDateRange(id, start, end);
  return end;
}

#if 0
void KMyMoneyPeriodCombo::dates(QDate& start, QDate& end, MyMoneyTransactionFilter::dateOptionE id)
{
}
#endif

KMyMoneyFrequencyCombo::KMyMoneyFrequencyCombo(QWidget* parent, const char* name) :
  KMyMoneyGeneralCombo(parent, name)
{
  insertItem(i18n("Once"), MyMoneySchedule::OCCUR_ONCE);
  insertItem(i18n("Daily"), MyMoneySchedule::OCCUR_DAILY);
  insertItem(i18n("Weekly"), MyMoneySchedule::OCCUR_WEEKLY);
  insertItem(i18n("Fortnightly"), MyMoneySchedule::OCCUR_FORTNIGHTLY);
  insertItem(i18n("Every other week"), MyMoneySchedule::OCCUR_EVERYOTHERWEEK);
  insertItem(i18n("Every three weeks"), MyMoneySchedule::OCCUR_EVERYTHREEWEEKS);
  insertItem(i18n("Every four weeks"), MyMoneySchedule::OCCUR_EVERYFOURWEEKS);
  insertItem(i18n("Monthly"), MyMoneySchedule::OCCUR_MONTHLY);
  insertItem(i18n("Every eight weeks"), MyMoneySchedule::OCCUR_EVERYEIGHTWEEKS);
  insertItem(i18n("Every two months"), MyMoneySchedule::OCCUR_EVERYOTHERMONTH);
  insertItem(i18n("Every three months"), MyMoneySchedule::OCCUR_EVERYTHREEMONTHS);
  insertItem(i18n("Quarterly"), MyMoneySchedule::OCCUR_QUARTERLY);
  insertItem(i18n("Every four months"), MyMoneySchedule::OCCUR_EVERYFOURMONTHS);
  insertItem(i18n("Twice yearly"), MyMoneySchedule::OCCUR_TWICEYEARLY);
  insertItem(i18n("Yearly"), MyMoneySchedule::OCCUR_YEARLY);
  insertItem(i18n("Every other year"), MyMoneySchedule::OCCUR_EVERYOTHERYEAR);
}

MyMoneySchedule::occurenceE KMyMoneyFrequencyCombo::currentItem(void) const
{
  return static_cast<MyMoneySchedule::occurenceE>(KMyMoneyGeneralCombo::currentItem());
}

int KMyMoneyFrequencyCombo::daysBetweenEvents(void) const
{
  int rc = 0;

  switch(currentItem()) {
    case MyMoneySchedule::OCCUR_DAILY:
      rc = 1;
      break;
    case MyMoneySchedule::OCCUR_WEEKLY:
      rc = 7;
      break;
    case MyMoneySchedule::OCCUR_FORTNIGHTLY:
      rc = 14;
      break;
    case MyMoneySchedule::OCCUR_EVERYOTHERWEEK:
      rc = 15;
      break;
    case MyMoneySchedule::OCCUR_EVERYTHREEWEEKS:
      rc = 21;
      break;
    case MyMoneySchedule::OCCUR_EVERYFOURWEEKS:
      rc = 28;
      break;
    case MyMoneySchedule::OCCUR_MONTHLY:
      rc = 30;
      break;
    case MyMoneySchedule::OCCUR_EVERYEIGHTWEEKS:
      rc = 56;
      break;
    case MyMoneySchedule::OCCUR_EVERYOTHERMONTH:
      rc = 60;
      break;
    case MyMoneySchedule::OCCUR_QUARTERLY:
      rc = 90;
      break;
    case MyMoneySchedule::OCCUR_EVERYFOURMONTHS:
      rc = 120;
      break;
    case MyMoneySchedule::OCCUR_TWICEYEARLY:
      rc = 180;
      break;
    case MyMoneySchedule::OCCUR_YEARLY:
      rc = 360;
      break;
    default:
      qWarning("Occurence not supported by financial calculator");
  }

  return rc;
}

int KMyMoneyFrequencyCombo::eventsPerYear(void) const
{
  int rc = 0;

  switch(currentItem()) {
    case MyMoneySchedule::OCCUR_DAILY:
      rc = 365;
      break;
    case MyMoneySchedule::OCCUR_WEEKLY:
      rc = 52;
      break;
    case MyMoneySchedule::OCCUR_FORTNIGHTLY:
      rc = 24;
      break;
    case MyMoneySchedule::OCCUR_EVERYOTHERWEEK:
      rc = 26;
      break;
    case MyMoneySchedule::OCCUR_EVERYTHREEWEEKS:
      rc = 17;
      break;
    case MyMoneySchedule::OCCUR_EVERYFOURWEEKS:
      rc = 13;
      break;
    case MyMoneySchedule::OCCUR_MONTHLY:
      rc = 12;
      break;
    case MyMoneySchedule::OCCUR_EVERYEIGHTWEEKS:
      rc = 6;
      break;
    case MyMoneySchedule::OCCUR_EVERYOTHERMONTH:
      rc = 6;
      break;
    case MyMoneySchedule::OCCUR_QUARTERLY:
      rc = 4;
      break;
    case MyMoneySchedule::OCCUR_EVERYFOURMONTHS:
      rc = 3;
      break;
    case MyMoneySchedule::OCCUR_TWICEYEARLY:
      rc = 2;
      break;
    case MyMoneySchedule::OCCUR_YEARLY:
      rc = 1;
      break;
    default:
      qWarning("Occurence not supported by financial calculator");
  }

  return rc;

}
#include "kmymoneycombo.moc"
