/***************************************************************************
                          kbanklistitem.cpp
                             -------------------
    copyright            : (C) 2000 by Michael Edwardes
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
#include <kglobal.h>
#include <kconfig.h>
#include <klocale.h>
#if QT_VERSION > 300
#include <kstandarddirs.h>
#else
#include <kstddirs.h>
#endif

#include <qpixmap.h>
#include <qcolor.h>

#include "kbanklistitem.h"

#if QT_VERSION > 300
#include <qpainter.h>
#endif

KAccountListItem::KAccountListItem(KListView *parent, const QString& accountName, const QCString& accountID, bool viewNormal)
  : QListViewItem(parent), m_accountID(accountID), m_bViewNormal(viewNormal)
{
  if (m_bViewNormal)
  {
    m_nAccountColumn = 1;
    m_nInstitutionColumn = 0;
  }
  else
  {
    m_nAccountColumn = 0;
    m_nInstitutionColumn = -1;
  }

  setText(/*m_nAccountColumn*/0, accountName);
  // setPixmap(1, QPixmap(KGlobal::dirs()->findResource("appdata", "icons/hicolor/22x22/actions/account.png")));
  //setText(1, m_account.getTypeName());
	//setText(2, KGlobal::locale()->formatMoney(m_account.balance().amount(), "",
  //                                          KGlobal::locale()->fracDigits()));
}

KAccountListItem::KAccountListItem(KAccountListItem *parent, const QString& accountName, const QCString& accountID, bool viewNormal)
  : QListViewItem(parent), m_accountID(accountID), m_bViewNormal(viewNormal)
{
  if (m_bViewNormal)
  {
    m_nAccountColumn = 1;
    m_nInstitutionColumn = 0;
  }
  else
  {
    m_nAccountColumn = 0;
    m_nInstitutionColumn = -1;
  }

  setText(/*m_nAccountColumn*/0, accountName);
  //setPixmap(0, QPixmap(KGlobal::dirs()->findResource("appdata", "icons/hicolor/22x22/actions/account.png")));
}

KAccountListItem::KAccountListItem(KListView *parent, const QString& institutionName, const QCString& institutionID)
  : QListViewItem(parent), m_institutionID(institutionID), m_bViewNormal(true)
{
  if (m_bViewNormal)
  {
    m_nAccountColumn = 1;
    m_nInstitutionColumn = 0;
  }
  else
  {
    m_nAccountColumn = 0;
    m_nInstitutionColumn = -1;
  }

  setText(/*m_nInstitutionColumn*/0, institutionName);
}

KAccountListItem::~KAccountListItem()
{
}

QCString KAccountListItem::accountID(void)
{
  return m_accountID;
}

void KAccountListItem::paintCell(QPainter *p, const QColorGroup & cg, int column, int width, int align)
{
	KConfig *config = KGlobal::config();
  config->setGroup("List Options");
  QFont defaultFont = QFont("helvetica", 12);
  p->setFont(config->readFontEntry("listCellFont", &defaultFont));

/*
  if (column==2)
  {
    QFont font = p->font();
    if (m_account.balance().amount()<0)
      font.setItalic(true);

    p->setFont(font);
    QListViewItem::paintCell(p, cg, column, width, 2);
  }
  else
*/
    QListViewItem::paintCell(p, cg, column, width, align);
}
