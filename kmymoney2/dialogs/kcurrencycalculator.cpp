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
#include <kconfig.h>
#include <kiconloader.h>
#include <kguiitem.h>
#include <kcombobox.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kcurrencycalculator.h"
#include "../widgets/kmymoneyedit.h"
#include "../widgets/kmymoneycurrencyselector.h"
#include "../mymoney/mymoneyprice.h"
#include "../kmymoneyutils.h"

KCurrencyCalculator::KCurrencyCalculator(const MyMoneySecurity& from, const MyMoneySecurity& to, const MyMoneyMoney& value, const MyMoneyMoney& shares, const QDate& date, const int resultFraction, QWidget *parent, const char *name ) :
  KCurrencyCalculatorDecl(parent, name),
  m_fromCurrency(from),
  m_toCurrency(to),
  m_result(shares.abs()),
  m_value(value.abs()),
  m_date(date),
  m_resultFraction(resultFraction)
{
  MyMoneyFile* file = MyMoneyFile::instance();

  m_fromCurrencyText->setText(m_fromCurrency.isCurrency() ? m_fromCurrency.id() : m_fromCurrency.tradingSymbol());
  m_toCurrencyText->setText(m_toCurrency.isCurrency() ? m_toCurrency.id() : m_toCurrency.tradingSymbol());

  m_fromAmount->setText(m_value.formatMoney("", MyMoneyMoney::denomToPrec(m_fromCurrency.smallestAccountFraction())));
  m_dateText->setText(KGlobal::locale()->formatDate(m_date, true));

  m_fromType->setText(KMyMoneyUtils::securityTypeToString(m_fromCurrency.securityType()));
  m_toType->setText(KMyMoneyUtils::securityTypeToString(m_toCurrency.securityType()));

  // load button icons
  KIconLoader* il = KGlobal::iconLoader();
  KGuiItem cancelButtenItem( i18n( "&Cancel" ),
                      QIconSet(il->loadIcon("button_cancel", KIcon::Small, KIcon::SizeSmall)),
                      i18n("Reject any changes"),
                      i18n("Use this to abort the dialog"));
  m_cancelButton->setGuiItem(cancelButtenItem);

  KGuiItem okButtenItem( i18n( "&OK" ),
                      QIconSet(il->loadIcon("button_ok", KIcon::Small, KIcon::SizeSmall)),
                      i18n("Accept modifications"),
                      i18n("Use this to accept the data and possibly update the exchange rate"));
  m_okButton->setGuiItem(okButtenItem);

  KConfig *kconfig = KGlobal::config();
  kconfig->setGroup("General Options");
  m_ratePrec = kconfig->readNumEntry("PricePrecision", 4);
  m_updateButton->setChecked(kconfig->readBoolEntry("PriceHistoryUpdate", true));

  // setup initial result
  if(m_result == MyMoneyMoney() && !m_value.isZero()) {
    MyMoneyPrice pr = file->price(m_fromCurrency.id(), m_toCurrency.id(), m_date);
    if(pr.isValid()) {
      m_result = m_value * pr.rate(m_fromCurrency.id());
    }
  }

  // fill in initial values
  m_toAmount->loadText(m_result.formatMoney("", MyMoneyMoney::denomToPrec(m_toCurrency.smallestAccountFraction())));

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
}

KCurrencyCalculator::~KCurrencyCalculator()
{
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

    m_conversionRate->loadText(price.formatMoney("", m_ratePrec));
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
    m_conversionRate->loadText(price.formatMoney("", m_ratePrec));
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
                                 .arg(price.formatMoney("", m_ratePrec))
                                 .arg(m_toCurrency.tradingSymbol());
    if(m_fromCurrency.isCurrency()) {
      msg += QString("\n");
      msg += QString("1 %1 = %2 %3").arg(m_toCurrency.tradingSymbol())
                                    .arg((MyMoneyMoney(1,1)/price).formatMoney("", m_ratePrec))
                                    .arg(m_fromCurrency.tradingSymbol());
    }
  }
  m_conversionExample->setText(msg);
}

void KCurrencyCalculator::accept(void)
{
  if(m_conversionRate->isEnabled())
    slotUpdateRate(QString());
  else
    slotUpdateResult(QString());

  if(m_updateButton->isChecked()) {
    MyMoneyPrice pr = MyMoneyFile::instance()->price(m_fromCurrency.id(), m_toCurrency.id(), m_date);
    if(!pr.isValid()
    || pr.date() != m_date
    || (pr.date() == m_date && pr.rate(m_fromCurrency.id()) != price())) {
      pr = MyMoneyPrice(m_fromCurrency.id(), m_toCurrency.id(), m_date, price(), i18n("User"));
      MyMoneyFile::instance()->addPrice(pr);
    }
  }

  KConfig *kconfig = KGlobal::config();
  kconfig->setGroup("General Options");
  kconfig->writeEntry("PriceHistoryUpdate", m_updateButton->isChecked());

  KCurrencyCalculatorDecl::accept();
}

const MyMoneyMoney KCurrencyCalculator::price(void) const
{
  return m_result / m_value;
}


#include "kcurrencycalculator.moc"
