/***************************************************************************
                          mymoneybanking.h
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
#ifndef KMYMONEYBANKING_H
#define KMYMONEYBANKING_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_KBANKING
#  include <kbanking/kbanking.h>
#  include <kbanking/jobview.h>
#else
  class KBanking
  {
  public:
    KBanking(const char *appname, const char *fname) {};
    virtual ~KBanking() {};
    bool askMapAccount(const char *id, const char *bankCode, const char *accountId) { return false; };
    bool requestBalance(const char *accountId) { return false; };
    bool requestTransactions(const char *accountId,
                            const QDate &fromDate,
                            const QDate &toDate) { return false; };
    bool interactiveImport(void) { return false; }
    int fini(void) { return 1; };
  };
#endif

class KMyMoneyBanking: public KBanking
{
public:
  /**
    * This method returns a pointer to a KMyMoneyBanking singleton object.
    * This ensures, that only one KMyMoneyBanking object can exist per task.
    */
  static KMyMoneyBanking* instance(void);

  virtual ~KMyMoneyBanking();

  /**
    * This method returns status information if KBanking is available
    * at runtime and initialized correctly or not.
    *
    * @retval true KBanking is available and initialized
    * @retval false KBanking is not available
    */
  const bool isAvailable(void) const;

  /**
    * This method starts the KBaning settings dialog if KBanking::isAvailable()
    * returns true. Otherwise, it just returns.
    */
  void settingsDialog(QWidget* parent, const char* name = 0, QWidget::WFlags fl = 0);

  /**
    * This method creates a KBanking JobView object if KBanking::isAvailable()
    * returns true. Otherwise, it returns a simple QWidget.
    */
  QWidget* createJobView(QWidget* parent, const char* name = 0);

  /**
    * This method updates the jobview created with createJobView() if KBanking::available()
    * returns true. Otherwise, it just returns.
    */
  void updateJobView(void);

private:
  KMyMoneyBanking(const char *appname, const char *fname=0);

#ifdef HAVE_KBANKING
public:
  virtual bool importAccountInfo(AB_IMEXPORTER_ACCOUNTINFO *ai);

private:
  const AB_ACCOUNT_STATUS *_getAccountStatus(AB_IMEXPORTER_ACCOUNTINFO *ai);

private:
  JobView*     m_jobView;
#endif

};

#endif
