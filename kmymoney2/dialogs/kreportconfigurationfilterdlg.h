/***************************************************************************
                          kreportconfigurationdlg.h  -  description
                             -------------------
    begin                : Mon Jun 21 2004
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

#ifndef KREPORTCONFIGURATIONFILTERDLG_H
#define KREPORTCONFIGURATIONFILTERDLG_H

#include "kfindtransactiondlg.h"
#include "../views/pivottable.h"
#include "../mymoney/mymoneyreport.h"

/**
@author Ace Jones
*/
class KReportConfigurationFilterDlg : public KFindTransactionDlg
{
Q_OBJECT
public:
    KReportConfigurationFilterDlg(MyMoneyReport report, QWidget *parent = 0, const char *name = 0);

    ~KReportConfigurationFilterDlg();
    
    const MyMoneyReport& getConfig(void) const { return m_currentState; }

    QFrame* m_reportFrame;
    QButtonGroup* bgrpShow;
    QRadioButton* radioCategoriesTop;
    QRadioButton* radioCategoriesAll;
    QLabel* labelReportName;
    QLineEdit* editReportname;
    QLabel* labelReportComment;
    QLineEdit* editReportComment;
    QButtonGroup* bgrpRows;
    QRadioButton* radioRowsIE;
    QRadioButton* radioRowsAL;
    QButtonGroup* bgrpCurrency;
    QCheckBox* checkConvertCurrency;
    QButtonGroup* bgrpColumns;
    QRadioButton* radioMonthCols;
    QRadioButton* radioBimonthCols;
    QRadioButton* radioQuarterCols;
    QRadioButton* radioYearCols;

protected:
    QGridLayout* m_reportLayout;
    QVBoxLayout* reportLayout74;
    QHBoxLayout* reportLayout67;
    QHBoxLayout* reportLayout68;
    QVBoxLayout* reportLayout75;
    QVBoxLayout* reportLayout76;
    QHBoxLayout* bgrpColumnsLayout;
    QVBoxLayout* reportLayout12;

    MyMoneyReport m_initialState;
    MyMoneyReport m_currentState;

protected slots:
  void slotReset(void);
  void slotSearch(void);
        
};

#endif
