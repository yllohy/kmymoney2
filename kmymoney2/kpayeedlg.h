/***************************************************************************
                          kpayeedlg.h
                             -------------------
    copyright            : (C) 2000 by Michael Edwardes
    email                : mte@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KPAYEEDLG_H
#define KPAYEEDLG_H

//#include <klocale.h>
#include <kcombobox.h>
#include <qmultilineedit.h>
#include <qlineedit.h>
#include <qlabel.h>

#include "mymoney/mymoneyfile.h"
#include "kpayeedlgdecl.h"

// This dialog lets the user edit payee details.
class KPayeeDlg : public KPayeeDlgDecl {
   Q_OBJECT
public:
	KPayeeDlg(MyMoneyFile *file, QWidget *parent=0, const char *name=0);
	~KPayeeDlg();

protected slots:
  void payeeHighlighted(const QString&);
  void slotAddClicked();
  void slotPayeeTextChanged(const QString& text);
  void slotUpdateClicked();
  void slotDeleteClicked();

private:
  MyMoneyFile *m_file;
};

#endif
