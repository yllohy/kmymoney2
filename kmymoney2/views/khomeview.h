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

// ----------------------------------------------------------------------------
// KDE Includes
#include <khtml_part.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "../mymoney/mymoneyscheduled.h"
#include "../mymoney/mymoneyaccount.h"

/**
  * Displays a 'home page' for the user.  Similar to concepts used in
  * quicken and m$-money.
  *
  * @author Michael Edwardes
  * $Id: khomeview.h,v 1.5 2003/08/03 07:34:21 ipwizard Exp $
  *
  * @short A view containing the home page for kmymoney2.
**/
class KHomeView : public QWidget  {
   Q_OBJECT

private:
  KHTMLPart *m_part;
  QVBoxLayout *m_qvboxlayoutPage;
  QString m_filename;
  
signals:
  void signalViewActivated();
  void ledgerSelected(const QCString& id, const QCString& transaction);


public:
  /**
    * Standard constructor.
    *
    * @param parent The QWidget this is used in.
    * @param name The QT name.
    *
    * @return An object of type KHomeView
    *
    * @see ~KHomeView
  **/
  KHomeView(QWidget *parent=0, const char *name=0);

  /**
    * Standard destructor.
    *
    * @return Nothing.
    *
    * @see KHomeView
  **/
  ~KHomeView();

  /**
    * Overridden so we can emit the activated signal.
    *
    * @return Nothing.
  **/
  void show();

  void refreshView(void);
  
protected:
  /**
    * Definition of bitmap used as argument for showAccounts().
    */
  enum paymentTypeE {
    Preferred = 1,          ///< show preferred accounts
    Payment = 2             ///< show payment accounts
  };
  
  void showPayments(void);
  void showPaymentEntry(const MyMoneySchedule&);
  void showAccounts(paymentTypeE type, const QString& hdr);
  void showAccountEntry(const MyMoneyAccount&);
  const QString link(const QString& view, const QString& query) const;
  const QString linkend(void) const;
  
public slots:
  void slotOpenURL(const KURL &url, const KParts::URLArgs& args);
};

#endif
