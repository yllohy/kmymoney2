/***************************************************************************
                          kenterscheduledlg.cpp
                             -------------------
    begin                : Sat Apr  7 2007
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
#include <qlabel.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <klocale.h>
#include <kmessagebox.h>
#include <kpushbutton.h>
#include <kstdguiitem.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kenterscheduledlg.h"
#include "../dialogs/kcurrencycalculator.h"
#include <kmymoney/register.h>
#include <kmymoney/transactionform.h>
#include <kmymoney/transaction.h>
#include <kmymoney/transactioneditor.h>
#include <kmymoney/kmymoneyutils.h>
#include <kmymoney/mymoneyfinancialcalculator.h>
#include <kmymoney/kmymoneylineedit.h>

#include "../kmymoney2.h"

class KEnterScheduleDlgPrivate
{
public:
  KEnterScheduleDlgPrivate() : m_item(0), m_showWarningOnce(true) {};
  ~KEnterScheduleDlgPrivate() {};

  MyMoneySchedule                m_schedule;
  KMyMoneyRegister::Transaction* m_item;
  QWidgetList                    m_tabOrderWidgets;
  bool                           m_showWarningOnce;
};

KEnterScheduleDlg::KEnterScheduleDlg(QWidget *parent, const MyMoneySchedule& schedule) :
  KEnterScheduleDlgDecl(parent, "kenterscheduledlg"),
  d(new KEnterScheduleDlgPrivate)
{
  d->m_schedule = schedule;
  buttonOk->setGuiItem(KStdGuiItem::ok());
  buttonCancel->setGuiItem(KStdGuiItem::cancel());
  buttonHelp->setGuiItem(KStdGuiItem::help());

  // make sure, we have a tabbar with the form
  KMyMoneyTransactionForm::TabBar* tabbar = m_form->tabBar(m_form->parentWidget());

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
  m_form->slotSetTransaction(d->m_item);

  // no need to see the tabbar
  tabbar->hide();

  // setup name and type
  m_scheduleName->setText(d->m_schedule.name());
  m_type->setText(KMyMoneyUtils::scheduleTypeToString(d->m_schedule.type()));

  // force the initial height to be as small as possible
  QTimer::singleShot(0, this, SLOT(slotSetupSize()));
}

KEnterScheduleDlg::~KEnterScheduleDlg()
{
  delete d;
}

MyMoneyTransaction KEnterScheduleDlg::transaction(void)
{
  MyMoneyTransaction t = d->m_schedule.transaction();
  QDate dueDate;

  try {
    if (d->m_schedule.type() == MyMoneySchedule::TYPE_LOANPAYMENT) {
      MyMoneySplit interestSplit, amortizationSplit;
      QValueList<MyMoneySplit>::ConstIterator it_s;
      for(it_s = t.splits().begin(); it_s != t.splits().end(); ++it_s) {
        if((*it_s).value() == MyMoneyMoney::autoCalc) {
          if((*it_s).action() == MyMoneySplit::ActionAmortization) {
            amortizationSplit = (*it_s);
          } else if((*it_s).action() == MyMoneySplit::ActionInterest) {
            interestSplit = (*it_s);
          }
        }
      }

      if(!amortizationSplit.id().isEmpty() && !interestSplit.id().isEmpty()) {
        MyMoneyAccountLoan acc(MyMoneyFile::instance()->account(amortizationSplit.accountId()));
        MyMoneyFinancialCalculator calc;

        // FIXME: setup dueDate according to when the interest should be calculated
        // current implementation: take the date of the next payment according to
        // the schedule. If the calculation is based on the payment reception, and
        // the payment is overdue then take the current date
        dueDate = d->m_schedule.nextPayment(d->m_schedule.lastPayment());
        if(acc.interestCalculation() == MyMoneyAccountLoan::paymentReceived) {
          if(dueDate < QDate::currentDate())
            dueDate = QDate::currentDate();
        }

        // we need to calculate the balance at the time the payment is due
        MyMoneyMoney balance = MyMoneyFile::instance()->balance(acc.id(), dueDate.addDays(-1));

  /*
        QValueList<MyMoneyTransaction> list;
        QValueList<MyMoneyTransaction>::ConstIterator it;
        MyMoneySplit split;
        MyMoneyTransactionFilter filter(acc.id());

        filter.setDateFilter(QDate(), dueDate.addDays(-1));
        list = MyMoneyFile::instance()->transactionList(filter);

        for(it = list.begin(); it != list.end(); ++it) {
          try {
            split = (*it).splitByAccount(acc.id());
            balance += split.value();

          } catch(MyMoneyException *e) {
            // account is not referenced within this transaction
            delete e;
          }
        }
  */

        // FIXME: for now, we only support interest calculation at the end of the period
        calc.setBep();
        // FIXME: for now, we only support periodic compounding
        calc.setDisc();

        calc.setPF(KMyMoneyUtils::occurenceToFrequency(d->m_schedule.occurence()));
        // FIXME: for now we only support compounding frequency == payment frequency
        calc.setCF(KMyMoneyUtils::occurenceToFrequency(d->m_schedule.occurence()));

        calc.setPv(balance.toDouble());
        calc.setIr(static_cast<FCALC_DOUBLE> (acc.interestRate(dueDate).abs().toDouble()));
        calc.setPmt(acc.periodicPayment().toDouble());

        MyMoneyMoney interest(calc.interestDue()), amortization;
        interest = interest.abs();    // make sure it's positive for now
        amortization = acc.periodicPayment() - interest;

        if(acc.accountType() == MyMoneyAccount::AssetLoan) {
          interest = -interest;
          amortization = -amortization;
        }
        amortizationSplit.setValue(amortization);
        interestSplit.setValue(interest);

        // FIXME: for now we only assume loans to be in the base currency
        amortizationSplit.setShares(amortization);
        interestSplit.setShares(interest);

        t.modifySplit(amortizationSplit);
        t.modifySplit(interestSplit);
      }
    } else {
      dueDate = date(d->m_schedule.nextPayment(d->m_schedule.lastPayment()));
    }
  } catch (MyMoneyException* e) {
    KMessageBox::detailedError(this, i18n("Unable to load schedule details"), e->what());
    delete e;
  }

  t.clearId();
  t.setEntryDate(QDate());
  t.setPostDate(dueDate);
  return t;
}

QDate KEnterScheduleDlg::date(const QDate& _date) const
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


void KEnterScheduleDlg::slotSetupSize(void)
{
  resize(width(), minimumSizeHint().height());
}

int KEnterScheduleDlg::exec(void)
{
  if(d->m_showWarningOnce) {
    d->m_showWarningOnce = false;
    KMessageBox::information(this, QString("<qt>")+i18n("<p>Please check that all the details in the following dialog are correct and press OK.</p><p>Editable data can be changed and can either be applied to just this occurence or for all subsequent occurences for this schedule.  (You will be asked what you intend after pressing OK in the following dialog)</p>")+QString("</qt>"), i18n("Enter scheduled transaction"), "EnterScheduleDlgInfo");
  }

  return KEnterScheduleDlgDecl::exec();
}

TransactionEditor* KEnterScheduleDlg::startEdit(void)
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

    // create the widgets, place them in the parent and load them with data
    // setup tab order
    d->m_tabOrderWidgets.clear();
    KMyMoneyRegister::Action action = KMyMoneyRegister::ActionWithdrawal;
    switch(d->m_schedule.type()) {
      case MyMoneySchedule::TYPE_DEPOSIT:
        action = KMyMoneyRegister::ActionDeposit;
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

    // don't forget our three buttons
    d->m_tabOrderWidgets.append(buttonOk);
    d->m_tabOrderWidgets.append(buttonCancel);
    d->m_tabOrderWidgets.append(buttonHelp);

    // install event filter in all taborder widgets
    for(QWidget* w = d->m_tabOrderWidgets.first(); w; w = d->m_tabOrderWidgets.next()) {
      w->installEventFilter(this);
    }

    // Check if the editor has some preference on where to set the focus
    // If not, set the focus to the first widget in the tab order
    QWidget* focusWidget = editor->firstWidget();
    if(!focusWidget)
      focusWidget = d->m_tabOrderWidgets.first();
  }

  return editor;
}

bool KEnterScheduleDlg::focusNextPrevChild(bool next)
{
  bool  rc = false;

  // qDebug("KGlobalLedgerView::focusNextPrevChild(editmode=%s)", m_inEditMode ? "true" : "false");
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

#include "kenterscheduledlg.moc"

