/***************************************************************************
                          khomeview.h  -  description
                             -------------------
    begin                : Tue Jan 22 2002
    copyright            : (C) 2000-2002 by Michael Edwardes
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
#ifndef KHOMEVIEW_H
#define KHOMEVIEW_H

// ----------------------------------------------------------------------------
// QT Includes
#include <qwidget.h>

// ----------------------------------------------------------------------------
// KDE Includes
#include <khtml_part.h>

// ----------------------------------------------------------------------------
// Project Includes

/**
  * Displays a 'home page' for the user.  Similar to concepts used in
  * quicken and m$-money.
  *
  * @author Michael Edwardes
  * $Id: khomeview.h,v 1.2 2002/02/07 23:46:41 mte Exp $
  *
  * @short A view containing the home page for kmymoney2.
**/
class KHomeView : public QWidget  {
   Q_OBJECT

private:
  KHTMLPart *m_part;

signals:
  void signalViewActivated();

public:
  /**
    * Standard constructor.
    *
    * @param parent The QWidget this is used in.
    * @param name The QT name.
    *
    * @return An object of type KHomeView
    *
    * @see ~KHomeView
  **/
  KHomeView(QWidget *parent=0, const char *name=0);

  /**
    * Standard destructor.
    *
    * @return Nothing.
    *
    * @see KHomeView
  **/
  ~KHomeView();

  /**
    * Overridden so we can emit the activated signal.
    *
    * @return Nothing.
  **/
  void show();
};

#endif
