/***************************************************************************
                          kreportconfigurationdlg.h  -  description
                             -------------------
    begin                : Sun May 23 2004
    copyright            : (C) 2000-2004 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
                           Ace Jones <ace.j@hotpop.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KREPORTCONFIGURATIONDLG_H
#define KREPORTCONFIGURATIONDLG_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "../dialogs/kreportconfigurationdecl.h"
#include "../views/pivottable.h"

/**
  * This dialog lets you create/edit an account.
  */
class KReportConfigurationDlg : public KReportConfigurationDecl
{
  Q_OBJECT

private:
  reports::ReportConfiguration m_currentConfiguration;

public:

  KReportConfigurationDlg(const reports::ReportConfiguration& config, QWidget *parent=0, const char *name=0, const char *title=0);
  const reports::ReportConfiguration& getResult(void);

};

#endif //KREPORTCONFIGURATIONDLG_H


