/***************************************************************************
                          mymoneytemplate.cpp  -  description
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

#include "kdecompat.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <qfile.h>
#include <qapplication.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <klocale.h>
#include <kmessagebox.h>
#include <kio/netaccess.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneytemplate.h"

MyMoneyTemplate::MyMoneyTemplate()
{
}

MyMoneyTemplate::MyMoneyTemplate(const KURL& url)
{
  loadTemplate(url);
}

MyMoneyTemplate::~MyMoneyTemplate()
{
}

const bool MyMoneyTemplate::loadTemplate(const KURL& url)
{
  QString filename;

#if KDE_IS_VERSION(3,2,0)
  if(!url.isValid()) {
#else
  if(url.isMalformed()) {
#endif
    qDebug("Invalid template URL '%s'", url.url().latin1());
    return false;
  }

  m_source = url;
  if(url.isLocalFile()) {
    filename = url.path();

  } else {
    bool rc;
#if KDE_IS_VERSION(3,2,0)
    rc = KIO::NetAccess::download(url, filename, qApp->mainWidget());
#else
    rc = KIO::NetAccess::download(url, filename);
#endif
    if(!rc) {
      KMessageBox::detailedError(qApp->mainWidget(),
             i18n("Error while loading file '%1'!").arg(url.url()),
             KIO::NetAccess::lastErrorString(),
             i18n("File access error"));
      return false;
    }
  }

  bool rc = true;
  QFile file(filename);
  if(file.open(IO_ReadOnly)) {
    QString errMsg;
    int errLine, errColumn;
    if(!m_doc.setContent(&file, &errMsg, &errLine, &errColumn)) {
      QString msg=i18n("Error while reading template file in line %1, column %2").arg(errLine).arg(errColumn);
      KMessageBox::detailedError(qApp->mainWidget(), msg, errMsg, i18n("Template Error"));
      rc = false;
    } else {
      rc = loadDescription();
    }
    file.close();
  } else {
    KMessageBox::sorry(qApp->mainWidget(), i18n("File '%1' not found!").arg(filename));
    rc = false;
  }

  // if a temporary file was constructed by NetAccess::download,
  // then it will be removed with the next call. Otherwise, it
  // stays untouched on the local filesystem
  KIO::NetAccess::removeTempFile(filename);
  return rc;
}

const bool MyMoneyTemplate::loadDescription(void)
{
  int validMask = 0x00;
  const int validAccount = 0x01;
  const int validTitle = 0x02;
  const int validShort = 0x04;
  const int validLong = 0x08;
  const int invalid = 0x10;
  const int validHeader = 0x0F;

  QDomElement rootElement = m_doc.documentElement();
  if(!rootElement.isNull()
  && rootElement.tagName() == "kmymoney-account-template") {
    QDomNode child = rootElement.firstChild();
    while(!child.isNull() && child.isElement()) {
      QDomElement childElement = child.toElement();
      // qDebug("MyMoneyTemplate::import: Processing child node %s", childElement.tagName().data());
      if(childElement.tagName() == "accounts") {
        m_accounts = childElement.firstChild();
        validMask |= validAccount;
      } else if(childElement.tagName() == "title") {
        m_title = childElement.text();
        validMask |= validTitle;
      } else if(childElement.tagName() == "shortdesc") {
        m_shortDesc = childElement.text();
        validMask |= validShort;
      } else if(childElement.tagName() == "longdesc") {
        m_longDesc = childElement.text();
        validMask |= validLong;
      } else {
        KMessageBox::error(qApp->mainWidget(), QString("<p>")+i18n("Invalid tag <b>%1</b> in template file <b>%2</b>!").arg(childElement.tagName()).arg(m_source.prettyURL()));
        validMask |= invalid;
      }
      child = child.nextSibling();
    }
  }
  return validMask == validHeader;
}

const bool MyMoneyTemplate::import(void)
{
  bool rc = !m_accounts.isNull();
  MyMoneyFile* file = MyMoneyFile::instance();

  while(rc == true && !m_accounts.isNull() && m_accounts.isElement()) {
    QDomElement childElement = m_accounts.toElement();
    if(childElement.tagName() == "account"
    && childElement.attribute("name") == "") {
      MyMoneyAccount parent;
      switch(childElement.attribute("type").toUInt()) {
        case MyMoneyAccount::Asset:
          parent = file->asset();
          break;
        case MyMoneyAccount::Liability:
          parent = file->liability();
          break;
        case MyMoneyAccount::Income:
          parent = file->income();
          break;
        case MyMoneyAccount::Expense:
          parent = file->expense();
          break;
#if 0
        case MyMoneyAccount::Equity:
          parent = file->equity();
          break;
#endif
        default:
          KMessageBox::error(qApp->mainWidget(), QString("<p>")+i18n("Invalid top-level account type <b>%1</b> in template file <b>%2</b>!").arg(childElement.attribute("type")).arg(m_source.prettyURL()));
          rc = false;
      }

      if(rc == true) {
        rc = createAccounts(parent, childElement.firstChild());
      }
    } else {
      rc = false;
    }
    m_accounts = m_accounts.nextSibling();
  }
  return rc;
}

const bool MyMoneyTemplate::createAccounts(MyMoneyAccount& parent, QDomNode account)
{
  bool rc = true;
  while(rc == true && !account.isNull()) {
    MyMoneyAccount acc;
    if(account.isElement()) {
      QDomElement accountElement = account.toElement();
      if(accountElement.tagName() == "account") {
        QValueList<MyMoneyAccount> subAccountList;
        QValueList<MyMoneyAccount>::ConstIterator it;
        it = subAccountList.end();
        if(!parent.accountList().isEmpty()) {
          subAccountList = MyMoneyFile::instance()->accountList(parent.accountList());
          for(it = subAccountList.begin(); it != subAccountList.end(); ++it) {
            if((*it).name() == accountElement.attribute("name")) {
              acc = *it;
              break;
            }
          }
        }
        if(it == subAccountList.end()) {
          // not found, we need to create it
          acc.setName(accountElement.attribute("name"));
          acc.setAccountType(static_cast<MyMoneyAccount::_accountTypeE>(accountElement.attribute("type").toUInt()));
          setFlags(acc, account.firstChild());
          try {
            MyMoneyFile::instance()->addAccount(acc, parent);
          } catch(MyMoneyException *e) {
            delete e;
          }
        }
        createAccounts(acc, account.firstChild());
      }
    }
    account = account.nextSibling();
  }
  return rc;
}

const bool MyMoneyTemplate::setFlags(MyMoneyAccount& acc, QDomNode flags)
{
  bool rc = true;
  while(rc == true && !flags.isNull()) {
    if(flags.isElement()) {
      QDomElement flagElement = flags.toElement();
      if(flagElement.tagName() == "flag") {
        // make sure, we only store flags we know!
        QString value = flagElement.attribute("name");
        if(value == "Tax") {
          acc.setValue(value.latin1(), "Yes");
        } else {
           KMessageBox::error(qApp->mainWidget(), QString("<p>")+i18n("Invalid flag type <b>%1</b> for account <b>%3</b> in template file <b>%2</b>!").arg(flagElement.attribute("name")).arg(m_source.prettyURL()).arg(acc.name()));
          rc = false;
       }
      }
    }
    flags = flags.nextSibling();
  }
  return rc;
}
