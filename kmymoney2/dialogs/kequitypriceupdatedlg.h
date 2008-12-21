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
#include "../dialogs/kequitypriceupdatedlgdecl.h"

/**
  * @author Kevin Tambascio & Ace Jones
  */

class KEquityPriceUpdateDlg : public KEquityPriceUpdateDlgDecl
{
  Q_OBJECT
public:
  KEquityPriceUpdateDlg(QWidget *parent, const QString& securityId = QString());
  ~KEquityPriceUpdateDlg();
  void storePrices(void);
  MyMoneyPrice price(const QString& id) const;

protected slots:
  void slotUpdateSelectedClicked(void);
  void slotUpdateAllClicked(void);
  void slotUpdateSelection(void);

  void logStatusMessage(const QString&);
  void logErrorMessage(const QString&);
  void slotReceivedQuote(const QString&, const QString&,const QDate&, const double&);
  void slotQuoteFailed(const QString& _id, const QString& _symbol);

protected:
  void addPricePair(const MyMoneySecurityPair& pair, bool dontCheckExistance = false);
  void addInvestment(const MyMoneySecurity& inv);
  void finishUpdate(void);

private:
  bool m_fUpdateAll;
  WebPriceQuote m_webQuote;
};

#endif
// vim:cin:si:ai:et:ts=2:sw=2:
