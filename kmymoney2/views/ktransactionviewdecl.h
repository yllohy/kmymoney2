/****************************************************************************
** Form interface generated from reading ui file './ktransactionviewdecl.ui'
**
** Created: Tue Jan 22 20:21:48 2002
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef KTRANSACTIONVIEWDECL_H
#define KTRANSACTIONVIEWDECL_H

#include <qvariant.h>
#include <qwidget.h>
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QComboBox;
class QLabel;
class kMyMoneyTable;

class KTransactionViewDecl : public QWidget
{ 
    Q_OBJECT

public:
    KTransactionViewDecl( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~KTransactionViewDecl();

    QComboBox* viewTypeCombo;
    kMyMoneyTable* transactionsTable;
    QLabel* lblBalance;
    QLabel* lblBalanceAmt;

protected:
    QVBoxLayout* kTransactionViewDeclLayout;
    QHBoxLayout* Layout2;
    QHBoxLayout* Layout4;
    bool event( QEvent* );
};

#endif // KTRANSACTIONVIEWDECL_H
