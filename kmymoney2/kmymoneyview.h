/***************************************************************************
                          kmymoneyview.h
                             -------------------
    copyright            : (C) 2000 by Michael Edwardes
    email                : Michael.Edwardes@students.dmu.ac.uk
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KMYMONEYVIEW_H
#define KMYMONEYVIEW_H

#include <kpopupmenu.h>
#include <qwidget.h>
#include <ktabctl.h>

#include "kmainview.h"
#include "kreconciledlg.h"
#include "kfindtransactiondlg.h"
#include "ktfindresultsdlg.h"
#include "kscheduleview.h"

// This class is used by KMainWnd as the main work horse of the app.
// It does fucking everything.  This will be rectified in a future release.
// When KDE 3 comes out !!!
class KMyMoneyView : public KTabCtl  {
   Q_OBJECT

private:
  KMainView *m_mainView;
  MyMoneyFile m_file;  // The interface to the transaction code
//  MyMoneyAccount accountIndex;
//  MyMoneyBank bankIndex;
  KReconcileDlg *reconcileDlg;  // This exists during app run time ?
  bool m_inReconciliation;  // True if the reconciliaton dialog needs updating when the user adds/deletes transactions
  bool m_reconcileInited;  // True if a reconciliation has already been completed this execution
  KFindTransactionDlg *transactionFindDlg;
  KTFindResultsDlg *transactionResultsDlg;
  KScheduleView *m_scheduledView;

  void loadDefaultCategories(void);  // Loads catgegories from default_categories.dat
  bool parseDefaultCategory(QString& line, bool& income, QString& name, QStringList& minors);
  void viewBankList(void);
  void viewTransactionList(void);
  bool checkTransactionDates(const MyMoneyTransaction *transaction, const bool enabled, const QDate start, const QDate end);
  bool checkTransactionAmount(const MyMoneyTransaction *transaction, const bool enabled, const QString id, const MyMoneyMoney amount);
  bool checkTransactionCredit(const MyMoneyTransaction *transaction, const bool enabled, const QString id);
  bool checkTransactionStatus(const MyMoneyTransaction *transaction, const bool enabled, const QString id);
  bool checkTransactionDescription(const MyMoneyTransaction *transaction, const bool enabled, const QString description, const bool isRegExp);
  bool checkTransactionNumber(const MyMoneyTransaction *transaction, const bool enabled, const QString number, const bool isRegExp);

public:
	KMyMoneyView(QWidget *parent=0, const char *name=0);
	~KMyMoneyView();
  bool fileOpen(void);
  void closeFile(void);
  bool readFile(QString);
  void saveFile(QString);
  bool dirty(void);
  void setDirty(bool dirty);
  void editCategories(void);
  void editPayees(void);
  void newFile(void);
  void viewPersonal(void);
  void viewUp(void);
  void fileInfo(void);
  void settingsLists();
  void accountFind();
  QString currentBankName(void);
  QString currentAccountName(void);
  void showTransactionInputBox(bool);

public slots:
  void slotBankNew(void);
  void slotAccountNew(void);
  void slotAccountReconcile(void);
  void slotAccountImportAscii(void);
  void slotAccountExportAscii(void);
	
protected slots:
  void slotTransactionListChanged();
  void slotAccountRightMouse(const MyMoneyAccount, bool);
  void slotAccountDoubleClick(const MyMoneyAccount);
  void slotBankRightMouse(const MyMoneyBank, bool);
  void slotAccountEdit();
  void slotAccountDelete();
  void slotBankEdit();
  void slotBankDelete();
  void slotReconcileFinished(bool success);
  void doTransactionSearch();

signals:
  void bankOperations(bool);
  void accountOperations(bool);
  void fileOperations(bool);
  void transactionOperations(bool);
};

#endif
