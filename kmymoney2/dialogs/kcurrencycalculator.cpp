/***************************************************************************
                          kcurrencycalculator.cpp  -  description
                             -------------------
    begin                : Thu Apr 8 2004
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

#include <qlabel.h>
#include <qradiobutton.h>
#include <qcheckbox.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <klocale.h>
#include <kpushbutton.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kcurrencycalculator.h"
#include "../widgets/kmymoneyedit.h"

KCurrencyCalculator::KCurrencyCalculator(const MyMoneyCurrency& from, const MyMoneyCurrency& to, const MyMoneyMoney& value, const MyMoneyMoney& shares, const QDate& date, const int resultFraction, QWidget *parent, const char *name ) :
  KCurrencyCalculatorDecl(parent, name),
  m_fromCurrency(from),
  m_toCurrency(to),
  m_result(shares.abs()),
  m_value(value.abs()),
  m_date(date),
  m_sign((value < 0) ? -1 : 1),
  m_resultFraction(resultFraction)
{
  MyMoneyFile* file = MyMoneyFile::instance();

  m_fromCurrencyName->setText(m_fromCurrency.name());
  m_toCurrencyName->setText(m_toCurrency.name());

  m_fromAmount->loadText(m_value.formatMoney("", MyMoneyMoney::denomToPrec(m_fromCurrency.smallestAccountFraction())));
  m_fromAmount->setReadOnly(true);

  m_fromToButton->setText(i18n("%1 for 1 %2").arg(m_fromCurrency.name()).arg(m_toCurrency.name()));
  m_toFromButton->setText(i18n("%1 for 1 %2").arg(m_toCurrency.name()).arg(m_fromCurrency.name()));

  MyMoneyMoney price(1,1);

  // setup initial result
  if(m_result == MyMoneyMoney() && m_value != 0) {
    qDebug("result == 0");
    if(m_fromCurrency.id() == file->baseCurrency().id()) {
      price = MyMoneyMoney(1,1) / m_toCurrency.price(m_date);
    } else if(m_toCurrency.id() == file->baseCurrency().id()) {
      price = m_fromCurrency.price(m_date);
    }
    qDebug("Price is %s", price.formatMoney().data());
    m_result = m_value * price;
  }

  // check if we have a conversion rate for this date  
  m_updateButton->setEnabled(false);
  m_fromToButton->setChecked(true);

  if(m_fromCurrency.id() == file->baseCurrency().id()) {
    qDebug("from == base");
    m_updateCurrency = &m_toCurrency;
    m_updateButton->setEnabled(true);
    m_toFromButton->setChecked(true);
  }

  if(m_toCurrency.id() == file->baseCurrency().id()) {
    qDebug("to == base");
    m_updateCurrency = &m_fromCurrency;
    m_updateButton->setEnabled(true);
    m_fromToButton->setChecked(true);
  }

  // fill in initial values
  m_toAmount->loadText(m_result.formatMoney("", MyMoneyMoney::denomToPrec(m_toCurrency.smallestAccountFraction())));
  slotUpdateResult(m_toAmount->text());

  connect(m_fromToButton, SIGNAL(clicked()), this, SLOT(slotSetFromTo()));
  connect(m_toFromButton, SIGNAL(clicked()), this, SLOT(slotSetToFrom()));
  connect(m_toAmount, SIGNAL(valueChanged(const QString&)), this, SLOT(slotUpdateResult(const QString&)));
  connect(m_conversionRate, SIGNAL(valueChanged(const QString&)), this, SLOT(slotUpdateRate(const QString&)));
  connect(m_cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
  connect(m_okButton, SIGNAL(clicked()), this, SLOT(accept()));
}

KCurrencyCalculator::~KCurrencyCalculator()
{
}

void KCurrencyCalculator::slotSetFromTo(void)
{
  qDebug("slotSetFromTo");
  slotUpdateResult(m_toAmount->text());
}

void KCurrencyCalculator::slotSetToFrom(void)
{
  qDebug("slotSetToFrom");
  slotUpdateResult(m_toAmount->text());
}

void KCurrencyCalculator::slotUpdateResult(const QString& txt)
{
  // make sure we only have positive numbers
  if(txt.left(1) == "-") {
    m_toAmount->loadText(txt.mid(1));
    slotUpdateResult(m_toAmount->text());
    return;
  }

  MyMoneyMoney price;
  if(MyMoneyMoney(txt) != 0) {
    m_result = MyMoneyMoney(txt).abs();
    m_result.convert(m_resultFraction);
    if(m_fromToButton->isChecked()) {
      price = m_value / m_result;
    } else {
      price = m_result / m_value;
    }
    m_conversionRate->loadText(price.formatMoney("", 5));
  }
  m_toAmount->loadText(m_result.formatMoney("", MyMoneyMoney::denomToPrec(m_resultFraction)));
}

void KCurrencyCalculator::slotUpdateRate(const QString& txt)
{
  // make sure we only have positive numbers
  if(txt.left(1) == "-") {
    m_conversionRate->loadText(txt.mid(1));
    slotUpdateRate(m_conversionRate->text());
    return;
  }

  MyMoneyMoney price(txt);

  if(price != 0) {
    if(m_fromToButton->isChecked()) {
      m_result = m_value / price;
    } else {
      m_result = m_value * price;
    }
    m_toAmount->loadText(m_result.formatMoney("", MyMoneyMoney::denomToPrec(m_resultFraction)));
    
  } else {
    slotUpdateResult(m_toAmount->text());
  }
}

void KCurrencyCalculator::accept(void)
{
  if(m_updateButton->isChecked()) {
    MyMoneyMoney price;
    price = m_result / m_value;
    if(m_updateCurrency->price(m_date) != price) {
      m_updateCurrency->addPriceHistory(m_date, price);
      MyMoneyFile::instance()->modifyCurrency(*m_updateCurrency);
    }
  }
  KCurrencyCalculatorDecl::accept();
}

const MyMoneyMoney KCurrencyCalculator::price(void) const
{
  return m_value / m_result;
}
