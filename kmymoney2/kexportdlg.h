/***************************************************************************
                          kexportdlg.h  -  description
                             -------------------
    begin                : Tue May 22 2001
    copyright            : (C) 2001 by Michael Edwardes
    email                : mte@users.sourceforge.net
                             Javier Campos Morales <javi_c@ctv.es>
                             Felix Rodriguez <frodriguez@mail.wesleyan.edu>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef KEXPORTDLG_H
#define KEXPORTDLG_H

// ----------------------------------------------------------------------------
// QT Headers
#include <qstring.h>

// ----------------------------------------------------------------------------
// KDE Headers

// ----------------------------------------------------------------------------
// Project Headers
#include "kexportdlgdecl.h"

/**
  *@author Michael Edwardes
  */
class KExportDlg : public KExportDlgDecl  {
	Q_OBJECT
private:
  void readConfig(void);
  void writeConfig(void);

	QString m_qstringLastFormat;

public:
	KExportDlg();
	~KExportDlg();
protected slots:
  void slotOkClicked();

public slots: // Public slots
  /** No descriptions */
  void slotBrowse();
};

#endif
