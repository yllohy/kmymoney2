/***************************************************************************
                          kmmstatementinterface.h
                             -------------------
    begin                : Wed Jan 5 2005
    copyright            : (C) 2005 Thomas Baumgart
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

#ifndef KMMSTATEMENTINTERFACE_H
#define KMMSTATEMENTINTERFACE_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class KMyMoney2App;
#include "../statementinterface.h"

namespace KMyMoneyPlugin {

/**
  * This class represents the implementation of the
  * StatementInterface.
  */
class KMMStatementInterface : public StatementInterface
{
  Q_OBJECT

public:
  KMMStatementInterface(KMyMoney2App* app, QObject* parent, const char* name = 0);
  ~KMMStatementInterface() {};

  /**
    * This method imports a MyMoneyStatement into the engine
    */
  bool import(MyMoneyStatement& s);

private:
  KMyMoney2App*    m_app;
};

}; // namespace
#endif
