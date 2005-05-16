/***************************************************************************
                          konlinequoteconfigurationdlg.cpp  -  description
                             -------------------
    begin                : Tuesday July 1st, 2004
    copyright            : (C) 2004 by Kevin Tambascio
    email                : ktambascio@users.sourceforge.net
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

#include <qlabel.h>
#include <qpushbutton.h>
#include <qradiobutton.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <klocale.h>
#include <kmessagebox.h>
#include <kpushbutton.h>
#include <klistbox.h>
#include <klistview.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "konlinequoteconfigurationdecl.h"
#include "konlinequoteconfigurationdlg.h"


KOnlineQuoteConfigurationDlg::KOnlineQuoteConfigurationDlg(QWidget *parent) : kOnlineQuoteConfigurationDecl(parent)
{
  
}

KOnlineQuoteConfigurationDlg::~KOnlineQuoteConfigurationDlg()
{

}

#include "konlinequoteconfigurationdlg.moc"
