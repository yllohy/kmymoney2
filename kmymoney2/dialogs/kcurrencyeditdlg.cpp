/***************************************************************************
                          kcurrencyeditdlg.cpp  -  description
                             -------------------
    begin                : Wed Mar 24 2004
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

#include <qheader.h>
#include <qtimer.h>
#include <qpixmap.h>
#include <qbitmap.h>
#include <qlabel.h>
#include <qgroupbox.h>
#include <qcursor.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <klocale.h>
#include <kpushbutton.h>
#include <klistview.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>
#include <kiconloader.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kcurrencyeditdlg.h"

#include "../mymoney/mymoneysecurity.h"
// #include "../mymoney/mymoneycurrency.h"
#include "../mymoney/mymoneyfile.h"

#include "../widgets/kmymoneyaccountselector.h"
#include "../widgets/kmymoneylineedit.h"
#include "../widgets/kmymoneypriceview.h"

KCurrencyEditDlg::KCurrencyEditDlg(QWidget *parent, const char *name ) :
  KCurrencyEditDlgDecl(parent,name)
{
  m_currencyList->addColumn(i18n("Currency"));
  m_currencyList->header()->hide();

  KIconLoader *kiconloader = KGlobal::iconLoader();

  m_contextMenu = new KPopupMenu(this);
  m_contextMenu->insertTitle(i18n("Currency Options"));
  m_contextMenu->insertItem(kiconloader->loadIcon("filenew", KIcon::Small),
                        i18n("New"),
                        this, SLOT(slotNewCurrency()));

  m_contextMenu->insertItem(kiconloader->loadIcon("edit", KIcon::Small),
                        i18n("Rename ..."),
                        this, SLOT(slotRenameCurrency()));

  m_contextMenu->insertItem(kiconloader->loadIcon("delete", KIcon::Small),
                        i18n("Delete ..."),
                        this, SLOT(slotDeleteCurrency()));

  connect(m_currencyList, SIGNAL(rightButtonPressed(QListViewItem* , const QPoint&, int)),
          this, SLOT(slotListClicked(QListViewItem*, const QPoint&, int)));
  connect(m_currencyList, SIGNAL(itemRenamed(QListViewItem*,int,const QString&)), this, SLOT(slotRenameCurrency(QListViewItem*,int,const QString&)));

  loadCurrencies();

  checkBaseCurrency();

  connect(m_currencyList, SIGNAL(selectionChanged(QListViewItem*)), this, SLOT(slotSelectCurrency(QListViewItem*)));
  connect(m_baseCurrencyButton, SIGNAL(clicked()), this, SLOT(slotSetBaseCurrency()));
  connect(buttonClose, SIGNAL(clicked()), this, SLOT(slotClose()));

  // FIXME: currently, no online help available
  buttonHelp->hide();

  resize(width()-1, height()-1);
  QTimer::singleShot(10, this, SLOT(timerDone()));
}

void KCurrencyEditDlg::timerDone(void)
{
  if(!m_currency.id().isEmpty()) {
    QListViewItem* it;
    for(it = m_currencyList->firstChild(); it; it = it->nextSibling()) {
      kMyMoneyListViewItem* p = static_cast<kMyMoneyListViewItem *>(it);
      if(p->id() == m_currency.id()) {
        m_currencyList->ensureItemVisible(it);
        break;
      }
    }
  }
  // the resize operation does the trick to adjust
  // all widgets in the view to the size they should
  // have and show up correctly. Don't ask me, why
  // this is, but it cured the problem (ipwizard).
  resize(width()+1, height()+1);
}

KCurrencyEditDlg::~KCurrencyEditDlg()
{
}

void KCurrencyEditDlg::resizeEvent(QResizeEvent* /* e*/)
{
  int w = m_currencyList->visibleWidth();

  m_currencyList->setColumnWidth(0, w);
}

void KCurrencyEditDlg::loadCurrencies(void)
{
  QValueList<MyMoneySecurity> list = MyMoneyFile::instance()->currencyList();
  QValueList<MyMoneySecurity>::ConstIterator it;
  QListViewItem *first = 0;

  QCString baseCurrency = MyMoneyFile::instance()->baseCurrency().id();
  // construct a transparent 16x16 pixmap
  QPixmap empty(16, 16);
  empty.setMask(QBitmap(16, 16, true));

  m_currencyList->clear();
  for(it = list.begin(); it != list.end(); ++it) {
    kMyMoneyListViewItem* p = new kMyMoneyListViewItem(m_currencyList, (*it).name(), (*it).id());
    p->setRenameEnabled(0, true);

    if((*it).id() == baseCurrency) {
      p->setPixmap(0, QPixmap( locate("icon","hicolor/16x16/apps/kmymoney2.png")));
    } else {
      p->setPixmap(0, empty);
// FIXME PRICE
#if 0
      if((*it).priceHistory().count() != 0 && first == 0)
        first = p;
#endif
    }
  }
  if(first == 0)
    first = m_currencyList->firstChild();
  if(first != 0)
    m_currencyList->setCurrentItem(first);

  slotSelectCurrency(first);
}

void KCurrencyEditDlg::checkBaseCurrency(void)
{
  if(MyMoneyFile::instance()->baseCurrency().id().isEmpty()) {
    m_baseCurrencyButton->setEnabled(true);
    buttonClose->setEnabled(false);
    m_detailGroup->setEnabled(true);
    m_priceList->setEnabled(false);
  } else {
    buttonClose->setEnabled(true);
    m_baseCurrencyFrame->hide();
    m_priceList->setEnabled(true);
  }
}

void KCurrencyEditDlg::updateCurrency(void)
{
// FIXME PRICE
#if 0
  if(!m_currency.id().isEmpty()) {
    if(m_priceList->dirty()
    || (m_symbolEdit->text() != m_currency.tradingSymbol())) {
      m_currency.setPriceHistory(m_priceList->history());
      MyMoneyFile::instance()->modifyCurrency(m_currency);
    }
  }
#endif
}

void KCurrencyEditDlg::slotSelectCurrency(const QCString& id)
{
  QListViewItemIterator it(m_currencyList);

  while(it.current()) {
    kMyMoneyListViewItem* p = static_cast<kMyMoneyListViewItem*>(it.current());
    if(p->id() == id) {
      m_currencyList->setSelected(p, true);
      break;
    }
    ++it;
  }
}

void KCurrencyEditDlg::slotSelectCurrency(QListViewItem *item)
{
  QMap<QDate, MyMoneyMoney> history;
  MyMoneyFile* file = MyMoneyFile::instance();

  m_detailGroup->setEnabled(item != 0);
// FIXME PRICE
#if 0
  if(item) {
    try {
      updateCurrency();
      kMyMoneyListViewItem* p = static_cast<kMyMoneyListViewItem *>(item);
      m_currency = file->currency(p->id());
      m_idLabel->setText(m_currency.id());
      m_symbolEdit->setText(m_currency.tradingSymbol());
      m_priceList->setHistory(m_currency.priceHistory());
      if(!file->baseCurrency().id().isEmpty() && file->baseCurrency().id() != p->id()) {
        m_priceList->setEnabled(true);
        m_description->setText(i18n("1 %2 costs <i>price</i<> %1").arg(file->baseCurrency().name()).arg(m_currency.name()));
      } else {
        m_priceList->setEnabled(false);
        m_description->setText("");
      }
    } catch(MyMoneyException *e) {
      delete e;
      m_priceList->setHistory(history);
      m_idLabel->setText(QString());
      m_symbolEdit->setText(QString());
    }
  }
#endif
}

void KCurrencyEditDlg::slotSetBaseCurrency(void)
{
  MyMoneyFile* file = MyMoneyFile::instance();

  kMyMoneyListViewItem* p = static_cast<kMyMoneyListViewItem *>(m_currencyList->currentItem());
  if(p) {
    QString name = file->currency(p->id()).name();
    QString question = i18n("Do you really want to select %1 as your base currency? This selection can currently not be modified! If unsure, press 'No' now.").arg(name);
    if(KMessageBox::questionYesNo(this, question, i18n("Select base currency")) == KMessageBox::Yes) {
      file->setBaseCurrency(file->currency(p->id()));
      accept();
    }
  }
}

void KCurrencyEditDlg::slotClose(void)
{
  updateCurrency();
  accept();
}

void KCurrencyEditDlg::slotNewCurrency(void)
{
  KMessageBox::sorry(this, i18n("This feature needs to be implemented."), i18n("Implementation missing"));
}

void KCurrencyEditDlg::slotRenameCurrency(void)
{
  QListViewItem *item = m_currencyList->currentItem();
  if(item) {
    item->startRename(0);
  }
}

void KCurrencyEditDlg::slotDeleteCurrency(void)
{
  KMessageBox::sorry(this, i18n("This feature needs to be implemented."), i18n("Implementation missing"));
}

void KCurrencyEditDlg::slotListClicked(QListViewItem* item, const QPoint&, int)
{
  int editId = m_contextMenu->idAt(2);
  int delId = m_contextMenu->idAt(3);

  m_contextMenu->setItemEnabled(editId, item != 0);
  m_contextMenu->setItemEnabled(delId, item != 0);
  m_contextMenu->exec(QCursor::pos());
}

void KCurrencyEditDlg::slotRenameCurrency(QListViewItem* item, int /* col */, const QString& txt)
{
  MyMoneyFile* file = MyMoneyFile::instance();
  kMyMoneyListViewItem* p = static_cast<kMyMoneyListViewItem *>(item);

  try {
    MyMoneySecurity currency = file->currency(p->id());
    currency.setName(txt);
    file->modifyCurrency(currency);
    m_currency = currency;
  } catch(MyMoneyException *e) {
    delete e;
    updateCurrency();
  }
}
