/****************************************************************************
** Form interface generated from reading ui file './kbankviewdecl.ui'
**
** Created: Tue Jan 22 20:21:48 2002
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef KBANKLISTVIEW_H
#define KBANKLISTVIEW_H

#include <qvariant.h>
#include <qwidget.h>
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class KListView;
class QLabel;
class QListViewItem;

class KBankViewDecl : public QWidget
{ 
    Q_OBJECT

public:
    KBankViewDecl( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~KBankViewDecl();

    KListView* bankListView;
    QLabel* totalProfitsLabel;

protected:
    QVBoxLayout* kbankListViewLayout;
    QHBoxLayout* Layout1;
};

#endif // KBANKLISTVIEW_H
