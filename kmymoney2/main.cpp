/***************************************************************************
                          main.cpp
                             -------------------
    copyright            : (C) 2001 by Michael Edwardes
    email                : mte@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <stdio.h>

// ----------------------------------------------------------------------------
// QT Includes
#include <qwidgetlist.h>
#include <qdatetime.h>
#include <qstringlist.h>

// ----------------------------------------------------------------------------
// KDE Includes
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <ktip.h>

// ----------------------------------------------------------------------------
// Project Includes
#include "kmymoney2.h"
#include "kstartuplogo.h"

#include "mymoney/mymoneyfile.h"
#include "views/kbanklistitem.h"

static const char *description =
  I18N_NOOP("KMyMoney, the personal finances application for KDE.\n\nPlease consider contributing to this project with code and or suggestions.");

static KCmdLineOptions options[] =
{
  { "lang <lang-code>", I18N_NOOP("language to be used"), 0 },
  // INSERT YOUR COMMANDLINE OPTIONS HERE
  { "+[File]", I18N_NOOP("file to open"), 0 },
  { 0, 0, 0 }
};

QTime timer;

KMyMoney2App* kmymoney2;

int main(int argc, char *argv[])
{
  QString feature;

#ifdef _CHECK_MEMORY
  feature += "\t- Memory leakage detection\n";
#endif

  if(!feature.isEmpty())
    feature = I18N_NOOP("Compiled with the following settings:\n") + feature;

  KAboutData aboutData( "kmymoney2", I18N_NOOP("KMyMoney"),
    VERSION, description, KAboutData::License_GPL,
    "(c) 2000-2003, Michael Edwardes", feature,
    "http://kmymoney2.sourceforge.net/");
  aboutData.addAuthor("Michael Edwardes", I18N_NOOP("Project Manager"), "mte@users.sourceforge.net");
  aboutData.addAuthor("Felix Rodriguez", I18N_NOOP("Project Admin"), "frodriguez@users.sourceforge.net");
  aboutData.addCredit("Javier Campos Morales", I18N_NOOP("Developer & Artist"), "javi_c@users.sourceforge.net");
  aboutData.addAuthor("John C", I18N_NOOP("Developer"), "tacoturtle@users.sourceforge.net");
  aboutData.addAuthor("Thomas Baumgart", I18N_NOOP("Developer & Release Manager"), "ipwizard@users.sourceforge.net");
  aboutData.addAuthor("Kevin Tambascio", I18N_NOOP("Developer"), "ktambascio@users.sourceforge.net");
  aboutData.addCredit("Arni Ingimundarson", I18N_NOOP("Developer"), "arniing@users.sourceforge.net");
  KCmdLineArgs::init( argc, argv, &aboutData );
  KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.

  // create the singletons before we start memory checking
  // to avoid false error reports
  MyMoneyFile::instance();

#ifdef _CHECK_MEMORY
  _CheckMemory_Init(0);
#endif

  timer.start();

  KApplication* a = new KApplication();
  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
  
  // setup the MyMoneyMoney locale settings according to the KDE settings
  MyMoneyMoney::setThousandSeparator(*(KGlobal::locale()->thousandsSeparator().latin1()));
  MyMoneyMoney::setDecimalSeparator(*(KGlobal::locale()->decimalSymbol().latin1()));

  QCString language = args->getOption("lang");
  if(!language.isEmpty()) {
    if(!KGlobal::locale()->setLanguage(language)) {
      qWarning("Unable to select language '%s'. This has one of two reasons:\n\ta) the standard KDE message catalogue is not installed\n\tb) the KMyMoney message catalogue is not installed", language.data());
    }
  }
  
  kmymoney2 = new KMyMoney2App();
  a->setMainWidget( kmymoney2 );
  kmymoney2->show();
  kmymoney2->setEnabled(false);
  
  // force complete paint of widgets
  qApp->processEvents();

  int rc = 0;


  KURL url;
  // make sure, we take the file provided on the command
  // line before we go and open the last one used
  if(args->count() > 0) {
    url = args->url(0);
  } else {
    url = kmymoney2->lastOpenedURL();
  }
  
  if (url.url().isEmpty()) {
    if (kmymoney2->initWizard()) {
      KTipDialog::showTip(kmymoney2, "", false);
      
    } else {
      delete kmymoney2;
      kmymoney2 = 0;
    }
    
  } else {
    KTipDialog::showTip(kmymoney2, "", false);
    kmymoney2->slotFileOpenRecent(url);
  }

  if(kmymoney2 != 0) {
    args->clear();
    kmymoney2->setEnabled(true);
    kmymoney2->createInitialAccount();
    rc = a->exec();
  }
  
  delete a;
  KAccountListItem::cleanCache();

#ifdef _CHECK_MEMORY
  chkmem.CheckMemoryLeak( false );
  _CheckMemory_End();
#endif

  return rc;
}


void timetrace(char *txt)
{
  qDebug("Timer(%s): %d elapsed", txt, timer.elapsed());
  timer.restart();
}

