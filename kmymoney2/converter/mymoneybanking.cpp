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

// ----------------------------------------------------------------------------
// KDE Includes

#include <klocale.h>

// ----------------------------------------------------------------------------
// Library Includes

#ifdef HAVE_KBANKING
#include <aqbanking/imexporter.h>
#include <gwenhywfar/logger.h>
#include <gwenhywfar/debug.h>
#include <kbanking/settings.h>
#endif

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneybanking.h"
#include "../kmymoney2.h"
#include "../mymoney/mymoneystatement.h"

static KMyMoneyBanking* _instance = 0;
static bool _available = 0;

KMyMoneyBanking::KMyMoneyBanking(const char *appname, const char *fname)
  : KBanking(appname, fname)
{
}

KMyMoneyBanking::~KMyMoneyBanking()
{
  _instance = 0;
}

KMyMoneyBanking* KMyMoneyBanking::instance(void)
{
  if(!_instance) {
#ifdef HAVE_KBANKING
    GWEN_Logger_SetLevel(0, GWEN_LoggerLevelInfo);
    GWEN_Logger_SetLevel(AQBANKING_LOGDOMAIN, GWEN_LoggerLevelInfo);
#endif

    _instance = new KMyMoneyBanking("kmymoney");
    _available = false;
    Q_CHECK_PTR(_instance);

#ifdef HAVE_KBANKING
    if(_instance->init()) {
      qWarning("Could not initialize KBanking online banking interface");
    } else {
      _available = true;
    }
    _instance->m_jobView = 0;
#endif
  }
  return _instance;
}

const bool KMyMoneyBanking::isAvailable(void) const
{
  return _available;
}

void KMyMoneyBanking::settingsDialog(QWidget* parent, const char* name, QWidget::WFlags fl)
{
#ifdef HAVE_KBANKING
  KBankingSettings bs(this, parent, name, fl);

  if (bs.init()) {
    qWarning("Error on ini of settings dialog.");
  } else {
    bs.exec();
    if (!bs.fini()) {
      qWarning("Error on fini of settings dialog.");
    }
  }
#endif
}

QWidget* KMyMoneyBanking::createJobView(QWidget* parent, const char *name)
{
#ifdef HAVE_KBANKING
  m_jobView = new JobView(this, parent, name);
  return m_jobView;
#else
  return new QWidget(parent, name);
#endif
}

void KMyMoneyBanking::updateJobView(void)
{
#ifdef HAVE_KBANKING
  if(m_jobView)
    m_jobView->slotQueueUpdated();
#endif
}


#ifdef HAVE_KBANKING
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

bool KMyMoneyBanking::importAccountInfo(AB_IMEXPORTER_ACCOUNTINFO *ai)
{
  QString s;
  const char *p;
  AB_TRANSACTION *t;
  MyMoneyStatement ks;
  const GWEN_TIME *startTime;
  const AB_ACCOUNT_STATUS *ast;
  const AB_VALUE *val;
  const GWEN_TIME *ti;

  startTime=0;

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
    MyMoneyStatement::Transaction kt;
    const GWEN_STRINGLIST *sl;

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
    }
    else {
      DBG_WARN(0, "No date for transaction");
    }

    // value
    val=AB_Transaction_GetValue(t);
    if (val) {
      if (ks.m_strCurrency.isEmpty()) {
        p=AB_Value_GetCurrency(val);
        if (p)
          ks.m_strCurrency=p;
      }
      else {
        p=AB_Value_GetCurrency(val);
        if (p)
          s=p;
        if (ks.m_strCurrency.lower()!=s.lower()) {
          // TODO: handle currency difference
          DBG_ERROR(0, "Mixed currencies currently not allowed");
          break;
        }
      }

      kt.m_moneyAmount=AB_Value_GetValue(val);
    }
    else {
      DBG_WARN(0, "No value for transaction");
    }
    // store transaction
    DBG_NOTICE(0, "Adding transaction");
    ks.m_listTransactions+=kt;

    t=AB_ImExporterAccountInfo_GetNextTransaction(ai);
  } /* while */

  if (startTime) {
    int year, month, day;

    if (!GWEN_Time_GetBrokenDownDate(startTime, &day, &month, &year))
      ks.m_dateBegin=QDate(year, month+1, day);
  }
  else {
    DBG_WARN(0, "No start date");
    ks.m_dateBegin=ks.m_dateEnd;
  }

  // import it
  if (!kmymoney2->slotStatementImport(ks)) {
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

#endif


