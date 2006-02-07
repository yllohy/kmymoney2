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
#ifndef MYMONEYBANKING_H
#define MYMONEYBANKING_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

class KAction;
class KBanking;

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoney/kmymoneyplugin.h>
#include <kmymoney/mymoneyaccount.h>
#include <kbanking/kbanking.h>

/**
  * This class represents the KBanking plugin towards KMymoney.
  * All GUI related issues are handled in this object.
  */
class KBankingPlugin : public KMyMoneyPlugin::OnlinePlugin
{
  Q_OBJECT
public:
  KBankingPlugin(QObject* parent, const char* name, const QStringList&);
  virtual ~KBankingPlugin();

  bool importStatement(MyMoneyStatement& s);

  void protocols(QStringList& protocolList) const;

protected:
  /**
    * creates the job view and hooks it into the main view
    */
  void createJobView(void);

  /**
    * creates the action objects available through the application menus
    */
  void createActions(void);

  /**
    * creates the context menu
    */
  void createContextMenu(void);

  /**
    * checks whether a given KMyMoney account with id @p id is
    * already mapped or not.
    *
    * @param id KMyMoney internal id of the account
    * @retval false account is not mapped to an AqBanking account
    * @retval true account is mapped to an AqBanking account
    */
  const bool accountIsMapped(const QCString& id);

protected slots:
  void slotSettings(void);
  void slotImport(void);

  /**
    * Called when an account has been selected by the application
    * and the context menu has to be adjusted.
    */
  void slotAccountSelected(const MyMoneyAccount& acc);

  /**
    * Called by the context menu created in createContextMenu().
    * Calls KBanking to set up HBCI mappings.
    */
  void slotAccountOnlineMap(void);

  /**
    * Called by the context menu created in createContextMenu().  Calls KBanking to update
    * the account.  Only valid if the account is mapped for HBCI.
    */
  void slotAccountOnlineUpdate(void);

private:
  MyMoneyAccount        m_account;
  KAction*              m_configAction;
  KAction*              m_importAction;
  KBanking*             m_kbanking;
  KPopupMenu*           m_accountMenu;
  int                   m_menuMapId;
  int                   m_menuUpdateId;
};

/**
  * This class is the special implementation to glue the KBanking class
  * with the KMyMoneyPlugin structure.
  */
class KMyMoneyBanking : public KBanking
{

public:
  KMyMoneyBanking(KBankingPlugin* parent, const char* appname, const char* fname = 0);
  virtual ~KMyMoneyBanking() {};

  bool importAccountInfo(AB_IMEXPORTER_ACCOUNTINFO *ai);

protected:
  const AB_ACCOUNT_STATUS* _getAccountStatus(AB_IMEXPORTER_ACCOUNTINFO *ai);
  void _xaToStatement(const AB_TRANSACTION *t,
                      MyMoneyStatement &ks);

private:
  KBankingPlugin* m_parent;

};

#endif
