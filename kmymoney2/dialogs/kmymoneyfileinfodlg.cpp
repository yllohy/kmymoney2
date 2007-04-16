/***************************************************************************
                          kmymoneyfileinfodlg.cpp  -  description
                             -------------------
    begin                : Sun Oct 9 2005
    copyright            : (C) 2005 by Thomas Baumgart
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

#include <qpushbutton.h>
#include <qlabel.h>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyfileinfodlg.h"
#include <kmymoney/imymoneystorage.h>
#include <kmymoney/mymoneyfile.h>

KMyMoneyFileInfoDlg::KMyMoneyFileInfoDlg(QWidget *parent, const char *name )
 : KMyMoneyFileInfoDlgDecl(parent, name)
{
  // Hide the unused buttons.
  buttonCancel->hide();
  buttonHelp->hide();

  // Now fill the fields with data
  IMyMoneyStorage* storage = MyMoneyFile::instance()->storage();

  m_creationDate->setText(storage->creationDate().toString(Qt::ISODate));
  m_lastModificationDate->setText(storage->lastModificationDate().toString(Qt::ISODate));
  m_baseCurrency->setText(storage->value("kmm-baseCurrency"));

  m_payeeCount->setText(QString("%1").arg(storage->payeeList().count()));
  m_institutionCount->setText(QString("%1").arg(storage->institutionList().count()));
  m_accountCount->setText(QString("%1").arg(storage->accountList().count()));

  MyMoneyTransactionFilter filter;
  filter.setReportAllSplits(false);
  m_transactionCount->setText(QString("%1").arg(storage->transactionList(filter).count()));
  m_scheduleCount->setText(QString("%1").arg(storage->scheduleList().count()));
  m_priceCount->setText(QString("%1").arg(storage->priceList().count()));

}

KMyMoneyFileInfoDlg::~KMyMoneyFileInfoDlg()
{
}

#include "kmymoneyfileinfodlg.moc"
