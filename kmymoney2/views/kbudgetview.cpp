/***************************************************************************
                          kbudgetview.cpp
                          ---------------
    begin                : Thu Jan 10 2006
    copyright            : (C) 2006 by Darren Gould
    email                : darren_gould@gmx.de
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

#include <qpushbutton.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qmultilineedit.h>
#include <qpixmap.h>
#include <qtabwidget.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kglobal.h>
#include <klocale.h>
#include <kconfig.h>
#include <kmessagebox.h>
#include <kpushbutton.h>
#include <kiconloader.h>
#include <kguiitem.h>
#include <kstandarddirs.h>
#include <kdebug.h>

// ----------------------------------------------------------------------------
// STL includes
#include <algorithm>
// for std::find since the find() functions provided by QValueList do not
// allow the lookup of a payee based on its id.

// ----------------------------------------------------------------------------
// Project Includes

#include "kbudgetview.h"
#include "../mymoney/mymoneyfile.h"
#include "../dialogs/knewbudgetdlg.h"

// *** KBudgetView Implementation ***

KBudgetView::KBudgetView(QWidget *parent, const char *name )
  : KBudgetViewDecl(parent,name),
    m_suspendUpdate(false)
{
  MyMoneyFile::instance()->attach(MyMoneyFile::NotifyClassPayeeSet, this);
}

KBudgetView::~KBudgetView()
{
  MyMoneyFile::instance()->detach(MyMoneyFile::NotifyClassPayeeSet, this);
}

void KBudgetView::update(const QCString & /*id*/)
{
  if(m_suspendUpdate == false)
    slotRefreshView();
}

void KBudgetView::show()
{
  QTimer::singleShot(50, this, SLOT(rearrange()));
  emit signalViewActivated();
  QWidget::show();
}

void KBudgetView::rearrange(void)
{
  resizeEvent(0);
}

void KBudgetView::resizeEvent(QResizeEvent* ev)
{
  // resize the register
  KBudgetViewDecl::resizeEvent(ev);
}

void KBudgetView::slotReloadView(void)
{
  ::timetrace("Start KBudgetView::slotReloadView");
  rearrange();
  ::timetrace("Done KBudgetView::slotReloadView");
}


void KBudgetView::slotRefreshView(void)
{

}

void KBudgetView::m_bNewBudget_clicked()
{
  KNewBudgetDlg *dlg = new KNewBudgetDlg(this, QString("New Budget"));
  if (dlg->exec())
  {
  }
}

#include "kbudgetview.moc"
