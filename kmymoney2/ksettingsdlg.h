/***************************************************************************
                          ksettingsdlg.h
                             -------------------
    copyright            : (C) 2000 by Michael Edwardes
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

#include <kdialogbase.h>

#include <qcheckbox.h>
#include <kfontdialog.h>
#include <kcolorbutton.h>
#include <klineedit.h>

// This dialog lets the user change the program settings.
// Doesn't do much at the moment !
class KSettingsDlg : public KDialogBase  {
   Q_OBJECT
public:
  KSettingsDlg(QWidget *parent=0, const char *name=0, bool modal=true);
	~KSettingsDlg();
private: // Private methods
  /** Set page general */
  void setPageGeneral();
  /** Set page list settings */
  void setPageList();
  /** Write settings */
  void configWrite();
  /** Read settings */
  void configRead();
private: // Private attributes
  /** Start prompt dialog */
  QCheckBox *start_prompt;
  /** Color list */
  KColorButton *color_list;
  /** Color background */
  KColorButton *color_back;
  /** Select font header */
  KFontChooser *font_header;
  /** Font cell setting */
  KFontChooser *font_cell;

  KLineEdit *m_klineeditRowCount;

  QCheckBox *m_qcheckboxShowGrid;

private slots: // Private slots
  /** Slot ok */
  void slotOk();
};

#endif
