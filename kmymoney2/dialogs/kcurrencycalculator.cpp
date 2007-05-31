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
#include <qwidgetstack.h>
#include <qgroupbox.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <klocale.h>
#include <kpushbutton.h>
#include <kcombobox.h>
#include <kstdguiitem.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kcurrencycalculator.h"

#include <kmymoney/kmymoneyedit.h>
#include <kmymoney/kmymoneydateinput.h>
#include <kmymoney/kmymoneycurrencyselector.h>
#include <kmymoney/mymoneyprice.h>
#include <kmymoney/mymoneytransaction.h>
#include <kmymoney/kmymoneyglobalsettings.h>

#include "../kmymoneyutils.h"

bool KCurrencyCalculator::setupSplitPrice(MyMoneyMoney& shares, const MyMoneyTransaction& t, const MyMoneySplit& s, const QMap<QCString, MyMoneyMoney>& priceInfo, QWidget* parentWidget)
{
  bool rc = true;
  MyMoneyFile* file = MyMoneyFile::instance();

  if(!s.value().isZero()) {
    MyMoneyAccount cat = file->account(s.accountId());
    MyMoneySecurity toCurrency;
    toCurrency = file->security(cat.currencyId());
    // determine the fraction required for this category/account
    int fract = cat.fraction(toCurrency);

    if(cat.currencyId() != t.commodity()) {

      MyMoneySecurity fromCurrency;
      MyMoneyMoney fromValue, toValue;
      fromCurrency = file->security(t.commodity());
      // display only positive values to the user
      fromValue = s.value().abs();

      // if we had a price info in the beginning, we use it here
      if(priceInfo.find(cat.currencyId()) != priceInfo.end()) {
        toValue = (fromValue * priceInfo[cat.currencyId()]).convert(fract);
      }

      // if the shares are still 0, we need to change that
      if(toValue.isZero()) {
        MyMoneyPrice price = file->price(fromCurrency.id(), toCurrency.id());
        // if the price is valid calculate the shares. If it is invalid
        // assume a conversion rate of 1.0
        if(price.isValid()) {
          toValue = (price.rate(toCurrency.id()) * fromValue).convert(fract);
        } else {
          toValue = fromValue;
        }
      }

      // now present all that to the user
      KCurrencyCalculator calc(fromCurrency,
                              toCurrency,
                              fromValue,
                              toValue,
                              t.postDate(),
                              fract,
                              parentWidget, "currencyCalculator");

      if(calc.exec() == QDialog::Rejected) {
        rc = false;
      } else
        shares = (s.value() * calc.price()).convert(fract);

    } else {
      shares = s.value().convert(fract);
    }
  } else
    shares = s.value();

  return rc;
}

KCurrencyCalculator::KCurrencyCalculator(const MyMoneySecurity& from, const MyMoneySecurity& to, const MyMoneyMoney& value, const MyMoneyMoney& shares, const QDate& date, const signed64 resultFraction, QWidget *parent, const char *name ) :
  KCurrencyCalculatorDecl(parent, name),
  m_fromCurrency(from),
  m_toCurrency(to),
  m_result(shares.abs()),
  m_value(value.abs()),
  m_resultFraction(resultFraction)
{
  MyMoneyFile* file = MyMoneyFile::instance();

  m_dateFrame->hide();
  m_dateEdit->setDate(date);

  m_fromCurrencyText->setText(m_fromCurrency.isCurrency() ? m_fromCurrency.id() : m_fromCurrency.tradingSymbol());
  m_toCurrencyText->setText(m_toCurrency.isCurrency() ? m_toCurrency.id() : m_toCurrency.tradingSymbol());

  m_fromAmount->setText(m_value.formatMoney("", MyMoneyMoney::denomToPrec(m_fromCurrency.smallestAccountFraction())));
  m_dateText->setText(KGlobal::locale()->formatDate(date, true));

  m_fromType->setText(KMyMoneyUtils::securityTypeToString(m_fromCurrency.securityType()));
  m_toType->setText(KMyMoneyUtils::securityTypeToString(m_toCurrency.securityType()));

  // load button icons
  m_cancelButton->setGuiItem(KStdGuiItem::cancel());
  m_okButton->setGuiItem(KStdGuiItem::ok());

  m_updateButton->setChecked(KMyMoneySettings::priceHistoryUpdate());

  // setup initial result
  if(m_result == MyMoneyMoney() && !m_value.isZero()) {
    MyMoneyPrice pr = file->price(m_fromCurrency.id(), m_toCurrency.id(), date);
    if(pr.isValid()) {
      m_result = m_value * pr.rate(m_fromCurrency.id());
    }
  }

  // fill in initial values
  m_toAmount->loadText(m_result.formatMoney("", MyMoneyMoney::denomToPrec(m_resultFraction)));

  connect(m_amountButton, SIGNAL(clicked()), this, SLOT(slotSetToAmount()));
  connect(m_rateButton, SIGNAL(clicked()), this, SLOT(slotSetExchangeRate()));

  connect(m_toAmount, SIGNAL(valueChanged(const QString&)), this, SLOT(slotUpdateResult(const QString&)));
  connect(m_conversionRate, SIGNAL(valueChanged(const QString&)), this, SLOT(slotUpdateRate(const QString&)));
  connect(m_cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
  connect(m_okButton, SIGNAL(clicked()), this, SLOT(accept()));

  // use this as the default
  m_amountButton->animateClick();
  slotUpdateResult(m_toAmount->text());

  // If the from security is not a currency, we only allow entering a price
  if(!m_fromCurrency.isCurrency()) {
    m_rateButton->animateClick();
    m_amountButton->hide();
    m_toAmount->hide();
  }
  m_okButton->setFocus();
}

KCurrencyCalculator::~KCurrencyCalculator()
{
}

void KCurrencyCalculator::setupPriceEditor(void)
{
  m_dateFrame->show();
  m_amountDateFrame->hide();
  m_updateButton->setChecked(true);
  m_updateButton->hide();
}

void KCurrencyCalculator::slotSetToAmount(void)
{
  m_rateButton->setChecked(false);
  m_toAmount->setEnabled(true);
  m_conversionRate->setEnabled(false);
}

void KCurrencyCalculator::slotSetExchangeRate(void)
{
  m_amountButton->setChecked(false);
  m_toAmount->setEnabled(false);
  m_conversionRate->setEnabled(true);
}

void KCurrencyCalculator::slotUpdateResult(const QString& /*txt*/)
{
  MyMoneyMoney result = m_toAmount->value();
  MyMoneyMoney price(0, 1);

  if(result.isNegative()) {
    m_toAmount->setValue(-result);
    slotUpdateResult(QString());
    return;
  }

  if(!result.isZero()) {
    price = result / m_value;

    m_conversionRate->loadText(price.formatMoney("", KMyMoneySettings::pricePrecision()));
    m_result = (m_value * price).convert(m_resultFraction);
    m_toAmount->loadText(m_result.formatMoney("", MyMoneyMoney::denomToPrec(m_resultFraction)));
  }
  updateExample(price);
}

void KCurrencyCalculator::slotUpdateRate(const QString& /*txt*/)
{
  MyMoneyMoney price = m_conversionRate->value();

  if(price.isNegative()) {
    m_conversionRate->setValue(-price);
    slotUpdateRate(QString());
    return;
  }

  if(!price.isZero()) {
    m_conversionRate->loadText(price.formatMoney("", KMyMoneySettings::pricePrecision()));
    m_result = (m_value * price).convert(m_resultFraction);
    m_toAmount->loadText(m_result.formatMoney("", MyMoneyMoney::denomToPrec(m_resultFraction)));
  }
  updateExample(price);
}

void KCurrencyCalculator::updateExample(const MyMoneyMoney& price)
{
  QString msg;
  if(price.isZero()) {
    msg = QString("1 %1 = ? %2").arg(m_fromCurrency.tradingSymbol())
                                .arg(m_toCurrency.tradingSymbol());
    if(m_fromCurrency.isCurrency()) {
      msg += QString("\n");
      msg += QString("1 %1 = ? %2").arg(m_toCurrency.tradingSymbol())
                                     .arg(m_fromCurrency.tradingSymbol());
    }
  } else {
    msg = QString("1 %1 = %2 %3").arg(m_fromCurrency.tradingSymbol())
                                 .arg(price.formatMoney("", KMyMoneySettings::pricePrecision()))
                                 .arg(m_toCurrency.tradingSymbol());
    if(m_fromCurrency.isCurrency()) {
      msg += QString("\n");
      msg += QString("1 %1 = %2 %3").arg(m_toCurrency.tradingSymbol())
                                    .arg((MyMoneyMoney(1,1)/price).formatMoney("", KMyMoneySettings::pricePrecision()))
                                    .arg(m_fromCurrency.tradingSymbol());
    }
  }
  m_conversionExample->setText(msg);
  m_okButton->setEnabled(!price.isZero());
}

void KCurrencyCalculator::accept(void)
{
  if(m_conversionRate->isEnabled())
    slotUpdateRate(QString());
  else
    slotUpdateResult(QString());

  if(m_updateButton->isChecked()) {
    MyMoneyPrice pr = MyMoneyFile::instance()->price(m_fromCurrency.id(), m_toCurrency.id(), m_dateEdit->date());
    if(!pr.isValid()
    || pr.date() != m_dateEdit->date()
    || (pr.date() == m_dateEdit->date() && pr.rate(m_fromCurrency.id()) != price())) {
      pr = MyMoneyPrice(m_fromCurrency.id(), m_toCurrency.id(), m_dateEdit->date(), price(), i18n("User"));
      MyMoneyFileTransaction ft;
      try {
        MyMoneyFile::instance()->addPrice(pr);
        ft.commit();
      } catch(MyMoneyException *e) {
        qDebug("Cannot add price");
        delete e;
      }
    }
  }

  // remember setting for next round
  KMyMoneySettings::setPriceHistoryUpdate(m_updateButton->isChecked());

  KCurrencyCalculatorDecl::accept();
}

const MyMoneyMoney KCurrencyCalculator::price(void) const
{
  return m_result / m_value;
}


#include "kcurrencycalculator.moc"
