/***************************************************************************
                          khomeview.cpp  -  description
                             -------------------
    begin                : Tue Jan 22 2002
    copyright            : (C) 2000-2002 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
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
#include <qlayout.h>

// ----------------------------------------------------------------------------
// KDE Includes
#include <kglobal.h>
#if QT_VERSION > 300
#include <kstandarddirs.h>
#else
#include <kstddirs.h>
#endif

#include <khtmlview.h>

// ----------------------------------------------------------------------------
// Project Includes
#include "khomeview.h"

KHomeView::KHomeView(QWidget *parent, const char *name )
 : QWidget(parent,name)
{
  QVBoxLayout *qvboxlayoutPage = new QVBoxLayout(this);
  qvboxlayoutPage->setSpacing( 6 );
  qvboxlayoutPage->setMargin( 11 );

  m_part = new KHTMLPart(this, "htmlpart_km2");
  qvboxlayoutPage->addWidget(m_part->view());

  QString filename = KGlobal::dirs()->findResource("appdata", "html/home.html");
  m_part->openURL(filename);
}

KHomeView::~KHomeView()
{
}

void KHomeView::show()
{
  emit signalViewActivated();
  QWidget::show();
}
