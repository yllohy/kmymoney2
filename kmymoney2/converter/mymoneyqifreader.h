/***************************************************************************
                          mymoneyqifreader.h  -  description
                             -------------------
    begin                : Mon Jan 27 2003
    copyright            : (C) 2000-2003 by Michael Edwardes
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

#ifndef MYMONEYQIFREADER_H
#define MYMONEYQIFREADER_H

// ----------------------------------------------------------------------------
// QT Headers

#include <qobject.h>
#include <qstring.h>
#include <qstringlist.h>

// ----------------------------------------------------------------------------
// KDE Headers

#include <ktempfile.h>

// ----------------------------------------------------------------------------
// Project Headers

#include "mymoneyqifprofile.h"
#include "../mymoney/mymoneyaccount.h"

/**
  * @author Thomas Baumgart
  */

class MyMoneyQifReader : public QObject
{
  Q_OBJECT

public: 
  MyMoneyQifReader();
  ~MyMoneyQifReader();

  /**
    * This method is used to store the filename into the object.
    * The file should exist. If it does and an external filter
    * program is specified with the current selected profile,
    * the file is send through this filter and the result
    * is stored in the m_tempFile file.
    *
    * @param name path and name of the file to be imported
    */
  void setFilename(const QString& name);

  /**
    * This method is used to store the name of the profile into the object.
    * The selected profile will be loaded if it exists. If an external
    * filter program is specified with the current selected profile,
    * the file is send through this filter and the result
    * is stored in the m_tempFile file.
    *
    * @param name QString reference to the name of the profile
    */
  void setProfile(const QString& name);

  void import(void);
  
  const QString scanFileForAccount(void);

  const MyMoneyAccount& account() const { return m_account; };

private:
  void runFilter(void);

  /**
    * This method scans a transaction contained in
    * a QIF file formatted as an account record. This
    * format is used by MS-Money. If the specific data
    * is not found, then the data in the entry is treated
    * as a transaction. In this case, the user will be asked to
    * specify the account to which the transactions should be imported.
    *
    * @param lines A list of all the lines for this transaction
    * @param accountType see MyMoneyAccount() for details. Defaults to MyMoneyAccount::Checkings
    */
  void processMSAccountEntry(const QStringList& lines, const MyMoneyAccount::accountTypeE accountType = MyMoneyAccount::Checkings);

  /**
    * This method scans an account record as specified
    * by Quicken.
    *
    * @param lines A list of all the lines for this account
    */
  void processAccountEntry(const QStringList& lines);

  /**
    * This method scans a category record as specified
    * by Quicken.
    *
    * @param lines A list of all the lines for this category
    */
  void processCategoryEntry(const QStringList& lines);
  
  /**
    * This method scans a transaction record as specified
    * by Quicken.
    *
    * @param lines A list of all the lines for this category
    */
  void processTransactionEntry(const QStringList& lines);

  /**
    * This method reads lines from the QTextStream @p s into
    * a QStringList object until it encounters a line
    * containing a caret (^) as the first character or EOF.
    * The collected lines are returned.
    *
    * @param s QTextStream to read from
    *
    * @return QStringList containing the lines read
    */
  const QStringList readEntry(QTextStream& s) const;

  /**
    * This method extracts the line beginning with the letter @p id
    * from the lines contained in the QStringList object @p lines.
    * An empty QString is returned, if the line is not found.
    *
    * @param id QChar containing the letter to be found
    * @param lines QStringList containing the lines
    * @param cnt return cnt'th of occurance of id in lines. cnt defaults to 1.
    *
    * @return QString with the remainder of the line or empty if
    *         @p id is not found in @p lines
    */
  const QString extractLine(const QChar id, const QStringList& lines, int cnt = 1) const;

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
    * If an account has been selected, account will be set to contain it's data.
    * If the skip operation was requested, account will be empty.
    *
    * Depending on @p mode the bahaviour of this method is slightly different.
    * The following table shows the dependencies:
    *
    * @code
    * case                              mode            operation
    * -----------------------------------------------------------------------------
    * account with same name exists     Create          returns immediately
    *                                                   m_account contains data
    *                                                   of existing account
    *
    * account does not exist            Create          immediately calls dialog
    *                                                   to create account
    *
    * account with same name exists     Select          User will be asked if
    *                                                   he wants to use the existing
    *                                                   account or create a new one
    *
    * account does not exist            Select          User will be asked to
    *                                                   select a different account
    *                                                   or create a new one
    *
    * @endcode
    *
    * @param mode Is either Create or Select depending on the above table
    * @param account Reference to MyMoneyAccount object
    */
  void selectOrCreateAccount(const SelectCreateMode mode, MyMoneyAccount& account);

private:
  QString                 m_originalFilename;
  QString                 m_filename;
  MyMoneyQifProfile       m_qifProfile;
  KTempFile               m_tempFile;
  MyMoneyAccount          m_account;
  bool                    m_skipAccount;
  unsigned long           m_transactionsSkipped;
  unsigned long           m_transactionsProcessed;
  QStringList             m_dontAskAgain;
  QMap<QString, QCString> m_accountTranslation;
};

#endif
