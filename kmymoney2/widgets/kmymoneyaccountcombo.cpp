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
    KPushButton( parent, name )
{
  m_selector = new kMyMoneyAccountCompletion(this, "selector");
  connect(this, SIGNAL(clicked()), this, SLOT(slotButtonPressed()));
  connect(m_selector, SIGNAL(itemSelected(const QCString&)), this, SLOT(slotSelected(const QCString&)));
  connect(m_selector, SIGNAL(itemSelected(const QCString&)), this, SIGNAL(accountSelected(const QCString&)));

  // make sure that we can display a minimum of characters
  QFontMetrics fm(font());
  setMinimumWidth(fm.maxWidth()*15);

  setMaximumHeight(height());
}

kMyMoneyAccountCombo::~kMyMoneyAccountCombo()
{
}

void kMyMoneyAccountCombo::slotButtonPressed(void)
{
  m_selector->loadList();
  m_selector->show();
}

void kMyMoneyAccountCombo::slotSelected(const QCString& id)
{
  try {
    MyMoneyAccount acc = MyMoneyFile::instance()->account(id);
    setText(acc.name());
  } catch(MyMoneyException *e) {
    delete e;
  }
}

void kMyMoneyAccountCombo::setSelected(const MyMoneyAccount& acc)
{
  m_selector->setSelected(acc.id());
  setText(acc.name());
}


void kMyMoneyAccountCombo::drawButton( QPainter *paint )
{
  QStyle::SFlags flags = QStyle::Style_Default;

  if (isEnabled())
    flags |= QStyle::Style_Enabled;
  if (hasFocus())
    flags |= QStyle::Style_HasFocus;
  if (isDown())
    flags |= QStyle::Style_Down;
  if (isOn())
    flags |= QStyle::Style_On;
  if (! isFlat() && ! isDown())
    flags |= QStyle::Style_Raised;
  if (isDefault())
    flags |= QStyle::Style_ButtonDefault;

  const QColorGroup & g = colorGroup();
  style().drawComplexControl( QStyle::CC_ComboBox, paint, this, rect(), g,
        flags, QStyle::SC_All & ~QStyle::SC_ComboBoxEditField,
        (true ?                        // d->arrowDown
          QStyle::SC_ComboBoxArrow :
          QStyle::SC_None ));

  QRect re = style().subRect(QStyle::SR_PushButtonContents, this);
  if (isDown()) {
    re.moveBy(style().pixelMetric(QStyle::PM_ButtonShiftHorizontal, this),
              style().pixelMetric(QStyle::PM_ButtonShiftVertical, this));
  }

  QFontMetrics fm(font());
  int ofs = (re.height() - fm.height())/2;
  re.addCoords(8, ofs, 0, ofs);

  style().drawItem(paint, re, QStyle::ShowPrefix, colorGroup(), flags & QStyle::Style_Enabled, 0, text(), text().length(), &(colorGroup().buttonText()));
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

