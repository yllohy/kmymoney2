/***************************************************************************
                          ksettingsdlg.h
                             -------------------
    copyright            : (C) 2000,2001 by Michael Edwardes
    email                : mte@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef KSETTINGSDLG_H
#define KSETTINGSDLG_H

// ----------------------------------------------------------------------------
// QT Includes
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <qcolor.h>
#include <qfont.h>

// ----------------------------------------------------------------------------
// KDE Includes
#include <kdialogbase.h>
#include <kfontdialog.h>
#include <kcolorbutton.h>
#include <klineedit.h>

// ----------------------------------------------------------------------------
// Project Includes

/**
  * This class is used to manipulate all the settings available for KMyMoney2.
  * It currently stores the values for the list settings and whether to show
  * the start dialog when KMyMoney2 starts.
  *
  * It uses KDialogBase to implement it's interface.
  *
  * It uses the global KConfig object to read and write the application
  * settings.
  *
  * @see KDialogBase
  *
  * @author Michael Edwardes 2000-2001
  * $Id: ksettingsdlg.h,v 1.1 2002/01/17 00:34:16 mte Exp $
  *
  * @short A class to manipulate the settings needed for running KMyMoney2
**/
class KSettingsDlg : public KDialogBase  {
   Q_OBJECT

private:
  /** Start prompt dialog */
  QRadioButton *m_qradiobuttonStartPrompt;
  /** Start file */
  QRadioButton *m_qradiobuttonStartFile;
  /** Color list */
  KColorButton *m_kcolorbuttonList;
  /** Color background */
  KColorButton *m_kcolorbuttonBack;
  /** Color grid */
  KColorButton *m_kcolorbuttonGrid;
  /** Select font header */
  KFontChooser *m_kfontchooserHeader;
  /** Font cell setting */
  KFontChooser *m_kfontchooserCell;

  /** No rows to show in register */
  KLineEdit *m_klineeditRowCount;

  /** Show grid in register ? */
  QCheckBox *m_qcheckboxShowGrid;

  /** Show text in register ? */
  QCheckBox *m_qcheckboxTextPrompt;

  /** colour options */
  QRadioButton *m_qradiobuttonPerTransaction;
  QRadioButton *m_qradiobuttonOtherRow;

  /** Set page general */
  void setPageGeneral();
  /** Set page list settings */
  void setPageList();
  /** Write settings */
  void configWrite();
  /** Read settings */
  void configRead();

  /** Attributes to store the variables so they can be undone when cancelling
    * the apply.
  **/
  QColor m_qcolorTempList;
  QColor m_qcolorTempListBG;
  QColor m_qcolorTempListGrid;
  QFont m_qfontTempHeader;
  QFont m_qfontTempCell;
  QString m_qstringTempRowCount;
  bool m_bTempShowGrid;
  bool m_bTempColourPerTransaction;
  bool m_bTempStartPrompt;
  bool m_bDoneApply;
  bool m_bTempTextPrompt;

private slots:
  /** Called when OK pressed */
  void slotOk();

  /** Called when Apply pressed */
  void slotApply();

  /** Called when Cancel pressed */
  void slotCancel();

  /** Called when Reset pressed */
  void slotUser1();

public:
  /**
    * Standard constructor.
    *
    * @param parent The QWidget this is used in.
    * @param name The QT name.
    * @param modal True if we want the dialog to be application modal.
    *
    * @return An object of type KSettingsDlg.
    *
    * @see ~KSettingsDlg
  **/
  KSettingsDlg(QWidget *parent=0, const char *name=0, bool modal=true);

  /**
    * Standard destructor.
    *
    * @return Nothing.
    *
    * @see KSettingsDlg
  **/
  ~KSettingsDlg();

signals:
  /**
    * Emitted when the Apply button is clicked to allow the application to
    * show the changes without having to close the dialog.
    *
    * @return Nothing
  **/
  void signalApply();
};

#endif
