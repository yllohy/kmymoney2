/***************************************************************************
                          mymoneytemplate.h  -  description
                             -------------------
    begin                : Sat Aug 14 2004
    copyright            : (C) 2004 by Thomas Baumgart
    email                : ipwizard@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MYMONEYTEMPLATE_H
#define MYMONEYTEMPLATE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qdom.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kurl.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "../mymoney/mymoneyaccount.h"
#include "../mymoney/mymoneyfile.h"

/**
  * @author Thomas Baumgart
  */

/**
  * This class represents an account template handler. It is capable
  * to read an XML formatted account template file and import it into
  * the current engine. Also, it can save the current account structure
  * of the engine to an XML formatted template file.
  */
class MyMoneyTemplate
{
public:
  MyMoneyTemplate();
  MyMoneyTemplate(const KURL& url);
  ~MyMoneyTemplate();

  const bool loadTemplate(const KURL& url);
  const bool saveTemplate(const KURL& url);
  const bool importTemplate(void(*callback)(int, int, const QString&));
  const bool exportTemplate(void(*callback)(int, int, const QString&));

protected:
  const bool loadDescription(void);
  const bool createAccounts(MyMoneyAccount& parent, QDomNode account);
  const bool setFlags(MyMoneyAccount& acc, QDomNode flags);
  const bool saveToLocalFile(QFile* qfile);
  const bool addAccountStructure(QDomElement& parent, const MyMoneyAccount& acc);

  /**
    * This method is used to update the progress information. It
    * checks if an appropriate function is known and calls it.
    *
    * For a parameter description see KMyMoneyView::progressCallback().
    */
  void signalProgress(int current, int total, const QString& = "");

private:
  QDomDocument    m_doc;
  QDomNode        m_accounts;
  QString         m_title;
  QString         m_shortDesc;
  QString         m_longDesc;
  KURL            m_source;
  void            (*m_progressCallback)(int, int, const QString&);
  int             m_accountsRead;
};

#endif
