/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename slots use Qt Designer which will
** update this file, preserving your code. Create an init() slot in place of
** a constructor, and a destroy() slot in place of a destructor.
*****************************************************************************/

#include <kdebug.h>
void KReportConfigurationDecl::init(void)
{
    radioMonthCols->setEnabled(false);
    radioQuarterCols->setEnabled(false);
    radioYearCols->setEnabled(false);
    checkConvertCurrency->setEnabled(false);
    buttonHelp->setEnabled(false);
}

void KReportConfigurationDecl::updateDateRange( const QString & range )
{
    if ( range == "Custom...")
    {
	DateEditStart->setEnabled(true);	
	DateEditEnd->setEnabled(true);	
    }
    else
    {
	DateEditStart->setEnabled(false);	
	DateEditEnd->setEnabled(false);	
	
	const QDate& currentdate = QDate::currentDate();
	int year = currentdate.year();
	int month = currentdate.month();
	int quarter = month - ((month-1)%3);
	
	if ( range == "Current Year")
	{
	    DateEditStart->setDate(QDate(year,1,1));
	    DateEditEnd->setDate(QDate(year+1,1,1).addDays(-1));
	}
	else if ( range == "Previous Year" )
	{
	    DateEditStart->setDate(QDate(year-1,1,1));
	    DateEditEnd->setDate(QDate(year,1,1).addDays(-1));
	}
	else if ( range == "Current Quarter" )
	{
	    DateEditStart->setDate(QDate(year,quarter,1));
	    DateEditEnd->setDate(QDate(year,quarter+3,1).addDays(-1));
	}
	else if ( range == "Previous Quarter" )
	{
	    DateEditEnd->setDate(QDate(year,quarter,1).addDays(-1));
	    if ( quarter == 1 )
	    {
		year--;
		quarter+=12;
	    }
	    DateEditStart->setDate(QDate(year,quarter-3,1));
	}
	else if ( range == "Current Month" )
	{
	    DateEditStart->setDate(QDate(year,month,1));
	    DateEditEnd->setDate(QDate(year,month+1,1).addDays(-1));
	}
	else if ( range == "Previous Month" )
	{
	    DateEditEnd->setDate(QDate(year,month,1).addDays(-1));
	    if ( month == 1 )
	    {
		year--;
		month+=12;
	    }
	    DateEditStart->setDate(QDate(year,month-1,1));
	}
    }
}



void KReportConfigurationDecl::butCategoriesAll_pressed()
{
    lvCategories->selectAll(true);
}


void KReportConfigurationDecl::butCategoriesNone_pressed()
{
    lvCategories->selectAll(false);
}


void KReportConfigurationDecl::butAccountsAll_pressed()
{
    lvAccounts->selectAll(true);
}


void KReportConfigurationDecl::butAccountsNone_pressed()
{
    lvAccounts->selectAll(false);
}
