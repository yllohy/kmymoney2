/***************************************************************************
                          kimportdlg.cpp  -  description
                             -------------------
    begin                : Wed May 16 2001
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
#include <qpushbutton.h>
#include <kmessagebox.h>
#include "kimportdlg.h"
#include <qlineedit.h>
#include <qcombobox.h>
#include <kfiledialog.h>
#include <qtextstream.h>
#include <qmessagebox.h>

#include "../mymoney/mymoneycategory.h"

KImportDlg::KImportDlg(MyMoneyFile *file, MyMoneyAccount *account):KImportDlgDecl(0,0,TRUE)
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

KImportDlg::~KImportDlg()
{
  writeConfig();
}
/** No descriptions */
void KImportDlg::slotBrowse()
{
	//KFileDialog *browseFile = new KFileDialog();
	QString s(KFileDialog::getOpenFileName(QString::null,"*.QIF"));
  //delete browseFile;
	txtFileImport->setText(s);

}

void KImportDlg::slotOkClicked()
{
  if (txtFileImport->text().isEmpty()) {
    KMessageBox::information(this, "Please enter the path to the QIF file", "Import");
    txtFileImport->setFocus();
    return;
  }
  accept();
}

void KImportDlg::readConfig(void)
{
  KConfig *config = KGlobal::config();
  config->setGroup("Last Use Settings");
  txtFileImport->setText(config->readEntry("KImportDlg_LastFile"));
	m_qstringLastFormat = config->readEntry("KImportDlg_LastFormat");
}

void KImportDlg::writeConfig(void)
{
  KConfig *config = KGlobal::config();
  config->setGroup("Last Use Settings");
  config->writeEntry("KImportDlg_LastFile", txtFileImport->text());
	config->writeEntry("KImportDlg_LastFormat", comboDateFormat->currentText());
  config->sync();
}

/** No descriptions */
void KImportDlg::readQIFFile(const QString& name, const QString& dateFormat, MyMoneyAccount *account){

	bool catmode = false;
  bool transmode = false;
	bool writecat = false;
	bool writetrans = false;
  bool cleared = false;
	int numcat = 0;
	int numtrans = 0;
  QString catname = "";
	QString expense = "";
	QString date = "";
	QString amount = "";
	QString type = "";
	QString payee = "";
	QString category = "";
  MyMoneyCategory *oldcategory = 0;

    QFile f(name);
    if ( f.open(IO_ReadOnly) ) {    // file opened successfully
        QTextStream t( &f );        // use a text stream
        QString s;
        //int n = 1;
        while ( !t.eof() ) {        // until end of file...
            s = t.readLine();       // line of text excluding '\n'
						if(s.left(9) == "!Type:Cat")
						{
							catmode = true;
							transmode = false;
						}
						else if(s.left(10) == "!Type:Bank")
						{
							qDebug("Found Bank Type");
             				transmode = true;
						  	catmode = false;
						}
						else if(s.left(5) == "!Type")
						{
							qDebug("Found Just Type");
             	catmode = false;
							transmode = false;
						}
						else if(catmode)
						{
							if(s.left(1) == "N")
							{
             		catname = s.mid(1);
							}
							else if(s.left(1) == "E")
							{
             		expense = "E";
							}
							else if(s.left(1) == "I")
							{
								expense = "I";
							}
            	else if(s.left(1) == "^")
							{
								writecat = true;
							}
						}
					  else if(transmode)
						{
             				if(s.left(1) == "^")
							{
               				writetrans = true;
							}
							if(s.left(1) == "D")
							{
			         			date = s.mid(1);
							}
							if(s.left(1) == "T")
							{
               				amount = s.mid(1);
							}
							if(s.left(1) == "N")
							{
								type = s.mid(1);
							}
							if(s.left(1) == "P")
							{
		           			payee = s.mid(1);
							}
							if(s.left(1) == "L")
							{
               				category = s.mid(1);
							}
							if(s.left(1) == "C")
							{
               				cleared = true;
							}
						}
						

						if(transmode && writetrans)
						{
							int slash = -1;
							int apost = -1;
							int checknum = 0;
							bool isnumber = false;
							int intyear = 0;
							int intmonth = 0;
							int intday = 0;
							QString checknumber = "";
							MyMoneyTransaction::transactionMethod transmethod;
							if(dateFormat == "MM/DD'YY")
							{
								slash = date.find("/");							
								apost = date.find("'");
								QString month = date.left(slash);
								QString day = date.mid(slash + 1,2);
								day = day.stripWhiteSpace();
								QString year = date.mid(apost + 1,2);
								year = year.stripWhiteSpace();
								intyear = year.toInt();
								if(intyear > 80)
									intyear = 1900 + year.toInt();
								else
									intyear = 2000 + year.toInt();
								intmonth = month.toInt();
								intday = day.toInt();
							}
							else if(dateFormat == "MM/DD/YYYY")
							{
								slash = date.find("/");							
								apost = date.findRev("/");
								QString month = date.left(slash);
								QString day = date.mid(slash + 1,2);
								day = day.stripWhiteSpace();
								QString year = date.mid(apost + 1,4);
								year = year.stripWhiteSpace();
								intyear = year.toInt();
								intmonth = month.toInt();
								intday = day.toInt();
							}			
							checknum = type.toInt(&isnumber);
							if(isnumber == false)
							{
               				if(type == "ATM")
								{
								  if(amount.find("-") > -1)
                 					transmethod = MyMoneyTransaction::ATM;
									else
										transmethod = MyMoneyTransaction::Deposit;
								}
								else if(type == "DEP")
								{
									if(amount.find("-") == -1)
                 						transmethod = MyMoneyTransaction::Deposit;
									else
										transmethod = MyMoneyTransaction::Withdrawal;
								}
								else if(type == "TXFR")
								{
									if(amount.find("-") > -1)
                 						transmethod = MyMoneyTransaction::Transfer;
									else
										transmethod = MyMoneyTransaction::Deposit;
								}
								else if(type == "WTHD")
								{
									if(amount.find("-") > -1)
                 						transmethod = MyMoneyTransaction::Withdrawal;
									else
										transmethod = MyMoneyTransaction::Deposit;
								}
								else
								{
									if(amount.find("-") > -1)
                 						transmethod = MyMoneyTransaction::Withdrawal;
									else
										transmethod = MyMoneyTransaction::Deposit;
								}
								
							}
							else
							{
									if(amount.find("-") > -1)
               						transmethod = MyMoneyTransaction::Cheque;
									else
										transmethod = MyMoneyTransaction::Deposit;
									checknumber=type;
							}
							QDate transdate(intyear,intmonth,intday);
             				 int commaindex = amount.find(",");
							double dblamount = 0;
		          if(commaindex != -1)
			          dblamount = amount.remove(commaindex,1).toDouble();
		          else
			          dblamount = amount.toDouble();
							if(dblamount < 0)
								dblamount = dblamount * -1;
							MyMoneyMoney moneyamount(dblamount);
							QString majorcat = "";
							QString minorcat = "";
							int catindex = category.find(":");
							if(catindex == -1)				
								majorcat = category;
							else
							{
               	majorcat = category.left(catindex);
								minorcat = category.mid(catindex + 1);
							}
							
							MyMoneyTransaction::stateE transcleared;

							if(cleared == true)
								transcleared = MyMoneyTransaction::Reconciled;
							else
								transcleared = MyMoneyTransaction::Unreconciled;

              if (!payee.isEmpty())
                m_file->addPayee(payee);
              account->addTransaction(transmethod, checknumber, payee,
                                      moneyamount, transdate, majorcat, minorcat, "",payee,
                                      "", "", transcleared);
							qDebug("Date:%s",date.latin1());
							qDebug("Amount:%s",amount.latin1());
							qDebug("Type:%s",type.latin1());
							qDebug("Payee:%s",payee.latin1());
							qDebug("Category:%s",category.latin1());

			        date = "";
              amount = "";
							type = "";
		          payee = "";
              category = "";
              cleared = false;
							writetrans = false;
							numtrans += 1;
						}
						if(catmode && writecat)
						{
							QString majorcat = "";
							QString minorcat = "";
							bool minorcatexists = false;
							bool majorcatexists = false;
							int catindex = catname.find(":");
							if(catindex == -1)
								majorcat = catname;
							else
							{
               				majorcat = catname.left(catindex);
								minorcat = catname.mid(catindex + 1);
							}
  							QListIterator<MyMoneyCategory> it = m_file->categoryIterator();
  							for ( ; it.current(); ++it ) {
    							MyMoneyCategory *data = it.current();
								if((majorcat == data->name()) && (minorcat == ""))
								{
									majorcatexists = true;
                  					minorcatexists = true;
								}
								else if(majorcat == data->name())
								{
									oldcategory = it.current();
									majorcatexists = true;
    								for ( QStringList::Iterator it2 = data->minorCategories().begin(); it2 != data->minorCategories().end(); ++it2 ) {
                  						 if((*it2) == minorcat)
									 	{
									 		 minorcatexists = true;
									 	}
									}

    							}
  						}

  						MyMoneyCategory category;
							category.setName(majorcat);
							if(expense == "E")
								category.setIncome(false);
							if(expense == "I")
								category.setIncome(true);
							if(majorcatexists == true)
							{
								if((minorcatexists == false) && (minorcat != ""))
								{
									category.addMinorCategory(minorcat);
									oldcategory->addMinorCategory(minorcat);
									numcat += 1;
								}
							}
							else
							{
								if(minorcat != "")
									category.addMinorCategory(minorcat);
								m_file->addCategory(category.isIncome(), category.name(), category.minorCategories());
               	           numcat += 1;
							}
							catname = "";
							expense = "";
							writecat = false;
						}	
						//qDebug("%s",s.latin1());
        }
        f.close();
    }
	QString importmsg;
	importmsg.sprintf("%d Categories imported.\n%d Transactions imported.",numcat,numtrans);
    QMessageBox::information(this,"QIF Import",importmsg);

}
