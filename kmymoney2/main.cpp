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
#include <dcopclient.h>
#include <kmessagebox.h>

// ----------------------------------------------------------------------------
// Project Includes
#include "kmymoney2.h"
#include "kstartuplogo.h"
#include "mymoney/mymoneyfile.h"
#include "views/kbanklistitem.h"
#include "kapptest.h"
#include "kmymoneyutils.h"

static const char *description =
  I18N_NOOP("KMyMoney, the Personal Finance Manager for KDE.\n\nPlease consider contributing to this project with code and or suggestions.");

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
    "(c) 2000-2004, Michael Edwardes", feature,
    "http://kmymoney2.sourceforge.net/",
    "kmymoney2-developer@lists.sourceforge.net");

  aboutData.addAuthor("Michael Edwardes (babelfish_mte on jabber & msn).", I18N_NOOP("Project Manager"), "mte@users.sourceforge.net");
  aboutData.addAuthor("Felix Rodriguez", I18N_NOOP("Project Admin"), "frodriguez@users.sourceforge.net");
  aboutData.addCredit("Javier Campos Morales", I18N_NOOP("Developer & Artist"), "javi_c@users.sourceforge.net");
  aboutData.addAuthor("John C", I18N_NOOP("Developer"), "tacoturtle@users.sourceforge.net");
  aboutData.addAuthor("Thomas Baumgart", I18N_NOOP("Developer & Release Manager"), "ipwizard@users.sourceforge.net");
  aboutData.addAuthor("Kevin Tambascio", I18N_NOOP("Developer"), "ktambascio@users.sourceforge.net");
  aboutData.addAuthor("Ace Jones", I18N_NOOP("Developer of reporting logic"), "");
  aboutData.addCredit("Arni Ingimundarson", I18N_NOOP("Developer"), "arniing@users.sourceforge.net");
  KCmdLineArgs::init( argc, argv, &aboutData );
  KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.

  // create the singletons before we start memory checking
  // to avoid false error reports
  MyMoneyFile::instance();

#ifdef _CHECK_MEMORY
  _CheckMemory_Init(0);
#endif

  KMyMoneyUtils::checkConstants();

  timer.start();

  KApplication* a = new KApplication();
  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

  // setup the MyMoneyMoney locale settings according to the KDE settings
  MyMoneyMoney::setThousandSeparator(*(KGlobal::locale()->monetaryThousandsSeparator().latin1()));
  MyMoneyMoney::setDecimalSeparator(*(KGlobal::locale()->monetaryDecimalSymbol().latin1()));
  MyMoneyMoney::setNegativeMonetarySignPosition(static_cast<MyMoneyMoney::signPosition>(KGlobal::locale()->negativeMonetarySignPosition()));
  MyMoneyMoney::setPositiveMonetarySignPosition(static_cast<MyMoneyMoney::signPosition>(KGlobal::locale()->positiveMonetarySignPosition()));
  MyMoneyMoney::setNegativePrefixCurrencySymbol(KGlobal::locale()->negativePrefixCurrencySymbol());
  MyMoneyMoney::setPositivePrefixCurrencySymbol(KGlobal::locale()->positivePrefixCurrencySymbol());

  QCString language = args->getOption("lang");
  if(!language.isEmpty()) {
    if(!KGlobal::locale()->setLanguage(language)) {
      qWarning("Unable to select language '%s'. This has one of two reasons:\n\ta) the standard KDE message catalogue is not installed\n\tb) the KMyMoney message catalogue is not installed", language.data());
    }
  }

  kmymoney2 = new KMyMoney2App();
  a->setMainWidget( kmymoney2 );

  // connect to DCOP server
  if(a->dcopClient()->registerAs("kmymoney", true) != false) {
    if(kmymoney2->instanceList().count() > 0) {
      if(KMessageBox::questionYesNo(0, i18n("Another instance of KMyMoney is already running. Do you want to quit?")) == KMessageBox::Yes) {
        delete a;
        exit(1);
      }
    }
  } else {
    qDebug("DCOP registration failed. Some functions are not available.");
  }


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
    url = kmymoney2->readLastUsedFile();
  }

  if(url.isValid()) {
    KTipDialog::showTip(kmymoney2, "", false);
    kmymoney2->slotFileOpenRecent(url);
  } else {
    // kmymoney2->slotFileNew();
    // kmymoney2->createInitialAccount();
    KTipDialog::showTip(kmymoney2, "", false);
  }

  CREATE_TEST_CONTAINER();

  if(kmymoney2 != 0) {
    kmymoney2->updateCaption();
    args->clear();
    kmymoney2->setEnabled(true);
    rc = a->exec();
  }

  DESTROY_TEST_CONTAINER();

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

