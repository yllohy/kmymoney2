/***************************************************************************
                          kledgerviewcreditcard.cpp  -  description
                             -------------------
    begin                : Wed Nov 27 2002
    copyright            : (C) 2000-2002 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
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

// ----------------------------------------------------------------------------
// KDE Includes

#include <kpushbutton.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kledgerviewcreditcard.h"

#include "../views/kledgerviewcheckings.h"
#include "../widgets/kmymoneypayee.h"
#include "../widgets/kmymoneycategory.h"
#include "../widgets/kmymoneycombo.h"
#include "../widgets/kmymoneylineedit.h"
#include "../widgets/kmymoneyedit.h"
#include "../widgets/kmymoneydateinput.h"
#include "../widgets/kmymoneyregister.h"
#include "../widgets/kmymoneytransactionform.h"
#include "../mymoney/mymoneyfile.h"

KLedgerViewCreditCard::KLedgerViewCreditCard(QWidget *parent, const char *name )
  : KLedgerViewCheckings(parent,name)
{
  m_register->horizontalHeader()->setLabel(4, i18n("Charge"));
  m_register->horizontalHeader()->setLabel(5, i18n("Payment"));

  m_form->tabBar()->removeTab(m_tabCheck);
  m_form->tabBar()->removeTab(m_tabAtm);
  m_tabCheck = m_tabAtm = 0;
  m_form->tabBar()->tabAt(0)->setText(i18n("&Payment"));
  m_form->tabBar()->tabAt(2)->setText(i18n("C&harge"));

  m_register->setAction(QCString(MyMoneySplit::ActionDeposit), i18n("Payment"));
  m_register->setAction(QCString(MyMoneySplit::ActionWithdrawal), i18n("Charge"));

  m_register->repaintContents(false);

  // setup action index
  m_actionIdx[0] =
  m_actionIdx[1] =
  m_actionIdx[3] = 0;
  m_actionIdx[2] = 1;
  m_actionIdx[3] = 2;
}

KLedgerViewCreditCard::~KLedgerViewCreditCard()
{
}

void KLedgerViewCreditCard::fillSummary(void)
{
  MyMoneyMoney balance;
  MyMoneyFile* file = MyMoneyFile::instance();
  KLedgerViewCheckingsSummaryLine* summary = dynamic_cast<KLedgerViewCheckingsSummaryLine*>(m_summaryLine);

  if(summary) {
    summary->clear();

    if(!accountId().isEmpty()) {
      try {
        balance = file->balance(accountId());
        summary->setBalance(i18n("You currently owe: ") + (-balance).formatMoney(file->currency(m_account.currencyId()).tradingSymbol()));
  /* the fancy version. don't know, if we should use it
        if(balance < 0)
          summary->setText(i18n("You currently owe: ") + (-balance).formatMoney());
        else
          summary->setText(i18n("Current balance: ") + balance.formatMoney());
  */

        QDate date;
        if(!m_account.value("lastStatementDate").isEmpty())
          date = QDate::fromString(m_account.value("lastStatementDate"), Qt::ISODate);
        if(date.isValid())
          summary->setReconciliationDate(i18n("Reconciled: %1").arg(KGlobal::locale()->formatDate(date, true)));
      } catch(MyMoneyException *e) {
          qDebug("Unexpected exception in KLedgerViewCreditCard::fillSummary");
      }
    }
  }
}

bool KLedgerViewCreditCard::eventFilter( QObject *o, QEvent *e )
{
  return KLedgerViewCheckings::eventFilter(o, e);
}

#include "kledgerviewcreditcard.moc"
