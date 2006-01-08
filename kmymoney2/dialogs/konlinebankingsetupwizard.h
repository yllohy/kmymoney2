/***************************************************************************
                          konlinebankingsetupwizard.h
                             -------------------
    begin                : Sat Jan 7 2006
    copyright            : (C) 2006 by Ace Jones
    email                : acejones@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KONLINEBANKINGSETUPWIZARD_H
#define KONLINEBANKINGSETUPWIZARD_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qwidget.h>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include <../dialogs/konlinebankingsetupdecl.h>

/**
  * @author Ace Jones 
  */

/**
  * This class implementes a wizard for setting up an existing account
  * with online banking.
  *
  * The user is asked to choose his bank from the supported bank, and
  * his account.
  *
  * Currently works only with OFX Direct Connect, but I imagined that
  * other protocols could be included here.  To accomodate this, we'd 
  * add another page at the start of the wizard to ask which protocol
  * they wanted.
  *  
  */
class KOnlineBankingSetupWizard : public KOnlineBankingSetupDecl
{
  Q_OBJECT
public:
  KOnlineBankingSetupWizard(QWidget *parent=0, const char *name=0);
  ~KOnlineBankingSetupWizard();

public slots:
  void next();

};

#endif
// vim:cin:si:ai:et:ts=2:sw=2:
