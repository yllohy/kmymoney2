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

#include <kglobal.h>
#include <klocale.h>
#if QT_VERSION > 300
#include <kstandarddirs.h>
#else
#include <kstddirs.h>
#endif

#include <qpixmap.h>

// ----------------------------------------------------------------------------
// QT Includes
#include <qfile.h>
#include <qtextstream.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qprogressbar.h>
#include <qlineedit.h>
#include <qgroupbox.h>
#include <qlistview.h>

// ----------------------------------------------------------------------------
// KDE Includes
#include <kfiledialog.h>
#include <kglobal.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kconfig.h>
#include <knuminput.h>
#include <klistview.h>
                          
#include "keditequityentrydlg.h"

KEditEquityEntryDlg::KEditEquityEntryDlg(MyMoneyEquity* selectedEquity, QWidget *parent, const char *name)
  : kEditEquityEntryDecl(parent, name, true)
{
  m_selectedEquity = selectedEquity;
  lvPriceHistory->addColumn(QString("Date"));
  lvPriceHistory->addColumn(QString("Price"));
  
  connect(btnOK, SIGNAL(clicked()), this, SLOT(slotOKClicked()));
	connect(btnCancel, SIGNAL(clicked()), this, SLOT(slotCancelClicked()));
  connect(lvPriceHistory, SIGNAL(doubleClicked(QListViewItem*, const QPoint&, int)), this, SLOT(slotPriceHistoryDoubleClicked(QListViewItem *, const QPoint&, int)));
  connect(edtEquityName, SIGNAL(textChanged(const QString &)), this, SLOT(slotEquityNameChanged(const QString&)));
  connect(edtMarketSymbol, SIGNAL(textChanged(const QString &)), this, SLOT(slotEquitySymbolChanged(const QString&)));
  
  edtEquityName->setText(m_selectedEquity->getEquityName());

  m_changes = false;
}

KEditEquityEntryDlg::~KEditEquityEntryDlg()
{
  
}

/** No descriptions */
void KEditEquityEntryDlg::slotOKClicked()
{
	accept();
}

void KEditEquityEntryDlg::slotCancelClicked()
{
	reject();
}

void KEditEquityEntryDlg::slotPriceHistoryDoubleClicked(QListViewItem *item, const QPoint &point, int c)
{

}

void KEditEquityEntryDlg::slotEquityNameChanged(const QString& str)
{
  m_changes = true;
}

void KEditEquityEntryDlg::slotEquitySymbolChanged(const QString& str)
{
  m_changes = true;
}
