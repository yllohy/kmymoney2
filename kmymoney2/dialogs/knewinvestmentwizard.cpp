/***************************************************************************
                         knewinvestmentwizard  -  description
                            -------------------
   begin                : Sat Dec 4 2004
   copyright            : (C) 2004 by Thomas Baumgart
   email                : kmymoney2-developer@lists.sourceforge.net
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

#include <kpushbutton.h>
#include <kcombobox.h>
#include <kurlrequester.h>
#include <klocale.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "knewinvestmentwizard.h"

#include "../widgets/kmymoneylineedit.h"
#include "../widgets/kmymoneyedit.h"
#include "../widgets/kmymoneycurrencyselector.h"
#include "../mymoney/mymoneysecurity.h"
#include "../mymoney/mymoneyfile.h"
#include "../converter/webpricequote.h"
#include "../kmymoneyutils.h"

KNewInvestmentWizard::KNewInvestmentWizard( QWidget *parent, const char *name ) :
  KNewInvestmentWizardDecl( parent, name )
{
  init();
}

KNewInvestmentWizard::KNewInvestmentWizard( const MyMoneyAccount& acc, QWidget *parent, const char *name ) :
  KNewInvestmentWizardDecl( parent, name )
{
  setCaption(i18n("Investment detail wizard"));
  init();

  m_account = acc;

  // make sure the first page is not shown
  setAppropriate(m_investmentTypePage, false);
  showPage(m_investmentDetailsPage);

  // load the widgets with the data
  m_investmentName->setText(m_account.name());

  m_security = MyMoneyFile::instance()->security(m_account.currencyId());
  MyMoneySecurity tradingCurrency = MyMoneyFile::instance()->currency(m_security.tradingCurrency());

  m_investmentSymbol->setText(m_security.tradingSymbol());
  m_tradingMarket->setCurrentText(m_security.tradingMarket());
  m_fraction->setValue(MyMoneyMoney(m_security.smallestAccountFraction(), 1));
  m_tradingCurrencyEdit->setSecurity(tradingCurrency);

  m_onlineSourceCombo->setCurrentText(m_security.value("kmm-online-source"));

  // we can't see this one because the page is hidden, but we have to
  // set it anyway because during createObjects() we use the value
  m_securityType->setCurrentText(KMyMoneyUtils::securityTypeToString(m_security.securityType()));
}

KNewInvestmentWizard::KNewInvestmentWizard( const MyMoneySecurity& security, QWidget *parent, const char *name ) :
  KNewInvestmentWizardDecl( parent, name )
{
  setCaption(i18n("Security detail wizard"));
  init();
  m_createAccount = false;

  // make sure the first page is not shown
  setAppropriate(m_investmentTypePage, false);
  showPage(m_investmentDetailsPage);

  // load the widgets with the data
  m_security = security;
  m_investmentName->setText(security.name());
  MyMoneySecurity tradingCurrency = MyMoneyFile::instance()->currency(m_security.tradingCurrency());

  m_investmentSymbol->setText(m_security.tradingSymbol());
  m_tradingMarket->setCurrentText(m_security.tradingMarket());
  m_fraction->setValue(MyMoneyMoney(m_security.smallestAccountFraction(), 1));
  m_tradingCurrencyEdit->setSecurity(tradingCurrency);

  m_onlineSourceCombo->setCurrentText(m_security.value("kmm-online-source"));

  // we can't see this one because the page is hidden, but we have to
  // set it anyway because during createObjects() we use the value
  m_securityType->setCurrentText(KMyMoneyUtils::securityTypeToString(m_security.securityType()));
}

void KNewInvestmentWizard::init(void)
{
  m_onlineSourceCombo->insertStringList( WebPriceQuote::quoteSources() );

  m_fraction->setPrecision(0);
  QIntValidator* fractionValidator = new QIntValidator(1, 100000, this);
  m_fraction->setValidator(fractionValidator);

  // FIXME for now, we don't have online help
  helpButton()->hide();

  connect(m_investmentName, SIGNAL(textChanged(const QString&)), this, SLOT(slotCheckPage(void)));
  connect(m_investmentSymbol, SIGNAL(textChanged(const QString&)), this, SLOT(slotCheckPage(void)));
  connect(m_fraction, SIGNAL(textChanged(const QString&)), this, SLOT(slotCheckPage(void)));
  connect(m_investmentIdentification, SIGNAL(textChanged(const QString&)), this, SLOT(slotCheckPage(void)));

  m_createAccount = true;
}

KNewInvestmentWizard::~KNewInvestmentWizard()
{
}

void KNewInvestmentWizard::next(void)
{
  KNewInvestmentWizardDecl::next();
  slotCheckPage();
}

void KNewInvestmentWizard::slotCheckPage(void)
{
  if(currentPage() == m_investmentDetailsPage) {
    setNextEnabled(m_investmentDetailsPage, false);
    if(m_investmentName->text().length() > 0
    && m_investmentSymbol->text().length() > 0
    && !m_fraction->value().isZero()) {
      setNextEnabled(m_investmentDetailsPage, true);
    }
  } else if(currentPage() == m_onlineUpdatePage) {
    setFinishEnabled(m_onlineUpdatePage, true);
  }
}

void KNewInvestmentWizard::createObjects(const QCString& parentId)
{
  MyMoneyFile* file = MyMoneyFile::instance();

  QValueList<MyMoneySecurity> list = MyMoneyFile::instance()->securityList();
  QValueList<MyMoneySecurity>::ConstIterator it;

  // check if we already have the security
  MyMoneySecurity::eSECURITYTYPE type = KMyMoneyUtils::stringToSecurity(m_securityType->currentText());
  if(m_security.id().isEmpty()) {
    for(it = list.begin(); m_security.id().isEmpty() && it != list.end(); ++it) {
      if((*it).securityType() == type
      && (*it).tradingSymbol() == m_investmentSymbol->text()) {
        m_security = *it;
      }
    }
  }

  // update all relevant attributes only, if we create a stock
  // account and the security is unknown or we modifiy the security
  MyMoneySecurity newSecurity(m_security);
  newSecurity.setName(m_investmentName->text());
  newSecurity.setTradingSymbol(m_investmentSymbol->text());
  newSecurity.setTradingMarket(m_tradingMarket->currentText());
  newSecurity.setSmallestAccountFraction(m_fraction->value());
  newSecurity.setTradingCurrency(m_tradingCurrencyEdit->security().id());
  newSecurity.setValue("kmm-online-source", m_onlineSourceCombo->currentText());

  if(m_security.id().isEmpty() || newSecurity != m_security) {
    m_security = newSecurity;

    // if the security was not found, we have to create it while not forgetting
    // to setup the type
    if(m_security.id().isEmpty()) {
      m_security.setSecurityType(type);
      file->addSecurity(m_security);

    } else {
      file->modifySecurity(m_security);
    }
  }

  if(m_createAccount) {
    // now that the security exists, we can add the account to store it
    m_account.setName(m_investmentName->text());
    m_account.setAccountType(MyMoneyAccount::Stock);
    m_account.setCurrencyId(m_security.id());

    MyMoneyAccount parent = file->account(parentId);
    if(m_account.id().isEmpty())
      file->addAccount(m_account, parent);
    else
      file->modifyAccount(m_account);
  }
}

#include "knewinvestmentwizard.moc"
