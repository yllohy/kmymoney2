/***************************************************************************
                          ksplittransactiondlg.cpp  -  description
                             -------------------
    begin                : Thu Jan 10 2002
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

#include <qpushbutton.h>
#include <qlabel.h>
#include <qtable.h>
#include <qtimer.h>
#include <qptrlist.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qcursor.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kglobal.h>
#include <kconfig.h>
#include <klocale.h>
//#include <kiconloader.h>
//#include <kguiitem.h>
#include <kmessagebox.h>
#include <kpushbutton.h>
#include <kactivelabel.h>
#include <kstdguiitem.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "ksplittransactiondlg.h"
#include <kmymoney/kmymoneyedit.h>
#include <kmymoney/kmymoneylineedit.h>
#include <kmymoney/mymoneyfile.h>

#include "kmymoneysplittable.h"
#include "../dialogs/ksplitcorrectiondlg.h"

KSplitTransactionDlg::KSplitTransactionDlg(const MyMoneyTransaction& t,
                                           const MyMoneySplit& s,
                                           const MyMoneyAccount& acc,
                                           const bool amountValid,
                                           const bool deposit,
                                           const MyMoneyMoney& calculatedValue,
                                           const QMap<QString, MyMoneyMoney>& priceInfo,
                                           QWidget* parent, const char* name) :
  KSplitTransactionDlgDecl(parent, name, true),
  m_account(acc),
  m_split(s),
  m_precision(2),
  m_amountValid(amountValid),
  m_isDeposit(deposit),
  m_calculatedValue(calculatedValue)
{
  // add icons to buttons
  KIconLoader *il = KGlobal::iconLoader();
  KGuiItem finishButtenItem( i18n( "&Finish" ),
                    QIconSet(il->loadIcon("button_ok", KIcon::Small, KIcon::SizeSmall)),
                    i18n("Accept splits and return to transaction form"),
                    i18n("Use this to accept all changes to the splits and return to the transaction"));
  finishBtn->setGuiItem(finishButtenItem);

  cancelBtn->setGuiItem(KStdGuiItem::cancel());

  KGuiItem clearButtenItem( i18n( "Clear &All" ),
                    QIconSet(il->loadIcon("edittrash", KIcon::Small, KIcon::SizeSmall)),
                    i18n("Clear all splits"),
                    i18n("Use this to clear all splits of this transaction"));
  clearAllBtn->setGuiItem(clearButtenItem);


  KGuiItem mergeButtenItem( i18n( "&Merge" ),
                             QIconSet(il->loadIcon("sum", KIcon::Small, KIcon::SizeSmall)),
                                      "", "");
  mergeBtn->setGuiItem(mergeButtenItem);

  // make finish the default
  finishBtn->setDefault(true);

  // setup the focus
  cancelBtn->setFocusPolicy(QWidget::NoFocus);
  finishBtn->setFocusPolicy(QWidget::NoFocus);
  clearAllBtn->setFocusPolicy(QWidget::NoFocus);

  // connect signals with slots
  connect(transactionsTable, SIGNAL(transactionChanged(const MyMoneyTransaction&)),
          this, SLOT(slotSetTransaction(const MyMoneyTransaction&)));
  connect(transactionsTable, SIGNAL(createCategory(const QString&, QString&)), this, SLOT(slotCreateCategory(const QString&, QString&)));
  connect(transactionsTable, SIGNAL(objectCreation(bool)), this, SIGNAL(objectCreation(bool)));

  connect(transactionsTable, SIGNAL(returnPressed()), this, SLOT(accept()));
  connect(transactionsTable, SIGNAL(escapePressed()), this, SLOT(reject()));

  connect(cancelBtn, SIGNAL(clicked()), this, SLOT(reject()));
  connect(finishBtn, SIGNAL(clicked()), this, SLOT(accept()));
  connect(clearAllBtn, SIGNAL(clicked()), this, SLOT(slotClearAllSplits()));
  connect(mergeBtn, SIGNAL(clicked()), this, SLOT(slotMergeSplits()));

  // setup the precision
  try {
    m_precision = MyMoneyMoney::denomToPrec(m_account.fraction());
  } catch(MyMoneyException *e) {
    delete e;
  }

  slotSetTransaction(t);

  // pass on those vars
  transactionsTable->setup(priceInfo);

  // Trick: it seems, that the initial sizing of the dialog does
  // not work correctly. At least, the columns do not get displayed
  // correct. Reason: the return value of transactionsTable->visibleWidth()
  // is incorrect. If the widget is visible, resizing works correctly.
  // So, we let the dialog show up and resize it then. It's not really
  // clean, but the only way I got the damned thing working.
  QTimer::singleShot( 10, this, SLOT(initSize()) );
}

KSplitTransactionDlg::~KSplitTransactionDlg()
{
}

int KSplitTransactionDlg::exec(void)
{
  // for deposits, we invert the sign of all splits.
  // don't forget to revert when we're done ;-)
  if(m_isDeposit) {
    for(unsigned i = 0; i < m_transaction.splits().count(); ++i) {
      MyMoneySplit split = m_transaction.splits()[i];
      split.setValue(-split.value());
      split.setShares(-split.shares());
      m_transaction.modifySplit(split);
    }
  }

  int rc;
  do {
    transactionsTable->setFocus();

    // initialize the display
    transactionsTable->setTransaction(m_transaction, m_split, m_account);
    updateSums();

    rc = KSplitTransactionDlgDecl::exec();

    if(rc == QDialog::Accepted) {
      if(!diffAmount().isZero()) {
        KSplitCorrectionDlgDecl* corrDlg = new KSplitCorrectionDlgDecl(this, 0, true);

        // add icons to buttons
        corrDlg->okBtn->setGuiItem(KStdGuiItem::ok());
        corrDlg->cancelBtn->setGuiItem(KStdGuiItem::cancel());

        MyMoneySplit split = m_transaction.splits()[0];
        QString total = (-split.value()).formatMoney("", m_precision);
        QString sums = splitsValue().formatMoney("", m_precision);
        QString diff = diffAmount().formatMoney("", m_precision);

        // now modify the text items of the dialog to contain the correct values
        QString q = i18n("The total amount of this transaction is %1 while "
                                "the sum of the splits is %2. The remaining %3 are "
                                "unassigned.")
                    .arg(total)
                    .arg(sums)
                    .arg(diff);
        corrDlg->explanation->setText(q);

        q = i18n("Change &total amount of transaction to %1.").arg(sums);
        corrDlg->changeBtn->setText(q);

        q = i18n("&Distribute difference of %1 among all splits.").arg(diff);
        corrDlg->distributeBtn->setText(q);
        // FIXME remove the following line once distribution among
        //       all splits is implemented
        corrDlg->distributeBtn->hide();


        // if we have only two splits left, we don't allow leaving sth. unassigned.
        if(m_transaction.splitCount() < 3) {
          q = i18n("&Leave total amount of transaction at %1.").arg(total);
        } else {
          q = i18n("&Leave %1 unassigned.").arg(diff);
        }
        corrDlg->leaveBtn->setText(q);

        if((rc = corrDlg->exec()) == QDialog::Accepted) {
          QButton* button = corrDlg->buttonGroup->selected();
          if(button != 0) {
            switch(corrDlg->buttonGroup->id(button)) {
              case 0:       // continue to edit
                rc = QDialog::Rejected;
                break;

              case 1:       // modify total
                split.setValue(-splitsValue());
                split.setShares(-splitsValue());
                m_transaction.modifySplit(split);
                break;

              case 2:       // distribute difference
                qDebug("distribution of difference not yet supported in KSplitTransactionDlg::slotFinishClicked()");
                break;

              case 3:       // leave unassigned
                break;
            }
          }
        }
        delete corrDlg;
      }
    } else
      break;

  } while(rc != QDialog::Accepted);

  // for deposits, we inverted the sign of all splits.
  // now we revert it back, so that things are left correct
  if(m_isDeposit) {
    for(unsigned i = 0; i < m_transaction.splits().count(); ++i) {
      MyMoneySplit split = m_transaction.splits()[i];
      split.setValue(-split.value());
      split.setShares(-split.shares());
      m_transaction.modifySplit(split);
    }
  }

  return rc;
}

void KSplitTransactionDlg::initSize(void)
{
  QDialog::resize(width(), height()+1);
}

void KSplitTransactionDlg::accept()
{
  transactionsTable->slotCancelEdit();
  KSplitTransactionDlgDecl::accept();
}

void KSplitTransactionDlg::reject()
{
  // cancel any edit activity in the split register
  transactionsTable->slotCancelEdit();
  KSplitTransactionDlgDecl::reject();
}

void KSplitTransactionDlg::slotClearAllSplits(void)
{
  int answer;
  answer = KMessageBox::warningContinueCancel (this,
     i18n("You are about to delete all splits of this transaction. "
          "Do you really want to continue?"),
     i18n("KMyMoney"),
     i18n("Continue")
     );

  if(answer == KMessageBox::Continue) {
    transactionsTable->slotCancelEdit();
    QValueList<MyMoneySplit> list = transactionsTable->getSplits(m_transaction);
    QValueList<MyMoneySplit>::ConstIterator it;

    // clear all but the one referencing the account
    for(it = list.begin(); it != list.end(); ++it) {
      m_transaction.removeSplit(*it);
    }

    transactionsTable->setTransaction(m_transaction, m_split, m_account);
    slotSetTransaction(m_transaction);
  }
}

void KSplitTransactionDlg::slotMergeSplits(void)
{
  QValueList<MyMoneySplit> list = transactionsTable->getSplits(m_transaction);
  QValueList<MyMoneySplit>::ConstIterator it;

  try {
    // collect all splits, merge them if needed and remove from transaction
    QValueList<MyMoneySplit> splits;
    for(it = list.begin(); it != list.end(); ++it) {
      QValueList<MyMoneySplit>::iterator it_s;
      for(it_s = splits.begin(); it_s != splits.end(); ++it_s) {
        if((*it_s).accountId() == (*it).accountId()
        && (*it_s).memo().isEmpty() && (*it).memo().isEmpty())
          break;
      }
      if(it_s != splits.end()) {
        (*it_s).setShares((*it).shares() + (*it_s).shares());
        (*it_s).setValue((*it).value() + (*it_s).value());
      } else {
        splits << *it;
      }
      m_transaction.removeSplit(*it);
    }

    // now add them back to the transaction
    QValueList<MyMoneySplit>::iterator it_s;
    for(it_s = splits.begin(); it_s != splits.end(); ++it_s) {
      (*it_s).clearId();
      m_transaction.addSplit(*it_s);
    }

    transactionsTable->setTransaction(m_transaction, m_split, m_account);
    slotSetTransaction(m_transaction);
  } catch(MyMoneyException* e) {
    delete e;
  }
}

void KSplitTransactionDlg::slotSetTransaction(const MyMoneyTransaction& t)
{
  m_transaction = t;
  QValueList<MyMoneySplit> list = transactionsTable->getSplits(m_transaction);
  QValueList<MyMoneySplit>::ConstIterator it;

  // check if we can merge splits or not
  QMap<QString, int> splits;
  for(it = list.begin(); it != list.end(); ++it) {
    splits[(*it).accountId()]++;
  }
  QMap<QString, int>::const_iterator it_s;
  for(it_s = splits.begin(); it_s != splits.end(); ++it_s) {
    if((*it_s) > 1)
      break;
  }
  mergeBtn->setDisabled(it_s == splits.end());

  updateSums();
}

void KSplitTransactionDlg::updateSums(void)
{
  MyMoneyMoney splits(splitsValue());
  MyMoneySplit split = m_transaction.splits()[0];

  if(m_amountValid == false) {
    split.setValue(-splits);
    m_transaction.modifySplit(split);
  }

  splitSum->setText("<b>" + splits.formatMoney("", m_precision) + " ");
  splitUnassigned->setText("<b>" + diffAmount().formatMoney("", m_precision) + " ");
  transactionAmount->setText("<b>" + (-split.value()).formatMoney("", m_precision) + " ");
}

MyMoneyMoney KSplitTransactionDlg::splitsValue(void)
{
  MyMoneyMoney splitsValue(m_calculatedValue);
  QValueList<MyMoneySplit> list = transactionsTable->getSplits(m_transaction);
  QValueList<MyMoneySplit>::ConstIterator it;

  // calculate the current sum of all split parts
  for(it = list.begin(); it != list.end(); ++it) {
    if((*it).value() != MyMoneyMoney::autoCalc)
      splitsValue += (*it).value();
  }

  return splitsValue;
}

MyMoneyMoney KSplitTransactionDlg::diffAmount(void)
{
  MyMoneyMoney diff(0);

  // if there is an amount specified in the transaction, we need to calculate the
  // difference, otherwise we display the difference as 0 and display the same sum.
  if(m_amountValid) {
    MyMoneySplit split = m_transaction.splits()[0];

    diff = -(splitsValue() + split.value());
  }
  return diff;
}

void KSplitTransactionDlg::slotCreateCategory(const QString& name, QString& id)
{
  MyMoneyAccount acc, parent;
  acc.setName(name);

  if(m_isDeposit)
    parent = MyMoneyFile::instance()->income();
  else
    parent = MyMoneyFile::instance()->expense();

  // TODO extract possible first part of a hierarchy and check if it is one
  // of our top categories. If so, remove it and select the parent
  // according to this information.

  emit createCategory(acc, parent);

  // return id
  id = acc.id();
}

#include "ksplittransactiondlg.moc"
