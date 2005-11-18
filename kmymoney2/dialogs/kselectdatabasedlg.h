/***************************************************************************
                          kselectdatabase.h
                             -------------------
    copyright            : (C) 2005 by Tony Bloomfield
    author               : Tony Bloomfield
    email                : tonybloom@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KSELECTDATABASEDLG_H
#define KSELECTDATABASEDLG_H

// ----------------------------------------------------------------------------
// QT Includes
#include <qlistbox.h>
#include <qlineedit.h>
#include <qlabel.h>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes
#include "kselectdatabasedlgdecl.h"

class KSelectDatabaseDlg : public KSelectDatabaseDlgDecl
{
Q_OBJECT
public:
  KSelectDatabaseDlg(QWidget *parent = 0, const char *name = 0);
  ~KSelectDatabaseDlg();
  
  const QString driverName() const {return (listDrivers->currentText().section (' ', 0, 0));}
  const QString dbName() const {return (textDbName->text());}
  const QString hostName() const {return (textHostName->text());}
  const QString userName() const {return (textUserName->text());}
  const QString password() const {return (textPassword->text());}
    
public slots:
  void slotDriverSelected(const QString&);
  void slotBrowse();
  void slotHelp();
  void slotGenerateSQL();
};

#endif
