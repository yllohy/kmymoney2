/***************************************************************************
                          kjobview.h  -  description
                             -------------------
    begin                : Thu Aug 26 2004
    copyright            : (C) 2004 Martin Preuss
    email                : aquamaniac@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef KJOBVIEW_H
#define KJOBVIEW_H

class JobView;


// ----------------------------------------------------------------------------
// QT Includes
#include <qwidget.h>
class QVBoxLayout;

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes


/**
  * Displays a 'home page' for the user.  Similar to concepts used in
  * quicken and m$-money.
  *
  * @author Michael Edwardes
  * $Id: kjobview.h,v 1.1 2004/09/05 18:31:07 ipwizard Exp $
  *
  * @short A view containing the home page for kmymoney2.
**/
class KJobView : public QWidget  {
   Q_OBJECT

private:
  QVBoxLayout *m_qvboxlayoutPage;
  JobView *m_jobview;
  
signals:
  void signalViewActivated();


public:
  /**
    * Standard constructor.
    *
    * @param parent The QWidget this is used in.
    * @param name The QT name.
    *
    * @return An object of type KJobView
    *
    * @see ~KJobView
  **/
  KJobView(QWidget *parent=0, const char *name=0);

  /**
    * Standard destructor.
    *
    * @return Nothing.
    *
    * @see KJobView
  **/
  ~KJobView();

  /**
    * Overridden so we can emit the activated signal.
    *
    * @return Nothing.
  **/
  void show();

protected:
  
public slots:
  void slotRefreshView(void);
  void slotReloadView(void) { slotRefreshView(); };
};



#endif


