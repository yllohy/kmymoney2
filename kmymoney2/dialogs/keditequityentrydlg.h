/***************************************************************************
                          keditequityentrydlg.h  -  description
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

#ifndef KEDITEQUITYENTRYDLG_H
#define KEDITEQUITYENTRYDLG_H


// ----------------------------------------------------------------------------
// QT Includes
#include <qdialog.h>

// ----------------------------------------------------------------------------
// KDE Includes
#include <klistview.h>
#include <klocale.h>
#include <klineedit.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "keditequityentrydecl.h"
#include "../mymoney/mymoneyequity.h"

/**
  *@author Kevin Tambascio
  */

class KEditEquityEntryDlg : public kEditEquityEntryDecl
{
  Q_OBJECT
public:
  KEditEquityEntryDlg(const MyMoneyEquity& selectedEquity, QWidget *parent = NULL, const char *name = NULL);
  ~KEditEquityEntryDlg();

  void updatedEquity(MyMoneyEquity& equity) { equity = m_selectedEquity; }

protected slots:
  void slotOKClicked();
  void slotCancelClicked();
  void slotPriceHistoryDoubleClicked(QListViewItem *item, const QPoint &point, int c);
  void slotPriceHistoryClicked(QListViewItem* item, const QPoint& point, int c);
  void slotDataChanged(void);
  void slotAddPriceClicked();
  void slotEditPriceClicked();
  void slotRemovePriceClicked();

private:
  MyMoneyEquity m_selectedEquity;
  bool m_changes;
};

#endif
