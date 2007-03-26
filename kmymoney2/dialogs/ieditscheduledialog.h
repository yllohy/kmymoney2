/***************************************************************************
                          keditscheduledialog.h  -  description
                             -------------------
    begin                : Tue Jul 22 2003
    copyright            : (C) 2000-2003 by Michael Edwardes
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
#ifndef KEDITSCHEDULEDIALOG_H
#define KEDITSCHEDULEDIALOG_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoney/mymoneyfile.h>
#include <kmymoney/mymoneyscheduled.h>
#include "../dialogs/keditschedtransdlgdecl.h"
#include "../widgets/kguiutils.h"

/**
  * @author Michael Edwardes, Thomas Baumgart
  */

class KEditScheduleDialog : public kEditScheduledTransferDlgDecl
{
  Q_OBJECT
public:
  /**
    * Standard QWidget constructor.
    **/
  KEditScheduleDialog(const QCString& action, const MyMoneySchedule& schedule, QWidget *parent=0, const char *name=0);

  /**
    * Standard destructor.
    **/
  ~KEditScheduleDialog();

  /**
    * Returns the edited schedule.
    *
    * @return MyMoneySchedule The schedule details.
    **/
  const MyMoneySchedule& schedule(void) { return m_schedule; };

protected slots:
  /**
    * Called when the split button is clicked
    **/
  void slotSplitClicked();

  /**
    * Called when the 'will end at some time' check box is clicked.
    **/
  void slotWillEndToggled(bool on);

  /**
    * Called when the OK button is clicked.
    **/
  void okClicked();

  void slotRemainingChanged(const QString& text);
  void slotEndDateChanged(const QDate& date);

  void slotAmountChanged(const QString&);
  void slotAccountChanged(const QCString&);
  void slotScheduleNameChanged(const QString&);
  void slotToChanged(const QCString&);
  void slotMethodChanged(int);
  void slotPayeeChanged(const QString&);
  void slotDateChanged(const QDate&);
  void slotFrequencyChanged(int);
  void slotEstimateChanged();
  void slotCategoryChanged(const QString&);
  /**
    * the category with @a id has been selected.
    */
  void slotCategoryChanged(const QCString& id);
  void slotAutoEnterChanged();
  void slotMemoChanged(const QString& text);
  void slotHelp(void);
  void slotReloadEditWidgets(void);

signals:
  /**
    * This signal is sent out, when a new category needs to be created
    * @sa KMyMoneyCombo::createItem()
    *
    * @param txt The name of the category to be created
    * @param id A connected slot should store the id of the created object in this variable
    */
  void createCategory(const QString& txt, QCString& id);

  /**
    * This signal is sent out, when a new payee needs to be created
    * @sa KMyMoneyCombo::createItem()
    *
    * @param txt The name of the payee to be created
    * @param id A connected slot should store the id of the created object in this variable
    */
  void createPayee(const QString& txt, QCString& id);

private:
  /// Save the current account ids (there must be a better way...)
  QCString m_fromAccountId, m_toAccountId;
  /// Save last payee used for convenience
  QString m_lastPayee;

  /// The transaction details
  MyMoneyTransaction m_transaction;

  /// The schedule details.
  MyMoneySchedule m_schedule;

  /// the action
  QCString m_actionType;
  // occurrences in order
  static MyMoneySchedule::occurenceE occurMasks[];

  /**
    * This method makes sure, that m_transaction has at least
    * two splits when currently only one is present.
    */
  void createSecondSplit(void);

  /**
    * Sets up the widgets based on whats in MyMoneyFile.
    */
  void reloadFromFile(void);

  /**
    * Read stored settings.
    **/
  void readConfig(void);

  /**
    * Write setting to config file.
    **/
  void writeConfig(void);

  /**
    * Reloads the qidgets from the global schedule.
    **/
  void loadWidgetsFromSchedule(void);

  MyMoneySchedule::occurenceE comboToOccurence(void);
  void createSplits();
  bool checkCategory();
  void checkPayee();
  QCString theAccountId();

  /* To hold list of mandatory fields */
  kMandatoryFieldGroup *m_requiredFields;

};

#endif
