/***************************************************************************
                          viewinterface.cpp
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

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "../../kmymoney2.h"
#include "../../views/kmymoneyview.h"
#include "kmmviewinterface.h"

KMyMoneyPlugin::KMMViewInterface::KMMViewInterface(KMyMoney2App* app, KMyMoneyView* view, QObject* parent, const char* name) :
  ViewInterface(parent, name),
  m_app(app),
  m_view(view)
{
  connect(m_view, SIGNAL(accountSelectedForContextMenu(const MyMoneyAccount&)), this, SIGNAL(accountSelectedForContextMenu(const MyMoneyAccount&)));
  connect(m_view, SIGNAL(viewStateChanged(bool)), this, SIGNAL(viewStateChanged(bool)));
}

KPopupMenu* KMyMoneyPlugin::KMMViewInterface::accountContextMenu(void)
{
  return m_view->accountContextMenu();
}

QFrame* KMyMoneyPlugin::KMMViewInterface::addPage(const QString& item, const QString& header, const QPixmap& pixmap)
{
  return m_view->addPage(item, header, pixmap);
}
