/***************************************************************************
                          kmymoneytransactionform.cpp  -  description
                             -------------------
    begin                : Thu Jul 25 2002
    copyright            : (C) 2002 by Michael Edwardes
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

// #include <qvariant.h>
#include <qframe.h>
#include <qtabbar.h>
#include <qtable.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
// #include <qimage.h>
// #include <qpixmap.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <klocale.h>
#include <kglobal.h>
#include <kpushbutton.h>
#include <kiconloader.h>
#include <kguiitem.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneytransactionform.h"
#include "../views/kledgerview.h"

/* -------------------------------------------------------------------------------*/
/*                           kMyMoneyTransactionFormTable                         */
/* -------------------------------------------------------------------------------*/

kMyMoneyTransactionFormTable::kMyMoneyTransactionFormTable( KLedgerView* view, QWidget* parent, const char* name)
  : QTable(parent, name)
{
  m_view = view;
}

/** Override the QTable member function to avoid display of focus */
void kMyMoneyTransactionFormTable::paintFocus(QPainter* /* p */, const QRect& /* cr */)
{
}

QWidget* kMyMoneyTransactionFormTable::createEditor(int row, int col, bool initFromCell) const
{
  if(!m_editable[row * numCols() + col])
    return 0;
  return QTable::createEditor(row, col, initFromCell);
}

void kMyMoneyTransactionFormTable::setEditable(int row, int col, bool flag)
{
  if(row >= 0 && row < numRows() && col >= 0 && col < numCols())
    m_editable[row * numCols() + col] = flag;
}

void kMyMoneyTransactionFormTable::setNumCols(int c)
{
  resizeEditable(numRows(), c);
  QTable::setNumCols(c);
}

void kMyMoneyTransactionFormTable::setNumRows(int r)
{
  resizeEditable(r, numCols());
  QTable::setNumRows(r);
  for(int i = 0; i < r; ++i)
    QTable::setRowHeight(i, 22);
}

void kMyMoneyTransactionFormTable::resizeEditable(int r, int c)
{
  QBitArray newArray(r * c);
  int oldc = numCols();
  newArray.fill(false);

  for(int i = 0; i < numRows() && i < r; ++i) {
    for(int j = 0; j < oldc && j < c; ++j) {
      newArray[i*c + j] = m_editable[i*oldc + j];
    }
  }
  m_editable = newArray;
}

void kMyMoneyTransactionFormTable::clearEditable(void)
{
  m_editable.fill(false);
}

bool kMyMoneyTransactionFormTable::eventFilter( QObject * o, QEvent * e)
{
  if(e->type() == QEvent::KeyPress
  || e->type() == QEvent::KeyRelease)
    return false;
  else
    return QTable::eventFilter(o ,e);

}

bool kMyMoneyTransactionFormTable::focusNextPrevChild(bool next)
{
  return m_view->focusNextPrevChild(next);
}



/* -------------------------------------------------------------------------------*/
/*                         kMyMoneyTransactionFormTableItem                       */
/* -------------------------------------------------------------------------------*/


kMyMoneyTransactionFormTableItem::kMyMoneyTransactionFormTableItem(QTable* table, EditType ed, const QString& str)
  : QTableItem(table, ed, str)
{
  m_alignment = standard;
}

kMyMoneyTransactionFormTableItem::~kMyMoneyTransactionFormTableItem()
{
}

int kMyMoneyTransactionFormTableItem::alignment() const
{
  int rc;

  switch(m_alignment) {
    default:
      rc = QTableItem::alignment();
      break;
    case left:
      rc = AlignLeft | AlignVCenter;
      break;
    case right:
      rc = AlignRight | AlignVCenter;
      break;
  }
  return rc;
}

void kMyMoneyTransactionFormTableItem::setAlignment(alignmentTypeE type)
{
  m_alignment = type;
}


/* -------------------------------------------------------------------------------*/
/*                             kMyMoneyTransactionForm                            */
/* -------------------------------------------------------------------------------*/


/*
 *  Constructs a kMyMoneyTransactionForm which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 */
kMyMoneyTransactionForm::kMyMoneyTransactionForm( KLedgerView* parent,  const char* name, WFlags fl, const int rows, const int cols)
    : QWidget( parent, name, fl )
{
  m_view = parent;
  formLayout = new QVBoxLayout( this, 0, 0, "formLayout");

  m_tabBar = new QTabBar( this, "tabBar" );
  m_tabBar->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)0, 0, 0, m_tabBar->sizePolicy().hasHeightForWidth() ) );
  formLayout->addWidget( m_tabBar );

  formFrame = new QFrame( this, "formFrame" );
  formFrame->setGeometry( QRect( 12, 44, 462, 170 ));
  formFrame->setSizePolicy( QSizePolicy( QSizePolicy::MinimumExpanding,
                                         QSizePolicy::Minimum,
                                         0, 0,
                                         formFrame->sizePolicy().hasHeightForWidth() ) );
  formFrame->setFrameShape( QFrame::StyledPanel );
  formFrame->setFrameShadow( QFrame::Raised );
  formFrameLayout = new QGridLayout( formFrame, 1, 1, 11, 6, "formFrameLayout");

  buttonLayout = new QHBoxLayout( 0, 0, 10, "buttonLayout");

  KIconLoader *il = KGlobal::iconLoader();
  
  KGuiItem newButtItem( i18n( "&New" ),
                    QIconSet(il->loadIcon("filenew", KIcon::Small, KIcon::SizeSmall)),  
                    i18n("Create a new transaction"),
                    i18n("Use this to create a new transaction in the ledger"));
  buttonNew = new KPushButton( newButtItem, formFrame, "buttonNew" );
  buttonNew->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)0, 0, 0, buttonNew->sizePolicy().hasHeightForWidth() ) );
  buttonLayout->addWidget( buttonNew );

  KGuiItem editButtItem( i18n( "&Edit" ),
                    QIconSet(il->loadIcon("edit", KIcon::Small, KIcon::SizeSmall)),
                    i18n("Modify a transaction"),
                    i18n("Use this to modify the current selected transaction in the ledger"));
  buttonEdit = new KPushButton( editButtItem, formFrame, "buttonEdit" );
  buttonEdit->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)0, 0, 0, buttonEdit->sizePolicy().hasHeightForWidth() ) );
  buttonLayout->addWidget( buttonEdit );

  KGuiItem enterButtItem( i18n( "Enter" ),
                    QIconSet(il->loadIcon("button_ok", KIcon::Small, KIcon::SizeSmall)),
                    i18n("Enter transaction into ledger"),
                    i18n("Use this to enter the current transaction into the ledger"));
  buttonEnter = new KPushButton( enterButtItem, formFrame, "buttonEnter" );
  buttonEnter->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)0, 0, 0, buttonEnter->sizePolicy().hasHeightForWidth() ) );
  buttonLayout->addWidget( buttonEnter );

  KGuiItem cancelButtItem( i18n( "&Cancel" ),
                    QIconSet(il->loadIcon("button_cancel", KIcon::Small, KIcon::SizeSmall)),
                    i18n("Forget changes made to this transaction"),
                    i18n("Use this to abort the changes to the current transaction"));
  buttonCancel = new KPushButton( cancelButtItem, formFrame, "buttonCancel" );
  buttonCancel->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)0, 0, 0, buttonCancel->sizePolicy().hasHeightForWidth() ) );
  buttonLayout->addWidget( buttonCancel );

  KGuiItem moreButtItem( i18n( "&More" ),
                    QIconSet(il->loadIcon("configure", KIcon::Small, KIcon::SizeSmall)),
                    i18n("Access more functions"),
                    i18n("Use this to access special functions"));
  buttonMore = new KPushButton( moreButtItem, formFrame, "buttonMore" );
  buttonMore->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)0, 0, 0, buttonMore->sizePolicy().hasHeightForWidth() ) );
  buttonLayout->addWidget( buttonMore );
  
  QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
  buttonLayout->addItem( spacer );

  formFrameLayout->addLayout( buttonLayout, 0, 0 );


  formTable = new kMyMoneyTransactionFormTable( m_view, formFrame, "formTable" );
  formTable->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding, 0, 0, formTable->sizePolicy().hasHeightForWidth() ) );
  formTable->setBackgroundOrigin(QTable::WindowOrigin);
  formTable->setFrameShape( QTable::NoFrame );
  formTable->setFrameShadow( QTable::Plain );
  formTable->setNumCols( cols );
  formTable->setNumRows( rows );
  formTable->setShowGrid( FALSE );
  formTable->setSelectionMode( QTable::NoSelection );
  formTable->verticalHeader()->hide();
  formTable->horizontalHeader()->hide();
  formTable->setLeftMargin(0);
  formTable->setTopMargin(0);

  formFrameLayout->addWidget( formTable, 1, 0 );

  // make sure, that the table is 'invisible'

  QPalette palette = formTable->palette();
  QColorGroup cg = palette.active();
  cg.setBrush(QColorGroup::Base, cg.brush(QColorGroup::Background));
  palette.setActive(cg);
  palette.setInactive(cg);
  formTable->setPalette(palette);

  formLayout->addWidget( formFrame );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
kMyMoneyTransactionForm::~kMyMoneyTransactionForm()
{
    // no need to delete child widgets, Qt does it all for us
}
