#include <klocale.h>
/****************************************************************************
** Form implementation generated from reading ui file './ktransactionviewdecl.ui'
**
** Created: Tue Jan 22 20:22:31 2002
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "ktransactionviewdecl.h"

#include <qcombobox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qwidget.h>
#include "../widgets/kmymoneytable.h"
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/* 
 *  Constructs a KTransactionViewDecl which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 */
KTransactionViewDecl::KTransactionViewDecl( QWidget* parent,  const char* name, WFlags fl )
    : QWidget( parent, name, fl )
{
    if ( !name )
	setName( "kTransactionViewDecl" );
    resize( 785, 510 ); 
    setCaption( i18n( "Form1" ) );
    kTransactionViewDeclLayout = new QVBoxLayout( this ); 
    kTransactionViewDeclLayout->setSpacing( 6 );
    kTransactionViewDeclLayout->setMargin( 11 );

    Layout2 = new QHBoxLayout; 
    Layout2->setSpacing( 6 );
    Layout2->setMargin( 0 );

    viewTypeCombo = new QComboBox( FALSE, this, "viewTypeCombo" );
    viewTypeCombo->insertItem( i18n( "All Transactions (default)" ) );
    viewTypeCombo->insertItem( i18n( "Search Results" ) );
    viewTypeCombo->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)0, viewTypeCombo->sizePolicy().hasHeightForWidth() ) );
    viewTypeCombo->setMinimumSize( QSize( 260, 0 ) );
    Layout2->addWidget( viewTypeCombo );
    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    Layout2->addItem( spacer );
    kTransactionViewDeclLayout->addLayout( Layout2 );

    transactionsTable = new kMyMoneyTable( this, "transactionsTable" );
    transactionsTable->setMinimumSize( QSize( 0, 0 ) );
    transactionsTable->setBackgroundOrigin( kMyMoneyTable::ParentOrigin );
    kTransactionViewDeclLayout->addWidget( transactionsTable );

    Layout4 = new QHBoxLayout; 
    Layout4->setSpacing( 6 );
    Layout4->setMargin( 0 );
    QSpacerItem* spacer_2 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    Layout4->addItem( spacer_2 );

    lblBalance = new QLabel( this, "lblBalance" );
    lblBalance->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, lblBalance->sizePolicy().hasHeightForWidth() ) );
    QFont lblBalance_font(  lblBalance->font() );
    lblBalance_font.setPointSize( 12 );
    lblBalance->setFont( lblBalance_font ); 
    lblBalance->setText( i18n( "Ending Balance:" ) );
    lblBalance->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );
    Layout4->addWidget( lblBalance );

    lblBalanceAmt = new QLabel( this, "lblBalanceAmt" );
    lblBalanceAmt->setFrameShape( QLabel::MShape );
    lblBalanceAmt->setFrameShadow( QLabel::MShadow );
    lblBalanceAmt->setText( i18n( "ggg" ) );
    lblBalanceAmt->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );
    Layout4->addWidget( lblBalanceAmt );
    kTransactionViewDeclLayout->addLayout( Layout4 );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
KTransactionViewDecl::~KTransactionViewDecl()
{
    // no need to delete child widgets, Qt does it all for us
}

/*  
 *  Main event handler. Reimplemented to handle application
 *  font changes
 */
bool KTransactionViewDecl::event( QEvent* ev )
{
    bool ret = QWidget::event( ev ); 
    if ( ev->type() == QEvent::ApplicationFontChange ) {
	QFont lblBalance_font(  lblBalance->font() );
	lblBalance_font.setPointSize( 12 );
	lblBalance->setFont( lblBalance_font ); 
    }
    return ret;
}

#include "ktransactionviewdecl.moc"
