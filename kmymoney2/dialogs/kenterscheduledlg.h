/***************************************************************************
                          kenterscheduledlg.h  -  description
                             -------------------
    begin                : Sat Apr  7 2007
    copyright            : (C) 2007 by Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KENTERSCHEDULEDLG_H
#define KENTERSCHEDULEDLG_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class TransactionEditor;

#include "../dialogs/kenterscheduledlgdecl.h"
#include <kmymoney/mymoneyscheduled.h>

class KEnterScheduleDlgPrivate;

/**
  * @author Thomas Baumgart
  */
class KEnterScheduleDlg : public KEnterScheduleDlgDecl
{
  Q_OBJECT
public:
  KEnterScheduleDlg(QWidget *parent, const MyMoneySchedule& schedule);
  ~KEnterScheduleDlg();

  TransactionEditor* startEdit(void);
  MyMoneyTransaction transaction(void);

protected:
  /// Overridden for internal reasons. No API changes.
  bool focusNextPrevChild(bool next);

  /**
    * This method returns the adjusts @a _date according to
    * the setting of the schedule's weekend option.
    */
  QDate date(const QDate& _date) const;

public slots:
  int exec(void);

private slots:
  void slotSetupSize(void);

private:
  KEnterScheduleDlgPrivate*  d;
};

#endif