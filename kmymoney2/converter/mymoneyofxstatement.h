/***************************************************************************
                          mymoneyofxstatement.cpp
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

#ifndef MYMONEYOFXSTATEMENT_H
#define MYMONEYOFXSTATEMENT_H

#include <qstringlist.h>
#include "../mymoney/mymoneystatement.h"

/**
  * This is a collection of MyMoneyStatements as loaded from an OFX file.
  * @author Ace Jones
  */
class MyMoneyOfxStatement: public QValueList<MyMoneyStatement>
{
public:
    MyMoneyOfxStatement(const QString& filename);
    ~MyMoneyOfxStatement();

    void addnew(void) { push_back(MyMoneyStatement()); }
    bool isValid(void) const { return m_valid; }
    void setValid(void) { m_valid = true; }
    void addInfo(const QString& _msg ) { m_infos+=_msg; }
    void addWarning(const QString& _msg )  { m_warnings+=_msg; }
    void addError(const QString& _msg )  { m_errors+=_msg; }
    const QStringList& infos(void) const { return m_infos; }
    const QStringList& warnings(void) const { return m_warnings; }
    const QStringList& errors(void) const { return m_errors; }

    static bool isOfxFile(const QString&);
private:
  bool m_valid;
  QStringList m_infos;
  QStringList m_warnings;
  QStringList m_errors;

};


#endif
