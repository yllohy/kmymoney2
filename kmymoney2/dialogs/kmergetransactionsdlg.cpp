/***************************************************************************
                          kmergetransactionsdlg.cpp
                             -------------------
    begin                : Sun Aug 20 2006
    copyright            : (C) 2006 by Ace Jones
    email                : <acejones@users.sf.net>
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

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoney/mymoneyfile.h>
#include <kmymoney/mymoneytransaction.h>
#include <kmymoney/kmymoneyglobalsettings.h>

#include "kmergetransactionsdlg.h"

KMergeTransactionsDlg::KMergeTransactionsDlg(const MyMoneyAccount& _account, QWidget* parent, const char* name) :
  KMergeTransactionsDlgDecl(parent, name),
  m_account(_account)
{
  // clear current register contents
  m_register->clear();

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

  connect(m_helpButton, SIGNAL(clicked()), this, SLOT(slotHelp()));
}

void KMergeTransactionsDlg::addTransaction(const MyMoneyTransaction& t)
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

int KMergeTransactionsDlg::exec(void)
{
  m_register->updateRegister(true);
  m_register->updateContents();

  return KMergeTransactionsDlgDecl::exec();
}

void KMergeTransactionsDlg::slotHelp(void)
{
  kapp->invokeHelp("details.ledgers.match");
}

void KMergeTransactionsDlg::show(void)
{
  KMergeTransactionsDlgDecl::show();
  m_register->resize(KMyMoneyRegister::DetailColumn);
}

void KMergeTransactionsDlg::resizeEvent(QResizeEvent* ev)
{
  // don't forget the resizer
  KMergeTransactionsDlgDecl::resizeEvent(ev);

  // resize the register
  m_register->resize(KMyMoneyRegister::DetailColumn);
}

#include "kmergetransactionsdlg.moc"
// vim:cin:si:ai:et:ts=2:sw=2:
