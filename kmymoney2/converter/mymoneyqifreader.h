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

  const QString scanFileForAccount(void);

  const MyMoneyAccount& account() const { return m_account; };

private:
  void runFilter(void);

  /**
    * This method scans a transaction contained in
    * a QIF file formatted as an account record. This
    * format is used by MS-Money.
    *
    * @param lines A list of all the lines for this transaction
    */
  void processMSAccountEntry(const QStringList& lines);

  /**
    * This method scans an account record as specified
    * by Quicken.
    *
    * @param lines A list of all the lines for this account
    */
  void processAccountEntry(const QStringList& lines);

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
    *
    * @return QString with the remainder of the line or empty if
    *         @p id is not found in @p lines
    */
  const QString extractLine(const QChar id, const QStringList& lines) const;

private:
  QString           m_originalFilename;
  QString           m_filename;
  MyMoneyQifProfile m_qifProfile;
  KTempFile         m_tempFile;
  MyMoneyAccount    m_account;
};

#endif
