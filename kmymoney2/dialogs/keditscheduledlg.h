/***************************************************************************
                          keditscheduledlg.h  -  description
                             -------------------
    begin                : Mon Sep  3 2007
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

#ifndef KEDITSCHEDULEDLG_H
#define KEDITSCHEDULEDLG_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoney/mymoneyfile.h>
#include <kmymoney/mymoneyscheduled.h>
#include "../dialogs/keditscheduledlgdecl.h"

class KEditScheduleDlgPrivate;
class TransactionEditor;

/**
  * @author Thomas Baumgart
  */

class KEditScheduleDlg : public KEditScheduleDlgDecl
{
  Q_OBJECT
public:
  /**
    * Standard QWidget constructor.
    **/
  KEditScheduleDlg(const MyMoneySchedule& schedule, QWidget *parent=0, const char *name=0);

  /**
    * Standard destructor.
    **/
  ~KEditScheduleDlg();

  TransactionEditor* startEdit(void);

  /**
    * Returns the edited schedule.
    *
    * @return MyMoneySchedule The schedule details.
    **/
  const MyMoneySchedule& schedule(void) const;

protected:
  MyMoneyTransaction transaction(void) const;
  /**
    * This method adjusts @a _date according to the rules specified by
    * the schedule's weekend option.
    */
  QDate adjustDate(const QDate& _date) const;

  /// Overridden for internal reasons. No API changes.
  bool focusNextPrevChild(bool next);

private slots:
  void slotSetupSize(void);
  void slotRemainingChanged(const QString& text);
  void slotEndDateChanged(const QDate& date);
  void slotSetPaymentMethod(int);
  void slotFrequencyChanged(int item);
  void slotShowHelp(void);

private:
  KEditScheduleDlgPrivate* d;
};

#endif