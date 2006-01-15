/***************************************************************************
                          konlinebankingsetupwizard.cpp
                             -------------------
    begin                : Sat Jan 7 2006
    copyright            : (C) 2006 by Ace Jones
    email                : acejones@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifdef USE_OFX_DIRECTCONNECT

// ----------------------------------------------------------------------------
// System Includes
#include <string>
#include <vector>
#include <curl/curl.h>

// ----------------------------------------------------------------------------
// QT Includes
#include <qlistview.h>
#include <qtextbrowser.h>
#include <qlineedit.h>
#include <qlabel.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <klocale.h>
#include <kdebug.h>
#include <kmessagebox.h>

// ----------------------------------------------------------------------------
// Project Includes

#include <../dialogs/konlinebankingsetupwizard.h>
#include <../converter/ofxpartner.h>

using std::string;
using std::vector;

KOnlineBankingSetupWizard::KOnlineBankingSetupWizard(QWidget *parent, const char *name):
  KOnlineBankingSetupDecl(parent,name), m_fDone(false)
{
  // fill the list view with banks
  vector<string> banks = OfxPartner::BankNames();
  vector<string>::const_iterator it_bank = banks.begin();
  while (it_bank != banks.end())
  {
    new QListViewItem( m_listFi, QString((*it_bank).c_str()));
    ++it_bank;
  }
}

KOnlineBankingSetupWizard::~KOnlineBankingSetupWizard()
{
}

void KOnlineBankingSetupWizard::next(void)
{
  bool ok = true;
  
  switch (indexOf(currentPage()))
  {
  case 0:
    ok = finishFiPage();
    break;
  case 1:
    ok = finishLoginPage();
    break;
  case 2:
    m_fDone = ok = finishAccountPage();
    break;
  }

  if (ok)
    KOnlineBankingSetupDecl::next();

  setFinishEnabled(currentPage(), m_fDone );
}

bool KOnlineBankingSetupWizard::finishFiPage(void)
{
  bool result = false;
  
  // Get the fipids for the selected bank
  QListViewItem* item = m_listFi->currentItem();
  if ( item )
  {
    QString bank = item->text(0);
    m_textDetails->clear();
    m_textDetails->append(QString("<p>Details for %1:</p>").arg(bank));
    vector<string> fipids = OfxPartner::FipidForBank(bank);

    m_bankInfo.clear();
    vector<string>::const_iterator it_fipid = fipids.begin();
    while ( it_fipid != fipids.end() )
    {
      // For each fipid, get the connection details
      OfxFiServiceInfo info = OfxPartner::ServiceInfo(*it_fipid);

      // Print them to the text browser
      QString message = QString("<p>Fipid: %1<br>").arg(*it_fipid);

      // If the bank supports retrieving statements 
      if ( info.accountlist )
      {
        m_bankInfo.push_back(info);
        
        message += QString("URL: %1<br>Org: %2<br>Fid: %3<br>").arg(info.url,info.org,info.fid);
        if ( info.statements )
          message += i18n("Supports online statements<br>");
        if ( info.investments )
          message += i18n("Supports investments<br>");
        if ( info.billpay )
          message += i18n("Supports bill payment (but not supported in KMyMoney yet<br>");
      }
      else
      {
        message += i18n("Does not support online banking</p>");
      }
      m_textDetails->append(message);
          
      ++it_fipid;
    }
    result = true;
  }
  else
    // error!  No current item
    KMessageBox::sorry(this,i18n("Please choose a bank."));

  return result;
}

bool post(const char* request, const char* url,const char* filename)
{
  CURL *curl = curl_easy_init();
  if(! curl)
    return false;

  unlink(filename);  
  FILE* file = fopen(filename,"wb");
  if (! file )
  {
    curl_easy_cleanup(curl);
    return false;
  }
    
  curl_easy_setopt(curl, CURLOPT_URL, url);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request);

  struct curl_slist *headerlist=NULL;
  headerlist=curl_slist_append(headerlist, "Content-type: application/x-ofx");
  headerlist=curl_slist_append(headerlist, "Accept: */*, application/x-ofx");    
  
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, fwrite);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)file);
    
  /*CURLcode res = */ curl_easy_perform(curl);

  curl_easy_cleanup(curl);
  curl_slist_free_all (headerlist);
  
  fclose(file);
  
  return true;
}

bool KOnlineBankingSetupWizard::finishLoginPage(void)
{
  bool result = true;
  
  QString username = m_editUsername->text();
  QString password = m_editPassword->text();

  m_listAccount->clear();
    
  // Process an account request for each fipid
  m_it_info = m_bankInfo.begin();
  while ( m_it_info != m_bankInfo.end() )
  {
    OfxFiLogin fi;
    memset(&fi,0,sizeof(OfxFiLogin));
    strncpy(fi.fid,(*m_it_info).fid,OFX_FID_LENGTH-1);
    strncpy(fi.org,(*m_it_info).org,OFX_ORG_LENGTH-1);
    strncpy(fi.userid,username.latin1(),OFX_USERID_LENGTH-1);
    strncpy(fi.userpass,password.latin1(),OFX_USERPASS_LENGTH-1);

    // who owns this memory?!?!
    char* request = libofx_request_accountinfo( &fi );
   
    char* filename = "response.ofx";
    post(request,(*m_it_info).url,filename);

    LibofxContextPtr ctx = libofx_get_new_context();
    Q_CHECK_PTR(ctx);

    ofx_set_account_cb(ctx, ofxAccountCallback, this);
    ofx_set_status_cb(ctx, ofxStatusCallback, this);
    // Add resulting accounts to the account list
    libofx_proc_file(ctx, filename, AUTODETECT);
    libofx_free_context(ctx);
 
    ++m_it_info;
  }
  
  if ( ! m_listAccount->childCount() )
  {
    KMessageBox::sorry(this,i18n("No suitable accounts were found at this bank."));
    result = false;
  }
  return result;
}

bool KOnlineBankingSetupWizard::finishAccountPage(void)
{
  bool result = true;
  
  if ( ! m_listAccount->currentItem() )
  {
    KMessageBox::sorry(this,i18n("Please choose an account"));
    result = false;
  }

  return result;
}

int KOnlineBankingSetupWizard::ofxAccountCallback(struct OfxAccountData data, void * pv)
{
  KOnlineBankingSetupWizard* pthis = reinterpret_cast<KOnlineBankingSetupWizard*>(pv);
  // Put the account info in the view

  MyMoneyKeyValueContainer kvps;
  
  if ( data.account_type_valid )
  {
    QString type;
    switch ( data.account_type )
    {
      case OfxAccountData::OFX_CHECKING:  /**< A standard checking account */
      type = i18n("CHECKING");
      break;
    case OfxAccountData::OFX_SAVINGS:   /**< A standard savings account */
      type = i18n("SAVINGS");
      break;
    case OfxAccountData::OFX_MONEYMRKT: /**< A money market account */
      type = i18n("MONEY MARKET");
      break;
    case OfxAccountData::OFX_CREDITLINE: /**< A line of credit */
      type = i18n("CREDIT LINE");
      break;
    case OfxAccountData::OFX_CMA:       /**< Cash Management Account */
      type = i18n("CMA");
      break;
    case OfxAccountData::OFX_CREDITCARD: /**< A credit card account */
      type = i18n("CREDIT CARD");
      break;
    case OfxAccountData::OFX_INVESTMENT: /**< An investment account */
      type = i18n("INVESTMENT");
      break;
    default:
      type = i18n("UNKNOWN");
      break;
    }
    kvps.setValue("type",type);
  }

  if ( data.bank_id_valid )
    kvps.setValue("bankid",data.bank_id);

  if ( data.broker_id_valid )
    kvps.setValue("bankid",data.broker_id);

  if ( data.branch_id_valid )
    kvps.setValue("branchid",data.branch_id);

  if ( data.account_number_valid )
    kvps.setValue("accountid",data.account_number);
 
  kvps.setValue("username",pthis->m_editUsername->text());
  kvps.setValue("password",pthis->m_editPassword->text());
  
  kvps.setValue("url",(*(pthis->m_it_info)).url);
  kvps.setValue("fid",(*(pthis->m_it_info)).fid);
  kvps.setValue("org",(*(pthis->m_it_info)).org);
  kvps.setValue("fipid","");
  QListViewItem* item = pthis->m_listFi->currentItem();
  if ( item )
    kvps.setValue("bankname",item->text(0));
  
  kvps.setValue("protocol","OFX");
  
  new ListViewItem( pthis->m_listAccount, kvps );

  return 0;
}

int KOnlineBankingSetupWizard::ofxStatusCallback(struct OfxStatusData data, void * pv)
{
  KOnlineBankingSetupWizard* pthis = reinterpret_cast<KOnlineBankingSetupWizard*>(pv);
  
  QString message;
  
  if(data.code_valid==true)
  {
    message += QString("#%1 %2: \"%3\"\n").arg(data.code).arg(data.name,data.description);
  }
  
  if(data.server_message_valid==true){
    message += i18n("Server message: %1\n").arg(data.server_message);
  }

  if(data.severity_valid==true){
    switch(data.severity){
    case OfxStatusData::INFO : 
      break;
    case OfxStatusData::WARN :
      KMessageBox::detailedError( pthis, i18n("Your bank returned warnings when signing on"), i18n("WARNING %1").arg(message) );
      break;
    case OfxStatusData::ERROR :
      KMessageBox::detailedError( pthis, i18n("Error signing onto your bank"), i18n("ERROR %1").arg(message) );
      break;
    default:
      break;
    }
  }
  return 0;
}

bool KOnlineBankingSetupWizard::chosenSettings( MyMoneyKeyValueContainer& settings )
{
  bool result = false;;
  
  if ( m_fDone )
  {
    QListViewItem* qitem = m_listAccount->currentItem();
    ListViewItem* item = dynamic_cast<ListViewItem*>(qitem);
    if ( item )
    {
      settings = *item;
      result = true;
    }
  }

  return result;
}

KOnlineBankingSetupWizard::ListViewItem::ListViewItem( QListView* parent, const MyMoneyKeyValueContainer& kvps ):
  MyMoneyKeyValueContainer( kvps ), QListViewItem( parent )
{
  setText( 0, value("accountid") ); 
  setText( 1, value("type") ); 
  setText( 2, value("bankid") ); 
  setText( 3, value("branchid") ); 
}

void KOnlineBankingSetupWizard::ListViewItem::x(void) {}

#endif // USE_OFX_DIRECTCONNECT
// vim:cin:si:ai:et:ts=2:sw=2:
