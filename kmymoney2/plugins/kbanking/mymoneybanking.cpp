/***************************************************************************
                          mymoneybanking.cpp
                             -------------------
    begin                : Thu Aug 26 2004
    copyright            : (C) 2004 Martin Preuss
    email                : aquamaniac@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

// ----------------------------------------------------------------------------
// QT Includes

#include <qmessagebox.h>
#include <qlayout.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <klocale.h>
#include <kmessagebox.h>
#include <kgenericfactory.h>
#include <kaction.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kpopupmenu.h>
#include <kiconloader.h>

// ----------------------------------------------------------------------------
// Library Includes

#include <aqbanking/imexporter.h>
#include <gwenhywfar/logger.h>
#include <gwenhywfar/debug.h>
#include <kbanking/settings.h>
#include <kbanking/kbjobview.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneybanking.h"
#include <kmymoney/mymoneyfile.h>
#include <kmymoney/kmymoneyview.h>

K_EXPORT_COMPONENT_FACTORY( kmm_kbanking,
                            KGenericFactory<KBankingPlugin>( "kmm_kbanking" ) )

KBankingPlugin::KBankingPlugin(QObject *parent, const char* name, const QStringList&) :
  KMyMoneyPlugin::OnlinePlugin(parent, name )
{
  m_kbanking = new KMyMoneyBanking(this, "KMyMoney");
  if(m_kbanking) {
    GWEN_Logger_SetLevel(0, GWEN_LoggerLevelInfo);
    GWEN_Logger_SetLevel(AQBANKING_LOGDOMAIN, GWEN_LoggerLevelDebug);
    if(m_kbanking->init() == 0) {
      // Tell the host application to load my GUI component
      setInstance(KGenericFactory<KBankingPlugin>::instance());
      setXMLFile("kmm_kbanking.rc");

      // create view
      createJobView();

      // create actions
      createActions();

    } else {
      kdWarning() << "Could not initialize KBanking online banking interface" << endl;
      delete m_kbanking;
      m_kbanking = 0;
    }
  }
}

KBankingPlugin::~KBankingPlugin()
{
  if(m_kbanking) {
    m_kbanking->fini();
    delete m_kbanking;
  }
}

void KBankingPlugin::protocols(QStringList& protocolList) const
{
  std::list<std::string> list = m_kbanking->getActiveProviders();
  std::list<std::string>::iterator it;
  for(it = list.begin(); it != list.end(); ++it)
    protocolList << (*it);
}

void KBankingPlugin::createJobView(void)
{
  KMyMoneyViewBase* view = viewInterface()->addPage(i18n("Outbox"), "onlinebanking");
  QWidget* frm = dynamic_cast<QWidget*>(view->parent());
  QWidget* w = new KBJobView(m_kbanking, view, "JobView");
  viewInterface()->addWidget(view, w);
  connect(viewInterface(), SIGNAL(viewStateChanged(bool)), frm, SLOT(setEnabled(bool)));
  connect(viewInterface(), SIGNAL(accountSelected(const MyMoneyAccount&)), this, SLOT(slotAccountSelected(const MyMoneyAccount&)));
#if 0
  QFrame* frm = viewInterface()->addPage(i18n("Outbox"), i18n("Outbox"),DesktopIcon("outbox"));
  QVBoxLayout* layout = new QVBoxLayout(frm);
  layout->setSpacing( 6 );
  layout->setMargin( 0 );
  layout->addWidget(new KBJobView(m_kbanking, frm, "JobView"));
  connect(viewInterface(), SIGNAL(viewStateChanged(bool)), frm, SLOT(setEnabled(bool)));
#endif

}

void KBankingPlugin::createActions(void)
{
  new KAction(i18n("Configure Online &Banking..."), "configure", 0, this, SLOT(slotSettings()), actionCollection(), "settings_aqbanking");
  new KAction(i18n("AqBanking importer ..."), "", 0, this, SLOT(slotImport()), actionCollection(), "file_import_aqbanking");
  new KAction(i18n("Map to HBCI account..."), "news_subscribe", 0, this, SLOT(slotAccountOnlineMap()), actionCollection(), "account_map_aqbanking");
  new KAction(i18n("Online update using HBCI..."), "reload", 0, this, SLOT(slotAccountOnlineUpdate()), actionCollection(), "account_update_aqbanking");

  connect(viewInterface(), SIGNAL(viewStateChanged(bool)), action("file_import_aqbanking"), SLOT(setEnabled(bool)));
}

#if 0
void KBankingPlugin::createContextMenu(void)
{
  m_accountMenu = viewInterface()->accountContextMenu();
  if(m_accountMenu) {
    KIconLoader *il = KGlobal::iconLoader();
    m_accountMenu->insertSeparator();
    m_menuMapId = m_accountMenu->insertItem(il->loadIcon("news_subscribe", KIcon::Small),
                          i18n("Map to HBCI account..."),
                          this, SLOT(slotAccountOnlineMap()), 0);
    m_menuUpdateId = m_accountMenu->insertItem(il->loadIcon("reload", KIcon::Small),
                          i18n("Online update using HBCI..."),
                          this, SLOT(slotAccountOnlineUpdate()), 0);

    // make sure we receive a notification whenever an account is selected
    connect(viewInterface(), SIGNAL(accountSelectedForContextMenu(const MyMoneyAccount&)), this, SLOT(slotAccountSelected(const MyMoneyAccount&)));
  }
}
#endif

void KBankingPlugin::slotSettings(void)
{
  KBankingSettings bs(m_kbanking);
  if(bs.init())
    kdWarning() << "Error on ini of settings dialog." << endl;
  else {
    bs.exec();
    if(bs.fini())
      kdWarning() << "Error on fini of settings dialog." << endl;
  }
}

void KBankingPlugin::slotAccountOnlineMap(void)
{
  if(!m_account.id().isEmpty()) {
    MyMoneyFile* file = MyMoneyFile::instance();

    const MyMoneyInstitution &bank = file->institution(m_account.institutionId());
    if (bank.sortcode().isEmpty()) {
      KMessageBox::information(0,
        i18n("In order to map this account to an HBCI account, the account's institution "
        "must have a bank code assigned.  Please assign one before continuing."));
      return;
    }
    if (m_account.number().isEmpty()) {
      KMessageBox::information(0,
        i18n("In order to map this account to an HBCI account, this account "
        "must have an account number assigned."));
      return;
    }

    // open map dialog
    if (!m_kbanking->askMapAccount(m_account.id(),
                                  bank.sortcode().latin1(),
                                  m_account.number())) {
      // TODO: flash result
    }
    else {
      // TODO: flash result
    }
  }
}

const bool KBankingPlugin::accountIsMapped(const QCString& id)
{
  AB_ACCOUNT* ab_acc;
  ab_acc = AB_Banking_GetAccountByAlias(m_kbanking->getCInterface(), id);
  return ab_acc != 0;
}

void KBankingPlugin::slotAccountOnlineUpdate(void)
{
  if(!m_account.id().isEmpty()) {

    // TODO: get last statement date
    if(m_kbanking->requestBalance(m_account.id())) {
      // executeMyJobs();
      if(m_kbanking->requestTransactions(m_account.id(), QDate(), QDate())) {
        // TODO: flash status
      }
    }
  }
}

void KBankingPlugin::slotAccountSelected(const MyMoneyAccount& acc)
{
  MyMoneyInstitution institution;
  bool state = false;
  m_account = acc;

  action("account_map_aqbanking")->setEnabled(false);
  action("account_update_aqbanking")->setEnabled(false);

  if(!MyMoneyFile::instance()->isStandardAccount(acc.id())) {
    switch(m_account.accountGroup()) {
      case MyMoneyAccount::Asset:
      case MyMoneyAccount::Liability:
        state = true;
        if(acc.number().isEmpty() || acc.institutionId().isEmpty())
          state = false;
        else {
          try {
            institution = MyMoneyFile::instance()->institution(acc.institutionId());
            if(institution.sortcode().isEmpty())
              state = false;
          } catch(MyMoneyException* e) {
            state = false;
            delete e;
          }
        }
        break;

      default:
        break;
    }
  }

  if(state == true) {
    if(accountIsMapped(acc.id())) {
      action("account_update_aqbanking")->setEnabled(true);
    } else {
      action("account_map_aqbanking")->setEnabled(true);
    }
  }
}

void KBankingPlugin::slotImport(void)
{
  if(!m_kbanking->interactiveImport())
    kdWarning() << "Error on import dialog" << endl;
}

bool KBankingPlugin::importStatement(MyMoneyStatement& s)
{
  return statementInterface()->import(s);
}





KMyMoneyBanking::KMyMoneyBanking(KBankingPlugin* parent, const char* appname, const char* fname) :
  KBanking(appname, fname),
  m_parent(parent)
{
}

const AB_ACCOUNT_STATUS* KMyMoneyBanking::_getAccountStatus(AB_IMEXPORTER_ACCOUNTINFO *ai)
{
  const AB_ACCOUNT_STATUS *ast;
  const AB_ACCOUNT_STATUS *best;

  best=0;
  DBG_NOTICE(0, "Checking account (%s/%s)",
       AB_ImExporterAccountInfo_GetBankCode(ai),
       AB_ImExporterAccountInfo_GetAccountNumber(ai));
  ast=AB_ImExporterAccountInfo_GetFirstAccountStatus(ai);
  while(ast) {
    if (!best)
      best=ast;
    else {
      const GWEN_TIME *tiBest;
      const GWEN_TIME *ti;

      tiBest=AB_AccountStatus_GetTime(best);
      ti=AB_AccountStatus_GetTime(ast);

      if (!tiBest) {
        best=ast;
      }
      else {
        if (ti) {
          double d;

          /* we have two times, compare them */
          d=GWEN_Time_Diff(ti, tiBest);
          if (d>0)
            /* newer */
            best=ast;
        }
      }
    }
    ast=AB_ImExporterAccountInfo_GetNextAccountStatus(ai);
  } /* while */
  return best;
}



void KMyMoneyBanking::_xaToStatement(const AB_TRANSACTION *t,
                                     MyMoneyStatement &ks){
  const GWEN_STRINGLIST *sl;
  QString s;
  const char *p;
  const AB_VALUE *val;
  const GWEN_TIME *ti;
  const GWEN_TIME *startTime=0;
  MyMoneyStatement::Transaction kt;

  // payee
  s="";
  sl=AB_Transaction_GetRemoteName(t);
  if (sl) {
    GWEN_STRINGLISTENTRY *se;

    se=GWEN_StringList_FirstEntry(sl);
    if (se) {
      p=GWEN_StringListEntry_Data(se);
      assert(p);
      s=p;
    }
  }
  kt.m_strPayee=QString::fromUtf8(s);

  // memo
  s="";
  sl=AB_Transaction_GetPurpose(t);
  if (sl) {
    GWEN_STRINGLISTENTRY *se;

    se=GWEN_StringList_FirstEntry(sl);
    while (se) {
      p=GWEN_StringListEntry_Data(se);
      assert(p);
      if (!s.isEmpty())
        s+=" ";
      s+=p;
      se=GWEN_StringListEntry_Next(se);
    } // while
  }
  kt.m_strMemo=QString::fromUtf8(s);

  // date
  ti=AB_Transaction_GetDate(t);
  if (!ti)
    ti=AB_Transaction_GetValutaDate(t);
  if (ti) {
    int year, month, day;

    if (!startTime)
      startTime=ti;
    else {
      if (GWEN_Time_Diff(ti, startTime)<0)
        startTime=ti;
    }

    if (!GWEN_Time_GetBrokenDownDate(ti, &day, &month, &year))
      kt.m_datePosted=QDate(year, month+1, day);
  } else {
    DBG_WARN(0, "No date for transaction");
  }

  // value
  val=AB_Transaction_GetValue(t);
  if (val) {
    if (ks.m_strCurrency.isEmpty()) {
      p=AB_Value_GetCurrency(val);
      if (p)
        ks.m_strCurrency=p;
    } else {
      p=AB_Value_GetCurrency(val);
      if (p)
        s=p;
      if (ks.m_strCurrency.lower()!=s.lower()) {
        // TODO: handle currency difference
        DBG_ERROR(0, "Mixed currencies currently not allowed");
      }
    }

    kt.m_moneyAmount=AB_Value_GetValue(val);
  } else {
    DBG_WARN(0, "No value for transaction");
  }

  if (startTime) {
    int year, month, day;

    if (!GWEN_Time_GetBrokenDownDate(startTime, &day, &month, &year)) {
      QDate d(year, month+1, day);

      if (!ks.m_dateBegin.isValid())
        ks.m_dateBegin=d;
      else if (d<ks.m_dateBegin)
        ks.m_dateBegin=d;

      if (!ks.m_dateEnd.isValid())
        ks.m_dateEnd=d;
      else if (d>ks.m_dateEnd)
        ks.m_dateEnd=d;
    }
  }
  else {
    DBG_WARN(0, "No date in current transaction");
  }

  // store transaction
  DBG_NOTICE(0, "Adding transaction");
  ks.m_listTransactions+=kt;
}



bool KMyMoneyBanking::importAccountInfo(AB_IMEXPORTER_ACCOUNTINFO *ai)
{
  QString s;
  const char *p;
  const AB_TRANSACTION *t;
  MyMoneyStatement ks;
  const AB_ACCOUNT_STATUS *ast;
  const AB_VALUE *val;
  const GWEN_TIME *ti;

  DBG_NOTICE(0, "Importing account...");

  // account number
  p=AB_ImExporterAccountInfo_GetAccountNumber(ai);
  if (p)
    ks.m_strAccountNumber=p;

  // account name
  p=AB_ImExporterAccountInfo_GetAccountName(ai);
  if (p)
    ks.m_strAccountName=p;

  // account status
  ast=_getAccountStatus(ai);
  if (ast) {
    const AB_BALANCE *bal;

    bal=AB_AccountStatus_GetBookedBalance(ast);
    if (!bal)
      bal=AB_AccountStatus_GetNotedBalance(ast);
    if (bal) {
      val=AB_Balance_GetValue(bal);
      if (val) {
        DBG_NOTICE(0, "Importing balance");
        ks.m_moneyClosingBalance=AB_Value_GetValue(val);
        p=AB_Value_GetCurrency(val);
        if (p)
          ks.m_strCurrency=p;
      }
      ti=AB_Balance_GetTime(bal);
      if (ti) {
        int year, month, day;

        if (!GWEN_Time_GetBrokenDownDate(ti, &day, &month, &year))
          ks.m_dateEnd=QDate(year, month+1, day);
      }
      else {
        DBG_WARN(0, "No time for balance");
      }
    }
    else {
      DBG_WARN(0, "No account balance");
    }
  }
  else {
    DBG_WARN(0, "No account status");
  }

  // get all transactions
  t=AB_ImExporterAccountInfo_GetFirstTransaction(ai);
  while(t) {
    if (AB_Split_List_GetCount(AB_Transaction_GetSplits(t))) {
      AB_SPLIT *sp;

      sp=AB_Split_List_First(AB_Transaction_GetSplits(t));
      while(sp) {
        AB_TRANSACTION *nt;

        /* create one transaction for any split */
        nt=AB_Transaction_dup(t);
        /* clear split list */
        AB_Split_List_Clear(AB_Transaction_GetSplits(nt));
        /* get data from split, store it in statement */
        AB_Transaction_SetRemoteName(nt, AB_Split_GetName(sp));
        AB_Transaction_SetPurpose(nt, AB_Split_GetPurpose(sp));
        AB_Transaction_SetValue(nt, AB_Split_GetValue(sp));

        /* store this copy of the transaction */
        _xaToStatement(nt, ks);
        /* free copy */
        AB_Transaction_free(nt);
        /* next split */
        sp=AB_Split_List_Next(sp);
      } /* while */
    } else
      _xaToStatement(t, ks);

    t=AB_ImExporterAccountInfo_GetNextTransaction(ai);
  }

  // import it
  if(!m_parent->importStatement(ks)) {
    if (QMessageBox::critical(0,
                              i18n("Critical Error"),
                              i18n("Error importing statement."),
                              i18n("Continue"),
                              i18n("Abort"), 0, 0)!=0) {
      DBG_ERROR(0, "User aborted");
      return false;
    }
  }
  return true;
}




#include "mymoneybanking.moc"
