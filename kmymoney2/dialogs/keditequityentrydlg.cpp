/***************************************************************************
                          keditequityentrydlg.cpp  -  description
                             -------------------
    begin                : Sat Mar 6 2004
    copyright            : (C) 2000-2004 by Michael Edwardes
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

#include <kglobal.h>
#include <klocale.h>
#include <kconfig.h>
#include <knuminput.h>
#include <klistview.h>
#include <kpushbutton.h>
#include <klineedit.h>
#include <kcombobox.h>
#include <kiconloader.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "keditequityentrydlg.h"
#include "kupdatestockpricedlg.h"

KEditEquityEntryDlg::KEditEquityEntryDlg(const MyMoneyEquity& selectedEquity, QWidget *parent, const char *name)
  : kEditEquityEntryDecl(parent, name, true)
{
  m_selectedEquity = selectedEquity;
  lvPriceHistory->setSelectionMode(QListView::Single);
  lvPriceHistory->addColumn(QString("Date"));
  lvPriceHistory->addColumn(QString("Price"));
  lvPriceHistory->setResizeMode(QListView::AllColumns);
  lvPriceHistory->setSorting(0, FALSE);
  lvPriceHistory->setShowSortIndicator(true);

  connect(btnOK, SIGNAL(clicked()), this, SLOT(slotOKClicked()));
  connect(btnCancel, SIGNAL(clicked()), this, SLOT(slotCancelClicked()));
  connect(lvPriceHistory, SIGNAL(doubleClicked(QListViewItem*, const QPoint&, int)), this, SLOT(slotPriceHistoryDoubleClicked(QListViewItem *, const QPoint&, int)));
  connect(edtEquityName, SIGNAL(textChanged(const QString &)), this, SLOT(slotDataChanged()));
  connect(edtMarketSymbol, SIGNAL(textChanged(const QString &)), this, SLOT(slotDataChanged()));
  connect(edtFraction, SIGNAL(textChanged(const QString&)), this, SLOT(slotDataChanged()));
  connect(btnAddEntry, SIGNAL(clicked()), this, SLOT(slotAddPriceClicked()));
  connect(btnEditEntry, SIGNAL(clicked()), this, SLOT(slotEditPriceClicked()));
  connect(btnRemoveEntry, SIGNAL(clicked()), this, SLOT(slotRemovePriceClicked()));
  connect(lvPriceHistory, SIGNAL(clicked(QListViewItem *, const QPoint&, int)), this, SLOT(slotPriceHistoryClicked(QListViewItem*, const QPoint&, int)));

  //fill in the fields for what we know.
  edtEquityName->setText(m_selectedEquity.name());
  edtMarketSymbol->setText(m_selectedEquity.tradingSymbol());
  edtFraction->setPrecision(0);
  edtFraction->hideCalculatorButton();
  edtFraction->loadText(QString::number(m_selectedEquity.smallestAccountFraction()));
  cmbInvestmentType->setCurrentItem((int)m_selectedEquity.equityType());
  equity_price_history priceHistory = m_selectedEquity.priceHistory();
  if(priceHistory.size())
  {
    for(equity_price_history::ConstIterator it = priceHistory.begin(); it != priceHistory.end(); ++it)
    {
      QListViewItem *item = new QListViewItem(lvPriceHistory, it.key().toString(), it.data().formatMoney());
      lvPriceHistory->insertItem(item);
    }
  }


  //disable controls that can't be used until the user selects a price history item.
  btnEditEntry->setEnabled(false);
  btnRemoveEntry->setEnabled(false);

  // add icons to buttons
  KIconLoader *il = KGlobal::iconLoader();
  KGuiItem okButtenItem( i18n("&Ok" ),
                    QIconSet(il->loadIcon("button_ok", KIcon::Small, KIcon::SizeSmall)),
                    i18n("Accepts the value and stores them"),
                    i18n("Use this to accept all values and close the dialog."));
  btnOK->setGuiItem(okButtenItem);

  KGuiItem cancelButtenItem( i18n( "&Cancel" ),
                    QIconSet(il->loadIcon("button_cancel", KIcon::Small, KIcon::SizeSmall)),
                    i18n("Cancel the operation"),
                    i18n("Use this to dismiss all the changes made in this dialog."));
  btnCancel->setGuiItem(cancelButtenItem);

  m_changes = false;
}

KEditEquityEntryDlg::~KEditEquityEntryDlg()
{

}

/** No descriptions */
void KEditEquityEntryDlg::slotOKClicked()
{
  if(m_changes)
  {
    m_selectedEquity.setName(edtEquityName->text());
    m_selectedEquity.setTradingSymbol(edtMarketSymbol->text());
    m_selectedEquity.setSmallestAccountFraction(edtFraction->getMoneyValue().abs());
    //m_selectedEquity.setEquityType((int)cmbInvestmentType->currentItem());

    m_selectedEquity.setPriceHistory(equity_price_history());
    QListViewItemIterator it(lvPriceHistory);
    while(it.current())
    {
      QDate date = QDate::fromString(it.current()->text(0));
      MyMoneyMoney money(it.current()->text(1));
      m_selectedEquity.addPriceHistory(date, money);
      ++it;
    }
  }

  accept();
}

void KEditEquityEntryDlg::slotCancelClicked()
{
  reject();
}

void KEditEquityEntryDlg::slotPriceHistoryDoubleClicked(QListViewItem *item, const QPoint &point, int c)
{

}

void KEditEquityEntryDlg::slotPriceHistoryClicked(QListViewItem* item, const QPoint& point, int c)
{

  btnEditEntry->setEnabled(true);
  btnRemoveEntry->setEnabled(true);
}

void KEditEquityEntryDlg::slotDataChanged(void)
{
  m_changes = true;
}

void KEditEquityEntryDlg::slotAddPriceClicked()
{
  KUpdateStockPriceDlg *pDlg = new KUpdateStockPriceDlg(this);
  if(pDlg->exec() == QDialog::Accepted)
  {
    KListViewItem *pItem = new KListViewItem(lvPriceHistory, pDlg->getDate().toString(), pDlg->getPrice().formatMoney());
    lvPriceHistory->insertItem(pItem);
    lvPriceHistory->sort();

    m_changes = true;
  }
}

void KEditEquityEntryDlg::slotEditPriceClicked()
{
  QListViewItem *pItem = lvPriceHistory->selectedItem();
  if(pItem)
  {
    QString date = pItem->text(0);
    QString price = pItem->text(1);
    KUpdateStockPriceDlg *pDlg = new KUpdateStockPriceDlg(QDate::fromString(date), price, this);
    if(pDlg->exec() == QDialog::Accepted)
    {
      pItem->setText(0, pDlg->getDate().toString());
      pItem->setText(1, pDlg->getPrice().formatMoney());
      lvPriceHistory->sort();

      m_changes = true;
    }
  }
}

void KEditEquityEntryDlg::slotRemovePriceClicked()
{

  QListViewItem *pItem = lvPriceHistory->selectedItem();
  if(pItem)
  {
    lvPriceHistory->takeItem(pItem);
    lvPriceHistory->sort();
    m_changes = true;
  }
}
