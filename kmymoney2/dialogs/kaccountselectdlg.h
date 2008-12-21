/***************************************************************************
                          kaccountselectdlg.h  -  description
                             -------------------
    begin                : Mon Feb 10 2003
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

#ifndef KACCOUNTSELECTDLG_H
#define KACCOUNTSELECTDLG_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qwidget.h>
#include <qstring.h>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoney/kmymoneycombo.h>
#include <kmymoney/mymoneyaccount.h>
#include <kmymoney/kmymoneyutils.h>
#include "../dialogs/kaccountselectdlgdecl.h"

/**
  * @author Thomas Baumgart
  */

class KAccountSelectDlg : public KAccountSelectDlgDecl
{
   Q_OBJECT
public:
  KAccountSelectDlg(const KMyMoneyUtils::categoryTypeE type, const QString& purpose = "General", QWidget *parent=0, const char *name=0);
  ~KAccountSelectDlg();

  /**
    * This method is used to setup the descriptive text in the account
    * selection dialog box. The @p msg should contain a descriptive
    * text about the purpose of the dialog and it's options.
    *
    * @param msg const reference to QString object containing the text.
    */
  void setDescription(const QString& msg);

  /**
    * This method is used to setup the buddy text of the account
    * selection box. the @p msg should contain a short text
    * which is placed above the selection box with the account
    * names.
    *
    * @param msg const reference to QString object containing the text.
    */
  void setHeader(const QString& msg);

  /**
    * This method is used to pass information to the account selection
    * dialog which will be used as initial selection in the account
    * selection combo box and during account creation.
    *
    * @param account MyMoneyAccount filled with the relevant and available information
    * @param id account id to be used.
    */
  void setAccount(const MyMoneyAccount& account, const QString& id);

  /**
    * This method returns the name of the selected account in the combo box.
    *
    * @return QString containing the id of the selected account
    */
  const QString& selectedAccount(void) const;

  /**
    * This method is used to set the mode of the dialog. Two modes
    * are supplied: a) select or create and b) create only.
    * If @p mode is 0, select or create is selected, otherwise create only
    * is selected.
    *
    * @param mode selected mode
    */
  void setMode(const int mode);

  /**
    * This method allows to control the visibilty of the abort button
    * in this dialog according to the the parameter @p visible.
    *
    * @param visible @p true shows the abort button, @p false hides it.
    */
  void showAbortButton(const bool visible);

  /**
    * This method is used to determine if the user pressed the 'Skip' or
    * the 'Abort' button. The return value is valid only, if the exec()
    * function of the dialog returns false.
    *
    * @retval false Dialog was left using the 'Skip' button
    * @retval true Dialog was left using the 'Abort' button
    */
  bool aborted(void) const { return m_aborted; };

public slots:
  /**
    * Reimplemented from QDialog
    */
  int exec();

protected slots:
  /**
    * This slot is used to fire up the new account wizard and preset it
    * with the values found in m_account. If an account was created using
    * the wizard, this will be the selected account.
    */
  void slotCreateAccount(void);

  /**
    * This slot is used to fire up the new institution dialog
    */
  void slotCreateInstitution(void);

  /**
    * This slot is used to react on the abort button
    */
  void abort(void);

  /**
    * This is the slot which will be called if the engine data is changed.
    */
  void slotReloadWidget(void);

private:
  QString         m_purpose;
  MyMoneyAccount  m_account;
  int             m_mode;       // 0 - select or create, 1 - create only
  KMyMoneyUtils::categoryTypeE   m_accountType;
  bool            m_aborted;
};

#endif
