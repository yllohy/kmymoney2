/***************************************************************************
                          mymoneystatementreader
                             -------------------
    begin                : Mon Aug 30 2004
    copyright            : (C) 2000-2004 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
                           Ace Jones <acejones@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MYMONEYSTATEMENTREADER_H
#define MYMONEYSTATEMENTREADER_H

// ----------------------------------------------------------------------------
// QT Headers

#include <qobject.h>
#include <qstring.h>
#include <qstringlist.h>

// ----------------------------------------------------------------------------
// KDE Headers

#include <ktempfile.h>
#include <kprocess.h>

// ----------------------------------------------------------------------------
// Project Headers

#include "mymoneyqifprofile.h"
#include "../mymoney/mymoneyaccount.h"
#include "../mymoney/mymoneystatement.h"

/** This is a pared-down version of a MyMoneyQifReader object
  *
  * @author Ace Jones 
  */

class MyMoneyStatementReader : public QObject
{
  Q_OBJECT

public: 
  MyMoneyStatementReader();
  ~MyMoneyStatementReader();

  /**
    * This method actually starts the import of data from the selected file
    * into the MyMoney engine.
    *
    * Make sure to connect the signal importFinished() to detect when
    * the import actually ended. Call the method finishImport() to clean
    * things up and get the overall result of the import.
    *
    * @retval true the import was started successfully
    * @retval false the import could not be started.
    */
  const bool startImport(const MyMoneyStatement& s);

  /**
    * This method must be called once the signal importFinished() has
    * been emitted. It will clean up the reader state and determines
    * the actual return code of the import.
    *
    * @retval true Import was successful.
    * @retval false Import failed because the filter program terminated
    *               abnormally or the user aborted the import process.
    */
  const bool finishImport(void);

  /**
    * This method is used to modify the auto payee creation flag.
    * If this flag is set, records for payees that are not currently
    * found in the engine will be automatically created with no
    * further user interaction required. If this flag is no set,
    * the user will be asked if the payee should be created or not.
    * If the MyMoneyQifReader object is created auto payee creation
    * is turned off.
    *
    * @param create flag if this feature should be turned on (@p true)
    *               or turned off (@p false)
    */
  void setAutoCreatePayee(const bool create);
  
  const MyMoneyAccount& account() const { return m_account; };

  void setProgressCallback(void(*callback)(int, int, const QString&));

private:
  /**
    * This method is used to update the progress information. It
    * checks if an appropriate function is known and calls it.
    *
    * For a parameter description see KMyMoneyView::progressCallback().
    */
  void signalProgress(int current, int total, const QString& = "");
  
  /**
    * This method scans the m_qifEntry object as a transaction record specified
    * by Quicken.
    */
  void processTransactionEntry(const MyMoneyStatement::Transaction&);

  enum SelectCreateMode {
    Create = 0,
    Select
  };
  /**
    * This method is used to find an account using the account's name
    * stored in @p account in the current MyMoneyFile object. If it does not
    * exist, the user has the chance to create it or to skip processing
    * of this account.
    *
    * Please see the documentation for this function in MyMoneyQifReader
    *
    * @param mode Is either Create or Select depending on the above table
    * @param account Reference to MyMoneyAccount object
    */
  bool selectOrCreateAccount(const SelectCreateMode mode, MyMoneyAccount& account);

signals:
  /**
    * This signal will be emitted when the import is finished.
    */
  void importFinished(void);
  
private:
  
  MyMoneyAccount          m_account;
  QStringList             m_dontAskAgain;
  bool                    m_skipAccount;
  bool                    m_userAbort;
  bool                    m_autoCreatePayee;
          
  void (*m_progressCallback)(int, int, const QString&);
};

#endif
