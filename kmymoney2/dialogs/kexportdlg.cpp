/***************************************************************************
                          kexportdlg.cpp  -  description
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
#include <kmessagebox.h>
#include "kexportdlg.h"
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include "../widgets/kmymoneydateinput.h"
#include <kfiledialog.h>
#include <qtextstream.h>
#include <qmessagebox.h>

#include "../mymoney/mymoneycategory.h"

KExportDlg::KExportDlg(MyMoneyFile *file, MyMoneyAccount *account):KExportDlgDecl(0,0,TRUE)
{
  m_file = file;

	comboDateFormat->insertItem("MM/DD\'YY");
	comboDateFormat->insertItem("MM/DD/YYYY");
	comboDateFormat->setEditable(false);

 	readConfig();

	if (m_qstringLastFormat == "MM/DD\'YY")
		comboDateFormat->setCurrentItem(0);
	else
		comboDateFormat->setCurrentItem(1);

  connect( btnBrowse, SIGNAL( clicked() ), this, SLOT( slotBrowse() ) );
  connect(buttonOk, SIGNAL(clicked()), this, SLOT(slotOkClicked()));
}
KExportDlg::~KExportDlg()
{
  writeConfig();
}
/** No descriptions */
void KExportDlg::slotBrowse(){

	QString s(KFileDialog::getSaveFileName(QString::null,"*.QIF"));
  txtFileExport->setText(s);
}

void KExportDlg::slotOkClicked()
{
  if (txtFileExport->text().isEmpty()) {
    KMessageBox::information(this, "Please enter the path to the QIF file", "Export");
    txtFileExport->setFocus();
    return;
  }
  accept();
}

void KExportDlg::readConfig(void)
{
  KConfig *config = KGlobal::config();
  config->setGroup("Last Use Settings");
  txtFileExport->setText(config->readEntry("KExportDlg_LastFile"));
  cbxAccount->setChecked(config->readBoolEntry("KExportDlg_AccountOpt", true));
  cbxCategories->setChecked(config->readBoolEntry("KExportDlg_CatOpt", true));
  dateStartDate->setDate(config->readDateTimeEntry("KExportDlg_StartDate").date());
  dateEndDate->setDate(config->readDateTimeEntry("KExportDlg_EndDate").date());
	m_qstringLastFormat = config->readEntry("KExportDlg_LastFormat");
}

void KExportDlg::writeConfig(void)
{
  KConfig *config = KGlobal::config();
  config->setGroup("Last Use Settings");
  config->writeEntry("KExportDlg_LastFile", txtFileExport->text());
  config->writeEntry("KExportDlg_AccountOpt", cbxAccount->isChecked());
  config->writeEntry("KExportDlg_CatOpt", cbxCategories->isChecked());
  config->writeEntry("KExportDlg_StartDate", QDateTime(dateStartDate->getQDate()));
  config->writeEntry("KExportDlg_EndDate", QDateTime(dateEndDate->getQDate()));
	config->writeEntry("KExportDlg_LastFormat", comboDateFormat->currentText());

  config->sync();
}

void KExportDlg::writeQIFFile(const QString& name, const QString& dateFormat, MyMoneyAccount *account,bool expCat,bool expAcct,
																QDate startDate, QDate endDate){
	int numcat = 0;
	int numtrans = 0;

    QFile f(name);
    if ( f.open(IO_WriteOnly) ) {    // file opened successfully
      QTextStream t( &f );        // use a text stream

			if(expCat)
			{
				t << "!Type:Cat" << endl;
  			QListIterator<MyMoneyCategory> it = m_file->categoryIterator();
  			for ( ; it.current(); ++it ) {
    			MyMoneyCategory *data = it.current();
						t << "N" + data->name() << endl;
						if(data->isIncome())
							t << "I" << endl;
						else
							t << "E" << endl;
						t << "^" << endl;
						numcat += 1;
    				for ( QStringList::Iterator it2 = data->minorCategories().begin(); it2 != data->minorCategories().end(); ++it2 ) {
								t << "N" << data->name() << ":" << *it2 << endl;
								if(data->isIncome())
									t << "I" << endl;
								else
									t << "E" << endl;
								t << "^" << endl;
								numcat += 1;
						}
  			}       		
			}
			if(expAcct)
			{
				t << "!Type:Bank" << endl;
				MyMoneyTransaction *transaction;
    		for ( transaction = account->transactionFirst(); transaction; transaction=account->transactionNext())
        {
        	if((transaction->date() >= startDate) && (transaction->date() <= endDate))
					{
          	int year = transaction->date().year();
						if(dateFormat == "MM/DD'YY")
						{
							if(year >=2000)
            					year -= 2000;
							else
								year -= 1900;
						}
						int month = transaction->date().month();
						int day = transaction->date().day();
						double amount = transaction->amount().amount();
						if(transaction->type() == MyMoneyTransaction::Debit)
						  amount = amount * -1;
						QString transmethod;
						if(transaction->method() == MyMoneyTransaction::ATM)
							transmethod = "ATM";
						if(transaction->method() == MyMoneyTransaction::Deposit)
							transmethod = "DEP";
						if(transaction->method() == MyMoneyTransaction::Transfer)
							transmethod = "TXFR";
						if(transaction->method() == MyMoneyTransaction::Withdrawal)
							transmethod = "WTHD";
						if(transaction->method() == MyMoneyTransaction::Cheque)
							transmethod = transaction->number();
						QString Payee = transaction->payee();
						QString Category;
						if(transaction->categoryMinor() == "")
							Category = transaction->categoryMajor();
						else
							Category = transaction->categoryMajor() + ":" + transaction->categoryMinor();
							
						if(dateFormat == "MM/DD'YY")
						{
							t << "D" << month << "/" << day << "'" << year << endl;
						}
						if(dateFormat == "MM/DD/YYYY")
						{
							t << "D" << month << "/" << day << "/" << year << endl;
						}
						t << "U" << amount << endl;
						t << "T" << amount << endl;
						if(transaction->state() == MyMoneyTransaction::Reconciled)
							t << "CX" << endl;
						t << "N" << transmethod << endl;
						t << "P" << Payee << endl;
						t << "L" << Category << endl;
						t << "^" << endl;
						numtrans += 1;

													
					}
				}
			}
      f.close();
		}
	QString exportmsg;
	exportmsg.sprintf("%d Categories exported.\n%d Transactions exported.",numcat,numtrans);
    QMessageBox::information(this,"QIF Export",exportmsg);

}
