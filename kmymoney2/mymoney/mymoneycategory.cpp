/***************************************************************************
                          mymoneycategory.cpp
                             -------------------
    copyright            : (C) 2000 by Michael Edwardes
    email                : Michael.Edwardes@students.dmu.ac.uk
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "mymoneycategory.h"

MyMoneyCategory::MyMoneyCategory()
{
}

MyMoneyCategory::MyMoneyCategory(const bool income, const QString name)
{
  m_income = income;
  m_name = name;
}

MyMoneyCategory::MyMoneyCategory(const bool income, const QString name, QStringList minors)
{
  m_income = income;
  m_name = name;
  m_minorCategories = minors;
}

MyMoneyCategory::~MyMoneyCategory()
{
}

// Functions use the find method to search the list
bool MyMoneyCategory::addMinorCategory(const QString val)
{
  if (val.isEmpty() || val.isNull())
    return false;

  if (m_minorCategories.find(val) == m_minorCategories.end()) {
    m_minorCategories.append(val);
    return true;
  }

  return false;
}

bool MyMoneyCategory::removeMinorCategory(const QString val)
{
  if (val.isEmpty() || val.isNull())
    return false;

  if (m_minorCategories.find(val) != m_minorCategories.end()) {
    m_minorCategories.remove(val);
    return true;
  }

  return false;
}

bool MyMoneyCategory::renameMinorCategory(const QString oldVal, const QString newVal)
{
  if (oldVal.isEmpty() || oldVal.isNull() || newVal.isEmpty() || newVal.isNull())
    return false;

  if (m_minorCategories.find(oldVal) != m_minorCategories.end()) {
    m_minorCategories.remove(oldVal);
    m_minorCategories.append(newVal);
    return true;
  }

  return false;
}

bool MyMoneyCategory::addMinorCategory(QStringList values)
{
  for (QStringList::Iterator it = values.begin(); it!=values.end(); ++it) {
    if (m_minorCategories.find(*it) == m_minorCategories.end())
      if (!(*it).isEmpty() && !(*it).isNull()) {
        m_minorCategories.append(*it);
        return true;
      } else
        return false;
  }
  return false;
}

bool MyMoneyCategory::removeAllMinors(void)
{
  m_minorCategories.clear();
  return true;
}

QString MyMoneyCategory::firstMinor(void)
{
  return m_minorCategories.first();
}

MyMoneyCategory::MyMoneyCategory(const MyMoneyCategory& right)
{
  m_income = right.m_income;
  m_name = right.m_name;
  m_minorCategories.clear();
  m_minorCategories = right.m_minorCategories;
}

MyMoneyCategory& MyMoneyCategory::operator = (const MyMoneyCategory& right)
{
  m_income = right.m_income;
  m_name = right.m_name;
  m_minorCategories.clear();
  m_minorCategories = right.m_minorCategories;
  return *this;
}

QDataStream &operator<<(QDataStream &s, MyMoneyCategory &category)
{
  if (category.m_income)
    s << (Q_INT32)1;
  else
    s << (Q_INT32)0;

  s << category.m_name;

  s << (Q_UINT32)category.m_minorCategories.count();
  for (QStringList::Iterator it = category.m_minorCategories.begin(); it!=category.m_minorCategories.end(); ++it) {
    s << (*it);
  }

  return s;
}

QDataStream &operator>>(QDataStream &s, MyMoneyCategory &category)
{
  Q_INT32 inc;
  s >> inc;
  if (inc==0)
    category.m_income = false;
  else
    category.m_income = true;

  s >> category.m_name;

  Q_UINT32 minorCount;
  QString buffer;

  s >> minorCount;
  category.m_minorCategories.clear();
  for (unsigned int i=0; i<minorCount; i++) {
    s >> buffer;
    category.m_minorCategories.append(buffer);
  }

  return s;
}
