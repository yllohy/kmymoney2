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
#include <qfileinfo.h>
#include <qapplication.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <klocale.h>
#include <kmessagebox.h>
#include <kio/netaccess.h>
#include <ksavefile.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneytemplate.h"

MyMoneyTemplate::MyMoneyTemplate() :
  m_progressCallback(0)
{
}

MyMoneyTemplate::MyMoneyTemplate(const KURL& url) :
  m_progressCallback(0)
{
  loadTemplate(url);
}

MyMoneyTemplate::~MyMoneyTemplate()
{
}

bool MyMoneyTemplate::loadTemplate(const KURL& url)
{
  QString filename;

  if(!url.isValid()) {
    qDebug("Invalid template URL '%s'", url.url().latin1());
    return false;
  }

  m_source = url;
  if(url.isLocalFile()) {
    filename = url.path();

  } else {
    bool rc;
    rc = KIO::NetAccess::download(url, filename, qApp->mainWidget());
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
  QFileInfo info(file);
  if(!info.isFile()) {
    QString msg=i18n("<b>%1</b> is not a template file.").arg(filename);
    KMessageBox::error(qApp->mainWidget(), QString("<p>")+msg, i18n("Filetype Error"));
    return false;
  }

  if(file.open(IO_ReadOnly)) {
    QString errMsg;
    int errLine, errColumn;
    if(!m_doc.setContent(&file, &errMsg, &errLine, &errColumn)) {
      QString msg=i18n("Error while reading template file <b>%1</b> in line %2, column %3").arg(filename).arg(errLine).arg(errColumn);
      KMessageBox::detailedError(qApp->mainWidget(), QString("<p>")+msg, errMsg, i18n("Template Error"));
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

bool MyMoneyTemplate::loadDescription(void)
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

bool MyMoneyTemplate::hierarchy(QMap<QString, QListViewItem*>& list, const QString& parent, QDomNode account)
{
  bool rc = true;
  while(rc == true && !account.isNull()) {
    if(account.isElement()) {
      QDomElement accountElement = account.toElement();
      if(accountElement.tagName() == "account") {
        QString name = QString("%1:%2").arg(parent).arg(accountElement.attribute("name"));
        list[name] = 0;
        hierarchy(list, name, account.firstChild());
      }
    }
    account = account.nextSibling();
  }
  return rc;
}

void MyMoneyTemplate::hierarchy(QMap<QString, QListViewItem*>& list)
{
  bool rc = !m_accounts.isNull();
  QDomNode accounts = m_accounts;
  while(rc == true && !accounts.isNull() && accounts.isElement()) {
    QDomElement childElement = accounts.toElement();
    if(childElement.tagName() == "account"
    && childElement.attribute("name") == "") {
      switch(childElement.attribute("type").toUInt()) {
        case MyMoneyAccount::Asset:
          list[i18n("Asset")] = 0;
          rc = hierarchy(list, i18n("Asset"), childElement.firstChild());
          break;
        case MyMoneyAccount::Liability:
          list[i18n("Liability")] = 0;
          rc = hierarchy(list, i18n("Liability"), childElement.firstChild());
          break;
        case MyMoneyAccount::Income:
          list[i18n("Income")] = 0;
          rc = hierarchy(list, i18n("Income"), childElement.firstChild());
          break;
        case MyMoneyAccount::Expense:
          list[i18n("Expense")] = 0;
          rc = hierarchy(list, i18n("Expense"), childElement.firstChild());
          break;
        case MyMoneyAccount::Equity:
          list[i18n("Equity")] = 0;
          rc = hierarchy(list, i18n("Equity"), childElement.firstChild());
          break;

        default:
          rc = false;
          break;
      }
    } else {
      rc = false;
    }
    accounts = accounts.nextSibling();
  }
}

bool MyMoneyTemplate::importTemplate(void(*callback)(int, int, const QString&))
{
  m_progressCallback = callback;
  bool rc = !m_accounts.isNull();
  MyMoneyFile* file = MyMoneyFile::instance();
  signalProgress(0, m_doc.elementsByTagName("account").count(), i18n("Loading template %1").arg(m_source.url()));
  m_accountsRead = 0;

  while(rc == true && !m_accounts.isNull() && m_accounts.isElement()) {
    QDomElement childElement = m_accounts.toElement();
    if(childElement.tagName() == "account"
    && childElement.attribute("name") == "") {
      ++m_accountsRead;
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
        case MyMoneyAccount::Equity:
          parent = file->equity();
          break;

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
  signalProgress(-1, -1);
  return rc;
}

bool MyMoneyTemplate::createAccounts(MyMoneyAccount& parent, QDomNode account)
{
  bool rc = true;
  while(rc == true && !account.isNull()) {
    MyMoneyAccount acc;
    if(account.isElement()) {
      QDomElement accountElement = account.toElement();
      if(accountElement.tagName() == "account") {
        signalProgress(++m_accountsRead, 0);
        QValueList<MyMoneyAccount> subAccountList;
        QValueList<MyMoneyAccount>::ConstIterator it;
        it = subAccountList.end();
        if(!parent.accountList().isEmpty()) {
          MyMoneyFile::instance()->accountList(subAccountList, parent.accountList());
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

bool MyMoneyTemplate::setFlags(MyMoneyAccount& acc, QDomNode flags)
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

void MyMoneyTemplate::signalProgress(int current, int total, const QString& msg)
{
  if(m_progressCallback != 0)
    (*m_progressCallback)(current, total, msg);
}

bool MyMoneyTemplate::exportTemplate(void(*callback)(int, int, const QString&))
{
  m_progressCallback = callback;

  m_doc = QDomDocument("KMYMONEY-TEMPLATE");

  QDomProcessingInstruction instruct = m_doc.createProcessingInstruction(QString("xml"), QString("version=\"1.0\" encoding=\"utf-8\""));
  m_doc.appendChild(instruct);

  QDomElement mainElement = m_doc.createElement("kmymoney-account-template");
  m_doc.appendChild(mainElement);

  QDomElement title = m_doc.createElement("title");
  mainElement.appendChild(title);

  QDomElement shortDesc = m_doc.createElement("shortdesc");
  mainElement.appendChild(shortDesc);

  QDomElement longDesc = m_doc.createElement("longdesc");
  mainElement.appendChild(longDesc);

  QDomElement accounts = m_doc.createElement("accounts");
  mainElement.appendChild(accounts);

  // addAccountStructure(accounts, MyMoneyFile::instance()->asset());
  // addAccountStructure(accounts, MyMoneyFile::instance()->liability());
  addAccountStructure(accounts, MyMoneyFile::instance()->income());
  addAccountStructure(accounts, MyMoneyFile::instance()->expense());
  // addAccountStructure(accounts, MyMoneyFile::instance()->equity());

  return true;
}

bool MyMoneyTemplate::addAccountStructure(QDomElement& parent, const MyMoneyAccount& acc)
{
  QDomElement account = m_doc.createElement("account");
  parent.appendChild(account);

  if(MyMoneyFile::instance()->isStandardAccount(acc.id()))
    account.setAttribute(QString("name"), QString());
  else
    account.setAttribute(QString("name"), acc.name());
  account.setAttribute(QString("type"), acc.accountType());

  // FIXME: add tax flag stuff

  // any child accounts?
  if(acc.accountList().count() > 0) {
    QValueList<MyMoneyAccount> list;
    MyMoneyFile::instance()->accountList(list, acc.accountList(), false);
    QValueList<MyMoneyAccount>::Iterator it;
    for(it = list.begin(); it != list.end(); ++it) {
      addAccountStructure(account, *it);
    }
  }
  return true;
}

bool MyMoneyTemplate::saveTemplate(const KURL& url)
{
  QString filename;

  if(!url.isValid()) {
    qDebug("Invalid template URL '%s'", url.url().latin1());
    return false;
  }

  if(url.isLocalFile()) {
    filename = url.path();
    KSaveFile qfile(filename, 0600);
    if(qfile.status() == 0) {
      saveToLocalFile(qfile.file());
      if(!qfile.close()) {
        throw new MYMONEYEXCEPTION(i18n("Unable to write changes to '%1'").arg(filename));
      }
    } else {
      throw new MYMONEYEXCEPTION(i18n("Unable to write changes to '%1'").arg(filename));
    }
  } else {
    KTempFile tmpfile;
    saveToLocalFile(tmpfile.file());
    if(!KIO::NetAccess::upload(tmpfile.name(), url, NULL))
      throw new MYMONEYEXCEPTION(i18n("Unable to upload to '%1'").arg(url.url()));
    tmpfile.unlink();
  }
  return true;
}

bool MyMoneyTemplate::saveToLocalFile(QFile* qfile)
{
  QTextStream stream(qfile);
  stream.setEncoding(QTextStream::UnicodeUTF8);
  stream << m_doc.toString();

  return true;
}
