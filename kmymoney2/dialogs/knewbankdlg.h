/***************************************************************************
                          knewbankdlg.h
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

#ifndef KNEWBANKDLG_H
#define KNEWBANKDLG_H

#include <klocale.h>
#include <qdialog.h>

#include "../mymoney/mymoneymoney.h"

#include "knewbankdlgdecl.h"

// This dialog lets the user create or edit
// a bank.
// Use the second constructor to edit the bank.
class KNewBankDlg : public KNewBankDlgDecl  {
   Q_OBJECT

public:
	KNewBankDlg(QWidget *parent=0, const char *name=0);
  KNewBankDlg(QString b_name, QString b_sortCode, QString b_city,
    QString b_street, QString b_postcode, QString b_telephone, QString b_manager,
    QString title, QWidget *parent=0, const char *name=0);
	~KNewBankDlg();

	QString m_name;
	QString m_street;
	QString m_city;
	QString m_postcode;
	QString m_telephone;
	QString m_managerName;
        QString m_sortCode;

protected slots:
  void okClicked();
};

#endif
