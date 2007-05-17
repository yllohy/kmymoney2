/***************************************************************************
                          kselecttransactionsdlg.cpp
                             -------------------
    begin                : Wed May 16 2007
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

// ----------------------------------------------------------------------------
// KDE Includes

#include <kpushbutton.h>
#include <kstdguiitem.h>
#include <kapplication.h>
#include <kactivelabel.h>
#include <klocale.h>

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoney/mymoneyfile.h>
#include <kmymoney/mymoneytransaction.h>
#include <kmymoney/kmymoneyglobalsettings.h>

#include "kmergetransactionsdlg.h"

KSelectTransactionsDlg::KSelectTransactionsDlg(const MyMoneyAccount& _account, QWidget* parent, const char* name) :
  KSelectTransactionsDlgDecl(parent, name),
  m_account(_account)
{
  // setup descriptive texts
  setCaption(i18n("Select Transaction"));
  m_description->setText(i18n("Select a transaction and press the OK button or use Cancel to select none."));

  // clear current register contents
  m_register->clear();

  // no selection possible
  m_register->setSelectionMode(QTable::Single);

  // setup header font
  QFont font = KMyMoneyGlobalSettings::listHeaderFont();
  QFontMetrics fm( font );
  int height = fm.lineSpacing()+6;
  m_register->horizontalHeader()->setMinimumHeight(height);
  m_register->horizontalHeader()->setMaximumHeight(height);
  m_register->horizontalHeader()->setFont(font);

  // setup cell font
  font = KMyMoneyGlobalSettings::listCellFont();
  m_register->setFont(font);

  // ... setup the register columns ...
  m_register->setupRegister(m_account);

  // setup buttons
  m_helpButton->setGuiItem(KStdGuiItem::help());
  buttonOk->setGuiItem(KStdGuiItem::ok());
  buttonCancel->setGuiItem(KStdGuiItem::cancel());

  // default is to need at least one transaction selected
  buttonOk->setDisabled(true);

  // catch some events from the register
  m_register->installEventFilter(this);

  connect(m_register, SIGNAL(selectionChanged(const QValueList<KMyMoneyRegister::SelectedTransaction>&)), this, SLOT(slotEnableOk(const QValueList<KMyMoneyRegister::SelectedTransaction>&)));
  connect(m_register, SIGNAL(editTransaction()), this, SLOT(accept()));

  connect(m_helpButton, SIGNAL(clicked()), this, SLOT(slotHelp()));
}

void KSelectTransactionsDlg::slotEnableOk(const QValueList<KMyMoneyRegister::SelectedTransaction>& list)
{
  buttonOk->setEnabled(list.count() != 0);
}

void KSelectTransactionsDlg::addTransaction(const MyMoneyTransaction& t)
{
  QValueList<MyMoneySplit>::const_iterator it_s;
  for(it_s = t.splits().begin(); it_s != t.splits().end(); ++it_s) {
    if((*it_s).accountId() == m_account.id()) {
      KMyMoneyRegister::Transaction* tr = KMyMoneyRegister::Register::transactionFactory(m_register, &m_objects, t, (*it_s), 0);
      // force full detail display
      tr->setNumRowsRegister(tr->numRowsRegister(true));
      break;
    }
  }
}

int KSelectTransactionsDlg::exec(void)
{
  m_register->updateRegister(true);
  m_register->updateContents();

  m_register->setFocus();

  return KSelectTransactionsDlgDecl::exec();
}

void KSelectTransactionsDlg::slotHelp(void)
{
  // kapp->invokeHelp("details.ledgers.match");
}

void KSelectTransactionsDlg::show(void)
{
  KSelectTransactionsDlgDecl::show();
  m_register->resize(KMyMoneyRegister::DetailColumn);
}

void KSelectTransactionsDlg::resizeEvent(QResizeEvent* ev)
{
  // don't forget the resizer
  KSelectTransactionsDlgDecl::resizeEvent(ev);

  // resize the register
  m_register->resize(KMyMoneyRegister::DetailColumn);
}

MyMoneyTransaction KSelectTransactionsDlg::transaction(void) const
{
  MyMoneyTransaction t;

  QValueList<KMyMoneyRegister::RegisterItem*> list;
  list = m_register->selectedItems();
  if(list.count()) {
    KMyMoneyRegister::Transaction* _t = dynamic_cast<KMyMoneyRegister::Transaction*>(list[0]);
    if(_t)
      t = _t->transaction();
  }
  return t;
}

bool KSelectTransactionsDlg::eventFilter(QObject* o, QEvent* e)
{
  bool rc = false;
  QKeyEvent* k;

  if(o == m_register) {
    switch(e->type()) {
      case QEvent::KeyPress:
        k = dynamic_cast<QKeyEvent*>(e);
        if((k->state() & Qt::KeyButtonMask) == 0) {
          switch(k->key()) {
            case Qt::Key_Return:
            case Qt::Key_Enter:
              if(buttonOk->isEnabled()) {
                accept();
                rc = true;
              }
              // tricky fall through here
            default:
              break;
          }
        }
        // tricky fall through here
      default:
        break;
    }
  }
  return rc;
}

#include "kselecttransactionsdlg.moc"
// vim:cin:si:ai:et:ts=2:sw=2:
