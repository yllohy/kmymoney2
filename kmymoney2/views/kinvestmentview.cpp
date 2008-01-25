/***************************************************************************
                          kinvestmentview.cpp  -  description
                             -------------------
    begin                : Mon Mar 12 2007
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

// ----------------------------------------------------------------------------
// KDE Includes

#include "kdecompat.h"
#include <klocale.h>

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoney/mymoneyfile.h>
#include <kmymoney/mymoneyutils.h>
#include <kmymoney/mymoneysecurity.h>
#include <kmymoney/mymoneytransaction.h>
#include <kmymoney/mymoneyinvesttransaction.h>
#include <kmymoney/mymoneyaccount.h>
#include <kmymoney/kmymoneyglobalsettings.h>
#include <kmymoney/kmymoneyaccountcombo.h>
#include <kmymoney/kmymoneycurrencyselector.h>

#include "../kmymoney2.h"

#include "kinvestmentview.h"
#include "kinvestmentlistitem.h"
// #include "kledgerviewinvestments.h"

class KInvestmentViewPrivate
{
public:
  KInvestmentViewPrivate() :
    m_needReload(false),
    m_newAccountLoaded(false),
    m_recursion(false),
    m_precision(2) {}

  MyMoneyAccount    m_account;
  bool              m_needReload;
  bool              m_newAccountLoaded;
  bool              m_recursion;
  int               m_precision;
};



KInvestmentView::KInvestmentView(QWidget *parent, const char *name) :
  KInvestmentViewDecl(parent,name),
  d(new KInvestmentViewPrivate)
{
  m_table->setRootIsDecorated(false);
  // m_table->setColumnText(0, i18n("Symbol"));
  m_table->addColumn(i18n("Name"));
  m_table->addColumn(i18n("Symbol"));

  int col = m_table->addColumn(i18n("Value"));
  m_table->setColumnAlignment(col, Qt::AlignRight);

  col = m_table->addColumn(i18n("Quantity"));
  m_table->setColumnAlignment(col, Qt::AlignRight);

  col = m_table->addColumn(i18n("Price"));
  m_table->setColumnAlignment(col, Qt::AlignRight);

  m_table->setMultiSelection(false);
  m_table->setColumnWidthMode(0, QListView::Maximum);
  m_table->header()->setResizeEnabled(true);
  m_table->setAllColumnsShowFocus(true);
  m_table->setShowSortIndicator(true);
  m_table->restoreLayout(KGlobal::config(), "Investment Settings");

  connect(m_table, SIGNAL(contextMenu(KListView*, QListViewItem* , const QPoint&)),
    this, SLOT(slotListContextMenu(KListView*, QListViewItem*, const QPoint&)));
  connect(m_table, SIGNAL(selectionChanged(QListViewItem *)), this, SLOT(slotSelectionChanged(QListViewItem *)));

  connect(m_accountComboBox, SIGNAL(accountSelected(const QCString&)),
    this, SLOT(slotSelectAccount(const QCString&)));

  connect(m_table, SIGNAL(doubleClicked(QListViewItem*,const QPoint&, int)), kmymoney2->action("investment_edit"), SLOT(activate()));

  connect(MyMoneyFile::instance(), SIGNAL(dataChanged()), this, SLOT(slotLoadView()));
}

KInvestmentView::~KInvestmentView()
{
  m_table->saveLayout(KGlobal::config(), "Investment Settings");
  delete d;
}

void KInvestmentView::slotSelectionChanged(QListViewItem *item)
{
  kmymoney2->slotSelectInvestment();

  KInvestmentListItem *pItem = dynamic_cast<KInvestmentListItem*>(item);
  if(pItem) {
    try {
      MyMoneyAccount account = MyMoneyFile::instance()->account(pItem->account().id());
      kmymoney2->slotSelectInvestment(account);

    } catch (MyMoneyException *e) {
      delete e;
    }
  }
}

void KInvestmentView::slotListContextMenu(KListView* /* lv */, QListViewItem* /*item*/, const QPoint& /*point*/)
{
  kmymoney2->slotSelectInvestment();
  KInvestmentListItem *pItem = dynamic_cast<KInvestmentListItem*>(m_table->selectedItem());
  if(pItem) {
    kmymoney2->slotSelectInvestment(MyMoneyFile::instance()->account(pItem->account().id()));
  }
  emit investmentRightMouseClick();
}

void KInvestmentView::slotLoadView(void)
{
  d->m_needReload = true;
  if(isVisible()) {
    loadView();
    d->m_needReload = false;
    // force a new account if the current one is empty
    d->m_newAccountLoaded = d->m_account.id().isEmpty();
  }
}

void KInvestmentView::loadAccounts(void)
{
  MyMoneyFile* file = MyMoneyFile::instance();

  // check if the current account still exists and make it the
  // current account
  if(!d->m_account.id().isEmpty()) {
    try {
      d->m_account = file->account(d->m_account.id());
    } catch(MyMoneyException *e) {
      delete e;
      d->m_account = MyMoneyAccount();
    }
  }

  m_accountComboBox->loadList((KMyMoneyUtils::categoryTypeE)(KMyMoneyUtils::asset | KMyMoneyUtils::liability));

  if(d->m_account.id().isEmpty()) {
    QCStringList list = m_accountComboBox->accountList();
    if(list.count()) {
      QCStringList::Iterator it;
      for(it = list.begin(); it != list.end(); ++it) {
        MyMoneyAccount a = file->account(*it);
        if(a.accountType() == MyMoneyAccount::Investment) {
          if(a.value("PreferredAccount") == "Yes") {
            d->m_account = a;
            break;
          } else if(d->m_account.id().isEmpty()) {
            d->m_account = a;
          }
        }
      }
    }
  }

  if(!d->m_account.id().isEmpty()) {
    m_accountComboBox->setSelected(d->m_account);
    try {
      MyMoneySecurity sec = MyMoneyFile::instance()->security(d->m_account.currencyId());
      d->m_precision = MyMoneyMoney::denomToPrec(d->m_account.fraction(sec));
    } catch(MyMoneyException *e) {
      qDebug("Security %s for account %s not found", d->m_account.currencyId().data(), d->m_account.name().data());
      delete e;
      d->m_precision = 2;
    }
  }
}


bool KInvestmentView::slotSelectAccount(const MyMoneyObject& obj)
{
  if(typeid(obj) != typeid(MyMoneyAccount))
    return false;

  if(d->m_recursion)
    return false;

  d->m_recursion = true;
  const MyMoneyAccount& acc = dynamic_cast<const MyMoneyAccount&>(obj);
  bool rc = slotSelectAccount(acc.id());
  d->m_recursion = false;
  return rc;
}

bool KInvestmentView::slotSelectAccount(const QCString& id, const QCString& transactionId, const bool /* reconciliation*/)
{
  bool    rc = true;

  if(!id.isEmpty()) {
    // if the account id differs, then we have to do something
    if(d->m_account.id() != id) {
      try {
        d->m_account = MyMoneyFile::instance()->account(id);
        // if a stock account is selected, we show the
        // the corresponding parent (investment) account
        if(d->m_account.isInvest()) {
          d->m_account = MyMoneyFile::instance()->account(d->m_account.parentAccountId());
        }
        // TODO if we don't have an investment account, then we should switch to the ledger view
        d->m_newAccountLoaded = true;
        if(d->m_account.accountType() == MyMoneyAccount::Investment) {
          slotLoadView();
        } else {
          emit accountSelected(id, transactionId);
          d->m_account = MyMoneyAccount();
          d->m_needReload = true;
          rc = false;
        }

      } catch(MyMoneyException* e) {
        qDebug("Unable to retrieve account %s", id.data());
        delete e;
        rc = false;
      }
    } else {
      emit accountSelected(d->m_account);
    }
  }

  return rc;
}

void KInvestmentView::clear(void)
{
  // setup header font
  QFont font = KMyMoneyGlobalSettings::listHeaderFont();
  QFontMetrics fm( font );
  int height = fm.lineSpacing()+6;
  m_table->header()->setMinimumHeight(height);
  m_table->header()->setMaximumHeight(height);
  m_table->header()->setFont(font);

  // setup cell font
  font = KMyMoneyGlobalSettings::listCellFont();
  m_table->setFont(font);

  // clear the table
  m_table->clear();

  // and the selected account in the combo box
  m_accountComboBox->setSelected(QCString());
}

void KInvestmentView::loadView(void)
{
  // no account selected
  emit accountSelected(MyMoneyAccount());

  // clear the current contents ...
  clear();

  // ... load the combobox widget and select current account ...
  loadAccounts();

  if(d->m_account.id().isEmpty()) {
    // if we don't have an account we bail out
    setEnabled(false);
    return;
  }
  setEnabled(true);

  MyMoneyFile* file = MyMoneyFile::instance();
  bool showClosedAccounts = kmymoney2->toggleAction("view_show_all_accounts")->isChecked()
                         || !KMyMoneyGlobalSettings::hideClosedAccounts();
  try {
    d->m_account = file->account(d->m_account.id());
    QCStringList securities = d->m_account.accountList();

    for(QCStringList::ConstIterator it = securities.begin(); it != securities.end(); ++it) {
      MyMoneyAccount acc = file->account(*it);
      if(!acc.isClosed() || showClosedAccounts)
        new KInvestmentListItem(m_table, acc);
    }
  } catch(MyMoneyException* e) {
    qDebug("KInvestmentView::loadView() - selected account does not exist anymore");
    d->m_account = MyMoneyAccount();
    delete e;
  }

  // and tell everyone what's selected
  emit accountSelected(d->m_account);
}

void KInvestmentView::show(void)
{
  if(d->m_needReload) {
    loadView();
    d->m_needReload = false;
    d->m_newAccountLoaded = false;

  } else {
    emit accountSelected(d->m_account);
  }

  // don't forget base class implementation
  KInvestmentViewDecl::show();
}

#include "kinvestmentview.moc"
