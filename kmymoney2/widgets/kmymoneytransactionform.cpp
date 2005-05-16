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
#include <ktoolbar.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneytransactionform.h"
#include "../views/kledgerview.h"
#include "../kmymoneyutils.h"

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

  m_tabBar = new QTabBar( this, "tabBar" );
  m_tabBar->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)0, 0, 0, m_tabBar->sizePolicy().hasHeightForWidth() ) );
  formLayout->addWidget( m_tabBar );

  formFrame = new QFrame( this, "formFrame" );
  // formFrame->setGeometry( QRect( 12, 44, 462, 170 ));
  formFrame->setSizePolicy( QSizePolicy( QSizePolicy::MinimumExpanding,
                                         QSizePolicy::Minimum,
                                         0, 0,
                                         formFrame->sizePolicy().hasHeightForWidth() ) );
  formFrame->setFrameShape( QFrame::Panel );
  formFrame->setFrameShadow( QFrame::Raised );
  formFrameLayout = new QGridLayout( formFrame, 3, 1, 3, 0, "formFrameLayout");

  KIconLoader *il = KGlobal::iconLoader();

  buttonLayout = new QHBoxLayout( 0, 0, 0, "buttonLayout");

  // The following code looks a bit weird. The problem I had was, that when I added
  // a KToolBar object as child to the formFrame, the frame parameters for the formFrame
  // where overridden and I could not control them anymore. Creating the KToolBarButtons
  // w/o a KToolBar parent does not allow to set the KToolBar::IconTextRight attribute
  // which I wanted.
  //
  // The trick is to create a KToolBar w/o parent, add the buttons then reparent the
  // buttons to the formFrame and throw away the KToolBar object. We cannot throw it
  // away immediately, because it is still required for some color/theme change. So we
  // keep it around until this object is destroyed.

  QPoint point(0,0);
  m_toolbar = new KToolBar(0, "ToolBar", true);
  m_toolbar->setIconText(KToolBar::IconTextRight);
  m_toolbar->insertButton(il->loadIcon("filenew", KIcon::Toolbar, KIcon::SizeSmall),
                        1,true,i18n("New"));
  buttonNew = m_toolbar->getButton(1);

  m_toolbar->insertButton(il->loadIcon("edit", KIcon::Small, KIcon::SizeSmall),
                        2,true,i18n("Edit"));
  buttonEdit = m_toolbar->getButton(2);

  m_toolbar->insertButton(il->loadIcon("button_ok", KIcon::Small, KIcon::SizeSmall),
                        3,true,i18n("Enter"));
  buttonEnter = m_toolbar->getButton(3);

  m_toolbar->insertButton(il->loadIcon("button_cancel", KIcon::Small, KIcon::SizeSmall),
                        4,true,i18n("Cancel"));
  buttonCancel = m_toolbar->getButton(4);

  m_toolbar->insertButton(il->loadIcon("configure", KIcon::Small, KIcon::SizeSmall),
                        5,true,i18n("More"));
  buttonMore = m_toolbar->getButton(5);

  m_toolbar->insertButton(il->loadIcon("document", KIcon::Small, KIcon::SizeSmall),
                        6,true,i18n("Account"));
  buttonAccount = m_toolbar->getButton(6);


  buttonNew->reparent(formFrame, point);
  buttonLayout->addWidget( buttonNew );
  buttonEdit->reparent(formFrame, point);
  buttonLayout->addWidget( buttonEdit );
  buttonEnter->reparent(formFrame, point);
  buttonLayout->addWidget( buttonEnter );
  buttonCancel->reparent(formFrame, point);
  buttonLayout->addWidget( buttonCancel );
  buttonMore->reparent(formFrame, point);
  buttonLayout->addWidget( buttonMore );

  QSpacerItem* spacer = new QSpacerItem( 0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum );
  buttonLayout->addItem( spacer );

  buttonAccount->reparent(formFrame, point);
  buttonLayout->addWidget( buttonAccount );

  // make sure that the button background is the same as for the frame
  // we need to do this after all the reparenting, because reparenting
  // also resets the palettes
  QPalette palette = formFrame->palette();
  QColorGroup cg = palette.active();
  cg.setBrush(QColorGroup::Button, cg.brush(QColorGroup::Background));
  palette.setActive(cg);
  palette.setInactive(cg);
  palette.setDisabled(cg);
  buttonNew->setPalette(palette);
  buttonEdit->setPalette(palette);
  buttonEnter->setPalette(palette);
  buttonCancel->setPalette(palette);
  buttonMore->setPalette(palette);
  buttonAccount->setPalette(palette);

  formFrameLayout->addLayout( buttonLayout, 0, 0 );

  QFrame *line = new QFrame(formFrame);
  line->setFrameShape(QFrame::ToolBarPanel);
  formFrameLayout->addWidget(line, 1, 0);

  formTable = new kMyMoneyTransactionFormTable( m_view, formFrame, "kMyMoneyTransactionFormTable");
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

  //setMinimumSize(QMAX(formTable->sizeHint().width(), buttonLayout->sizeHint().width()),
  //               formTable->sizeHint().height()+buttonMore->minimumHeight()+m_tabBar->minimumHeight());

  formFrameLayout->addWidget( formTable, 2, 0 );

  // make sure, that the table is 'invisible' by setting up the right background
  palette = formTable->palette();
  cg = palette.active();
  cg.setBrush(QColorGroup::Base, cg.brush(QColorGroup::Background));
  palette.setActive(cg);
  palette.setInactive(cg);
  palette.setDisabled(cg);
  formTable->setPalette(palette);

  formLayout->addWidget( formFrame );
}

/*
 *  Destroys the object and frees any allocated resources
 */
kMyMoneyTransactionForm::~kMyMoneyTransactionForm()
{
  delete m_toolbar;
}

#include "kmymoneytransactionform.moc"
