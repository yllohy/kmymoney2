/***************************************************************************
                          kcategorylistitem.cpp
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

#include "kcategorylistitem.h"

KCategoryListItem::KCategoryListItem( QListView *parent, QString text, QStringList minors, bool income, bool major, QString majorName)
  : QListViewItem(parent)
{
  m_major = major;
  if (!m_major)
    m_majorName = majorName;

  m_income = income;
  m_minors = minors;

  setText(0, text);
  setText(1, ((m_income) ? "Income" : "Expense" ));
}

KCategoryListItem::KCategoryListItem( KCategoryListItem *parent, QString text, bool income, bool major, QString majorName)
  : QListViewItem(parent)
{
  m_major = major;
  if (!m_major)
    m_majorName = majorName;

  m_income = income;

  setText(0, text);
  setText(1, ((m_income) ? "Income" : "Expense" ));
}

KCategoryListItem::~KCategoryListItem()
{
}

bool KCategoryListItem::isMajor(void)
{
  return m_major;
}

QString KCategoryListItem::majorName(void)
{
  return m_majorName;
}

bool KCategoryListItem::income(void)
{
  return m_income;
}

QStringList KCategoryListItem::minors(void)
{
  return m_minors;
}
