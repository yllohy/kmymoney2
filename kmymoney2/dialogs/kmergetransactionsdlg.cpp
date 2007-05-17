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
#include <kapplication.h>
#include <kactivelabel.h>
#include <klocale.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmergetransactionsdlg.h"

KMergeTransactionsDlg::KMergeTransactionsDlg(const MyMoneyAccount& account, QWidget* parent, const char* name) :
  KSelectTransactionsDlg(account, parent, name)
{

  // setup descriptive texts
  setCaption(i18n("Merge Transactions"));
  m_description->setText(i18n("Are you sure you wish to merge these transactions?"));

  // no selection possible
  m_register->setSelectionMode(QTable::NoSelection);

  // override default and enable ok button right away
  buttonOk->setEnabled(true);
}

void KMergeTransactionsDlg::slotHelp(void)
{
  kapp->invokeHelp("details.ledgers.match");
}

#include "kmergetransactionsdlg.moc"
// vim:cin:si:ai:et:ts=2:sw=2:
