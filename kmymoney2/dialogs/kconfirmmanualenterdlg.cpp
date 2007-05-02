/***************************************************************************
                          kconfirmmanualenterdlg.cpp
                             -------------------
    begin                : Mon Apr  9 2007
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

#include <qradiobutton.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <klocale.h>
#include <ktextedit.h>
#include <kpushbutton.h>
#include <kstdguiitem.h>
#include <kmessagebox.h>

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoney/mymoneyfile.h>
#include <kmymoney/kmymoneyutils.h>
#include "kconfirmmanualenterdlg.h"

KConfirmManualEnterDlg::KConfirmManualEnterDlg(const MyMoneySchedule& schedule, QWidget* parent, const char* name) :
  KConfirmManualEnterDlgDecl(parent, name)
{
  buttonOk->setGuiItem(KStdGuiItem::ok());
  buttonCancel->setGuiItem(KStdGuiItem::cancel());
  m_onceRadio->setChecked(true);

  if(schedule.type() == MyMoneySchedule::TYPE_LOANPAYMENT) {
    m_setRadio->setEnabled(false);
    m_discardRadio->setEnabled(false);
  }
}

void KConfirmManualEnterDlg::loadTransactions(const MyMoneyTransaction& to, const MyMoneyTransaction& tn)
{
  QString messageDetail("<qt>");
  MyMoneyFile* file = MyMoneyFile::instance();
  int noItemsChanged=0;

#if 0
  // if no schedule is present, we cannot enter it
  if(m_schedule.id().isEmpty())
    return false;

  if (m_fromAccountId == m_toAccountId)
  {
    KMessageBox::error(this, i18n("Account and transfer account are the same.  Please change one."));
    m_from->setFocus();
    return false;
  }

  if (!checkDateInPeriod(m_date->date()))
    return false;
#endif

  try
  {
    QString po, pn;
    if(to.splits()[0].payeeId())
      po = file->payee(to.splits()[0].payeeId()).name();
    if(tn.splits()[0].payeeId())
      pn = file->payee(tn.splits()[0].payeeId()).name();

    if (po != pn) {
      noItemsChanged++;
      messageDetail += i18n("Payee changed.<br>&nbsp;&nbsp;&nbsp;Old: <b>%1</b>, New: <b>%2</b><p>").arg(po).arg(pn);
    }
#if 0
    if (  (m_schedule.type() == MyMoneySchedule::TYPE_TRANSFER ||
          m_schedule.type() == MyMoneySchedule::TYPE_BILL) &&
          m_from->currentText() != m_schedule.account().name())
    {
      noItemsChanged++;
      messageDetail += i18n("Account changed.  Old: \"%1\", New: \"%2\"")
        .arg(m_schedule.account().name()).arg(m_from->currentText()) + QString("\n");
    }

    if (  m_schedule.type() == MyMoneySchedule::TYPE_DEPOSIT &&
          m_to->currentText() != m_schedule.account().name())
    {
      noItemsChanged++;
      messageDetail += i18n("Account changed.  Old: \"%1\", New: \"%2\"")
        .arg(m_schedule.account().name()).arg(m_to->currentText()) + QString("\n");
    }
#endif

    if(file->isTransfer(to) && file->isTransfer(tn)) {
      if(to.splits()[1].accountId() != tn.splits()[1].accountId()) {
        noItemsChanged++;
        messageDetail += i18n("Transfer account changed.<br>&nbsp;&nbsp;&nbsp;Old: <b>%1</b>, New: <b>%2</b><p>")
          .arg(file->account(to.splits()[0].accountId()).name())
          .arg(file->account(to.splits()[1].accountId()).name());
      }
    } else {
      QString co, cn;
      switch(to.splitCount()) {
        default:
          co = i18n("Split transaction (category replacement)", "Split transaction");
          break;
        case 2:
          co = file->accountToCategory(to.splits()[1].accountId());
        case 1:
          break;
      }

      switch(tn.splitCount()) {
        default:
          cn = i18n("Split transaction (category replacement)", "Split transaction");
          break;
        case 2:
          cn = file->accountToCategory(tn.splits()[1].accountId());
        case 1:
          break;
      }
      if (co != cn)
      {
        noItemsChanged++;
        messageDetail += i18n("Category changed.<br>&nbsp;&nbsp;&nbsp;Old: <b>%1</b>, New: <b>%2</b><p>").arg(co).arg(cn);
      }
    }

    QString mo, mn;
    mo = to.splits()[0].memo();
    mn = tn.splits()[0].memo();
    if(mo.isEmpty())
       mo = QString("<i>")+i18n("empty")+QString("</i>");
    if(mn.isEmpty())
       mn = QString("<i>")+i18n("empty")+QString("</i>");
    if (mo != mn)
    {
      noItemsChanged++;
      messageDetail += i18n("Memo changed.<br>&nbsp;&nbsp;&nbsp;Old: <b>%1</b>, New: <b>%2</b><p>").arg(mo).arg(mn);
    }

    MyMoneyMoney ao, an;
    ao = to.splits()[0].value();
    an = tn.splits()[0].value();
    if (ao != an) {
      noItemsChanged++;
      messageDetail += i18n("Amount changed.<br>&nbsp;&nbsp;&nbsp;Old: <b>%1</b>, New: <b>%2</b><p>")
        .arg(ao.formatMoney()).arg(an.formatMoney());
    }

    MyMoneySplit::reconcileFlagE fo, fn;
    fo = to.splits()[0].reconcileFlag();
    fn = tn.splits()[0].reconcileFlag();
    if(fo != fn) {
      noItemsChanged++;
      messageDetail += i18n("Reconciliation flag changed.<br>&nbsp;&nbsp;&nbsp;Old: <b>%1</b>, New: <b>%2</b><p>")
        .arg(KMyMoneyUtils::reconcileStateToString(fo, true))
        .arg(KMyMoneyUtils::reconcileStateToString(fn, true));
    }
  }
  catch (MyMoneyException *e)
  {
    KMessageBox::error(this, i18n("Fatal error in determining data: ") + e->what());
    delete e;
  }

  messageDetail += "</qt>";
  m_details->setText(messageDetail);
  return;
}

KConfirmManualEnterDlg::Action KConfirmManualEnterDlg::action(void) const
{
  if(m_discardRadio->isChecked())
    return UseOriginal;
  if(m_setRadio->isChecked())
    return ModifyAlways;
  return ModifyOnce;
}

#include "kconfirmmanualenterdlg.moc"
