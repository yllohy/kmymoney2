/***************************************************************************
                          kcategorytableitem.cpp  -  description
                             -------------------
    begin                : Wed Jan 31 2001
    copyright            : (C) 2001 by Michael Edwardes
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

#include "kcategorytableitem.h"
#include "kmymoneysettings.h"

KCategoryTableItem::KCategoryTableItem(QTable *t, EditType et, const QString& txt, MyMoneyFile *file)
  : QTableItem(t, et, txt), category_cb(0)
{
  setReplaceable(false);

  QString theText;
  if (file) {
    QListIterator<MyMoneyCategory> categoryIterator = file->categoryIterator();
    for ( ; categoryIterator.current(); ++categoryIterator) {
      MyMoneyCategory *category = categoryIterator.current();
      for ( QStringList::Iterator it = category->minorCategories().begin(); it != category->minorCategories().end(); ++it ) {
        theText.sprintf("%s:%s", category->name().latin1(), (*it).latin1());
        categoryList.append(theText);
      }
    }
  }
}

QWidget *KCategoryTableItem::createEditor() const
{
  ((KCategoryTableItem*)this)->category_cb = new QComboBox(table()->viewport());
  category_cb->insertStringList(categoryList);
  qDebug("Looking for %s", text().latin1());
  int pos=categoryList.findIndex(text());
  if (pos!=-1)
    category_cb->setCurrentItem(pos);
  else
    qDebug("Couldnt find pos");

  return category_cb;
}

void KCategoryTableItem::setContentFromEditor(QWidget *w)
{
  if (w->inherits("QComboBox")) {
    setText(((QComboBox*)w)->currentText());
  }
  else
    QTableItem::setContentFromEditor(w);
}

void KCategoryTableItem::setText(const QString &s)
{
  int pos;
  if (category_cb) {
    if ((pos=categoryList.findIndex(s))!=-1) {
      category_cb->setCurrentItem(pos);
      int pos = s.find(':');
      m_category.setName(s.left(pos));
      m_category.removeAllMinors();
      m_category.addMinorCategory(s.right(s.length()-(pos+1)));
//      qDebug("minor: %s", s.right(s.length()-pos).latin1());
    }
  }

  QTableItem::setText(s);
}

void KCategoryTableItem::paint(QPainter *p, const QColorGroup &cg, const QRect &cr, bool selected)
{
  KMyMoneySettings *p_settings = KMyMoneySettings::singleton();

  QColorGroup g(cg);
  if ((row()%2) && p_settings)
    g.setColor(QColorGroup::Base, p_settings->lists_BGColor());
  else if (p_settings)
    g.setColor(QColorGroup::Base, p_settings->lists_color());

  if (p_settings)
    p->setFont(p_settings->lists_cellFont());
  QTableItem::paint(p, g, cr, selected);
}
