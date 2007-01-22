/***************************************************************************
                          registeritem.cpp  -  description
                             -------------------
    begin                : Tue Jun 13 2006
    copyright            : (C) 2000-2006 by Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
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

#include <kmymoney/registeritem.h>
#include <kmymoney/register.h>

#include "../kmymoneyglobalsettings.h"

using namespace KMyMoneyRegister;

QDate RegisterItem::nullDate;
MyMoneyMoney RegisterItem::nullValue;
QCString RegisterItem::nullCString;

RegisterItem::RegisterItem() :
  m_parent(0),
  m_prev(0),
  m_next(0)
{
  init();
}

RegisterItem::RegisterItem(Register* parent) :
  m_parent(parent),
  m_prev(0),
  m_next(0)
{
  init();
  parent->addItem(this);
}

void RegisterItem::init(void)
{
  m_startRow = 0;
  m_rowsRegister = 1;
  m_rowsForm = 1;
  m_visible = true;
}

RegisterItem::~RegisterItem()
{
  m_parent->removeItem(this);
}

void RegisterItem::setParent(Register* parent)
{
  m_parent = parent;
}

void RegisterItem::setNumRowsRegister(int rows)
{
  if(rows != m_rowsRegister) {
    m_rowsRegister = rows;
    if(m_parent)
      m_parent->forceUpdateLists();
  }
}

void RegisterItem::setVisible(bool visible)
{
  if(m_visible == visible)
    return;

  m_visible = visible;
  if(m_parent) {
    if(visible) {
      for(int i = startRow(); i < startRow() + numRowsRegister(); ++i) {
        m_parent->showRow(i);
      }
    } else {
      for(int i = startRow(); i < startRow() + numRowsRegister(); ++i) {
        m_parent->hideRow(i);
      }
    }
  }
}

int RegisterItem::rowHeightHint(void) const
{
  if(!m_visible)
    return 0;

  if(m_parent) {
    return m_parent->rowHeightHint();
  }

  QFontMetrics fm( KMyMoneyGlobalSettings::listCellFont() );
  return fm.lineSpacing()+6;
}
