/***************************************************************************
                          kloadtemplatedlg.cpp
                             -------------------
    copyright            : (C) 2008 by Thomas Baumgart
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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

#include <kstdguiitem.h>
#include <kpushbutton.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kloadtemplatedlg.h"
#include "../widgets/kaccounttemplateselector.h"

KLoadTemplateDlg::KLoadTemplateDlg(QWidget* parent, const char* name) :
  KLoadTemplateDlgDecl(parent, name)
{
  buttonOk->setGuiItem(KStdGuiItem::ok());
  buttonCancel->setGuiItem(KStdGuiItem::cancel());
  buttonHelp->setGuiItem(KStdGuiItem::help());

  connect(buttonHelp, SIGNAL(clicked()), this, SLOT(slotHelp()));
}

QValueList<MyMoneyTemplate> KLoadTemplateDlg::templates(void) const
{
  return m_templateSelector->selectedTemplates();
}

void KLoadTemplateDlg::slotHelp(void)
{
}

#include "kloadtemplatedlg.moc"
