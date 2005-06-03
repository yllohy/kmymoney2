/***************************************************************************
                          kgncpricesourcedlg.cpp
                             -------------------
    copyright            : (C) 2005 by Ace Jones
    author               : Tony Bloomfield
    email                : tonybloom@users.sourceforge.net
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
#include <qlistbox.h>
#include <qlineedit.h>
#include <qbuttongroup.h>
#include <qlayout.h>
#include <qapplication.h>

// ----------------------------------------------------------------------------
// KDE Includes
#include <kurlrequester.h>
#include <ktextbrowser.h>
#include <klocale.h>

// ----------------------------------------------------------------------------
// Project Includes
#include "kgncpricesourcedlg.h"
#include "../converter/webpricequote.h"

KGncPriceSourceDlg::KGncPriceSourceDlg(QWidget *parent, const char *name)
 : KGncPriceSourceDlgDecl(parent, name)
{
}
KGncPriceSourceDlg::KGncPriceSourceDlg(QString stockName, QString gncSource){
  // signals and slots connections
  connect( buttonGroup5, SIGNAL( released(int) ), this, SLOT( buttonPressed(int) ) );
  connect( buttonHelp, SIGNAL( clicked() ), this, SLOT( slotHelp() ) );
  // initialize data fields
  textStockName->setText (i18n ("Investment: %1").arg(stockName));
  textGncSource->setText (i18n ("Quote source: %1").arg(gncSource));
  listKnownSource->insertStringList (WebPriceQuote::quoteSources());
  lineUserSource->setText (gncSource);
  checkAlwaysUse->setChecked(true);
  buttonGroup5->setButton (0);
  buttonPressed (0);
  return;
}

KGncPriceSourceDlg::~KGncPriceSourceDlg()
{
}

enum ButtonIds {NOSOURCE = 0, KMMSOURCE, USERSOURCE};

void KGncPriceSourceDlg::buttonPressed (int buttonId) {
  m_currentButton = buttonId;
  switch (m_currentButton) {
    case NOSOURCE:
      listKnownSource->clearSelection();
      listKnownSource->setEnabled (false);
      lineUserSource->deselect();
      lineUserSource->setEnabled (false);
      break;
    case KMMSOURCE:
      lineUserSource->deselect ();
      lineUserSource->setEnabled (false);
      listKnownSource->setEnabled (true);
      listKnownSource->setFocus();
      listKnownSource->setSelected (0, true);
      break;
    case USERSOURCE:
      listKnownSource->clearSelection();
      listKnownSource->setEnabled (false);
      lineUserSource->setEnabled (true);
      lineUserSource->selectAll();
      lineUserSource->setFocus ();
      break;
  }
}

QString KGncPriceSourceDlg::selectedSource() {
  QString s;
  switch (m_currentButton) {
    case NOSOURCE: s = ""; break;
    case KMMSOURCE: s = listKnownSource->currentText(); break;
    case USERSOURCE: s = lineUserSource->text(); break;
  }
  return (s);
}

void KGncPriceSourceDlg::slotHelp(void)
{

  QString helpstring = i18n(
  "<h1>Online Quote Price Sources</h1>"
  "<p>For obtaining the latest prices of investments, GnuCash uses a collection of Perl scripts"
  " under the name of Finance::Quote. A number of such scripts have been developed over the years,"
  " and contributed to the project. </p>"
  "<p>KMyMoney takes a different approach, namely the use of a URL in conjuction with the stock's"
  " ticker symbol to retrieve prices directly,where such facilities are available,"
  " and does not at this time support such a wide variety of sources. We do however"
  " offer the facility for defining your own sources without requiring a knowledge of Perl,"
  " (though an understanding of Regular Expressions may be helpful!!). The use of a URL means"
  " that the 'source' could be a shell script or other Linux executable, if you have the skills to produce these. Use"
  " the Settings menu, Configure KMyMoney and select Online Quotes, to supply new sources. (N.B. Due to a quirk"
  " of Qt, you may need to maximize the window to see the full instructions.)</p>"
  "<p>Do not worry too much about any mistakes you may make here. They can always be corrected"
  " later, via the Tools/Securities menu item.</p>"
  "<p>Please be aware that some of the sources used may have restrictions on the use"
  " which you make of their prices. You should consult the Terms and Conditions of these sites"
  " to ensure that you are abiding by any such rules.</p>" );

  QDialog dlg;
  QVBoxLayout layout( &dlg, 11, 6, "Layout17");
  KTextBrowser te(&dlg,"Help");
  layout.addWidget(&te);
  te.setReadOnly(true);
  te.setTextFormat(Qt::RichText);
  te.setText(helpstring);
  dlg.setCaption(i18n("Online Quote Sources Help"));
  unsigned width = QApplication::desktop()->width();
  unsigned height = QApplication::desktop()->height();
  te.setMinimumSize(width/2,height/2);  
  layout.setResizeMode(QLayout::Minimum);
  dlg.exec();
}

#include "kgncpricesourcedlg.moc"

