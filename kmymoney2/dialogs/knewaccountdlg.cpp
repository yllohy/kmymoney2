/***************************************************************************
                          knewaccountdlg.cpp
                             -------------------
    copyright            : (C) 2000 by Michael Edwardes
                               2004 by Thomas Baumgart
    email                : mte@users.sourceforge.net
                           ipwizard@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// ----------------------------------------------------------------------------
// QT Includes

#include <qpixmap.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qheader.h>
#include <qtooltip.h>
#include <qcheckbox.h>
#include <qtimer.h>
#include <qtabwidget.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qtextedit.h>
#include <qlayout.h>

// ----------------------------------------------------------------------------
// KDE Headers

#include <klocale.h>
#include <kglobal.h>
#include <kmessagebox.h>
#include <kcombobox.h>
#include <klistview.h>
#include <kstandarddirs.h>
#include <kiconloader.h>
#include <kguiitem.h>
#include <kpushbutton.h>
#include <kled.h>
#include <kdebug.h>
#include <kglobalsettings.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "knewaccountdlg.h"

#include <kmymoney/kmymoneyedit.h>
#include <kmymoney/kmymoneydateinput.h>
#include <kmymoney/mymoneyexception.h>
#include <kmymoney/mymoneyfile.h>
#include <kmymoney/kmymoneyaccounttree.h>
#include <kmymoney/kmymoneyglobalsettings.h>
#include <kmymoney/mymoneyreport.h>
#include <kmymoney/kguiutils.h>

#include "../widgets/kmymoneycurrencyselector.h"
#include "../widgets/kmymoneyaccountselector.h"

#include "../mymoney/mymoneyexception.h"
#include "../mymoney/mymoneykeyvaluecontainer.h"
#include "../dialogs/knewbankdlg.h"
#include "../views/kmymoneyfile.h"
#include "../kmymoneyutils.h"

#include "../reports/kreportchartview.h"
#include "../reports/pivottable.h"

#ifdef USE_OFX_DIRECTCONNECT
#include "../dialogs/konlinebankingsetupwizard.h"
#endif

// in KOffice version < 1.5 KDCHART_PROPSET_NORMAL_DATA was a static const
// but in 1.5 this has been changed into a #define'd value. So we have to
// make sure, we use the right one.
#ifndef KDCHART_PROPSET_NORMAL_DATA
#define KMM_KDCHART_PROPSET_NORMAL_DATA KDChartParams::KDCHART_PROPSET_NORMAL_DATA
#else
#define KMM_KDCHART_PROPSET_NORMAL_DATA KDCHART_PROPSET_NORMAL_DATA
#endif

KNewAccountDlg::KNewAccountDlg(const MyMoneyAccount& account, bool isEditing, bool categoryEditor, QWidget *parent, const char *name, const QString& title)
  : KNewAccountDlgDecl(parent,name,true),
    m_account(account),
    m_bSelectedParentAccount(false),
    m_chartWidget(0),
    m_categoryEditor(categoryEditor),
    m_isEditing(isEditing)
{
  QString columnName = ( (categoryEditor) ? i18n("Categories") : i18n("Accounts") );

  m_qlistviewParentAccounts->setRootIsDecorated(true);
  m_qlistviewParentAccounts->setAllColumnsShowFocus(true);
  m_qlistviewParentAccounts->addColumn(columnName);
  m_qlistviewParentAccounts->setMultiSelection(false);
  m_qlistviewParentAccounts->header()->setResizeEnabled(true);
  m_qlistviewParentAccounts->setColumnWidthMode(0, QListView::Maximum);
  m_qlistviewParentAccounts->setEnabled(false);
  // never show the horizontal scroll bar
  m_qlistviewParentAccounts->setHScrollBarMode(QScrollView::AlwaysOff);

  m_subAccountLabel->setText(i18n("Is a sub account"));

  m_qlistviewParentAccounts->header()->setFont(KMyMoneyUtils::headerFont());

  accountNameEdit->setText(account.name());
  descriptionEdit->setText(account.description());

  typeCombo->setEnabled(true);
  MyMoneyFile *file = MyMoneyFile::instance();

  bool haveMinBalance = false;
  bool haveMaxCredit = false;
  if (categoryEditor)
  {

    // get rid of the tabs that are not used for categories
    QWidget* tab = m_tab->page(m_tab->indexOf(m_balanceTab));
    if(tab)
      m_tab->removePage(tab);
    tab = m_tab->page(m_tab->indexOf(m_institutionTab));
    if(tab)
      m_tab->removePage(tab);
    tab = m_tab->page(m_tab->indexOf(m_limitsTab));
    if(tab)
      m_tab->removePage(tab);

    //m_qlistviewParentAccounts->setEnabled(true);
    startDateEdit->setEnabled(false);
    accountNoEdit->setEnabled(false);

    m_institutionBox->hide();
    m_onlineBankingBox->hide();
    m_qcheckboxNoVat->hide();

    typeCombo->insertItem(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::Income));
    typeCombo->insertItem(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::Expense));

    // Hardcoded but acceptable
    switch (account.accountType())
    {
      case MyMoneyAccount::Income:
        typeCombo->setCurrentItem(0);
        break;

      case MyMoneyAccount::Expense:
      default:
        typeCombo->setCurrentItem(1);
        break;
    }
    m_currency->setEnabled(true);
    if (m_isEditing)
    {
      typeCombo->setEnabled(false);
      m_currency->setDisabled(MyMoneyFile::instance()->isReferenced(m_account));
    }
    m_qcheckboxPreferred->hide();

    m_qcheckboxTax->setChecked(account.value("Tax") == "Yes");
    loadVatAccounts();
  }
  else
  {
    // get rid of the tabs that are not used for accounts
    QWidget* tab = m_tab->page(m_tab->indexOf(m_taxTab));
    if(tab)
      m_tab->removePage(tab);
#ifndef HAVE_KDCHART
    tab = m_tab->page(m_tab->indexOf(m_balanceTab));
    if(tab)
      m_tab->removePage(tab);
#endif

    switch(m_account.accountType()) {
      case MyMoneyAccount::Savings:
      case MyMoneyAccount::Cash:
        haveMinBalance = true;
        break;

      case MyMoneyAccount::Checkings:
        haveMinBalance = true;
        haveMaxCredit = true;
        break;

      case MyMoneyAccount::CreditCard:
        haveMaxCredit = true;
        break;

      default:
        // no limit available, so we might get rid of the tab
        tab = m_tab->page(m_tab->indexOf(m_limitsTab));
        if(tab)
          m_tab->removePage(tab);
        // don't try to hide the widgets we just wiped
        // in the next step
        haveMaxCredit = haveMinBalance = true;
        break;
    }

    if(!haveMaxCredit) {
      m_maxCreditLabel->hide();
      m_maxCreditEarlyEdit->hide();
      m_maxCreditAbsoluteEdit->hide();
    }
    if(!haveMinBalance) {
      m_minBalanceLabel->hide();
      m_minBalanceEarlyEdit->hide();
      m_minBalanceAbsoluteEdit->hide();
    }

    typeCombo->insertItem(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::Checkings));
    typeCombo->insertItem(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::Savings));
    typeCombo->insertItem(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::Cash));
    typeCombo->insertItem(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::CreditCard));
    typeCombo->insertItem(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::Loan));
    typeCombo->insertItem(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::Investment));
    typeCombo->insertItem(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::Asset));
    typeCombo->insertItem(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::Liability));
    typeCombo->insertItem(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::Stock));
/*
    typeCombo->insertItem(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::CertificateDep));
    typeCombo->insertItem(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::MoneyMarket));
    typeCombo->insertItem(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::Currency));
*/

    // Hardcoded but acceptable
    switch (account.accountType())
    {
      default:
      case MyMoneyAccount::Checkings:
        typeCombo->setCurrentItem(0);
        break;
      case MyMoneyAccount::Savings:
        typeCombo->setCurrentItem(1);
        break;
      case MyMoneyAccount::Cash:
        typeCombo->setCurrentItem(2);
        break;
      case MyMoneyAccount::CreditCard:
        typeCombo->setCurrentItem(3);
        break;
      case MyMoneyAccount::Loan:
        typeCombo->setCurrentItem(4);
        break;
      case MyMoneyAccount::Investment:
        typeCombo->setCurrentItem(5);
        break;
      case MyMoneyAccount::Asset:
        typeCombo->setCurrentItem(6);
        break;
      case MyMoneyAccount::Liability:
        typeCombo->setCurrentItem(7);
        break;
      case MyMoneyAccount::Stock:
        m_institutionBox->hide();
        m_onlineBankingBox->hide();
        typeCombo->setCurrentItem(8);
        break;
/*
      case MyMoneyAccount::CertificateDep:
        typeCombo->setCurrentItem(5);
        break;
      case MyMoneyAccount::MoneyMarket:
        typeCombo->setCurrentItem(7);
        break;
      case MyMoneyAccount::Currency:
        typeCombo->setCurrentItem(8);
        break;
*/
    }

    if(!m_account.openingDate().isValid())
      m_account.setOpeningDate(QDate::currentDate());

    startDateEdit->setDate(m_account.openingDate());
    accountNoEdit->setText(account.number());
    m_qcheckboxPreferred->setChecked(account.value("PreferredAccount") == "Yes");
    m_qcheckboxNoVat->setChecked(account.value("NoVat") == "Yes");
    loadKVP("iban", ibanEdit);
    loadKVP("minBalanceAbsolute", m_minBalanceAbsoluteEdit);
    loadKVP("minBalanceEarly", m_minBalanceEarlyEdit);
    loadKVP("maxCreditAbsolute", m_maxCreditAbsoluteEdit);
    loadKVP("maxCreditEarly", m_maxCreditEarlyEdit);
    loadKVP("lastNumberUsed", m_lastCheckNumberUsed);

    // we do not allow to change the account type once an account
    // was created. Same applies to currency if it is referenced.
    if (m_isEditing)
    {
      typeCombo->setEnabled(false);
      m_currency->setDisabled(MyMoneyFile::instance()->isReferenced(m_account));
    }
    if(m_account.isInvest()) {
      typeCombo->setEnabled(false);
      m_qcheckboxPreferred->hide();
      m_currencyText->hide();
      m_currency->hide();
    } else {
      // use the old field and override a possible new value
      if(!MyMoneyMoney(account.value("minimumBalance")).isZero()) {
        m_minBalanceAbsoluteEdit->setValue(MyMoneyMoney(account.value("minimumBalance")));
      }
    }

    m_qcheckboxTax->hide();

    displayOnlineBankingStatus();

  }

  m_currency->setSecurity(file->currency(account.currencyId()));

  // Load the institutions
  // then the accounts
  QString institutionName;

  try
  {
    if (m_isEditing && !account.institutionId().isEmpty())
      institutionName = file->institution(account.institutionId()).name();
    else
      institutionName = QString();
  }
  catch (MyMoneyException *e)
  {
    qDebug("exception in init for account dialog: %s", e->what().latin1());
    delete e;
  }

  initParentWidget(account.parentAccountId(), account.id());
  if(m_account.isInvest())
    m_qlistviewParentAccounts->setEnabled(false);

  if (!categoryEditor)
    slotLoadInstitutions(institutionName);

  accountNameEdit->setFocus();

  if (title)
    setCaption(title);

  // load button icons
  KIconLoader* il = KGlobal::iconLoader();
  KGuiItem cancelButtenItem( i18n( "&Cancel" ),
                      QIconSet(il->loadIcon("button_cancel", KIcon::Small, KIcon::SizeSmall)),
                      i18n("Reject any changes"),
                      i18n("Use this to abort the account/category dialog"));
  cancelButton->setGuiItem(cancelButtenItem);

  KGuiItem okButtenItem( i18n( "&OK" ),
                      QIconSet(il->loadIcon("button_ok", KIcon::Small, KIcon::SizeSmall)),
                      i18n("Accept modifications"),
                      i18n("Use this to accept the data and possibly create the account/category"));
  createButton->setGuiItem(okButtenItem);

  connect(cancelButton, SIGNAL(clicked()), SLOT(reject()));
  connect(createButton, SIGNAL(clicked()), this, SLOT(okClicked()));
  connect(m_qlistviewParentAccounts, SIGNAL(selectionChanged(QListViewItem*)),
    this, SLOT(slotSelectionChanged(QListViewItem*)));
  connect(m_qbuttonNew, SIGNAL(clicked()), this, SLOT(slotNewClicked()));
  connect(m_buttonOnlineSetup, SIGNAL(clicked()), this, SLOT(slotOnlineSetupClicked()));
  connect(typeCombo, SIGNAL(activated(const QString&)),
    this, SLOT(slotAccountTypeChanged(const QString&)));

  connect(accountNameEdit, SIGNAL(textChanged(const QString&)), this, SLOT(slotCheckFinished()));

  connect(m_vatCategory, SIGNAL(toggled(bool)), this, SLOT(slotVatChanged(bool)));
  connect(m_vatAssignment, SIGNAL(toggled(bool)), this, SLOT(slotVatAssignmentChanged(bool)));
  connect(m_vatCategory, SIGNAL(toggled(bool)), this, SLOT(slotCheckFinished()));
  connect(m_vatAssignment, SIGNAL(toggled(bool)), this, SLOT(slotCheckFinished()));
  connect(m_vatRate, SIGNAL(textChanged(const QString&)), this, SLOT(slotCheckFinished()));
  connect(m_vatAccount, SIGNAL(stateChanged()), this, SLOT(slotCheckFinished()));

  connect(m_minBalanceEarlyEdit->lineedit(), SIGNAL(textChanged(const QString&)), this, SLOT(slotMarkersChanged()));
  connect(m_minBalanceAbsoluteEdit->lineedit(), SIGNAL(textChanged(const QString&)), this, SLOT(slotMarkersChanged()));
  connect(m_maxCreditEarlyEdit->lineedit(), SIGNAL(textChanged(const QString&)), this, SLOT(slotMarkersChanged()));
  connect(m_maxCreditAbsoluteEdit->lineedit(), SIGNAL(textChanged(const QString&)), this, SLOT(slotMarkersChanged()));

  connect(m_minBalanceEarlyEdit, SIGNAL(valueChanged(const QString&)), this, SLOT(slotAdjustMinBalanceAbsoluteEdit(const QString&)));
  connect(m_minBalanceAbsoluteEdit, SIGNAL(valueChanged(const QString&)), this, SLOT(slotAdjustMinBalanceEarlyEdit(const QString&)));
  connect(m_maxCreditEarlyEdit, SIGNAL(valueChanged(const QString&)), this, SLOT(slotAdjustMaxCreditAbsoluteEdit(const QString&)));
  connect(m_maxCreditAbsoluteEdit, SIGNAL(valueChanged(const QString&)), this, SLOT(slotAdjustMaxCreditEarlyEdit(const QString&)));

  connect(m_qcomboboxInstitutions, SIGNAL(activated(const QString&)), this, SLOT(slotLoadInstitutions(const QString&)));

  m_vatCategory->setChecked(false);
  m_vatAssignment->setChecked(false);

  // make sure our account does not have an id and no parent assigned
  // and certainly no children in case we create a new account
  if(!m_isEditing) {
    m_account.clearId();
    m_account.setParentAccountId(QCString());
    QCStringList::ConstIterator it;
    while((it = m_account.accountList().begin()) != m_account.accountList().end())
      m_account.removeAccountId(*it);

    if(m_parentItem == 0) {
      // force loading of initial parent
      m_account.setAccountType(MyMoneyAccount::UnknownAccountType);
      MyMoneyAccount::_accountTypeE type = account.accountType();
      if(type == MyMoneyAccount::UnknownAccountType)
        type = MyMoneyAccount::Checkings;
      slotAccountTypeChanged(KMyMoneyUtils::accountTypeToString(type));
    }
  } else {
    if(categoryEditor) {
      if(!m_account.value("VatRate").isEmpty()) {
        m_vatCategory->setChecked(true);
        m_vatRate->setValue(MyMoneyMoney(m_account.value("VatRate"))*MyMoneyMoney(100,1));
      } else {
        if(!m_account.value("VatAccount").isEmpty()) {
          QCString accId = m_account.value("VatAccount").latin1();
          try {
            // make sure account exists
            MyMoneyFile::instance()->account(accId);
            m_vatAssignment->setChecked(true);
            m_vatAccount->setSelected(accId);
            m_grossAmount->setChecked(true);
            if(m_account.value("VatAmount") == "Net")
              m_netAmount->setChecked(true);
          } catch(MyMoneyException *e) {
            delete e;
          }
        }
      }
    }
  }
  slotVatChanged(m_vatCategory->isChecked());
  slotVatAssignmentChanged(m_vatAssignment->isChecked());
  slotCheckFinished();

#ifdef HAVE_KDCHART

  // only create the chart, if the resp. page is still available
  if(m_tab->indexOf(m_balanceTab) != -1) {
    MyMoneyReport reportCfg = MyMoneyReport(
      MyMoneyReport::eAssetLiability,
      MyMoneyReport::eMonths,
      MyMoneyTransactionFilter::userDefined, // overridden by the setDateFilter() call below
      false,
      i18n("%1 Balance History").arg(m_account.name()),
      i18n("Generated Report")
    );
    reportCfg.setChartByDefault(true);
    reportCfg.setChartGridLines(false);
    reportCfg.setChartDataLabels(false);
    reportCfg.setDetailLevel(MyMoneyReport::eDetailTotal);
    reportCfg.setChartType(MyMoneyReport::eChartLine);
    reportCfg.setIncludingSchedules( true );
    if(m_account.accountType() == MyMoneyAccount::Investment) {
      QCStringList::const_iterator it_a;
      for(it_a = m_account.accountList().begin(); it_a != m_account.accountList().end(); ++it_a)
        reportCfg.addAccount(*it_a);
    } else
      reportCfg.addAccount(m_account.id());
    reportCfg.setColumnsAreDays( true );
    reportCfg.setConvertCurrency( false );
    reportCfg.setDateFilter(QDate::currentDate().addDays(-90),QDate::currentDate().addDays(+90));

    reports::PivotTable table(reportCfg);

    m_chartWidget = new reports::KReportChartView(m_balanceTab, 0);

    QVBoxLayout* balanceTabLayout = new QVBoxLayout( m_balanceTab, 11, 6, "m_balanceTabLayout");
    balanceTabLayout->addWidget(m_chartWidget);

    table.drawChart(*m_chartWidget);

    m_chartWidget->params().setLineMarker(false);
    m_chartWidget->params().setLegendPosition(KDChartParams::NoLegend);
    m_chartWidget->params().setLineWidth(2);
    m_chartWidget->params().setDataColor(0, KGlobalSettings::textColor());

    // draw future values in a different line style
    KDChartPropertySet propSetFutureValue("future value", KMM_KDCHART_PROPSET_NORMAL_DATA);
    KDChartPropertySet propSetLastValue("last value", KMM_KDCHART_PROPSET_NORMAL_DATA);

    propSetLastValue.setExtraLinesAlign(KDChartPropertySet::OwnID, Qt::AlignLeft | Qt::AlignBottom);
    propSetLastValue.setExtraLinesWidth(KDChartPropertySet::OwnID, -4);
    propSetLastValue.setExtraLinesColor(KDChartPropertySet::OwnID, KMyMoneyGlobalSettings::listGridColor());
    propSetFutureValue.setLineStyle(KDChartPropertySet::OwnID, Qt::DotLine);

    m_idPropFutureValue = m_chartWidget->params().registerProperties(propSetFutureValue);
    m_idPropLastValue = m_chartWidget->params().registerProperties(propSetLastValue);

    KDChartPropertySet propSetMinBalance("min balance", m_idPropLastValue);
    propSetMinBalance.setLineStyle(KDChartPropertySet::OwnID, Qt::NoPen);
    propSetMinBalance.setExtraLinesAlign(KDChartPropertySet::OwnID, Qt::AlignLeft | Qt::AlignRight);
    m_idPropMinBalance = m_chartWidget->params().registerProperties(propSetMinBalance);

    KDChartPropertySet propSetMaxCredit("max credit", m_idPropMinBalance);
    propSetMaxCredit.setExtraLinesColor(KDChartPropertySet::OwnID, KMyMoneyGlobalSettings::listNegativeValueColor());
    propSetMaxCredit.setExtraLinesStyle(KDChartPropertySet::OwnID, Qt::DotLine);
    m_idPropMaxCredit = m_chartWidget->params().registerProperties(propSetMaxCredit);

    slotMarkersChanged();
  }
#endif

  kMandatoryFieldGroup* requiredFields = new kMandatoryFieldGroup (this);
  requiredFields->setOkButton(createButton); // button to be enabled when all fields present
  requiredFields->add(accountNameEdit);

  // using a timeout is the only way, I got the 'ensureItemVisible'
  // working when creating the dialog. I assume, this
  // has something to do with the delayed update of the display somehow.
  QTimer::singleShot(50, this, SLOT(timerDone()));
}

void KNewAccountDlg::timerDone(void)
{
  if(m_accountItem) m_qlistviewParentAccounts->ensureItemVisible(m_accountItem);
  if(m_parentItem) m_qlistviewParentAccounts->ensureItemVisible(m_parentItem);
  // KNewAccountDlgDecl::resizeEvent(0);
  m_qlistviewParentAccounts->setColumnWidth(0, m_qlistviewParentAccounts->visibleWidth());
  m_qlistviewParentAccounts->repaintContents(false);
}

void KNewAccountDlg::setOpeningBalance(const MyMoneyMoney& balance)
{
  m_openingBalanceEdit->setValue(balance);
}

void KNewAccountDlg::setOpeningBalanceShown(bool shown)
{
  m_openingBalanceEdit->setShown(shown);
}

void KNewAccountDlg::okClicked()
{
  MyMoneyFile* file = MyMoneyFile::instance();

  QString accountNameText = accountNameEdit->text();
  if (accountNameText.isEmpty())
  {
    KMessageBox::error(this, i18n("You have not specified a name.\nPlease fill in this field."));
    accountNameEdit->setFocus();
    return;
  }

  MyMoneyAccount parent = parentAccount();
  if (parent.name().length() == 0)
  {
    KMessageBox::error(this, i18n("Please select a parent account."));
    return;
  }

  if (!m_categoryEditor)
  {
    QString institutionNameText = m_qcomboboxInstitutions->currentText();
    if (institutionNameText != i18n("<No Institution>"))
    {
      try
      {
        MyMoneyFile *file = MyMoneyFile::instance();

        QValueList<MyMoneyInstitution> list = file->institutionList();
        QValueList<MyMoneyInstitution>::ConstIterator institutionIterator;
        for (institutionIterator = list.begin(); institutionIterator != list.end(); ++institutionIterator)
        {
          if ((*institutionIterator).name() == institutionNameText)
            m_account.setInstitutionId((*institutionIterator).id());
        }
      }
      catch (MyMoneyException *e)
      {
        qDebug("Exception in account institution set: %s", e->what().latin1());
        delete e;
      }
    }
    else
    {
      m_account.setInstitutionId(QCString());
    }
  }

  m_account.setName(accountNameText);
  m_account.setNumber(accountNoEdit->text());
  storeKVP("iban", ibanEdit);
  storeKVP("minBalanceAbsolute", m_minBalanceAbsoluteEdit);
  storeKVP("minBalanceEarly", m_minBalanceEarlyEdit);
  storeKVP("maxCreditAbsolute", m_maxCreditAbsoluteEdit);
  storeKVP("maxCreditEarly", m_maxCreditEarlyEdit);
  storeKVP("lastNumberUsed", m_lastCheckNumberUsed);
  // delete a previous version of the minimumbalance information
  storeKVP("minimumBalance", QString(), QString());

  MyMoneyAccount::accountTypeE acctype;
  if (!m_categoryEditor)
  {
    acctype = KMyMoneyUtils::stringToAccountType(typeCombo->currentText());
    // If it's a loan, check if the parent is asset or liability. In
    // case of asset, we change the account type to be AssetLoan
    if(acctype == MyMoneyAccount::Loan
    && parent.accountGroup() == MyMoneyAccount::Asset)
      acctype = MyMoneyAccount::AssetLoan;

#if 0
    // we do allow the same name for different accounts, so
    // we don't need this check anymore.
    if(!file->nameToAccount(accountNameText).isEmpty()
    && (file->nameToAccount(accountNameText) != m_account.id())) {
      KMessageBox::error(this, QString("<qt>")+i18n("An account named <b>%1</b> already exists. You cannot create a second account with the same name.").arg(accountNameText)+QString("</qt>"));
      return;
    }
#endif
  }
  else
  {
    acctype = parent.accountGroup();
    QString newName;
    if(!MyMoneyFile::instance()->isStandardAccount(parent.id())) {
      newName = MyMoneyFile::instance()->accountToCategory(parent.id()) + MyMoneyFile::AccountSeperator;
    }
    newName += accountNameText;
    if(!file->categoryToAccount(newName, acctype).isEmpty()
    && (file->categoryToAccount(newName, acctype) != m_account.id())) {
      KMessageBox::error(this, QString("<qt>")+i18n("A category named <b>%1</b> already exists. You cannot create a second category with the same name.").arg(newName)+QString("</qt>"));
      return;
    }
  }
  m_account.setAccountType(acctype);

  m_account.setDescription(descriptionEdit->text());

  if (!m_categoryEditor)
  {
    m_account.setOpeningDate(startDateEdit->date());
    m_account.setCurrencyId(m_currency->security().id());

    if(m_qcheckboxPreferred->isChecked())
      m_account.setValue("PreferredAccount", "Yes");
    else
      m_account.deletePair("PreferredAccount");
    if(m_qcheckboxNoVat->isChecked())
      m_account.setValue("NoVat", "Yes");
    else
      m_account.deletePair("NoVat");

    if(m_minBalanceAbsoluteEdit->isVisible()) {
      m_account.setValue("minimumBalance", m_minBalanceAbsoluteEdit->value().toString());
    }
  }
  else
  {
    if(KMyMoneySettings::hideUnusedCategory() && !m_isEditing) {
      KMessageBox::information(this, i18n("You have selected to suppress the display of unused categories in the KMyMoney configuration dialog. The category you just created will therefore only be shown if it is used. Otherwise, it will be hidden in the accounts/categories view."), i18n("Hidden categories"), "NewHiddenCategory");
    }
  }

  if (m_categoryEditor)
  {
    if ( m_qcheckboxTax->isChecked())
      m_account.setValue("Tax", "Yes");
    else
      m_account.deletePair("Tax");

    m_account.deletePair("VatAccount");
    m_account.deletePair("VatAmount");
    m_account.deletePair("VatRate");

    if(m_vatCategory->isChecked()) {
      m_account.setValue("VatRate", (m_vatRate->value().abs() / MyMoneyMoney(100,1)).toString());
    } else {
      if(m_vatAssignment->isChecked()) {
        m_account.setValue("VatAccount", m_vatAccount->selectedItems().first());
        if(m_netAmount->isChecked())
          m_account.setValue("VatAmount", "Net");
      }
    }
  }

  accept();
}

void KNewAccountDlg::loadKVP(const QCString& key, kMyMoneyEdit* widget)
{
  if(!widget)
    return;

  if(m_account.value(key).isEmpty()) {
    widget->clearText();
  } else {
    widget->setValue(MyMoneyMoney(m_account.value(key)));
  }
}

void KNewAccountDlg::loadKVP(const QCString& key, KLineEdit* widget)
{
  if(!widget)
    return;

  widget->setText(m_account.value(key));
}

void KNewAccountDlg::storeKVP(const QCString& key, const QString& text, const QString& value)
{
  if(text.isEmpty())
    m_account.deletePair(key);
  else
    m_account.setValue(key, value);
}

void KNewAccountDlg::storeKVP(const QCString& key, kMyMoneyEdit* widget)
{
  storeKVP(key, widget->lineedit()->text(), widget->text());
}

void KNewAccountDlg::storeKVP(const QCString& key, KLineEdit* widget)
{
  storeKVP(key, widget->text(), widget->text());
}

const MyMoneyAccount& KNewAccountDlg::account(void)
{
  // assign the right currency to the account
  m_account.setCurrencyId(m_currency->security().id());
  return m_account;
}

const MyMoneyAccount& KNewAccountDlg::parentAccount(void)
{
  if (!m_bSelectedParentAccount)
  {
    MyMoneyFile *file = MyMoneyFile::instance();

    switch (m_account.accountGroup())
    {
      case MyMoneyAccount::Asset:
        m_parentAccount = file->asset();
        break;
      case MyMoneyAccount::Liability:
        m_parentAccount = file->liability();
        break;
      case MyMoneyAccount::Income:
        m_parentAccount = file->income();
        break;
      case MyMoneyAccount::Expense:
        m_parentAccount = file->expense();
        break;
      case MyMoneyAccount::Equity:
        m_parentAccount = file->equity();
        break;
      default:
        qDebug("Seems we have an account that hasn't been mapped to the top five");
        if(m_categoryEditor)
          m_parentAccount = file->income();
        else
          m_parentAccount = file->asset();
    }
  }
  return m_parentAccount;
}

void KNewAccountDlg::initParentWidget(QCString parentId, const QCString& accountId)
{
  MyMoneyFile *file = MyMoneyFile::instance();

  MyMoneyAccount liabilityAccount = file->liability();
  MyMoneyAccount assetAccount = file->asset();
  MyMoneyAccount expenseAccount = file->expense();
  MyMoneyAccount incomeAccount = file->income();
  MyMoneyAccount equityAccount = file->equity();

  m_parentItem = 0;
  m_accountItem = 0;

  // Determine the parent account
  try
  {
    m_parentAccount = file->account(parentId);
  }
  catch (MyMoneyException *e)
  {
    m_bSelectedParentAccount = false;
    m_parentAccount = MyMoneyAccount();
    if(m_account.accountType() != MyMoneyAccount::UnknownAccountType) {
      parentAccount();
      parentId = m_parentAccount.id();
    }
    delete e;
  }
  m_bSelectedParentAccount = true;

  // extract the account type from the combo box
  MyMoneyAccount::accountTypeE type;
  MyMoneyAccount::accountTypeE groupType;
  type = KMyMoneyUtils::stringToAccountType(typeCombo->currentText());
  groupType = MyMoneyAccount::accountGroup(type);

  m_qlistviewParentAccounts->clear();

  // Now scan all 4 account roots to load the list and mark the parent
  try
  {
    if (!m_categoryEditor)
    {
      if(groupType == MyMoneyAccount::Asset || type == MyMoneyAccount::Loan) {
        // Asset
        KMyMoneyAccountTreeItem *assetTopLevelAccount = new KMyMoneyAccountTreeItem(m_qlistviewParentAccounts, assetAccount);

        if(m_parentAccount.id().isEmpty()) {
          m_parentAccount = assetAccount;
          parentId = m_parentAccount.id();
        }

        if (parentId == assetAccount.id())
          m_parentItem = assetTopLevelAccount;

        assetTopLevelAccount->setOpen(true);

        for ( QCStringList::ConstIterator it = assetAccount.accountList().begin();
              it != assetAccount.accountList().end();
              ++it )
        {
          MyMoneyAccount acc = file->account(*it);
          if(acc.isClosed())
            continue;

          KMyMoneyAccountTreeItem *accountItem = new KMyMoneyAccountTreeItem(assetTopLevelAccount, acc);

          if(parentId == acc.id()) {
            m_parentItem = accountItem;
          } else if(accountId == acc.id()) {
            if(m_isEditing)
              accountItem->setSelectable(false);
            m_accountItem = accountItem;
          }

          QCStringList subAccounts = acc.accountList();
          if (subAccounts.count() >= 1)
          {
            showSubAccounts(subAccounts, accountItem, parentId, acc.id());
          }
        }
      }

      if(groupType == MyMoneyAccount::Liability) {
        // Liability
        KMyMoneyAccountTreeItem *liabilityTopLevelAccount = new KMyMoneyAccountTreeItem(m_qlistviewParentAccounts, liabilityAccount);

        if(m_parentAccount.id().isEmpty()) {
          m_parentAccount = liabilityAccount;
          parentId = m_parentAccount.id();
        }

        if (parentId == liabilityAccount.id())
          m_parentItem = liabilityTopLevelAccount;

        liabilityTopLevelAccount->setOpen(true);

        for ( QCStringList::ConstIterator it = liabilityAccount.accountList().begin();
              it != liabilityAccount.accountList().end();
              ++it )
        {
          MyMoneyAccount acc = file->account(*it);
          if(acc.isClosed())
            continue;

          KMyMoneyAccountTreeItem *accountItem = new KMyMoneyAccountTreeItem(liabilityTopLevelAccount, acc);

          if(parentId == acc.id()) {
            m_parentItem = accountItem;
          } else if(accountId == acc.id()) {
            if(m_isEditing)
              accountItem->setSelectable(false);
            m_accountItem = accountItem;
          }

          QCStringList subAccounts = acc.accountList();
          if (subAccounts.count() >= 1)
          {
            showSubAccounts(subAccounts, accountItem, parentId, acc.id());
          }
        }
      }
    }
    else
    {
      if(groupType == MyMoneyAccount::Income) {
        // Income
        KMyMoneyAccountTreeItem *incomeTopLevelAccount = new KMyMoneyAccountTreeItem(m_qlistviewParentAccounts,
                          incomeAccount);

        if(m_parentAccount.id().isEmpty()) {
          m_parentAccount = incomeAccount;
          parentId = m_parentAccount.id();
        }

        if (parentId == incomeAccount.id())
          m_parentItem = incomeTopLevelAccount;

        incomeTopLevelAccount->setOpen(true);

        for ( QCStringList::ConstIterator it = incomeAccount.accountList().begin();
              it != incomeAccount.accountList().end();
              ++it )
        {
          KMyMoneyAccountTreeItem *accountItem = new KMyMoneyAccountTreeItem(incomeTopLevelAccount,
              file->account(*it));

          QCString id = file->account(*it).id();
          if(parentId == id) {
            m_parentItem = accountItem;
          } else if(accountId == id) {
            if(m_isEditing)
              accountItem->setSelectable(false);
            m_accountItem = accountItem;
          }

          QCStringList subAccounts = file->account(*it).accountList();
          if (subAccounts.count() >= 1)
          {
            showSubAccounts(subAccounts, accountItem, parentId, accountId);
          }
        }
      }

      if(groupType == MyMoneyAccount::Expense) {
        // Expense
        KMyMoneyAccountTreeItem *expenseTopLevelAccount = new KMyMoneyAccountTreeItem(m_qlistviewParentAccounts,
                          expenseAccount);

        if(m_parentAccount.id().isEmpty()) {
          m_parentAccount = expenseAccount;
          parentId = m_parentAccount.id();
        }

        if (parentId == expenseAccount.id())
          m_parentItem = expenseTopLevelAccount;

        expenseTopLevelAccount->setOpen(true);

        for ( QCStringList::ConstIterator it = expenseAccount.accountList().begin();
              it != expenseAccount.accountList().end();
              ++it )
        {
          KMyMoneyAccountTreeItem *accountItem = new KMyMoneyAccountTreeItem(expenseTopLevelAccount,
              file->account(*it));

          QCString id = file->account(*it).id();
          if(parentId == id) {
            m_parentItem = accountItem;
          } else if(accountId == id) {
            if(m_isEditing)
              accountItem->setSelectable(false);
            m_accountItem = accountItem;
          }

          QCStringList subAccounts = file->account(*it).accountList();
          if (subAccounts.count() >= 1)
          {
            showSubAccounts(subAccounts, accountItem, parentId, accountId);
          }
        }
      }
    }
  }
  catch (MyMoneyException *e)
  {
    qDebug("Exception in assets account refresh: %s", e->what().latin1());
    delete e;
  }

  m_qlistviewParentAccounts->setColumnWidth(0, m_qlistviewParentAccounts->width());

  if (m_parentItem)
  {
    m_subAccountLabel->setText(i18n("Is a sub account of %1").arg(m_parentAccount.name()));
    m_parentItem->setOpen(true);
    m_qlistviewParentAccounts->setSelected(m_parentItem, true);
  }

  m_qlistviewParentAccounts->setEnabled(true);
}

void KNewAccountDlg::showSubAccounts(QCStringList accounts, KMyMoneyAccountTreeItem *parentItem,
                                     const QCString& parentId, const QCString& accountId)
{
  MyMoneyFile *file = MyMoneyFile::instance();

  for ( QCStringList::ConstIterator it = accounts.begin(); it != accounts.end(); ++it )
  {
    KMyMoneyAccountTreeItem *accountItem  = new KMyMoneyAccountTreeItem(parentItem,
          file->account(*it));

    QCString id = file->account(*it).id();
    if(parentId == id) {
      m_parentItem = accountItem;
    } else if(accountId == id) {
      if(m_isEditing)
        accountItem->setSelectable(false);
      m_accountItem = accountItem;
    }

    QCStringList subAccounts = file->account(*it).accountList();
    if (subAccounts.count() >= 1)
    {
      showSubAccounts(subAccounts, accountItem, parentId, accountId);
    }
  }
}

void KNewAccountDlg::resizeEvent(QResizeEvent* e)
{
  m_qlistviewParentAccounts->setColumnWidth(0, m_qlistviewParentAccounts->width());

  // call base class resizeEvent()
  KNewAccountDlgDecl::resizeEvent(e);
}

void KNewAccountDlg::slotSelectionChanged(QListViewItem *item)
{
  KMyMoneyAccountTreeItem *accountItem = dynamic_cast<KMyMoneyAccountTreeItem*>(item);
  try
  {
    MyMoneyFile *file = MyMoneyFile::instance();

    //qDebug("Selected account id: %s", accountItem->accountID().data());
    m_parentAccount = file->account(accountItem->id());
    m_subAccountLabel->setText(i18n("Is a sub account of %1").arg(m_parentAccount.name()));
    if(m_qlistviewParentAccounts->isEnabled()) {
      m_bSelectedParentAccount = true;
    }
  }
  catch (MyMoneyException *e)
  {
    qDebug("This shouldn't happen! : %s", e->what().latin1());
    delete e;
  }
}

void KNewAccountDlg::loadVatAccounts(void)
{
  QValueList<MyMoneyAccount> list;
  MyMoneyFile::instance()->accountList(list);
  QValueList<MyMoneyAccount>::Iterator it;
  QCStringList loadListExpense;
  QCStringList loadListIncome;
  for(it = list.begin(); it != list.end(); ++it) {
    if(!(*it).value("VatRate").isEmpty()) {
      if((*it).accountType() == MyMoneyAccount::Expense)
        loadListExpense += (*it).id();
      else
        loadListIncome += (*it).id();
    }
  }
  AccountSet vatSet;
  vatSet.load(m_vatAccount, i18n("Income"), loadListIncome, true);
  vatSet.load(m_vatAccount, i18n("Expense"), loadListExpense, false);
}

void KNewAccountDlg::slotLoadInstitutions(const QString& name)
{
  int id=-1, counter=0;
  m_qcomboboxInstitutions->clear();
  QString bic;
  // Are we forcing the user to use institutions?
  m_qcomboboxInstitutions->insertItem(i18n("<No Institution>"));
  m_bicValue->setText(" ");
  ibanEdit->setEnabled(false);
  accountNoEdit->setEnabled(false);
  try
  {
    MyMoneyFile *file = MyMoneyFile::instance();

    QValueList<MyMoneyInstitution> list = file->institutionList();
    QValueList<MyMoneyInstitution>::ConstIterator institutionIterator;
    for (institutionIterator = list.begin(), counter=1; institutionIterator != list.end(); ++institutionIterator, counter++)
    {
      if ((*institutionIterator).name() == name) {
        id = counter;
        ibanEdit->setEnabled(true);
        accountNoEdit->setEnabled(true);
        m_bicValue->setText((*institutionIterator).value("bic"));
      }
      m_qcomboboxInstitutions->insertItem((*institutionIterator).name());
    }

    if (id != -1)
    {
      m_qcomboboxInstitutions->setCurrentItem(id);
    }
  }
  catch (MyMoneyException *e)
  {
    qDebug("Exception in institution load: %s", e->what().latin1());
    delete e;
  }
}

void KNewAccountDlg::slotNewClicked()
{
  MyMoneyInstitution institution;

  KNewBankDlg dlg(institution, this);
  if (dlg.exec())
  {
    MyMoneyFileTransaction ft;
    try
    {
      MyMoneyFile *file = MyMoneyFile::instance();

      institution = dlg.institution();
      file->addInstitution(institution);
      ft.commit();
      slotLoadInstitutions(institution.name());
    }
    catch (MyMoneyException *e)
    {
      delete e;
      KMessageBox::information(this, i18n("Cannot add institution"));
    }
  }
}

void KNewAccountDlg::slotOnlineSetupClicked(void)
{
#ifdef USE_OFX_DIRECTCONNECT
  // TODO (Ace) This is one place to add in logic for other online banking
  // protocols.  We could have a dialog that asks the user which
  // protocol to set up for this account, and then call the correct
  // wizard (using a plugin)
  //
  // Or, the wizard itself could ask the user.

  KOnlineBankingSetupWizard wiz(this,"onlinebankingsetup");
  wiz.exec();

  MyMoneyKeyValueContainer settings;
  if ( wiz.chosenSettings( settings ) )
  {
    m_account.setOnlineBankingSettings( settings );
    displayOnlineBankingStatus();
  }
#else
  KMessageBox::sorry(this,i18n("This version of KMyMoney has not been compiled with support for online banking"));
#endif
}

void KNewAccountDlg::slotAccountTypeChanged(const QString& typeStr)
{
  MyMoneyAccount::accountTypeE type;
  MyMoneyAccount::accountTypeE oldType;
  MyMoneyFile* file = MyMoneyFile::instance();

  type = KMyMoneyUtils::stringToAccountType(typeStr);
  try {
    oldType = m_account.accountType();
    if(oldType != type) {
      QCString parentId;
      switch(MyMoneyAccount::accountGroup(type)) {
        case MyMoneyAccount::Asset:
          parentId = file->asset().id();
          break;
        case MyMoneyAccount::Liability:
          parentId = file->liability().id();
          break;
        case MyMoneyAccount::Expense:
          parentId = file->expense().id();
          break;
        case MyMoneyAccount::Income:
          parentId = file->income().id();
          break;
        default:
          qWarning("Unknown account group in KNewAccountDlg::slotAccountTypeChanged()");
          break;
      }
      initParentWidget(parentId, QCString());
    }
  } catch(MyMoneyException *e) {
    delete e;
    qWarning("Unexpected exception in KNewAccountDlg::slotAccountTypeChanged()");
  }
}

void KNewAccountDlg::slotCheckFinished(void)
{
  bool showButton = true;

  if(accountNameEdit->text().length() == 0) {
    showButton = false;
  }

  if(m_vatCategory->isChecked() && m_vatRate->value() <= MyMoneyMoney(0)) {
    showButton = false;
  } else {
    if(m_vatAssignment->isChecked() && m_vatAccount->selectedItems().isEmpty())
      showButton = false;
  }
  createButton->setEnabled(showButton);
}

void KNewAccountDlg::slotVatChanged(bool state)
{
  if(state) {
    m_vatCategoryFrame->show();
    m_vatAssignmentFrame->hide();
  } else {
    m_vatCategoryFrame->hide();
    m_vatAssignmentFrame->show();
  }
}

void KNewAccountDlg::slotVatAssignmentChanged(bool state)
{
  m_vatAccount->setEnabled(state);
  m_amountGroup->setEnabled(state);
}

void KNewAccountDlg::displayOnlineBankingStatus(void)
{
// this should be a run-time conditional, checking whether there are any online
// banking plugins loaded
#ifdef USE_OFX_DIRECTCONNECT
    // Set up online banking settings if applicable
    MyMoneyKeyValueContainer settings = m_account.onlineBankingSettings();
    QString protocol = settings.value("protocol");
    if ( ! protocol.isEmpty() )
    {
      m_textOnlineStatus->setText(i18n("STATUS: Enabled & configured (%1)").arg(protocol));
      m_ledOnlineStatus->on();

      QString account = i18n("ACCOUNT: %1").arg(settings.value("accountid"));
      QString bank = i18n("BANK/BROKER: %1").arg(settings.value("bankname"));
      QString bankid = settings.value("bankid") + " " + settings.value("branchid");
      if ( bankid.length() > 1 )
        bank += " (" + bankid + ")";
      m_textBank->setText(bank);
      m_textOnlineAccount->setText(account);
    }
    else
      m_textOnlineStatus->setText(i18n("STATUS: Account not configured"));

#else
    m_textOnlineStatus->setText(i18n("STATUS: Disabled.  No online banking services are available"));
#endif
}

void KNewAccountDlg::slotMarkersChanged(void)
{
#ifdef HAVE_KDCHART
  // add another row for markers if required or remove it if not necessary
  // see http://www.klaralvdalens-datakonsult.se/kdchart/ProgrammersManual/KDChart.pdf
  // Chapter 6, "Adding separate Lines/Markers".

  bool needRow = false;
  bool haveMinBalance = false;
  bool haveMaxCredit = false;
  MyMoneyMoney minBalance, maxCredit;
  MyMoneyMoney factor(1,1);
  if(m_account.accountGroup() == MyMoneyAccount::Asset)
    factor = -factor;

  if(m_maxCreditLabel->isVisible()) {
    if(m_maxCreditEarlyEdit->text().length()) {
      needRow = true;
      haveMaxCredit = true;
      maxCredit = m_maxCreditEarlyEdit->value() * factor;
    }
    if(m_maxCreditAbsoluteEdit->text().length()) {
      needRow = true;
      haveMaxCredit = true;
      maxCredit = m_maxCreditAbsoluteEdit->value() * factor;
    }
  }

  if(m_minBalanceLabel->isVisible()) {
    if(m_minBalanceEarlyEdit->text().length()) {
      needRow = true;
      haveMinBalance = true;
      minBalance = m_minBalanceEarlyEdit->value();
    }
    if(m_minBalanceAbsoluteEdit->text().length()) {
      needRow = true;
      haveMinBalance = true;
      minBalance = m_minBalanceAbsoluteEdit->value();
    }
  }

  KDChartTableDataBase* data = m_chartWidget->data();
  if(!needRow && data->usedRows() == 2) {
    data->expand( data->usedRows()-1, data->usedCols() );
  } else if(needRow && data->usedRows() == 1) {
    data->expand( data->usedRows()+1, data->usedCols() );
  }

  if(needRow) {
    if(haveMinBalance) {
      data->setCell(1, 0, minBalance.toDouble());
      m_chartWidget->setProperty(1, 0, m_idPropMinBalance);
    }
    if(haveMaxCredit) {
      data->setCell(1, 1, maxCredit.toDouble());
      m_chartWidget->setProperty(1, 1, m_idPropMaxCredit);
    }
  }

  for(int iCell = 91; iCell < 180; ++iCell) {
    m_chartWidget->setProperty(0, iCell, m_idPropFutureValue);
  }
  m_chartWidget->setProperty(0, 90, m_idPropLastValue);
#endif
}

void KNewAccountDlg::adjustEditWidgets(kMyMoneyEdit* dst, kMyMoneyEdit* src, char mode)
{
  MyMoneyMoney factor(1,1);
  if(m_account.accountGroup() == MyMoneyAccount::Asset)
    factor = -factor;

  switch(mode) {
    case '<':
      if(src->value()*factor < dst->value()*factor)
        dst->setValue(src->value());
      break;

    case '>':
      if(src->value()*factor > dst->value()*factor)
        dst->setValue(src->value());
      break;
  }
}

void KNewAccountDlg::slotAdjustMinBalanceAbsoluteEdit(const QString&)
{
  adjustEditWidgets(m_minBalanceAbsoluteEdit, m_minBalanceEarlyEdit, '<');
}

void KNewAccountDlg::slotAdjustMinBalanceEarlyEdit(const QString&)
{
  adjustEditWidgets(m_minBalanceEarlyEdit, m_minBalanceAbsoluteEdit, '>');
}

void KNewAccountDlg::slotAdjustMaxCreditAbsoluteEdit(const QString&)
{
  adjustEditWidgets(m_maxCreditAbsoluteEdit, m_maxCreditEarlyEdit, '<');
}

void KNewAccountDlg::slotAdjustMaxCreditEarlyEdit(const QString&)
{
  adjustEditWidgets(m_maxCreditEarlyEdit, m_maxCreditAbsoluteEdit, '>');
}

#include "knewaccountdlg.moc"
// vim:cin:si:ai:et:ts=2:sw=2:
