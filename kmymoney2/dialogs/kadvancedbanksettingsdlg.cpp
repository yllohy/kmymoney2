/***************************************************************************
                          kadvancedbanksettingsdlg.cpp
                             -------------------
    copyright            : (C) 2004 by Ace Jones
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

// ----------------------------------------------------------------------------
// QT Includes

#include <qcheckbox.h>
#include <qlineedit.h>
#include <qlayout.h>
#include <qapplication.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kurlrequester.h>
#include <ktextbrowser.h>
#include <klocale.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kadvancedbanksettingsdlg.h"
#include "../mymoney/mymoneykeyvaluecontainer.h"

KAdvancedBankSettingsDlg::KAdvancedBankSettingsDlg(QWidget *parent, const char *name)
 : KAdvancedBankSettingsDlgDecl(parent, name)
{
  connect(checkEnable,SIGNAL(toggled(bool)),this,SLOT(slotToggleEnabled(bool)));
  connect(buttonHelp, SIGNAL(clicked()), this, SLOT(slotHelp()));
}

KAdvancedBankSettingsDlg::~KAdvancedBankSettingsDlg()
{
}

void KAdvancedBankSettingsDlg::slotToggleEnabled(bool _enabled)
{
  editUrl->setEnabled(_enabled);
  editFiorg->setEnabled(_enabled);
  editFiid->setEnabled(_enabled);
  editIban->setEnabled(_enabled);
  editUser->setEnabled(_enabled);
  editPassword->setEnabled(_enabled);
}

void KAdvancedBankSettingsDlg::setValues(const MyMoneyKeyValueContainer& _values)
{
  // set up dialog

  checkEnable->setChecked( _values.value("enabled").toUInt() );
  editUrl->setURL( _values.value("url") );
  editFiorg->setText( _values.value("fiorg") );
  editFiid->setText( _values.value("fiid") );
  editIban->setText( _values.value("iban") );
  editUser->setText( _values.value("user") );
  editPassword->setText( _values.value("password") );
}

MyMoneyKeyValueContainer KAdvancedBankSettingsDlg::values(void) const
{
  MyMoneyKeyValueContainer result;

  result.setValue( "enabled",QString::number(checkEnable->isChecked()) );
  result.setValue( "url", editUrl->url() );
  result.setValue( "fiorg", editFiorg->text() );
  result.setValue( "fiid", editFiid->text() );
  result.setValue( "iban", editIban->text() );
  result.setValue( "user", editUser->text() );
  result.setValue( "password", editPassword->text() );

  return result;
}

void KAdvancedBankSettingsDlg::slotHelp(void)
{
  QString helpstring =
  "<h1>Using OFX Data in KMyMoney</h1>"
  "<p>There are three basic ways to use OFX technology for retrieving financial data from your bank.  This help file will explain how to use all three.</p>"
  "<ol><li>Importing a File<li>Web Connect<li>Direct Connect</ol>"
  "<h2>Importing a File</h2>"
  "<p>The most basic way is to log on to your bank's website, and download an OFX file.  Then select File | Import | OFX... from the menu bar to import it into KMyMoney</p>"
  "<h2>Web Connect</h2>"
  "<p>KMyMoney supports the Web Connect feature, similiar to Quicken.  To activate this, first connect to your bank, and begin to download an OFX statement as described above.  Then, instead of saving the file to disk, choose to open the file with KMyMoney.  To make this more automatic, you can edit your web browser's settings to use KMyMoney2 as a 'helper application' for the \"application/vnd.intu.qfx\" and \"application/x-ofx\" mimetypes.</p>"
  "<p>Your web browser will then launch KMyMoney2 and begin automatically importing the OFX data from your bank into your account.</p>"
  "<h2>Direct Connect</h2>"
  "<p>KMyMoney2 can also talk directly to your bank to retrieve your statements, if your bank maintains a public OFX server.  At the moment, this option is rather involved to set up properly.  However, once you do set it up, you can download your data without ever leaving KMyMoney.</p>"
  "<p>First, edit the properties of the institution.  This requires that institutions are visible in the accounts view, which they are not by default.  To make them visible, choose the Settings | Configure KMyMoney... menu option.  Navigate to the Accounts View pane, and choose \"Use the normal institution view\".  Accept the settings, and close this dialog.  Then navigate to the main accounts view, select the institution you wish to connect, right click, and choose Edit.  On the next dialog, pick Advanced...  This will bring you to the OFX Direct Connect Settings dialog for this institution.  You will need to find the appropriate values for each of the fields on this dialog.</p>"
  "<p>Second, find the appropriate values for your bank.  There are scripts available at <a href=\"http://www.jongsma.org/gc\">http://www.jongsma.org/gc</a> to download all OFX configuration data for all banks.  You can also just download <a href=\"http://www.jongsma.org/gc/bankinfo/bankinfo.tgz\">http://www.jongsma.org/gc/bankinfo/bankinfo.tgz</a>, which is the latest snapshot as of the last time he posted one.  Search for your bank in banks.xml.  If it's not there, you probably can't connect using Direct Connect.  If it is, remember the &lt;bank:guid&gt; field.  For example, let's say it's 5002, the \"1st Source Bank\".  Then load up the .XML file corresponding to that guid.  In our example, you would load up 5002.xml.  Look for the following tags: &lt;provider:ofxserver&gt; &lt;provider:org&gt; &lt;provider:fid&gt;.  Those are the values you'll enter into URL, FIORG, and FIID respectively.  IBAN is your bank's 9-digit bank number, which you can find directly from your bank.  Username and Password are your own login information to the bank.</p>"
  "<p>Third, edit the properties of the account to enter the exact correct account number.  In order for Direct Connect to work, the account number has to be exactly as your bank is expecting it.  In the future, we will add some nice UI to show the valid values at your bank, but for now you have to get it right.</p>"
  "<p>Finally, to activate OFX Direct Connect, navigate to the accounts view, right click on the account to update, and choose \"Online Update using OFX...\".  If all is well, KMyMoney will connect to your bank, and download your data.  If not, we probably need to add some better error reporting and troubleshooting steps.</p>"
  "<p>The <a href=\"http://www.jongsma.org/gc/scripts/ofx-ba.py\">ofx-ba.py script</a> is a good start for troubleshooting.  Download that, and hack in your bank information.  If you can get this script to work, you should be able to get it working in KMyMoney.</p>";

  QDialog dlg;
  QVBoxLayout layout( &dlg, 11, 6, "Layout17");
  KTextBrowser te(&dlg,"Help");
  layout.addWidget(&te);
  te.setReadOnly(true);
  te.setTextFormat(Qt::RichText);
  te.setText(helpstring);
  dlg.setCaption(i18n("OFX Configuration Help"));
  unsigned width = QApplication::desktop()->width();
  unsigned height = QApplication::desktop()->height();
  te.setMinimumSize(width/2,height/2);
  layout.setResizeMode(QLayout::Minimum);
  dlg.exec();
}

#include "kadvancedbanksettingsdlg.moc"
