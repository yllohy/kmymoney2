/***************************************************************************
                          kpayeesview.cpp  -  description
                             -------------------
    begin                : Thu Jan 24 2002
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
#include <qcombobox.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qmultilineedit.h>
#include <qpixmap.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kglobal.h>
#include <klocale.h>
#include <kconfig.h>
#include <kmessagebox.h>
#include <klistview.h>

#if QT_VERSION > 300
#include <kstandarddirs.h>
#else
#include <kstddirs.h>
#endif

// ----------------------------------------------------------------------------
// Project Includes

#include "kpayeesview.h"
#include "kbanklistitem.h"
#include "../mymoney/mymoneyfile.h"

KPayeesView::KPayeesView(QWidget *parent, const char *name )
  : KPayeesViewDecl(parent,name),
    m_suspendUpdate(false)
{
//  QString filename = KGlobal::dirs()->findResource("appdata", "pics/dlg_payees.png");
//  QPixmap *pm = new QPixmap(filename);
//  m_qpixmaplabel->setPixmap(*pm);

  readConfig();

  m_transactionView->setSorting(-1);
  m_transactionView->setAllColumnsShowFocus(true);
  m_transactionView->setColumnWidthMode(2, QListView::Manual);
  m_transactionView->setColumnAlignment(3, Qt::AlignRight);
  // never show horizontal scroll bars
  m_transactionView->setHScrollBarMode(QScrollView::AlwaysOff);

  connect(payeeCombo, SIGNAL(activated(const QString&)), this, SLOT(payeeHighlighted(const QString&)));
  connect(addButton, SIGNAL(clicked()), this, SLOT(slotAddClicked()));
  connect(payeeEdit, SIGNAL(textChanged(const QString&)), this, SLOT(slotPayeeTextChanged(const QString&)));
  connect(updateButton, SIGNAL(clicked()), this, SLOT(slotUpdateClicked()));
  connect(deleteButton, SIGNAL(clicked()), this, SLOT(slotDeleteClicked()));

  connect(m_transactionView, SIGNAL(doubleClicked(QListViewItem*)),
          this, SLOT(slotTransactionDoubleClicked(QListViewItem*)));

  MyMoneyFile::instance()->attach(MyMoneyFile::NotifyClassPayee, this);
}

KPayeesView::~KPayeesView()
{
  MyMoneyFile::instance()->detach(MyMoneyFile::NotifyClassPayee, this);

  writeConfig();
}

void KPayeesView::update(const QCString & /* id */)
{
  if(m_suspendUpdate == false)
    slotRefreshView();
}

void KPayeesView::suspendUpdate(const bool suspend)
{
  // force a refresh, if update was off
  if(m_suspendUpdate == true
  && suspend == false)
    slotRefreshView();

  m_suspendUpdate = suspend;
}

void KPayeesView::payeeHighlighted(const QString& text)
{
  try {
    MyMoneyFile* file = MyMoneyFile::instance();

    m_payee = file->payeeByName(text);
    m_lastPayee = m_payee.name();

    nameLabel->setText(m_payee.name());

    addressEdit->setEnabled(true);
    addressEdit->setText(m_payee.address());
    postcodeEdit->setEnabled(true);
    postcodeEdit->setText(m_payee.postcode());
    telephoneEdit->setEnabled(true);
    telephoneEdit->setText(m_payee.telephone());
    emailEdit->setEnabled(true);
    emailEdit->setText(m_payee.email());
    updateButton->setEnabled(true);
    deleteButton->setEnabled(true);

    showTransactions();
    writeConfig();
  } catch(MyMoneyException *e) {
    m_transactionView->clear();
    m_payee = MyMoneyPayee();
    updateButton->setEnabled(false);
    deleteButton->setEnabled(false);
    delete e;
  }
}

void KPayeesView::showTransactions(void)
{
  MyMoneyFile* file = MyMoneyFile::instance();
  unsigned int   i;

  KConfig *config = KGlobal::config();
  config->setGroup("List Options");
  QDateTime defaultDate;
  QDate dateStart = config->readDateTimeEntry("StartDate", &defaultDate).date();

  // setup the list and the pointer vector
  MyMoneyTransactionFilter filter;
  filter.addPayee(m_payee.id());
  filter.setDateFilter(dateStart, QDate());

  QValueList<MyMoneyTransaction> list = file->transactionList(filter);
  m_transactionList.clear();
  
  m_transactionPtrVector.clear();
  m_transactionPtrVector.resize(list.size());
  m_transactionPtrVector.setPayeeId(m_payee.id());
  m_transactionPtrVector.setSortType(KTransactionPtrVector::SortPostDate);

  MyMoneyMoney balance(0);

  QValueList<MyMoneyTransaction>::ConstIterator it_t;
  QCString lastId;
  int ofs = 0;

  for(i = 0, it_t = list.begin(); it_t != list.end(); ++it_t) {
    KMyMoneyTransaction k(*it_t);
    
    filter.match(*it_t, MyMoneyFile::instance()->storage());
    if(lastId != (*it_t).id()) {
      ofs = 0;
      lastId = (*it_t).id();
    } else
      ofs++;

    k.setSplitId(filter.matchingSplits()[ofs].id());
    MyMoneyAccount acc = MyMoneyFile::instance()->account(filter.matchingSplits()[ofs].accountId());
    if(acc.accountGroup() == MyMoneyAccount::Asset
    || acc.accountGroup() == MyMoneyAccount::Liability) {
      QValueList<KMyMoneyTransaction>::ConstIterator it_k;
      it_k = m_transactionList.append(k);
      balance += k.splitById(k.splitId()).value();
      m_transactionPtrVector.insert(i, &(*it_k));
      ++i;
    }
  }

  // sort the transactions
  m_transactionPtrVector.sort();

  // and fill the m_transactionView
  m_transactionView->clear();
  KTransactionListItem *item = 0;
  for(i = 0; i < m_transactionPtrVector.size(); ++i) {
    KMyMoneyTransaction* t = m_transactionPtrVector[i];
    MyMoneySplit s = t->splitById(t->splitId());
    
    item = new KTransactionListItem(m_transactionView, item, s.accountId(), t->id());
    item->setText(0, s.number());
    item->setText(1, KGlobal::locale()->formatDate(t->postDate(), true));
    
    QString txt;
    if(s.action() == MyMoneySplit::ActionTransfer) {
      MyMoneyAccount acc = file->account(s.accountId());
      if(s.value() >= 0) {
        txt = i18n("Transfer to %1").arg(acc.name());
      } else {
        txt = i18n("Transfer from %1").arg(acc.name());
      }
    } else if(t->splitCount() > 2) {
      txt = i18n("Splitted transaction");
    } else {
      MyMoneySplit s0 = t->splitByAccount(s.accountId(), false);
      txt = MyMoneyFile::instance()->accountToCategory(s0.accountId());
    }
    item->setText(2, txt);
    item->setText(3, s.value().formatMoney());
/*
    QValueList<MyMoneySplit> list = t->splits();
    QValueList<MyMoneySplit>::Iterator it_s;
    for(it_s = list.begin(); it_s != list.end(); ++it_s) {
      if((*it_s).payeeId() == m_payee.id()) {
        item = new KTransactionListItem(m_transactionView, item, (*it_s).accountId(), t->id());
        item->setText(0, (*it_s).number());
        item->setText(1, KGlobal::locale()->formatDate(t->postDate(), true));

        QString txt;
        if((*it_s).action() == MyMoneySplit::ActionTransfer) {
          MyMoneyAccount acc = file->account((*it_s).accountId());
          if((*it_s).value() >= 0) {
            txt = i18n("Transfer to %1").arg(acc.name());
          } else {
            txt = i18n("Transfer from %1").arg(acc.name());
          }
        } else if(t->splitCount() > 2) {
          txt = i18n("Splitted transaction");
        } else {
          MyMoneySplit s = t->splitByAccount((*it_s).accountId(), false);
          txt = MyMoneyFile::instance()->accountToCategory(s.accountId());
        }
        item->setText(2, txt);
        item->setText(3, (*it_s).value().formatMoney());
      }
    }
*/
  }
  m_balanceLabel->setText(i18n("Balance: %1").arg(balance.formatMoney()));

  // Trick: it seems, that the initial sizing of the view does
  // not work correctly. At least, the columns do not get displayed
  // correct. Reason: the return value of m_transactionView->visibleWidth()
  // is incorrect. If the widget is visible, resizing works correctly.
  // So, we let the dialog show up and resize it then. It's not really
  // clean, but the only way I got the damned thing working.
  QTimer::singleShot(50, this, SLOT(rearrange()));
}

void KPayeesView::slotAddClicked()
{
  try {
    MyMoneyPayee p;

    p = MyMoneyFile::instance()->payeeByName(payeeEdit->text());
    KMessageBox::detailedSorry(0, i18n("Unable to add payee with same name"),
      i18n("Payee already exists"));

  } catch (MyMoneyException *e) {
    m_payee = MyMoneyPayee();

    try {
      m_payee.setName(payeeEdit->text());
      MyMoneyFile::instance()->addPayee(m_payee);

      payeeEdit->setText("");
      m_lastPayee = m_payee.name();

    } catch(MyMoneyException *e) {
      KMessageBox::detailedSorry(0, i18n("Unable to add payee"),
        (e->what() + " " + i18n("thrown in") + " " + e->file()+ ":%1").arg(e->line()));
      delete e;
    }
  }
}

void KPayeesView::slotPayeeTextChanged(const QString& text)
{
  if (text.isEmpty())
    addButton->setEnabled(false);
  else
    addButton->setEnabled(true);
}

void KPayeesView::slotUpdateClicked()
{
  try {
    m_payee.setName(nameLabel->text());
    m_payee.setAddress(addressEdit->text());
    m_payee.setPostcode(postcodeEdit->text());
    m_payee.setTelephone(telephoneEdit->text());
    m_payee.setEmail(emailEdit->text());

    MyMoneyFile::instance()->modifyPayee(m_payee);

  } catch(MyMoneyException *e) {
    KMessageBox::detailedSorry(0, i18n("Unable to modify payee"),
      (e->what() + " " + i18n("thrown in") + " " + e->file()+ ":%1").arg(e->line()));
    delete e;
  }
}

void KPayeesView::slotDeleteClicked()
{
  QString prompt(i18n("Remove this payee: "));
  prompt += nameLabel->text();

  if (KMessageBox::questionYesNo(this, prompt, i18n("Remove Payee"))==KMessageBox::No)
    return;

  try {
    MyMoneyPayee payee = m_payee;
    m_payee = MyMoneyPayee();
    m_lastPayee = "";
    MyMoneyFile::instance()->removePayee(payee);

  } catch(MyMoneyException *e) {
    KMessageBox::detailedSorry(0, i18n("Unable to remove payee"),
      (e->what() + " " + i18n("thrown in") + " " + e->file()+ ":%1").arg(e->line()));
    delete e;
  }
}

void KPayeesView::readConfig(void)
{
  KConfig *config = KGlobal::config();
  QFont font = QFont("helvetica", 10);
  config->setGroup("List Options");
  font = config->readFontEntry("listCellFont", &font);
  m_transactionView->setFont(font);

  font = config->readFontEntry("listHeaderFont", &font);
  QFontMetrics fm( font );
  int height = fm.lineSpacing()+6;

  m_transactionView->header()->setMinimumHeight(height);
  m_transactionView->header()->setMaximumHeight(height);
  m_transactionView->header()->setFont(font);
}

void KPayeesView::writeConfig(void)
{
  KConfig *config = KGlobal::config();
  config->setGroup("Last Use Settings");
  config->writeEntry("KPayeesView_LastPayee", payeeCombo->currentText());
  config->sync();
}

void KPayeesView::show()
{
  slotRefreshView();
  emit signalViewActivated();
  QWidget::show();
}

void KPayeesView::slotReloadView(void)
{
  KConfig *config = KGlobal::config();
  config->setGroup("Last Use Settings");
  m_lastPayee = config->readEntry("KPayeesView_LastPayee");

  slotRefreshView();
}

void KPayeesView::slotRefreshView(void)
{
  bool found = false;

  readConfig();
  
  payeeCombo->clear();

  QValueList<MyMoneyPayee>list = MyMoneyFile::instance()->payeeList();
  QValueList<MyMoneyPayee>::ConstIterator it;
  QStringList payees;

  for (it = list.begin(); it != list.end(); ++it) {
    payees += (*it).name();
    if(m_lastPayee.length() == 0)
      m_lastPayee = (*it).name();
    if((*it).name() == m_lastPayee) {
      m_payee = *it;
      found = true;
    }
  }
  payees.sort();

  payeeCombo->insertStringList(payees);

  if(found == true) {
    payeeCombo->setCurrentText(m_lastPayee);
    payeeHighlighted(payeeCombo->currentText());
  } else
    payeeHighlighted("");
}

void KPayeesView::rearrange(void)
{
  resizeEvent(0);
}

void KPayeesView::resizeEvent(QResizeEvent* ev)
{
  // resize the register
  int w = m_transactionView->visibleWidth();
  w -= m_transactionView->columnWidth(0);
  w -= m_transactionView->columnWidth(1);
  w -= m_transactionView->columnWidth(3);
  m_transactionView->setColumnWidth(2, w);

  KPayeesViewDecl::resizeEvent(ev);
}

void KPayeesView::slotTransactionDoubleClicked(QListViewItem* i)
{
  KTransactionListItem* item = static_cast<KTransactionListItem *>(i);
  emit transactionSelected(item->accountId(), item->transactionId());
}

void KPayeesView::slotSelectPayeeAndTransaction(const QCString& payeeId, const QCString& accountId, const QCString& transactionId)
{
  MyMoneyFile* file = MyMoneyFile::instance();
  MyMoneyPayee payee;

  try {
    payee = file->payee(payeeId);
    m_lastPayee = payee.name();
    slotRefreshView();

    KTransactionListItem* item = static_cast<KTransactionListItem*> (m_transactionView->firstChild());
    while(item != 0) {
      if(item->accountId() == accountId && item->transactionId() == transactionId)
        break;
      item = static_cast<KTransactionListItem*> (item->nextSibling());
    }
    if(!item) {
      item = static_cast<KTransactionListItem*> (m_transactionView->firstChild());
    }
    if(item) {
      m_transactionView->setSelected(item, true);
      m_transactionView->ensureItemVisible(item);
    }
  } catch(MyMoneyException *e) {
    qWarning("Unexpected exception in KPayeesView::slotSelectPayeeAndTransaction");
    delete e;
  }
}

