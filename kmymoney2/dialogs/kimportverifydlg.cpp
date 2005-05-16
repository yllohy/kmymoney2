/***************************************************************************
                          kimportverifydlg.cpp  -  description
                             -------------------
    begin                : Mon Jun 9 2003
    copyright            : (C) 2000-2003 by Thomas Baumgart
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
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

#include <qpushbutton.h>
#include <qtimer.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <klocale.h>
#include <kiconloader.h>
#include <kguiitem.h>
#include <kpushbutton.h>
#include <kmessagebox.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kimportverifydlg.h"
#include "../views/kgloballedgerview.h"
#include "../mymoney/mymoneyfile.h"

KImportVerifyDlg::KImportVerifyDlg(const MyMoneyAccount& account, QWidget *parent, const char *name)
 : KImportVerifyDlgDecl(parent,name,true),
   m_account(account)
{
  m_ledgerView->slotReloadView();
  m_ledgerView->slotSelectAccount(account.id());

  // add icons to buttons
  KIconLoader *il = KGlobal::iconLoader();
  KGuiItem okButtenItem( i18n("&Ok" ),
                    QIconSet(il->loadIcon("button_ok", KIcon::Small, KIcon::SizeSmall)),
                    i18n("Accepts the imported transactions and continues"),
                    i18n("Use this to accept all transactions and import them into the file."));
  buttonOk->setGuiItem(okButtenItem);

  KGuiItem cancelButtenItem( i18n( "&Cancel" ),
                    QIconSet(il->loadIcon("button_cancel", KIcon::Small, KIcon::SizeSmall)),
                    i18n("Cancel the import operation"),
                    i18n("Use this to abort the import and undo all changes made during import."));
  buttonCancel->setGuiItem(cancelButtenItem);

  // for now, we don't have online help
  buttonHelp->hide();
  
  connect(buttonOk, SIGNAL(clicked()), this, SLOT(slotOkClicked()));
  connect(buttonCancel, SIGNAL(clicked()), this, SLOT(slotCancelClicked()));
}

KImportVerifyDlg::~KImportVerifyDlg()
{
}

void KImportVerifyDlg::slotOkClicked(void)
{
  MyMoneyFile* file = MyMoneyFile::instance();
  MyMoneyTransactionFilter filter;
  QValueList<MyMoneyTransaction> list = file->transactionList(filter);
  QValueList<MyMoneyTransaction>::Iterator it_t;

  signalProgress(0, list.count(), i18n("Merging transactions ..."));
  // qDebug("step0");
  int cnt = 0;
  file->suspendNotify(true);
  for(it_t = list.begin(); it_t != list.end(); ++it_t) {
    if(!(*it_t).value("Imported").isEmpty()) {
      (*it_t).deletePair("Imported");
      // qDebug("  %s", (*it_t).id().data());
      file->modifyTransaction(*it_t);
      signalProgress(++cnt, 0);
    }
  }
  file->suspendNotify(false);
  
  accept();  
}

void KImportVerifyDlg::slotCancelClicked(void)
{
  if ( KMessageBox::warningYesNo(this, i18n("You are about to cancel the entire import.  Any changes you have made in this dialog will be lost.  Are you sure you want to cancel?"),i18n("Verify cancel"),KStdGuiItem::yes(),KStdGuiItem::no(),"CancelImportVerify") == KMessageBox::Yes )
    reject();
}

void KImportVerifyDlg::setProgressCallback(void(*callback)(int, int, const QString&))
{
  m_progressCallback = callback;
}

void KImportVerifyDlg::signalProgress(int current, int total, const QString& msg)
{
  if(m_progressCallback != 0)
    (*m_progressCallback)(current, total, msg);
}

int KImportVerifyDlg::exec(void)
{
  // force popup of message in 100ms so that the dialog is already visible
  QTimer::singleShot(100, this, SLOT(slotShowIntroduction(void)));
  
  return KImportVerifyDlgDecl::exec();
}

void KImportVerifyDlg::slotShowIntroduction(void) const
{
  // explain the user what's happening next
  KMessageBox::information(0, i18n(
                  "The imported data is displayed together with the "
                  "data already stored in the file. Imported transactions "
                  "are shown with a yellow background. Pressing \"OK\" will accept "
                  "all transactions, \"Cancel\" will remove all imported transactions.\n"
                  "You may modify the transactions - if need to - before you press \"OK\" "
                  "or even delete transactions that have been imported "
                  "incorrectly (e.g. duplicate transactions)."),
                  i18n("Import verification"),
                  "ImportVerificationIntroduction");
}

#include "kimportverifydlg.moc"
