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

#include <qwidgetlist.h>
#include <qdatetime.h>

#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klocale.h>

#include "kmymoney2.h"
#include "kstartuplogo.h"

static const char *description =
	I18N_NOOP("KMyMoney2 a personal finances application for KDE.\n\nPlease consider contributing to this project with code and or suggestions.");
	
static KCmdLineOptions options[] =
{
  { "+[File]", I18N_NOOP("file to open"), 0 },
  { 0, 0, 0 }
  // INSERT YOUR COMMANDLINE OPTIONS HERE
};

QTime timer;

int main(int argc, char *argv[])
{
  QString feature;

#if HAVE_LIBXMLPP
  feature += "\t- XML support\n";
#else
  feature += "\t- no XML support\n";
#endif
#if HAVE_LIBCPPUNIT
  feature += "\t- Automatic regression testing\n";
#else
  feature += "\t- Automatic regression testing disabled\n";
#endif
#ifdef _CHECK_MEMORY
  feature += "\t- Memory leakage detection\n";
#else
  feature += "\t- No memory leakage detection\n";
#endif

  if(feature.length() != 0)
    feature = I18N_NOOP("Compiled with the following settings:\n") + feature;

	KAboutData aboutData( "kmymoney2", I18N_NOOP("KMyMoney2"),
		VERSION, description, KAboutData::License_GPL,
		"(c) 2000-2002, Michael Edwardes", feature,
    "http://kmymoney2.sourceforge.net/");
	aboutData.addAuthor("Michael Edwardes", I18N_NOOP("Project Manager"), "mte@users.sourceforge.net");
	aboutData.addAuthor("Felix Rodriguez", I18N_NOOP("Project Admin"), "frodriguez@users.sourceforge.net");
	aboutData.addCredit("Javier Campos Morales", I18N_NOOP("Developer & Artist"), "javi_c@users.sourceforge.net");
	aboutData.addAuthor("John C", I18N_NOOP("Developer"), "tacoturtle@users.sourceforge.net");
	aboutData.addAuthor("Thomas Baumgart", I18N_NOOP("Developer & Release Manager"), "ipwizard@users.sourceforge.net");
	aboutData.addAuthor("Kevin Tambascio", I18N_NOOP("Developer"), "ktambascio@users.sourceforge.net");
  aboutData.addAuthor("Arni Ingimundarson", I18N_NOOP("Developer"), "arniing@users.sourceforge.net");
	KCmdLineArgs::init( argc, argv, &aboutData );
	KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.

  timer.start();

  KApplication a;
	KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

  // setup the MyMoneyMoney locale settings according to the KDE settings
  MyMoneyMoney::setThousandSeparator(*(KGlobal::locale()->thousandsSeparator().latin1()));
  MyMoneyMoney::setDecimalSeparator(*(KGlobal::locale()->decimalSymbol().latin1()));

	KMyMoney2App *kmymoney2 = new KMyMoney2App();
  a.setMainWidget( kmymoney2 );
  kmymoney2->show();

  int rc = 0;

#ifdef _CHECK_MEMORY
  _CheckMemory_Init(0);
#endif

	if (kmymoney2->startWithDialog()) {
	  if (kmymoney2->initWizard()) {
  		args->clear();
      rc = a.exec();
	  }
	} else {
		args->clear();
	  rc = a.exec();
	}

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

