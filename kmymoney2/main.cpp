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

#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klocale.h>

#include "kmymoney2.h"
#include "kstartuplogo.h"

static const char *description =
	I18N_NOOP("KMyMoney2 a personal finances application for KDE 2\n\nPlease consider contributing to this product with code and or suggestions");
	
static KCmdLineOptions options[] =
{
  { "+[File]", I18N_NOOP("file to open"), 0 },
  { 0, 0, 0 }
  // INSERT YOUR COMMANDLINE OPTIONS HERE
};

int main(int argc, char *argv[])
{
	KAboutData aboutData( "kmymoney2", I18N_NOOP("KMyMoney2"),
		VERSION, description, KAboutData::License_GPL,
		"(c) 2000, Michael Edwardes", 0, "http://kmymoney2.sourceforge.net");
	aboutData.addAuthor("Michael Edwardes", I18N_NOOP("Project Manager"), "mte@users.sourceforge.net");
	aboutData.addAuthor("Felix Rodriguez", I18N_NOOP("Project Admin"), "frodriguez@users.sourceforge.net");
	aboutData.addAuthor("Javier Campos Morales", I18N_NOOP("Developer & Artist"), "javi_c@users.sourceforge.net");
	aboutData.addAuthor("John C", I18N_NOOP("Developer"), "tacoturtle@users.sourceforge.net");
	KCmdLineArgs::init( argc, argv, &aboutData );
	KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.

  KApplication a;
	KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
	KMyMoney2App *kmymoney2 = new KMyMoney2App();
  a.setMainWidget( kmymoney2 );
  kmymoney2->show();

// FUTURE EXPANSION
//	if ( args->count() > 0 )
//	{	kmymoney2->openURL( args->url(0) ); 	args->clear();	return a.exec();}
//  else {
	if( kmymoney2->initWizard() )
		{
		args->clear();
	  return a.exec();
		} else { return false; }
// }

/*
  KApplication app;
 
  KStartupLogo* start_logo = new KStartupLogo;
  start_logo->show();
  bool blogo=true;

  if (app.isRestored()) {
    if (blogo) {
      start_logo->close();
      delete start_logo;
      blogo=false;
    }
    RESTORE(KMyMoney2App);
  }
  else {
    KMyMoney2App *kmymoney2 = new KMyMoney2App();
    if( kmymoney2!=0 ) kmymoney2->show();
    if (blogo) {
      start_logo->close();
      delete start_logo;
    }

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
		
		if (args->count())
		{
        fprintf(stderr, "arg count: %d (%s)\n", args->count(), args->arg(0));
        kmymoney2->openDocumentFile(args->arg(0));
		}
		else
		{
		  kmymoney2->openDocumentFile();
		}
		args->clear();
  }

  return app.exec();
*/
}
