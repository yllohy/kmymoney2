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
  id = QCString();

  QCStringList list;
  selectedItems(list);

  if(list.count() > 0) {
    id = list[0];
  }
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


// -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF --
// -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF --
// -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF --
// -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF --
// -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF --
// -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF --
// -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF --
// -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF --
// -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF --

/***************************************************************************
                          kmymoneycombo.cpp  -  description
                             -------------------
    begin                : Sat May 5 2001
    copyright            : (C) 2001 by Michael Edwardes
    email                : mte@users.sourceforge.net
                             Javier Campos Morales <javi_c@ctv.es>
                             Felix Rodriguez <frodriguez@mail.wesleyan.edu>
 ***************************************************************************/

#include "../mymoney/mymoneyfile.h"

kMyMoneyCombo::kMyMoneyCombo(QWidget *w, const char *name)
  : KComboBox(w, name)
{
  init();
}

kMyMoneyCombo::kMyMoneyCombo(bool rw, QWidget *w, const char *name)
  : KComboBox(rw, w, name)
{
  init();
}

void kMyMoneyCombo::init(void)
{
  m_type = NONE;
  connect(this, SIGNAL(activated(int)), SLOT(slotCheckValidSelection(int)));
}

kMyMoneyCombo::~kMyMoneyCombo()
{
}

void kMyMoneyCombo::loadCurrentItem(const int item)
{
  m_prevItem = m_item = item;
  resetCurrentItem();
}

void kMyMoneyCombo::resetCurrentItem(void)
{
  m_prevItem = m_item;
  setCurrentItem(m_item);
}

void kMyMoneyCombo::setCurrentItem(const QString& str)
{
  int i=0;

  for(; i < count(); ++i) {
    if(str == text(i)) {
      KComboBox::setCurrentItem(i);
      m_prevItem = i;
      break;
    }
  }

  // if not found, select the first one
  if(i == count()) {
    qDebug("kMyMoneyCombo::setCurrentItem: '%s' not found", str.latin1());
    KComboBox::setCurrentItem(0);
    m_prevItem = 0;
  }
}

void kMyMoneyCombo::slotCheckValidSelection(int id)
{
  QString txt = text(id);
  if(txt.left(4) == "--- "
  && txt.right(4) == " ---") {
    if(id < count()-1) {
      KComboBox::setCurrentItem(id+1);
    } else {
      KComboBox::setCurrentItem(id-1);
    }
  }
}

void kMyMoneyCombo::focusOutEvent(QFocusEvent *ev)
{
  // if the current text is not in the list of
  // possible completions, we have a new payee
  // and signal that to the outside world.
  if(currentItem() != m_prevItem) {
    emit selectionChanged(currentItem());
    m_prevItem = currentItem();
  }
  KComboBox::focusOutEvent(ev);
}

void kMyMoneyCombo::loadAccounts(bool asset, bool liability)
{
  try
  {
    MyMoneyFile* file = MyMoneyFile::instance();

    MyMoneyAccount acc;
    QCStringList::ConstIterator it_s;

    if (asset)
    {
      acc = file->asset();
      for(it_s = acc.accountList().begin(); it_s != acc.accountList().end(); ++it_s)
      {
        MyMoneyAccount a = file->account(*it_s);
        KComboBox::insertItem(a.name());
      }
    }

    if (liability)
    {
      acc = file->liability();
      for(it_s = acc.accountList().begin(); it_s != acc.accountList().end(); ++it_s)
      {
        MyMoneyAccount a = file->account(*it_s);
        KComboBox::insertItem(a.name());
      }
    }

    m_type = ACCOUNT;
  }
  catch (MyMoneyException *e)
  {
    delete e;
  }
}

QCString kMyMoneyCombo::currentAccountId(void)
{
  try
  {
    if (m_type == ACCOUNT)
      return MyMoneyFile::instance()->nameToAccount(currentText());
  }
  catch (MyMoneyException *e)
  {
    delete e;
  }

  return "";
}

#include "kmymoneycombo.moc"
