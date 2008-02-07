/***************************************************************************
                          khomeview.h  -  description
                             -------------------
    begin                : Tue Jan 22 2002
    copyright            : (C) 2000-2002 by Michael Edwardes
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
#ifndef KHOMEVIEW_H
#define KHOMEVIEW_H

// ----------------------------------------------------------------------------
// QT Includes
#include <qwidget.h>
class QVBoxLayout;
class QFrame;

// ----------------------------------------------------------------------------
// KDE Includes
#include <khtml_part.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "../mymoney/mymoneyscheduled.h"
#include "../mymoney/mymoneyaccount.h"
#include "../views/kmymoneyview.h"

/**
  * Displays a 'home page' for the user.  Similar to concepts used in
  * quicken and m$-money.
  *
  * @author Michael Edwardes
  *
  * @short A view containing the home page for kmymoney2.
**/
class KHomeView : public KMyMoneyViewBase
{
  Q_OBJECT
public:
  /**
    * Definition of bitmap used as argument for showAccounts().
    */
  enum paymentTypeE {
    Preferred = 1,          ///< show preferred accounts
    Payment = 2             ///< show payment accounts
  };

  KHomeView(QWidget *parent=0, const char *name=0);
  ~KHomeView();

protected:
  void showPayments(void);
  void showPaymentEntry(const MyMoneySchedule&, int cnt = 1);
  void showAccounts(paymentTypeE type, const QString& hdr);
  void showAccountEntry(const MyMoneyAccount&);
  void showFavoriteReports();
  void showForecast(void);
  void showNetWorthGraph(void);

  const QString link(const QString& view, const QString& query) const;
  const QString linkend(void) const;
  void loadView(void);

public slots:
  /**
    * Overridden so we can emit the activated signal.
    *
    * @return Nothing.
    */
  void show(void);

  void slotOpenURL(const KURL &url, const KParts::URLArgs& args);
  void slotLoadView(void);

  /**
    * Print the current view
    */
  void slotPrintView(void);

signals:
  void ledgerSelected(const QCString& id, const QCString& transaction);
  void scheduleSelected(const QCString& id);
  void reportSelected(const QCString& id);

private:
  KHTMLPart*      m_part;
  QVBoxLayout*    m_qvboxlayoutPage;
  QString         m_filename;
  bool            m_showAllSchedules;
  bool            m_needReload;
};

#endif
