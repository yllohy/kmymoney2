/***************************************************************************
                          keditscheduledlg.cpp  -  description
                             -------------------
    begin                : Mon Sep  3 2007
    copyright            : (C) 2007 by Thomas Baumgart
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

#include <qtimer.h>
#include <qwidgetlist.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qvaluevector.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <klocale.h>
#include <kmessagebox.h>
#include <kpushbutton.h>
#include <kstdguiitem.h>
#include <klineedit.h>

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoney/register.h>
#include <kmymoney/transactionform.h>
#include <kmymoney/transaction.h>
#include <kmymoney/transactioneditor.h>
#include <kmymoney/kmymoneylineedit.h>
#include <kmymoney/kmymoneydateinput.h>
#include <kmymoney/kmymoneycombo.h>
#include <kmymoney/kguiutils.h>
#include <kmymoney/kmymoneyutils.h>

#include "keditscheduledlg.h"
#include "../kmymoney2.h"

class KEditScheduleDlgPrivate {
public:
  MyMoneySchedule                m_schedule;
  KMyMoneyRegister::Transaction* m_item;
  QWidgetList                    m_tabOrderWidgets;
  TransactionEditor*             m_editor;
  kMandatoryFieldGroup*          m_requiredFields;
};

KEditScheduleDlg::KEditScheduleDlg(const MyMoneySchedule& schedule, QWidget *parent, const char *name) :
  KEditScheduleDlgDecl(parent, name, true),
  d(new KEditScheduleDlgPrivate)
{
  d->m_schedule = schedule;
  d->m_editor = 0;

  buttonOk->setGuiItem(KStdGuiItem::ok());
  buttonCancel->setGuiItem(KStdGuiItem::cancel());
  buttonHelp->setGuiItem(KStdGuiItem::help());

  d->m_requiredFields = new kMandatoryFieldGroup (this);
  d->m_requiredFields->setOkButton(buttonOk); // button to be enabled when all fields present

  // make sure, we have a tabbar with the form
  // insert it after the horizontal line
  m_paymentInformationLayout->insertWidget(2, m_form->tabBar(m_form->parentWidget()));

  // we never need to see the register
  m_register->hide();

  // ... setup the form ...
  m_form->setupForm(d->m_schedule.account());

  // ... and the register ...
  m_register->clear();

  // ... now add the transaction to register and form ...
  MyMoneyTransaction t = transaction();
  d->m_item = KMyMoneyRegister::Register::transactionFactory(m_register, t, d->m_schedule.transaction().splits()[0], 0);
  m_register->selectItem(d->m_item);
  // show the account row
  d->m_item->setShowRowInForm(0, true);

  m_form->slotSetTransaction(d->m_item);

  // setup widget contents
  m_nameEdit->setText(d->m_schedule.name());

  m_frequencyEdit->setCurrentItem(d->m_schedule.occurence());
  if(m_frequencyEdit->currentItem() == -1)
    m_frequencyEdit->setItem(MyMoneySchedule::OCCUR_MONTHLY);
  slotFrequencyChanged(m_frequencyEdit->currentItem());

  // load option widgets
  m_paymentMethodEdit->insertItem(i18n("Direct debit"), MyMoneySchedule::STYPE_DIRECTDEBIT);
  m_paymentMethodEdit->insertItem(i18n("Direct deposit"), MyMoneySchedule::STYPE_DIRECTDEPOSIT);
  m_paymentMethodEdit->insertItem(i18n("Manual deposit"), MyMoneySchedule::STYPE_MANUALDEPOSIT);
  m_paymentMethodEdit->insertItem(i18n("Write check"), MyMoneySchedule::STYPE_WRITECHEQUE);
  m_paymentMethodEdit->insertItem(i18n("Other"), MyMoneySchedule::STYPE_OTHER);

  MyMoneySchedule::paymentTypeE method = d->m_schedule.paymentType();
  if(method == MyMoneySchedule::STYPE_ANY)
    method = MyMoneySchedule::STYPE_OTHER;
  m_paymentMethodEdit->setCurrentItem(method);

  switch(d->m_schedule.weekendOption()) {
    case MyMoneySchedule::MoveNothing:
      m_weekendOptionEdit->setCurrentItem(0);
      break;
    case MyMoneySchedule::MoveFriday:
      m_weekendOptionEdit->setCurrentItem(1);
      break;
    case MyMoneySchedule::MoveMonday:
      m_weekendOptionEdit->setCurrentItem(2);
      break;
  }
  m_estimateEdit->setChecked(!d->m_schedule.isFixed());
  m_autoEnterEdit->setChecked(d->m_schedule.autoEnter());
  m_endSeriesEdit->setChecked(d->m_schedule.willEnd());

  m_endOptionsFrame->setEnabled(d->m_schedule.willEnd());
  if(d->m_schedule.willEnd()) {
    m_RemainingEdit->setText(QString::number(d->m_schedule.transactionsRemaining()));
    m_FinalPaymentEdit->setDate(d->m_schedule.endDate());
  }

  connect(m_RemainingEdit, SIGNAL(textChanged(const QString&)),
    this, SLOT(slotRemainingChanged(const QString&)));
  connect(m_FinalPaymentEdit, SIGNAL(dateChanged(const QDate&)),
    this, SLOT(slotEndDateChanged(const QDate&)));
  connect(m_frequencyEdit, SIGNAL(itemSelected(int)),
    this, SLOT(slotFrequencyChanged(int)));
  connect(buttonHelp, SIGNAL(clicked()), this, SLOT(slotShowHelp()));

  // force the initial height to be as small as possible
  QTimer::singleShot(0, this, SLOT(slotSetupSize()));
}

KEditScheduleDlg::~KEditScheduleDlg()
{
  delete d;
}

void KEditScheduleDlg::slotSetupSize(void)
{
  resize(width(), minimumSizeHint().height());
}

TransactionEditor* KEditScheduleDlg::startEdit(void)
{
  QValueList<KMyMoneyRegister::SelectedTransaction> list;
  m_register->selectedTransactions(list);
  TransactionEditor* editor = d->m_item->createEditor(m_form, list, QDate());

  // check that we use the same transaction commodity in all selected transactions
  // if not, we need to update this in the editor's list. The user can also bail out
  // of this operation which means that we have to stop editing here.
  if(editor) {
    if(!editor->fixTransactionCommodity(d->m_schedule.account())) {
      // if the user wants to quit, we need to destroy the editor
      // and bail out
      delete editor;
      editor = 0;
    }
  }

  if(editor) {
    connect(editor, SIGNAL(transactionDataSufficient(bool)), buttonOk, SLOT(setEnabled(bool)));
    connect(MyMoneyFile::instance(), SIGNAL(dataChanged()), editor, SLOT(slotReloadEditWidgets()));
    // connect(editor, SIGNAL(finishEdit(const QValueList<KMyMoneyRegister::SelectedTransaction >&)), this, SLOT(slotLeaveEditMode(const QValueList<KMyMoneyRegister::SelectedTransaction >&)));
    connect(editor, SIGNAL(createPayee(const QString&, QCString&)), kmymoney2, SLOT(slotPayeeNew(const QString&, QCString&)));
    connect(editor, SIGNAL(createCategory(MyMoneyAccount&, const MyMoneyAccount&)), kmymoney2, SLOT(slotCategoryNew(MyMoneyAccount&, const MyMoneyAccount&)));
    connect(editor, SIGNAL(createSecurity(MyMoneyAccount&, const MyMoneyAccount&)), kmymoney2, SLOT(slotInvestmentNew(MyMoneyAccount&, const MyMoneyAccount&)));
    connect(MyMoneyFile::instance(), SIGNAL(dataChanged()), editor, SLOT(slotReloadEditWidgets()));

    // create the widgets, place them in the parent and load them with data
    // setup tab order
    d->m_tabOrderWidgets.clear();
    KMyMoneyRegister::Action action = KMyMoneyRegister::ActionWithdrawal;
    switch(d->m_schedule.type()) {
      case MyMoneySchedule::TYPE_DEPOSIT:
        action = KMyMoneyRegister::ActionDeposit;
        break;
      case MyMoneySchedule::TYPE_BILL:
        action = KMyMoneyRegister::ActionWithdrawal;
        break;
      case MyMoneySchedule::TYPE_TRANSFER:
        action = KMyMoneyRegister::ActionTransfer;
        break;
      default:
        break;
    }
    editor->setup(d->m_tabOrderWidgets, d->m_schedule.account(), action);

    // if it's not a check, then we need to clear
    // a possibly assigned check number
    if(d->m_schedule.paymentType() != MyMoneySchedule::STYPE_WRITECHEQUE) {
      QWidget* w = editor->haveWidget("number");
      if(w)
        dynamic_cast<kMyMoneyLineEdit*>(w)->loadText(QString());
    }

    Q_ASSERT(!d->m_tabOrderWidgets.isEmpty());

    // don't forget our three buttons and additional widgets
    d->m_tabOrderWidgets.append(m_weekendOptionEdit);
    d->m_tabOrderWidgets.append(m_estimateEdit);
    d->m_tabOrderWidgets.append(m_autoEnterEdit);
    d->m_tabOrderWidgets.append(m_endSeriesEdit);
    d->m_tabOrderWidgets.append(m_RemainingEdit);
    d->m_tabOrderWidgets.append(m_FinalPaymentEdit);

    d->m_tabOrderWidgets.append(buttonOk);
    d->m_tabOrderWidgets.append(buttonCancel);
    d->m_tabOrderWidgets.append(buttonHelp);
    d->m_tabOrderWidgets.append(m_nameEdit);
    d->m_tabOrderWidgets.append(m_frequencyEdit);
    d->m_tabOrderWidgets.append(m_paymentMethodEdit);
    d->m_tabOrderWidgets.append(m_form);

    // install event filter in all taborder widgets
    QWidget* w;
    for(w = d->m_tabOrderWidgets.first(); w; w = d->m_tabOrderWidgets.next()) {
      w->installEventFilter(this);
    }

    m_nameEdit->setFocus();

    // add the required fields to the mandatory group
    d->m_requiredFields->add(m_nameEdit);
    d->m_requiredFields->add(editor->haveWidget("account"));
    d->m_requiredFields->add(editor->haveWidget("payee"));
    d->m_requiredFields->add(editor->haveWidget("category"));

    // fix labels
    QLabel* label = dynamic_cast<QLabel*>(editor->haveWidget("date-label"));
    if(label) {
      label->setText(i18n("Next due date"));
    }

    d->m_editor = editor;
    slotSetPaymentMethod(d->m_schedule.paymentType());

    connect(m_paymentMethodEdit, SIGNAL(itemSelected(int)), this, SLOT(slotSetPaymentMethod(int)));
  }

  return editor;
}

const MyMoneySchedule& KEditScheduleDlg::schedule(void) const
{
  if(d->m_editor) {
    d->m_schedule.setTransaction(transaction());
    d->m_schedule.setName(m_nameEdit->text());
    d->m_schedule.setFixed(!m_estimateEdit->isChecked());
    d->m_schedule.setOccurence(static_cast<MyMoneySchedule::occurenceE>(m_frequencyEdit->currentItem()));

    switch(m_weekendOptionEdit->currentItem())  {
      case 0:
        d->m_schedule.setWeekendOption(MyMoneySchedule::MoveNothing);
        break;
      case 1:
        d->m_schedule.setWeekendOption(MyMoneySchedule::MoveFriday);
        break;
      case 2:
        d->m_schedule.setWeekendOption(MyMoneySchedule::MoveMonday);
        break;
    }

    switch(static_cast<KMyMoneyRegister::Action>(m_form->tabBar()->currentTab())) {
      case KMyMoneyRegister::ActionDeposit:
        d->m_schedule.setType(MyMoneySchedule::TYPE_DEPOSIT);
        break;
      default:
      case KMyMoneyRegister::ActionWithdrawal:
        d->m_schedule.setType(MyMoneySchedule::TYPE_BILL);
        break;
      case KMyMoneyRegister::ActionTransfer:
        d->m_schedule.setType(MyMoneySchedule::TYPE_TRANSFER);
        break;
    }
    d->m_schedule.setAutoEnter(m_autoEnterEdit->isChecked());
    d->m_schedule.setPaymentType(static_cast<MyMoneySchedule::paymentTypeE>(m_paymentMethodEdit->item()));
    if(m_endSeriesEdit->isEnabled() && m_endSeriesEdit->isChecked()) {
      d->m_schedule.setEndDate(m_FinalPaymentEdit->date());
    } else {
      d->m_schedule.setEndDate(QDate());
    }
  }
  return d->m_schedule;
}

MyMoneyTransaction KEditScheduleDlg::transaction(void) const
{
  MyMoneyTransaction t = d->m_schedule.transaction();

  if(d->m_editor) {
    d->m_editor->createTransaction(t, d->m_schedule.transaction(), d->m_schedule.transaction().splits()[0], true);
  }

  t.clearId();
  t.setEntryDate(QDate());
  return t;
}
#if 0
QDate KEditScheduleDlg::adjustDate(const QDate& _date) const
{
  QDate date(_date);
  if (d->m_schedule.weekendOption() != MyMoneySchedule::MoveNothing) {
    int dayOfWeek = date.dayOfWeek();
    if (dayOfWeek >= 6) {
      if (d->m_schedule.weekendOption() == MyMoneySchedule::MoveFriday) {
        if (dayOfWeek == 7)
          date = date.addDays(-2);
        else
          date = date.addDays(-1);
      } else {
        if (dayOfWeek == 6)
          date = date.addDays(2);
        else
          date = date.addDays(1);
      }
    }
  }
  return date;
}
#endif

bool KEditScheduleDlg::focusNextPrevChild(bool next)
{
  bool  rc = false;

  // qDebug("KEditScheduleDlg::focusNextPrevChild(editmode=%s)", m_inEditMode ? "true" : "false");
  QWidget *w = 0;
  QWidget *currentWidget;

  w = qApp->focusWidget();
  while(w && d->m_tabOrderWidgets.find(w) == -1) {
    // qDebug("'%s' not in list, use parent", w->className());
    w = w->parentWidget();
  }
  // if(w) qDebug("tab order is at '%s'", w->className());
  currentWidget = d->m_tabOrderWidgets.current();
  w = next ? d->m_tabOrderWidgets.next() : d->m_tabOrderWidgets.prev();

  do {
    if(!w) {
      w = next ? d->m_tabOrderWidgets.first() : d->m_tabOrderWidgets.last();
    }

    if(w != currentWidget
    && ((w->focusPolicy() & TabFocus) == TabFocus)
    && w->isVisible() && w->isEnabled()) {
      // qDebug("Selecting '%s' as focus", w->className());
      w->setFocus();
      rc = true;
      break;
    }
    w = next ? d->m_tabOrderWidgets.next() : d->m_tabOrderWidgets.prev();
  } while(w != currentWidget);

  return rc;
}

void KEditScheduleDlg::slotRemainingChanged(const QString& text)
{
  // Make sure the required fields are set
  kMyMoneyDateInput* dateEdit = dynamic_cast<kMyMoneyDateInput*>(d->m_editor->haveWidget("postdate"));
  d->m_schedule.setStartDate(dateEdit->date());
  d->m_schedule.setOccurence(static_cast<MyMoneySchedule::occurenceE>(m_frequencyEdit->currentItem()));

  if(d->m_schedule.transactionsRemaining() != text.toInt()) {
    m_FinalPaymentEdit->setDate(d->m_schedule.dateAfter(text.toInt()));
  }
}

void KEditScheduleDlg::slotEndDateChanged(const QDate& date)
{
  // Make sure the required fields are set
  kMyMoneyDateInput* dateEdit = dynamic_cast<kMyMoneyDateInput*>(d->m_editor->haveWidget("postdate"));
  d->m_schedule.setStartDate(dateEdit->date());
  d->m_schedule.setOccurence(static_cast<MyMoneySchedule::occurenceE>(m_frequencyEdit->currentItem()));

  if(d->m_schedule.endDate() != date) {
    d->m_schedule.setEndDate(date);
    if(d->m_schedule.transactionsRemaining() != m_RemainingEdit->text().toInt());
      m_RemainingEdit->setText(QString::number(d->m_schedule.transactionsRemaining()));
  }
}

void KEditScheduleDlg::slotSetPaymentMethod(int item)
{
  kMyMoneyLineEdit* dateEdit = dynamic_cast<kMyMoneyLineEdit*>(d->m_editor->haveWidget("number"));
  if(dateEdit) {
    dateEdit->setShown(item == MyMoneySchedule::STYPE_WRITECHEQUE);

    // hiding the label does not work, because the label underneath will shine
    // through. So we either write the label or a blank
    QLabel* label = dynamic_cast<QLabel *>(d->m_editor->haveWidget("number-label"));
    if(label) {
      label->setText((item == MyMoneySchedule::STYPE_WRITECHEQUE) ? i18n("Number") : " ");
    }
  }
}

void KEditScheduleDlg::slotFrequencyChanged(int item)
{
  m_endSeriesEdit->setEnabled(item != MyMoneySchedule::OCCUR_ONCE);
  if(m_endSeriesEdit->isChecked())
    m_endOptionsFrame->setEnabled(item != MyMoneySchedule::OCCUR_ONCE);
}

void KEditScheduleDlg::slotShowHelp(void)
{
  kapp->invokeHelp("details.schedules.intro");
}

#include <keditscheduledlg.moc>