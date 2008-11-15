/***************************************************************************
                          kselectdatabasedlg.cpp
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

KSelectDatabaseDlg::KSelectDatabaseDlg(QWidget *parent, const char *name)
 : KSelectDatabaseDlgDecl(parent, name) {
  listDrivers->clear();
  // list drivers supported by KMM
  QMap<QString, QString> map = m_map.driverMap();
  // list drivers installed on system
  QStringList list = QSqlDatabase::drivers();
  if (list.count() == 0) {
    KMessageBox::error (0, i18n("There are no Qt SQL drivers installed in your system.\n"
        "Please consult documentation for your distro, or visit the Qt web site (www.trolltech.com)"
            " and search for SQL drivers."),
        "");
        reject();
  } else {
    QStringList::Iterator it = list.begin();
    while(it != list.end()) {
      QString dname = *it;
      if (map.keys().contains(dname)) { // only display if driver is supported
        dname = dname + " - " + map[dname];
        listDrivers->insertItem (dname);
      }
      it++;
    }
    textDbName->setText ("KMyMoney");
    textHostName->setText ("localhost");
    textUserName->setText("");
    struct passwd * pwd = getpwuid(geteuid());
    if (pwd != 0)
      textUserName->setText (QString(pwd->pw_name));
    textPassword->setText ("");
    buttonOK->setEnabled(false);
    connect (listDrivers, SIGNAL(clicked(QListBoxItem *)),
             this, SLOT(slotDriverSelected(QListBoxItem *)));
  }
  connect (buttonHelp, SIGNAL(clicked()), this, SLOT(slotHelp()));
  connect (buttonSQL, SIGNAL(clicked()), this, SLOT(slotGenerateSQL()));
  connect (buttonOK, SIGNAL(clicked()), this, SLOT(slotOKPressed()));
  checkPreLoad->setChecked(false);
}

KSelectDatabaseDlg::~KSelectDatabaseDlg() {}

void KSelectDatabaseDlg::setMode (int openMode) {
  m_mode = openMode;
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

void KSelectDatabaseDlg::slotDriverSelected (QListBoxItem *driver) {
  if (m_map.driverToType(driver->text().section(' ', 0, 0)) == Sqlite3){
    textDbName->clear();
    textHostName->setEnabled (false);  // but not host (how about user/password?)
    if (m_mode == IO_WriteOnly) getFileName(); // saveAs dialog - try to get file name now
  } else {
    textHostName->setEnabled (true);
  }
  buttonOK->setEnabled(true);
}

void KSelectDatabaseDlg::slotOKPressed () {
  if (m_map.driverToType(listDrivers->currentText().section(' ', 0, 0)) == Sqlite3){
    getFileName();   // SQLITE needs a file name
    if (textDbName->text().isEmpty()) reject();
    else accept();
  }
  else accept();
}

void KSelectDatabaseDlg::getFileName () { // relevant for SQLite only
  while (textDbName->text().isEmpty()) {
    textDbName->setText (QFileDialog::getOpenFileName(
    "",
    "SQLite files (*.sql);; All files (*.*)",
    this,
    "",
    "Select SQLite file"));
    if (textDbName->text().isEmpty()) {
      int rc = KMessageBox::warningYesNo
        (0, i18n("SQLite requires a file name; try again?"), "");
      if (rc == KMessageBox::No)
        return;
    }
  }
  return;
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
      " SQLite has one database per file; selecting this driver opens the file dialog."
      " For database types other than SQLite and MySql, the database itself must be pre-created,"
      " though KMyMoney will create all table structures where necessary."
      "</p>"
      "<h3>Host Name</h3>"
      "<p>"
      "For the average user, the default name of localhost, being the machine you are currently using,"
      " is correct. For networked databases, enter the host name of the system where the database is stored."
      " You may need to contact your database administrator for this information."
      "</p>"
      "<h3>User Name and Password</h3>"
      "<p>"
      "Check the permissions set up on your database, or contact the database administrator, for the"
      " correct values to use here. The user name must be capable of deleting, inserting and updating"
      " records. If the user name is the same as your login name, a password is not normally required."
      "</p>"
      "<h3>Generate SQL</h3>"
      "<p>Click this button to create a text file and write the commands needed to create the database tables and other objects."
      " With care, this may be edited if the in-built commands do not work for your database system."
      "</p>"
      "<p>"
      "Please read the appropriate chapter of the KMyMoney handbook for further information on database usage."
      "</p>"
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
