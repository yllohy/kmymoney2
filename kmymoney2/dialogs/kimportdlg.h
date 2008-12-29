/***************************************************************************
                          kimportdlg.h  -  description
                             -------------------
    begin                : Wed May 16 2001
    copyright            : (C) 2001 by Michael Edwardes
    email                : mte@users.sourceforge.net
                             Javier Campos Morales <javi_c@ctv.es>
                             Felix Rodriguez <frodriguez@mail.wesleyan.edu>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef KIMPORTDLG_H
#define KIMPORTDLG_H

// ----------------------------------------------------------------------------
// QT Headers

#include <qstring.h>
#include <qlineedit.h>
#include <qcheckbox.h>

// ----------------------------------------------------------------------------
// KDE Headers

#include <kurl.h>
#include <kcombobox.h>

// ----------------------------------------------------------------------------
// Project Headers

#include "../dialogs/kimportdlgdecl.h"

/**
  * This class is used to import a qif file to an account.
  * It relies upon the QIF file handling routines in MyMoneyAccount to do
  * the actual writing of QIF files.
  *
  * It uses the global KConfig object to read and write the application
  * settings.
  *
  * @see MyMoneyAccount
  *
  * @author Felix Rodriguez, Michael Edwardes 2000-2001
  *
  * @short A class to import a qif file to an account.
**/
class KImportDlg : public KImportDlgDecl
{
  Q_OBJECT

public:
  /**
    * Standard constructor
    */
  KImportDlg(QWidget *parent, const char *name = 0);

  /** Standard destructor */
  ~KImportDlg();

  /**
    */
  const QString filename(void) const { return m_qlineeditFile->text(); };

  /**
    */
  const QString profile(void) const { return m_profileComboBox->currentText(); };

protected slots:
  /** Called to let the user browse for a QIF file to import from. */
  void slotBrowse();

  /** Test whether to enable the buttons */
  void slotFileTextChanged(const QString& text);

  /**
    * Called when the user needs a new profile
    */
  void slotNewProfile(void);

  void slotOkClicked(void);

  void slotSelectProfile(const QString& text);

private:
  /**
    * This method loads the available profiles into
    * the combo box. The parameter @p selectLast controls if
    * the last profile used is preset or not. If preset is not
    * selected, the current selection remains. If the currently selected
    * text is not present in the list anymore, the first item will be
    * selected.
    *
    * @param selectLast If true, the last used profile is selected. The
    *                   default is false.
    */
  void loadProfiles(const bool selectLast = false);

  /**
    * This method is used to load an account hierarchy into a string list
    *
    * @param strList Reference to the string list to setup
    * @param id Account id to add
    * @param leadIn constant leadin to be added in front of the account name
    */
  void addCategories(QStringList& strList, const QString& id, const QString& leadIn) const;

  void readConfig(void);
  void writeConfig(void);
};

#endif
