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

#include "kequitypriceupdatedlgdecl.h"
#include "../mymoney/mymoneyonlinepriceupdate.h"

class KEquityPriceUpdateProcess: public KProcess
{
  Q_OBJECT
public:
  KEquityPriceUpdateProcess(void);
  void setItem(QListViewItem* _item) { m_item = _item; m_string.truncate(0); }

public slots:
  void slotReceivedDataFromFilter(KProcess*, char*, int);
  void slotProcessExited(KProcess*);

signals:
  void processExited(QListViewItem*,const QString&);

private:
  QListViewItem* m_item;
  QString m_string;
};

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
  void slotReceivedErrorFromFilter(KProcess*, char*, int);
  void slotInsertUpdate(QListViewItem* _item, const QString& _quotedata );

protected:
  void launchUpdate(QListViewItem* _item);
  void logStatusMessage(const QString& message);
  void logErrorMessage(const QString& message);

private:
  KEquityPriceUpdateProcess m_filter;
  QString m_strdata;
  bool m_fUpdateAll;
};

#endif
