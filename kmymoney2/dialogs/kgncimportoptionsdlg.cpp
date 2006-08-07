/***************************************************************************
                          kgncimportoptions.cpp
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
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qlayout.h>
#include <qapplication.h>
#include <qcombobox.h>

// ----------------------------------------------------------------------------
// KDE Includes
#include <kapplication.h>
#include <kurlrequester.h>
#include <ktextbrowser.h>
#include <klocale.h>

// ----------------------------------------------------------------------------
// Project Includes
#include "kgncimportoptionsdlg.h"

// dialog constructor
KGncImportOptionsDlg::KGncImportOptionsDlg(QWidget *parent, const char *name)
 : KGncImportOptionsDlgDecl(parent, name)
{
  buttonInvestGroup->setRadioButtonExclusive (true);
  buttonInvestGroup->setButton (0);
  checkSchedules->setChecked (false);
  buildCodecList (); // build list of codecs and insert into combo box
  checkDecode->setChecked (false);
  comboDecode->setEnabled (false);
  
  checkDebugGeneral->setChecked (false);
  checkDebugXML->setChecked (false);
  checkAnonymize->setChecked (false);
  
  connect (checkDecode, SIGNAL(toggled(bool)), this, SLOT(slotDecodeOptionChanged(bool)));
}

KGncImportOptionsDlg::~KGncImportOptionsDlg()
{
}

// enable the combo box for selection if required
void KGncImportOptionsDlg::slotDecodeOptionChanged(bool isOn) {
  if (isOn) {
    comboDecode->setEnabled (true);
    comboDecode->setCurrentItem (0);
  } else {
    comboDecode->setEnabled (false);
  }
}

// build a list of known codecs and sort it so that the locale codec is first
// try to get the others in some sort of order of likelihood
void KGncImportOptionsDlg::buildCodecList () {
  
  m_localeCodec = QTextCodec::codecForLocale();
  m_codecList.setAutoDelete (true);
  // retrieve all codec pointers
  QTextCodec *codec;
  int i;
  for (i = 0; (codec = QTextCodec::codecForIndex(i)); i++) {
    int rank;
    if (codec == m_localeCodec) rank = 999; // ensure locale rank comes first
    else rank = codec->heuristicNameMatch(m_localeCodec->name());
    
    codecData *p = new codecData(rank, codec);
    m_codecList.append (p);
  }
  m_codecList.sort();
  for (i = 0; i < m_codecList.count(); i++) {
    QString name (m_codecList.at(i)->second->name());
    comboDecode->insertItem (name);
  }
}

// this routine sorts the codec list on 1) rank descending 2) codec name ascending
int codecDataList::compareItems (void *a, void *b) {
  codecData *pa = reinterpret_cast<codecData *>(a);
  codecData *pb = reinterpret_cast<codecData *>(b);
  
  if (pa->first > pb->first) {
    return (-1); // greater rank is treated as less-than so gets sorted first
  } else { if (pb->first > pa->first)
        return (1);
  }
  // ranks are equal, sort on name, case insensitive
  QString sa(pa->second->name());
  QString sb(pb->second->name());
  if (sa.lower() > sb.lower()) {
    return (1);
  } else {
    return (-1);
  }
}

// return selected codec or 0
QTextCodec* KGncImportOptionsDlg::decodeOption(void) {
  if (!checkDecode->isChecked()) {
    return (0);
  } else {
    return (m_codecList.at(comboDecode->currentItem())->second);
  }
}

void KGncImportOptionsDlg::slotHelp(void)
{
  kapp->invokeHelp ("details.impexp.gncoptions");
}

#include "kgncimportoptionsdlg.moc"
