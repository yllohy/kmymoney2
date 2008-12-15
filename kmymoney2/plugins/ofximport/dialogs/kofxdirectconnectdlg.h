/***************************************************************************
                         kofxdirectconnectdlg.h
                             -------------------
    begin                : Sat Nov 13 2004
    copyright            : (C) 2002 by Ace Jones
    email                : acejones@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KOFXDIRECTCONNECTDLG_H
#define KOFXDIRECTCONNECTDLG_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef USE_OFX_DIRECTCONNECT

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

class KTempFile;
class KOfxDirectConnectDlgPrivate;

namespace KIO
{
class Job;
class TransferJob;
}

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyofxconnector.h"
#include "kofxdirectconnectdlgdecl.h"

/**
@author ace jones
*/
class KOfxDirectConnectDlg : public KOfxDirectConnectDlgDecl
{
Q_OBJECT
public:
  KOfxDirectConnectDlg(const MyMoneyAccount&, QWidget *parent = 0, const char *name = 0);
  ~KOfxDirectConnectDlg();
    
  void init(void);  

signals:
  /**
    * This signal is emitted when the statement is downloaded
    * and stored in file @a fname.
    */
  void statementReady(const QString& fname);

protected slots:
  void slotOfxFinished(KIO::Job*);
  void slotOfxData(KIO::Job*,const QByteArray&);
  void slotOfxConnected(KIO::Job*);
  virtual void reject(void);

protected:  
  void setStatus(const QString& _status);
  void setDetails(const QString& _details);  
  
  KTempFile* m_tmpfile;
  MyMoneyOfxConnector m_connector;
  KIO::TransferJob* m_job;

private:
  KOfxDirectConnectDlgPrivate*	d;
};

#endif // USE_OFX_DIRECTCONNECT
#endif // KOFXDIRECTCONNECTDLG_H
