/***************************************************************************
                          kmymoneyaccountcompletion.h  -  description
                             -------------------
    begin                : Mon Apr 26 2004
    copyright            : (C) 2000-2004 by Michael Edwardes
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

#ifndef KMYMONEYACCOUNTCOMPLETION_H
#define KMYMONEYACCOUNTCOMPLETION_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qwidget.h>
#include <qvbox.h>
class QListViewItem;

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyaccountselector.h"

/**
  * @author Thomas Baumgart
  */

class kMyMoneyAccountCompletion : public QVBox
{
  Q_OBJECT
public:

  kMyMoneyAccountCompletion(QWidget *parent=0, const char *name=0);
  ~kMyMoneyAccountCompletion();

  /**
    * Re-implemented for internal reasons.  API is unaffected.
    */
  virtual void show();

  /**
    * Re-implemented for internal reasons.  API is unaffected.
    */
  virtual void hide();

  /**
    * This method loads the set of accounts into the widget
    * as defined by the parameter @p typeMask. @p typeMask is
    * a bit mask. See KMyMoneyUtils::categoryTypeE for
    * possible values.
    *
    * If multiple sets should be displayed, several KMyMoneyUtils::categoryTypeE values
    * can be logically OR-ed.
    *
    * @param typeMask bitmask defining which types of accounts
    *                 should be loaded into the completion list
    *
    */
  void loadList(KMyMoneyUtils::categoryTypeE typeMask) { m_accountSelector->loadList(typeMask); };

  /**
    * This method sets the current account with id @p id as
    * the current selection.
    *
    * @param id id of account to be selected
    */
  void setSelected(const QCString& id) { m_id = id; };

public slots:
  void slotMakeCompletion(const QString& txt);

  void slotItemSelected(QListViewItem *item, const QPoint& pos, int col);

protected:
  /**
    * Reimplemented from kMyMoneyAccountSelector to get events from the viewport (to hide
    * this widget on mouse-click, Escape-presses, etc.
    */
  virtual bool eventFilter( QObject *, QEvent * );

  /**
    * This method resizes the widget to show a maximum of @p count lines
    */
  void adjustSize(const int count);

signals:
  void accountSelected(const QCString& id);

private:
  QWidget*                    m_parent;
  kMyMoneyAccountSelector*    m_accountSelector;
  QCString                    m_id;
};

#endif
