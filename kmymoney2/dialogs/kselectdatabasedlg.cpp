/***************************************************************************
                          kselectdatabase.cpp
                             -------------------
    copyright            : (C) 2005 by Tony Bloomfield
    author               : Tony Bloomfield
    email                : tonybloom@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

// ----------------------------------------------------------------------------
// QT Includes

#include <qlayout.h>
#include <qpushbutton.h>
#include <qapplication.h>
#include <qsqldatabase.h>
#include <qfiledialog.h>
#include <qstatusbar.h>
#include <qcheckbox.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kapplication.h>
#include <kurlrequester.h>
#include <ktextbrowser.h>
#include <klocale.h>
#include <kmessagebox.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kselectdatabasedlg.h"
#include "../mymoney/storage/mymoneystoragesql.h"

KSelectDatabaseDlg::KSelectDatabaseDlg(QWidget *parent, const char *name)
 : KSelectDatabaseDlgDecl(parent, name) {
  // list available drivers
  listDrivers->clear();
//  listDrivers->insertStringList (QSqlDatabase::drivers());
  typedef QMap<QString, QString> dnMap;
  dnMap map;
  // it would be nice if Qt provided us with meaningful names...
  map["QDB2"] = QString("IBM DB2");
  map["QIBASE"] = QString("Borland Interbase");
  map["QMYSQL3"] = QString("MySQL");
  map["QOCI8"] = QString("Oracle Call Interface, version 8 and 9");
  map["QODBC3"] = QString("Open Database Connectivity");
  map["QPSQL7"] = QString("PostgreSQL v6.x and v7.x");
  map["QSQLITE"] = QString("SQLite");
  map["QTDS7"] = QString("Sybase Adaptive Server and Microsoft SQL Server");

  QStringList list = QSqlDatabase::drivers();
  if (list.count() == 0) {
    KMessageBox::error (0, i18n("There are no Qt SQL drivers installed in your system.\n"
        "Please consult documentation for your distro, or visit the Qt web site (www.trolltech.com)"
            " and search for SQL drivers."),
        "");
        buttonOK->setEnabled(false);
  } else {
    QStringList::Iterator it = list.begin();
    while(it != list.end()) {
      QString dname = *it;
      if (map.keys().contains(dname)) dname = dname + " - " + map[dname];
      listDrivers->insertItem (dname);
      ++it;
    }

    listDrivers->setCurrentItem (0);
    slotDriverSelected (listDrivers->currentText());
    textDbName->setText ("KMyMoney");
    textHostName->setText ("localhost");
    textUserName->setText("");
    struct passwd * pwd = getpwuid(geteuid());
    if (pwd != 0)
      textUserName->setText (QString(pwd->pw_name));
    textPassword->setText ("");
    buttonOK->setEnabled(true);
    connect (listDrivers, SIGNAL(highlighted(const QString&)), this, SLOT(slotDriverSelected(const QString &)));
  }
  connect (buttonHelp, SIGNAL(released()), this, SLOT(slotHelp()));
  connect (buttonSQL, SIGNAL(released()), this, SLOT(slotGenerateSQL()));
  checkPreLoad->setChecked(false);
}

KSelectDatabaseDlg::~KSelectDatabaseDlg() {}

void KSelectDatabaseDlg::setMode (int openMode) {
  checkPreLoad->setEnabled (openMode == IO_ReadWrite);
}

const KURL KSelectDatabaseDlg::selectedURL() {
  KURL url;
  url.setProtocol("sql");
  url.setUser(textUserName->text());
  url.setPass(textPassword->text());
  url.setHost(textHostName->text());
  url.setPath("/" + textDbName->text());
  QString qs = QString("driver=%1&mode=single")
      .arg(listDrivers->currentText().section (' ', 0, 0));
  if (checkPreLoad->isChecked()) qs.append("&options=loadAll");
  url.setQuery(qs);
  return (url);
}

void KSelectDatabaseDlg::slotDriverSelected (const QString &driver) {

  if (driver.section(' ', 0, 0) == "QSQLITE") {
    slotBrowse();   // SQLITE needs a file name
    textHostName->setEnabled (false);  // but not host (how about user/password?)
  } else {
    textHostName->setEnabled (true);
  }
}

void KSelectDatabaseDlg::slotBrowse () { // relevant for SQLite only
  textDbName->setText (QFileDialog::getOpenFileName(
      "",
      "SQLite files (*.sql);; All files (*.*)",
      this,
      "",
      "Select SQLite file"));
}

void KSelectDatabaseDlg::slotGenerateSQL () {
  QString fileName = QFileDialog::getSaveFileName(
      "",
      "All files (*.*)",
      this,
      "",
      "Select output file");
  if (fileName == "") return;
  QFile out(fileName);
  if (!out.open(IO_WriteOnly)) return;
  QTextStream s(&out);
  MyMoneyDbDef db;
  s << db.generateSQL(listDrivers->currentText().section (' ', 0, 0));
  out.close();
}

void KSelectDatabaseDlg::slotHelp(void) {

  QString helpstring = i18n(
      "<h3>Database Type</h3>"
      "<p>"
      "This box lists all Qt SQL drivers installed on your system."
      " Select the driver for your database type."
      " If the one you want is not in the list, you need to install the appropriate driver."
      " See your distro documentation, or visit the Qt web Site (http://www.trolltech.com)"
      " and search for 'SQL drivers'."
      "</p>"
      "<h3>Database Name</h3>"
      "<p>"
      "The default database name is KMyMoney, but you may choose some other name if you like."
      " SQLite has one databases per file; selecting this driver opens the file dialog."
      " For database types other than MySql, the database name must be pre-created,"
      " though this application will create all table structures where necessary."
      "</p>"
      "<h3>Host Name</h3>"
      "<p>"
      "For the average user, the default name of localhost, being the machine you are currently using,"
      " is correct. For networked databases, enter the connected host name."
      "</p>"
      "<h3>User Name and Password</h3>"
      "<p>"
      "Check the permissions set up on your database, or contact the database administrator, for the"
      " correct values to use here. The user name must be capable of deleting, inserting and updating"
      " records. If the user name is the same as your login name, a password is not normally required."
      "</p>"
      "<h3>Generate SQL</h3>"
      "<p>This button will generate the CREATE TABLE commands to a text file, which may be edited"
      " if the in-built commands do not work for your database system."
      "</p"
  );

  QDialog dlg;
  QVBoxLayout layout( &dlg, 11, 6, "Layout17");
  KTextBrowser te(&dlg,"Help");
  layout.addWidget(&te);
  te.setReadOnly(true);
  te.setTextFormat(Qt::RichText);
  te.setText(helpstring);
  dlg.setCaption(i18n("Selecting a SQL database"));
  unsigned width = QApplication::desktop()->width();
  unsigned height = QApplication::desktop()->height();
  te.setMinimumSize(width/2,height/2);
  layout.setResizeMode(QLayout::Minimum);
  dlg.exec();
}

#include "kselectdatabasedlg.moc"
