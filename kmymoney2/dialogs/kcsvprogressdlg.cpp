/***************************************************************************
                          kcsvprogressdlg.cpp  -  description
                             -------------------
    begin                : Sun Jul 29 2001
    copyright            : (C) 2000-2001 by Michael Edwardes
    email                : mte@users.sourceforge.net
                             Javier Campos Morales <javi_c@users.sourceforge.net>
                             Felix Rodriguez <frodriguez@users.sourceforge.net>
                             John C <thetacoturtle@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <kglobal.h>
#include <klocale.h>
#include <kstddirs.h>
#include <qpixmap.h>
// ----------------------------------------------------------------------------
// QT Includes
#include <qfile.h>
#include <qtextstream.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qprogressbar.h>
#include <qlineedit.h>
#include <qgroupbox.h>

// ----------------------------------------------------------------------------
// KDE Includes
#include <kfiledialog.h>
#include <kglobal.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kconfig.h>

// ----------------------------------------------------------------------------
// Project Includes
#include "kcsvprogressdlg.h"

/** Simple constructor */
KCsvProgressDlg::KCsvProgressDlg(int type, MyMoneyAccount *account, QWidget *parent, const char *name )
 : KCsvProgressDlgDecl(parent,name, true)
{
	QString filename = KGlobal::dirs()->findResource("appdata", "pics/dlg_csv_import.png");
  	QPixmap *pm = new QPixmap(filename);
  	m_qpixmaplabel->setPixmap(*pm);

  m_nType = type;
  m_mymoneyaccount = account;
  m_bStopFlag = false;
  m_bProcessStarted = false;

  m_qbuttonOk->setText(i18n("C&lose"));
  m_qbuttonOk->setEnabled(false);

  readConfig();

  connect(m_qbuttonBrowse, SIGNAL(clicked()), this, SLOT(slotBrowseClicked()));
  connect(m_qbuttonRun, SIGNAL(clicked()), this, SLOT(slotRunClicked()));
  connect(m_qlineeditFile, SIGNAL(textChanged(const QString&)), this,
    SLOT(slotFileTextChanged(const QString&)));
  connect(m_qbuttonOk, SIGNAL(clicked()), this, SLOT(accept()));
  connect(m_qbuttonCancel, SIGNAL(clicked()), this, SLOT(slotCancelClicked()));
}

/** Simple destructor */
KCsvProgressDlg::~KCsvProgressDlg()
{
  writeConfig();
}

/** Perform the export process */
void KCsvProgressDlg::performExport(void)
{
  if (!m_mymoneyaccount)
    return;

  QString qstringName = m_qlineeditFile->text();
  if (qstringName.isEmpty())
    return;

  QString qstringBuffer;

  m_qlabelAccount->setText(m_mymoneyaccount->name());
  m_qlabelTransaction->setText(QString("0") + i18n(" of ") + QString::number(m_mymoneyaccount->transactionCount()));
  m_qprogressbar->setTotalSteps(m_mymoneyaccount->transactionCount());

  m_bProcessStarted = true;

  // Write header line(s) first
  QFile qfile(qstringName);
  if (qfile.open(IO_WriteOnly)) {
    QTextStream qtextstream(&qfile);

    QString qstringTmpBuf1, qstringTmpBuf2;
    MyMoneyTransaction *mymoneytransaction;

    int nCount = 0;
    for (nCount=1, mymoneytransaction = m_mymoneyaccount->transactionFirst();
        mymoneytransaction; mymoneytransaction=m_mymoneyaccount->transactionNext(), nCount++) {

      // Check the stop flag, (true when the user clicks cancel
      if (m_bStopFlag)
        break;

      m_qlabelTransaction->setText(QString::number(nCount) + i18n(" of ") + QString::number(m_mymoneyaccount->transactionCount()));

      switch (mymoneytransaction->type()) {
        case MyMoneyTransaction::Cheque:
          qstringTmpBuf1 = i18n("Cheque");
          break;
        case MyMoneyTransaction::Deposit:
          qstringTmpBuf1 = i18n("Deposit");
          break;
        case MyMoneyTransaction::ATM:
          qstringTmpBuf1 = i18n("ATM");
          break;
        case MyMoneyTransaction::Withdrawal:
          qstringTmpBuf1 = i18n("Withdrawal");
          break;
        case MyMoneyTransaction::Transfer:
          qstringTmpBuf1 = i18n("Transfer");
          break;
        default:
          qstringTmpBuf1 = i18n("Unknown");
          break;
      }

      switch (mymoneytransaction->state()) {
        case MyMoneyTransaction::Reconciled:
          qstringTmpBuf2 = i18n("Reconciled");
          break;
        case MyMoneyTransaction::Cleared:
          qstringTmpBuf2 = i18n("Cleared");
          break;
        case MyMoneyTransaction::Unreconciled:
          qstringTmpBuf2 = i18n("Unreconciled");
          break;
        default:
          qstringTmpBuf2 = i18n("Unknown");
          break;
      }

      qstringBuffer =
          /*
          QString::number(nCount)
          + ","
          */
          KGlobal::locale()->formatDate(mymoneytransaction->date(), true)
          + ","
          + qstringTmpBuf1
          + ","
          + mymoneytransaction->payee()
          + ","
          + qstringTmpBuf2
          + ","
          + ((mymoneytransaction->type()==MyMoneyTransaction::Credit) ?
            QString::number(mymoneytransaction->amount().amount()) :
            QString(""))
          + ","
          + ((mymoneytransaction->type()==MyMoneyTransaction::Credit) ?
            QString("") :
            QString::number(mymoneytransaction->amount().amount()))
          + ","
          + "\n"
          + ",,"
          + mymoneytransaction->number()
          + ","
          + (mymoneytransaction->categoryMajor() + ":" + mymoneytransaction->categoryMinor())
          + ","
          + mymoneytransaction->memo()
          + ",,\n";
      qtextstream << qstringBuffer;

      m_qprogressbar->setProgress(nCount);
    }
    qfile.close();
    m_qbuttonOk->setEnabled(true);
    m_qbuttonCancel->setEnabled(false);
  } else {
    KMessageBox::error(this, i18n("Unable to open export file for writing."));
  }
}

/** Cancel the import or export process */
void KCsvProgressDlg::slotCancelClicked()
{
  if (m_bProcessStarted)
    m_bStopFlag = true;
  else
    accept();
}

/** perform the import process */
void KCsvProgressDlg::performImport(void)
{
  KMessageBox::sorry(this, i18n("Sorry, Import for CSV files has not been implemented."));
  m_qprogressbar->setTotalSteps(m_mymoneyaccount->transactionCount());
  m_qprogressbar->setProgress(m_mymoneyaccount->transactionCount());
  m_qbuttonOk->setEnabled(true);
  m_qbuttonCancel->setEnabled(false);
}

/** Called when the user clicks on the Browser button */
void KCsvProgressDlg::slotBrowseClicked()
{
  QString qstring = KFileDialog::getSaveFileName(QString::null,"*.CSV");
  if (!qstring.isEmpty()) {
    m_qlineeditFile->setText(qstring);
    m_qbuttonRun->setEnabled(true);
  } else
    m_qbuttonRun->setEnabled(false);
}

/** Called when user clicks on the Run button */
void KCsvProgressDlg::slotRunClicked()
{
  m_qgroupbox->setEnabled(true);
  if (m_nType==0)
    performImport();
  else
    performExport();
}

/** Make sure the text input is ok */
void KCsvProgressDlg::slotFileTextChanged(const QString& text)
{
  if (!text.isEmpty()) {
    m_qlineeditFile->setText(text);
    m_qbuttonRun->setEnabled(true);
  } else
    m_qbuttonRun->setEnabled(false);
}

void KCsvProgressDlg::readConfig(void)
{
  KConfig *kconfig = KGlobal::config();
  kconfig->setGroup("Last Use Settings");
  m_qlineeditFile->setText(kconfig->readEntry("KCsvProgressDlg_LastFile", ""));
  if (m_qlineeditFile->text().length()>=1)
    m_qbuttonRun->setEnabled(true);
  else
    m_qbuttonRun->setEnabled(false);
}

void KCsvProgressDlg::writeConfig(void)
{
  KConfig *kconfig = KGlobal::config();
  kconfig->setGroup("Last Use Settings");
  kconfig->writeEntry("KCsvProgressDlg_LastFile", m_qlineeditFile->text());
  kconfig->sync();
}
