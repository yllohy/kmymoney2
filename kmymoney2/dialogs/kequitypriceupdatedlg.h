/***************************************************************************
                          kequitypriceupdatedlg.h  -  description
                             -------------------
    begin                : Tuesday June 22nd, 2004
    copyright            : (C) 2000-2004 by Kevin Tambascio
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

#ifndef KEQUITYPRICEUPDATEDIALOG_H
#define KEQUITYPRICEUPDATEDIALOG_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qwidget.h>
#include <qpair.h>
#include <qdatastream.h>
class QListViewItem;

// ----------------------------------------------------------------------------
// KDE Includes

#include <kprocess.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "../converter/webpricequote.h"
#include "../mymoney/mymoneysecurity.h"
#include "../mymoney/mymoneyprice.h"
#include "kequitypriceupdatedlgdecl.h"

/**
  * @author Kevin Tambascio & Ace Jones
  */

class KEquityPriceUpdateDlg : public KEquityPriceUpdateDlgDecl
{
  Q_OBJECT
public:
  KEquityPriceUpdateDlg(QWidget *parent, const QCString& securityId = QCString());
  ~KEquityPriceUpdateDlg();

protected slots:
  void slotOKClicked();
  void slotCancelClicked();
  void slotUpdateSelectedClicked();
  void slotUpdateAllClicked();
  void slotConfigureClicked();
  void slotUpdateSelection();

  void logStatusMessage(const QString&);
  void logErrorMessage(const QString&);
  void slotReceivedQuote(const QString&,const QDate&, const double&);

protected:
  void addPricePair(const MyMoneySecurityPair& pair);
  void addInvestment(const MyMoneySecurity& inv);
  
private:
  bool m_fUpdateAll;
  WebPriceQuote m_webQuote;
  unsigned m_pricePrecision;
};

#endif
