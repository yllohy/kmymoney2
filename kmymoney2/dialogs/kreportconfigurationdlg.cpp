/***************************************************************************
                          kreportconfigurationdlg.cpp  -  description
                             -------------------
    begin                : Sun May 23 2004
    copyright            : (C) 2000-2004 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
                           Ace Jones <ace.j@hotpop.com>
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
#include <qlineedit.h>
#include <qcombobox.h>
#include <qdatetimeedit.h>
#include <qradiobutton.h>
#include <qlistview.h>

// ----------------------------------------------------------------------------
// KDE Includes
#include <kdebug.h>

// ----------------------------------------------------------------------------
// Project Includes
#include "kreportconfigurationdlg.h"
#include "../views/pivottable.h"
using namespace reports;

KReportConfigurationDlg::KReportConfigurationDlg(const reports::ReportConfiguration& config, QWidget *parent, const char *name, const char */*title*/):
  KReportConfigurationDecl( parent, name, true, 0 ),
  m_currentConfiguration( config )
{
  editReportname->setText( config.getName() );
  comboDateRange->setCurrentText( "Custom..." );
  DateEditStart->setDate( config.getStart() );
  DateEditEnd->setDate( config.getEnd() );

  if ( config.getShowSubAccounts() )
    radioCategoriesAll->setChecked(true);
  else
    radioCategoriesTop->setChecked(true);

  if ( config.getRowFilter() == (ReportConfiguration::eExpense|ReportConfiguration::eIncome) )
    radioRowsIE->setChecked(true);
  else
    radioRowsAL->setChecked(true);

  // Populate the categories selector
  lvCategories->addColumn("ID",0);
  QMap<QCString,QListViewItem*> map;
  MyMoneyFile* file = MyMoneyFile::instance();
  map[file->expense().id()] = new QListViewItem( lvCategories, "Expense", file->expense().id() );
  map[file->income().id()] = new QListViewItem( lvCategories, "Income", file->income().id() );
  map[file->asset().id()] = new QListViewItem( lvAccounts, "Asset", file->asset().id() );
  map[file->liability().id()] = new QListViewItem( lvAccounts, "Liability", file->liability().id() );

  const QValueList<MyMoneyAccount>& accounts = file->accountList();
  QValueList<MyMoneyAccount>::const_iterator it_account = accounts.begin();

  while ( it_account != accounts.end() )
  {
    QListViewItem* item = new QListViewItem( map[(*it_account).parentAccountId()], (*it_account).name(), (*it_account).id() );
    map[(*it_account).id()] = item;

    if ( m_currentConfiguration.includesAccount( (*it_account).id()) )
      item->setSelected(true);
      
    ++it_account;
  }

  init();
}

const ReportConfiguration& KReportConfigurationDlg::getResult(void)
{
  m_currentConfiguration.setShowSubAccounts( radioCategoriesAll->isChecked() );
  m_currentConfiguration.setName( editReportname->text() );
  m_currentConfiguration.setDateRange( DateEditStart->date(), DateEditEnd->date() );

  if ( radioRowsIE->isChecked() )
    m_currentConfiguration.setRowFilter(ReportConfiguration::eExpense|ReportConfiguration::eIncome);
  if ( radioRowsAL->isChecked() )
    m_currentConfiguration.setRowFilter(ReportConfiguration::eAsset|ReportConfiguration::eLiability);

  // iterate over accounts & categories
  QListViewItem* it =  lvCategories->firstChild();
  while ( it )
  {
    // set the inclusion state equal to the selection state
    m_currentConfiguration.setIncludesAccount( it->text(1), it->isSelected() );

    // TODO: Fix this logic.  It will fail if the categories are nested >3 deep
    QListViewItem* next = it->firstChild();
    if ( !next )
    {
      next = it->nextSibling();
      if ( !next && it->parent() )
      {
        next = it->parent()->nextSibling();

        if ( !next && it->parent()->parent() )
          next = it->parent()->parent()->nextSibling();
      }
    }
    it = next;
  }

  it = lvAccounts->firstChild();
  while ( it )
  {
    // set the inclusion state equal to the selection state
    m_currentConfiguration.setIncludesAccount( it->text(1), it->isSelected() );

    // TODO: Fix this logic.  It will fail if the categories are nested >3 deep
    QListViewItem* next = it->firstChild();
    if ( !next )
    {
      next = it->nextSibling();
      if ( !next && it->parent() )
      {
        next = it->parent()->nextSibling();

        if ( !next && it->parent()->parent() )
          next = it->parent()->parent()->nextSibling();
      }
    }
    it = next;
  }
        
  return m_currentConfiguration;
}

