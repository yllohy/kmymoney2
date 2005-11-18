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
#include <qstringlist.h>
#include <qmap.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kdialogbase.h>
#include <kfontdialog.h>
#include <kcolorbutton.h>
#include <klineedit.h>

class KIntNumInput;
class KListView;
class KPushButton;
class QTabWidget;
class kMyMoneyOnlineQuoteConfig;
class kMyMoneyGPGConfig;

// ----------------------------------------------------------------------------
// Project Includes
#include "../widgets/kmymoneydateinput.h"


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
  *
  * @short A class to manipulate the settings needed for running KMyMoney2
  */
class KSettingsDlg : public KDialogBase
{
  Q_OBJECT

private:
  QMap<int, QString>         m_helpAnchor;

  /** Start prompt dialog */
  QRadioButton *m_qradiobuttonStartPrompt;
  /** Start file */
  QRadioButton *m_qradiobuttonStartFile;
  /** Start home page */
  QRadioButton *m_qradiobuttonStartHome;
  /** Start last page */
  QRadioButton *m_qradiobuttonStartLast;

  /** Color list */
  KColorButton *m_kcolorbuttonList;
  /** Color background */
  KColorButton *m_kcolorbuttonBack;
  /** Color grid */
  KColorButton *m_kcolorbuttonGrid;
  /** Missing conversion rate */
  KColorButton *m_kcolorbuttonRate;
  /** Select font header */
  KFontChooser *m_kfontchooserHeader;
  /** Font cell setting */
  KFontChooser *m_kfontchooserCell;
  /** Prefer system font */
  QCheckBox* m_systemFont;
  QTabWidget* m_fontTabWidget;

  /** No rows to show in register */
  // KLineEdit *m_klineeditRowCount;

  /** Show grid in register ? */
  QCheckBox *m_qcheckboxShowGrid;

  /** Show text in register ? */
  // QCheckBox *m_qcheckboxTextPrompt;

  /** Show lens over a transaction in ledger ? */
  QCheckBox *m_qcheckboxLedgerLens;

  /** Show transaction form */
  QCheckBox *m_qcheckboxTransactionForm;

  /** AutoSaveFile **/
  QCheckBox *m_qcheckboxAutoSaveFile;
  KIntNumInput* m_intAutoSavePeriod;

  /* copy type to nr field */
  QCheckBox* m_qcheckboxTypeToNr;

  /* increment check number */
  QCheckBox* m_qcheckboxIncCheckNumber;

  /* hide unused categories */
  QCheckBox* m_qcheckboxHideCategory;

  /* always show a nr field in the transaction form */
  QCheckBox* m_qcheckboxShowNrField;

  /* keep changes when selecting different transaction / split */
  QCheckBox* m_qcheckboxFocusChangeEnter;

  /* auto prefill a transaction with previous data of the same payee */
  QCheckBox* m_qcheckboxAutoFillTransaction;

  /** colour options */
  QRadioButton *m_qradiobuttonPerTransaction;
  QRadioButton *m_qradiobuttonOtherRow;

  /* Online quotes configuration widget */
  kMyMoneyOnlineQuoteConfig* m_onlineQuotesWidget;

  /* GPG encryption configuration widget */
  kMyMoneyGPGConfig* m_encryptionWidget;

  /** Restrict options */
  kMyMoneyDateInput *m_dateinputStart;
  QCheckBox* m_qcheckboxHideReconciledTransactions;

#if 0
  QRadioButton *m_qradiobuttonNormalView;
  QRadioButton *m_qradiobuttonAccountView;
#endif

  QRadioButton *m_qradiobuttonAccountDialog;
  QRadioButton *m_qradiobuttonAccountWizard;

  QCheckBox *m_qradiobuttonCheckSchedules;
  KIntNumInput* m_intSchedulePreview;

  KListView*    m_homePageList;
  KPushButton*  m_upButton;
  KPushButton*  m_downButton;
  int           m_currentItem;

  KLineEdit*    m_qIntPricePrecision;
  int           m_iTempPricePrecision;

  /** Set page general */
  void setPageGeneral();
  /** Set page list settings */
  void setPageList();

  void setPageAccountsView();

  /** Set homepage settings */
  void setHomePage();
  void fillHomePageItems(QStringList& list);
  const QStringList homePageItems(void) const;

  /** Set schedule options */
  void setPageSchedule();

  /** Set encryption options */
  void setPageEncryption(void);

  /** Set colour options */
  void setPageColour();

  /** Set font options */
  void setPageFont();

  /** Set online quote options */
  void setPageOnlineQuotes(void);

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
  QColor m_qcolorTempRate;
  QFont m_qfontTempHeader;
  QFont m_qfontTempCell;
//  QString m_qstringTempRowCount;
  bool m_bTempShowGrid;
//  bool m_bTempColourPerTransaction;
  bool m_bTempStartPrompt;
  bool m_bDoneApply;
  bool m_bTempHideCategory;
  QDate m_qdateTempStart;
  bool m_bTempHideReconciledTransactions;
//  bool m_bTempNormalView;
  bool m_bTempAccountWizard;
  bool m_bTempLedgerLens;
  bool m_bTempTransactionForm;
  bool m_bTempTypeToNr;
  bool m_bTempAutoIncCheckNumber;
  bool m_bTempShowNrField;
  bool m_bTempStartPage;
  bool m_bTempCheckSchedule;
  bool m_bTempUseSystemFont;
  bool m_bTempFocusChangeEnter;
  bool m_bTempAutoFillTransaction;
  bool m_bTempAutoSaveFile;
  int  m_iTempAutoSavePeriod;
  int  m_iTempSchedulePreview;
  QStringList m_tempHomePageItems;

protected slots:
  void slotHelp(void);

private slots:
  /** Called when OK pressed */
  void slotOk();

  /** Called when Apply pressed */
  void slotApply();

  /** Called when Cancel pressed */
  void slotCancel();

  /** Called when Reset pressed */
  void slotUser1();

  /** Called when 'Always show nr field' is toggled */
  void slotNrFieldToggled(bool state);

  /** Called when 'Copy Type to Nr field' is toggled */
  void slotTypeToNrToggled(bool state);

  /** Called when 'Use system font' is toggled */
  void slotSystemFontToggled(bool state);

  void slotSelectHomePageItem(QListViewItem *);

  void slotMoveUp(void);
  void slotMoveDown(void);


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
