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

#include <qapplication.h>
#include <qframe.h>
#include <qtabbar.h>
#include <qtable.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include "kdecompat.h"
#include <klocale.h>
#include <kglobal.h>
#include <kpushbutton.h>
#include <kiconloader.h>
#include <kguiitem.h>
#include <kglobalsettings.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneytransactionform.h"
#include "../views/kledgerview.h"
#include "../kmymoneyutils.h"
#include "../kapptest.h"

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

void kMyMoneyTransactionFormTable::setEditable(const int row, const int col, const bool flag)
{
  if(row >= 0 && row < numRows() && col >= 0 && col < numCols())
    m_editable[row * numCols() + col] = flag;
}

void kMyMoneyTransactionFormTable::setNumCols(int c)
{
  resizeEditable(numRows(), c);
  QTable::setNumCols(c);
}

void kMyMoneyTransactionFormTable::setNumRows(const int r, const int rowHeight)
{
  resizeEditable(r, numCols());
  QTable::setNumRows(r);
  for(int i = 0; i < r; ++i)
    QTable::setRowHeight(i, rowHeight);
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

// needed to duplicate this here, as the QTable::tableSize method is private :-(
QSize kMyMoneyTransactionFormTable::tableSize(void) const
{
  return QSize(columnPos(numCols()-1) + columnWidth(numCols()-1),
               rowPos(numRows()-1) + rowHeight(numRows()-1));
}

QSize kMyMoneyTransactionFormTable::sizeHint(void) const
{
  // I've taken this from qtable.cpp, QTable::sizeHint()
  int vmargin = QApplication::reverseLayout() ? rightMargin() : leftMargin();
  return QSize(tableSize().width() + vmargin + 5, tableSize().height() + topMargin() + 5);
}

void kMyMoneyTransactionFormTable::adjustColumn(const int col, const int minWidth)
{
  QFontMetrics fontMetrics(KGlobalSettings::generalFont());

  // scan through the rows
  int w = minWidth;
  for ( int i = numRows()-1; i >= 0; --i ) {
    w = QMAX(w, fontMetrics.width(text(i, col)+"  "));
  }
  setColumnWidth( col, w );
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
kMyMoneyTransactionForm::kMyMoneyTransactionForm( KLedgerView* parent,  const char* name, WFlags fl, const int rows, const int cols, const int rowHeight)
    : QWidget( parent, name, fl )
{
  m_view = parent;
  formLayout = new QVBoxLayout( this, 0, 0, "formLayout");
  int buttonWidth = 0;

  m_tabBar = new QTabBar( this, "tabBar" );
  m_tabBar->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)0, 0, 0, m_tabBar->sizePolicy().hasHeightForWidth() ) );
  formLayout->addWidget( m_tabBar );

  formFrame = new QFrame( this, "formFrame" );
  formFrame->setGeometry( QRect( 12, 44, 462, 170 ));
  formFrame->setSizePolicy( QSizePolicy( QSizePolicy::MinimumExpanding,
                                         QSizePolicy::Minimum,
                                         0, 0,
                                         formFrame->sizePolicy().hasHeightForWidth() ) );
  formFrame->setFrameShape( QFrame::Panel );
  formFrame->setFrameShadow( QFrame::Raised );
  formFrameLayout = new QGridLayout( formFrame, 1, 1, 11, 6, "formFrameLayout");

  buttonLayout = new QHBoxLayout( 0, 0, 10, "buttonLayout");

  KIconLoader *il = KGlobal::iconLoader();

  KGuiItem newButtItem( i18n( "&New" ),
                    QIconSet(il->loadIcon("filenew", KIcon::Small, KIcon::SizeSmall)),
                    i18n("Create a new transaction"),
                    i18n("Use this to create a new transaction in the ledger"));
  buttonNew = new KPushButton( newButtItem, formFrame, KAppTest::widgetName(this, "KPushButton/New") );
  buttonNew->setSizePolicy( QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed, 0, 0, buttonNew->sizePolicy().hasHeightForWidth() ) );
  buttonLayout->addWidget( buttonNew );

  KGuiItem editButtItem( i18n( "&Edit" ),
                    QIconSet(il->loadIcon("edit", KIcon::Small, KIcon::SizeSmall)),
                    i18n("Modify a transaction"),
                    i18n("Use this to modify the current selected transaction in the ledger"));
  buttonEdit = new KPushButton( editButtItem, formFrame, KAppTest::widgetName(this, "KPushButton/Edit" ));
  buttonNew->setSizePolicy( QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed, 0, 0, buttonNew->sizePolicy().hasHeightForWidth() ) );
  buttonLayout->addWidget( buttonEdit );

  KGuiItem enterButtItem( i18n( "Enter" ),
                    QIconSet(il->loadIcon("button_ok", KIcon::Small, KIcon::SizeSmall)),
                    i18n("Enter transaction into ledger"),
                    i18n("Use this to enter the current transaction into the ledger"));
  buttonEnter = new KPushButton( enterButtItem, formFrame, KAppTest::widgetName(this, "KPushButton/Enter" ));
  buttonNew->setSizePolicy( QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed, 0, 0, buttonNew->sizePolicy().hasHeightForWidth() ) );
  buttonLayout->addWidget( buttonEnter );

  KGuiItem cancelButtItem( i18n( "&Cancel" ),
                    QIconSet(il->loadIcon("button_cancel", KIcon::Small, KIcon::SizeSmall)),
                    i18n("Forget changes made to this transaction"),
                    i18n("Use this to abort the changes to the current transaction"));
  buttonCancel = new KPushButton( cancelButtItem, formFrame, KAppTest::widgetName(this , "KPushButton/Cancel" ));
  buttonNew->setSizePolicy( QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed, 0, 0, buttonNew->sizePolicy().hasHeightForWidth() ) );
  buttonLayout->addWidget( buttonCancel );

  KGuiItem moreButtItem( i18n( "M&ore" ),
                    QIconSet(il->loadIcon("configure", KIcon::Small, KIcon::SizeSmall)),
                    i18n("Access more functions"),
                    i18n("Use this to access special functions"));
  buttonMore = new KPushButton( moreButtItem, formFrame, KAppTest::widgetName(this, "KPushButton/More" ));
  buttonNew->setSizePolicy( QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed, 0, 0, buttonNew->sizePolicy().hasHeightForWidth() ) );
  buttonLayout->addWidget( buttonMore );

  // make all buttons the same width
  buttonNew->update();
  buttonWidth = buttonNew->sizeHint().width();
  buttonWidth = QMAX(buttonWidth, buttonEdit->sizeHint().width());
  buttonWidth = QMAX(buttonWidth, buttonEnter->sizeHint().width());
  buttonWidth = QMAX(buttonWidth, buttonCancel->sizeHint().width());
  buttonWidth = QMAX(buttonWidth, buttonMore->sizeHint().width());
  buttonNew->setMinimumWidth(buttonWidth);
  buttonEdit->setMinimumWidth(buttonWidth);
  buttonEnter->setMinimumWidth(buttonWidth);
  buttonCancel->setMinimumWidth(buttonWidth);
  buttonMore->setMinimumWidth(buttonWidth);

  QSpacerItem* spacer = new QSpacerItem( 0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum );
  buttonLayout->addItem( spacer );

  formFrameLayout->addLayout( buttonLayout, 0, 0 );


  formTable = new kMyMoneyTransactionFormTable( m_view, formFrame, KAppTest::widgetName(this, "kMyMoneyTransactionFormTable") );
  formTable->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding, 0, 0, formTable->sizePolicy().hasHeightForWidth() ) );
  formTable->setBackgroundOrigin(QTable::WindowOrigin);
  formTable->setFrameShape( QTable::NoFrame );
  formTable->setFrameShadow( QTable::Plain );
  formTable->setNumCols( cols );
  formTable->setNumRows( rows, rowHeight );
  formTable->setShowGrid( FALSE );
  formTable->setSelectionMode( QTable::NoSelection );
  formTable->verticalHeader()->hide();
  formTable->horizontalHeader()->hide();
  formTable->setLeftMargin(0);
  formTable->setTopMargin(0);

  setMinimumSize(QMAX(formTable->sizeHint().width(), buttonLayout->sizeHint().width()),
                 formTable->sizeHint().height()+buttonMore->minimumHeight()+m_tabBar->minimumHeight());

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
