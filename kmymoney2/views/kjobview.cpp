/***************************************************************************
                          kjobview.cpp  -  description
                             -------------------
    begin                : Thu Aug 26 2004
    copyright            : (C) 2004 Martin Preuss
    email                : aquamaniac@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

// ----------------------------------------------------------------------------
// QT Includes
#include <qlayout.h>
#include <qdatetime.h>

// ----------------------------------------------------------------------------
// KDE Includes
#include <kglobal.h>
#include <klocale.h>
#include <kstandarddirs.h>

// ----------------------------------------------------------------------------
// Project Includes
#include "kjobview.h"
#include "../kmymoneyutils.h"
#include "../kmymoney2.h"
#include "../converter/mymoneybanking.h"


KJobView::KJobView(QWidget *parent, const char *name )
 : QWidget(parent,name)
{
  // this view should only be added when banking is available
  m_qvboxlayoutPage = new QVBoxLayout(this);
  m_qvboxlayoutPage->setSpacing( 6 );
  m_qvboxlayoutPage->setMargin( 11 );

  m_qvboxlayoutPage->addWidget(KMyMoneyBanking::instance()->createJobView(this, "JobView"));
}

KJobView::~KJobView()
{
}

void KJobView::show()
{
  slotRefreshView();
  QWidget::show();
  emit signalViewActivated();
}


void KJobView::slotRefreshView(void)
{
  KMyMoneyBanking::instance()->updateJobView();
}



