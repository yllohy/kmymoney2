#include <klocale.h>
/****************************************************************************
** Form implementation generated from reading ui file './kbankviewdecl.ui'
**
** Created: Tue Jan 22 20:22:26 2002
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "kbankviewdecl.h"

#include <klistview.h>
#include <qheader.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qwidget.h>
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/* 
 *  Constructs a KBankViewDecl which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 */
KBankViewDecl::KBankViewDecl( QWidget* parent,  const char* name, WFlags fl )
    : QWidget( parent, name, fl )
{
    if ( !name )
	setName( "kbankListView" );
    resize( 599, 420 ); 
    setCaption( i18n( "Form1" ) );
    kbankListViewLayout = new QVBoxLayout( this ); 
    kbankListViewLayout->setSpacing( 6 );
    kbankListViewLayout->setMargin( 11 );

    bankListView = new KListView( this, "bankListView" );
    kbankListViewLayout->addWidget( bankListView );

    Layout1 = new QHBoxLayout; 
    Layout1->setSpacing( 6 );
    Layout1->setMargin( 0 );
    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    Layout1->addItem( spacer );

    totalProfitsLabel = new QLabel( this, "totalProfitsLabel" );
    totalProfitsLabel->setMinimumSize( QSize( 150, 0 ) );
    totalProfitsLabel->setText( i18n( "Total Profits:" ) );
    totalProfitsLabel->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );
    Layout1->addWidget( totalProfitsLabel );
    kbankListViewLayout->addLayout( Layout1 );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
KBankViewDecl::~KBankViewDecl()
{
    // no need to delete child widgets, Qt does it all for us
}

#include "kbankviewdecl.moc"
