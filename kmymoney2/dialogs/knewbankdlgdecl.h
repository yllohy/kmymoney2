/****************************************************************************
** Form interface generated from reading ui file './knewbankdlgdecl.ui'
**
** Created: Mon Nov 4 17:37:42 2002
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef KNEWBANKDLGDECL_H
#define KNEWBANKDLGDECL_H

#include <qvariant.h>
#include <qdialog.h>
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QFrame;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;

class KNewBankDlgDecl : public QDialog
{ 
    Q_OBJECT

public:
    KNewBankDlgDecl( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~KNewBankDlgDecl();

    QLabel* m_qpixmaplabel;
    QLabel* TextLabel1_2;
    QFrame* Line1;
    QGroupBox* GroupBox1;
    QLabel* TextLabel1;
    QLineEdit* nameEdit;
    QGroupBox* GroupBox2;
    QLabel* TextLabel2;
    QLineEdit* cityEdit;
    QLabel* TextLabel3;
    QLineEdit* streetEdit;
    QLabel* TextLabel4;
    QLineEdit* postcodeEdit;
    QLabel* TextLabel5;
    QLineEdit* telephoneEdit;
    QLabel* TextLabel6;
    QLineEdit* sortCodeEdit;
    QLabel* TextLabel7;
    QLineEdit* managerEdit;
    QPushButton* okBtn;
    QPushButton* cancelBtn;


protected:
    QVBoxLayout* kNewBankDlgDeclLayout;
    QHBoxLayout* Layout10;
    QVBoxLayout* GroupBox1Layout;
    QHBoxLayout* Layout6;
    QVBoxLayout* GroupBox2Layout;
    QHBoxLayout* Layout5;
    QHBoxLayout* Layout4;
    QHBoxLayout* Layout3;
    QHBoxLayout* Layout2;
    QHBoxLayout* Layout7;
    QHBoxLayout* Layout8;
    QHBoxLayout* Layout9;
};

#endif // KNEWBANKDLGDECL_H
