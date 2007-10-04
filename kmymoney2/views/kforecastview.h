/***************************************************************************
                             kforecastview.h
                             -------------------
    copyright            : (C) 2007 by Alvaro Soliverez
    email                : asoliverez@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KFORECASTVIEW_H
#define KFORECASTVIEW_H

// ----------------------------------------------------------------------------
// QT Includes
#include <qlistview.h>

// ----------------------------------------------------------------------------
// KDE Includes



// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoney/mymoneyaccount.h>
//#include <kmymoney/kmymoneyaccounttree.h>
#include <kmymoney/mymoneyutils.h>

#include "../views/kforecastviewdecl.h"

/**
  * @author Alvaro Soliverez
  */


/**
  * This class implements the forecast 'view'.
  */
class KForecastView : public KForecastViewDecl
{
  Q_OBJECT
private:

public:
  KForecastView(QWidget *parent=0, const char *name=0);
  virtual ~KForecastView();

  void show(void);

public slots:
  void slotLoadForecast(void);

protected:
  typedef enum {
    ListView = 0,
    // insert new values above this line
    MaxViewTabs
  } ForecastViewTab;
  
  //typedef 
      //QMap<int, MyMoneyMoney> dailyBalances;
  //QMap<QCString, dailyBalances> accountList;
  
  QMap<QString, QCString> nameIdx;
      

  /**
    * This method loads the forecast view.
    * 
    */
  void loadForecast(ForecastViewTab tab);
  void loadListView(void);

//  void calculateDailyBalances(int forecastDays, int forecastTerm, int forecastTerms);
  
  protected slots:
    void slotTabChanged(QWidget*);

private:
  bool                                m_needReload[MaxViewTabs];
  
};

#endif
