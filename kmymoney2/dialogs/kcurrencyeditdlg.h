/***************************************************************************
                          kcurrencyeditdlg.h  -  description
                             -------------------
    begin                : Wed Mar 24 2004
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

#ifndef KCURRENCYEDITDLG_H
#define KCURRENCYEDITDLG_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qwidget.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kpopupmenu.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kcurrencyeditdlgdecl.h"
#include "../mymoney/mymoneysecurity.h"
// #include "../mymoney/mymoneycurrency.h"

/**
  * @author Thomas Baumgart
  */

class KCurrencyEditDlg : public KCurrencyEditDlgDecl
{
  Q_OBJECT
public:
  KCurrencyEditDlg(QWidget *parent=0, const char *name=0);
  ~KCurrencyEditDlg();

public slots:
  void slotSelectCurrency(const QCString& id);

protected:
  /// the resize event
  virtual void resizeEvent(QResizeEvent*);
  void updateCurrency(void);

protected slots:
  void slotSelectCurrency(QListViewItem *);
  void slotSetBaseCurrency(void);
  void slotClose(void);
  void slotNewCurrency(void);
  void slotRenameCurrency(void);
  void slotDeleteCurrency(void);
  void slotListClicked(QListViewItem* item, const QPoint&, int);
  void slotRenameCurrency(QListViewItem* item, int col, const QString& txt);

private slots:
  void timerDone(void);

private:
  void loadCurrencies(void);
  void checkBaseCurrency(void);

private:
  MyMoneySecurity      m_currency;
  KPopupMenu*          m_contextMenu;
};

#endif
