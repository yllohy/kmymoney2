/***************************************************************************
                         kmymoneyaccountbutton  -  description
                            -------------------
   begin                : Mon May 31 2004
   copyright            : (C) 2000-2004 by Michael Edwardes
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

#include <qdrawutil.h>
#include <qpainter.h>
#include <qstyle.h>
#include <qapplication.h>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "../mymoney/mymoneyfile.h"
#include "kmymoneyaccountcombo.h"
#include "kmymoneyaccountcompletion.h"

kMyMoneyAccountCombo::kMyMoneyAccountCombo( QWidget* parent, const char* name ) :
  KComboBox( parent, name ),
  m_mlbDown(false)
{
  QCString myName(name);
  myName += "/completion";
  m_selector = new kMyMoneyAccountCompletion(this, myName.data());

  connect(this, SIGNAL(clicked()), this, SLOT(slotButtonPressed()));
  connect(m_selector, SIGNAL(itemSelected(const QCString&)), this, SLOT(slotSelected(const QCString&)));

  // make sure that we can display a minimum of characters
  QFontMetrics fm(font());
  setMinimumWidth(fm.maxWidth()*15);
  setMaximumHeight(height());

  // we only use this one item and replace the text as we have our own dropdown box
  insertItem(QString(""));
}

kMyMoneyAccountCombo::~kMyMoneyAccountCombo()
{
}

void kMyMoneyAccountCombo::slotButtonPressed(void)
{
  m_selector->show();
}

void kMyMoneyAccountCombo::slotSelected(const QCString& id)
{
  try {
    MyMoneyAccount acc = MyMoneyFile::instance()->account(id);
    setText(acc.name());
    emit accountSelected(id);
  } catch(MyMoneyException *e) {
    delete e;
  }
}

void kMyMoneyAccountCombo::setSelected(const QCString& id)
{
  if(!id.isEmpty()) {
    try {
      MyMoneyAccount acc = MyMoneyFile::instance()->account(id);
      setSelected(acc);
    } catch(MyMoneyException *e) {
      qDebug("Account '%s' not found in %s(%d)", id.data(), __FILE__, __LINE__);
      delete e;
    }
  } else {
    setText(QString());
    m_selector->setSelected(id);
  }
}

void kMyMoneyAccountCombo::setSelected(const MyMoneyAccount& acc)
{
  m_selector->setSelected(acc.id());
  setText(acc.name());
}

void kMyMoneyAccountCombo::setText(const QString& txt)
{
  changeItem(txt, currentItem());
}

const int kMyMoneyAccountCombo::loadList(const QString& baseName, const QValueList<QCString>& accountIdList, const bool clear)
{
  return m_selector->loadList(baseName, accountIdList, clear);
}

int kMyMoneyAccountCombo::loadList(KMyMoneyUtils::categoryTypeE typeMask)
{
  QValueList<int> typeList;

  if(typeMask & KMyMoneyUtils::asset) {
    typeList << MyMoneyAccount::Checkings;
    typeList << MyMoneyAccount::Savings;
    typeList << MyMoneyAccount::Cash;
    typeList << MyMoneyAccount::AssetLoan;
    typeList << MyMoneyAccount::CertificateDep;
    typeList << MyMoneyAccount::Investment;
    typeList << MyMoneyAccount::MoneyMarket;
    typeList << MyMoneyAccount::Asset;
    typeList << MyMoneyAccount::Currency;
  }
  if(typeMask & KMyMoneyUtils::liability) {
    typeList << MyMoneyAccount::CreditCard;
    typeList << MyMoneyAccount::Loan;
    typeList << MyMoneyAccount::Liability;
  }
  if(typeMask & KMyMoneyUtils::income) {
    typeList << MyMoneyAccount::Income;
  }
  if(typeMask & KMyMoneyUtils::expense) {
    typeList << MyMoneyAccount::Expense;
  }

  return m_selector->loadList(typeList);
}

void kMyMoneyAccountCombo::keyPressEvent(QKeyEvent* k)
{
  switch(k->key()) {
    case Qt::Key_Tab:
      break;

    case Qt::Key_Space:
      emit clicked();
      break;

    default:
      break;
  }
  return;
}

void kMyMoneyAccountCombo::mousePressEvent(QMouseEvent *e)
{
  if ( e->button() != LeftButton ) {
    e->ignore();
    return;
  }
  bool hit = rect().contains( e->pos() );
  if ( hit ) {                                // mouse press on button
    m_mlbDown = TRUE;                         // left mouse button down
    emit pressed();
  }
}

void kMyMoneyAccountCombo::mouseReleaseEvent(QMouseEvent *e)
{
  if ( e->button() != LeftButton ) {
      e->ignore();
      return;
  }
  if ( !m_mlbDown )
      return;
  m_mlbDown = FALSE;                            // left mouse button up
  emit released();
  if ( rect().contains( e->pos() ) ) {              // mouse release on button
    emit clicked();
  }
}

#include "kmymoneyaccountcombo.moc"
