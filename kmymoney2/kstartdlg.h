
/***************************************************************************
                          kcpreferences.h  -  description
                             -------------------
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KSTARTDLG_H
#define KSTARTDLG_H

#include <kdialogbase.h>

#include <klocale.h>
#include <kfontdialog.h>
#include <kurlrequester.h>
#include <kiconview.h>

#include <qstring.h>
#include <qradiobutton.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qpushbutton.h>

/**KMyMoney 2 start dialog
  */

class KStartDlg : public KDialogBase  {
   Q_OBJECT
public:
	KStartDlg( QWidget *parent=0, const char *name=0, bool modal=true );
	~KStartDlg();
  //NOTE: filename argument in KStarDlg is not necessary. This can be set in recent documents.
  bool isNewFile(void)          { return isnewfile;           }
  bool isOpenFile(void)         { return !kurlrequest->url().isEmpty();          }
  QString getFileName(void)     { return kurlrequest->url();  }
  QString getTemplateName(void) { return templatename;    }
public:
  /*NOTE: I change this to getFileName(void)*/   QString m_filename;
private: // Private methods
  void setPage_Template();
  void setPage_Documents();
  /** misc widgets */
  /** Write config window */
  void writeConfig();
  /** Read config window */
  void readConfig();
  KIconView *view_wizard;
  KIconView *view_recent;
  KURLRequester *kurlrequest;
  /** misc variables */
  bool isnewfile;
  bool isopenfile;
  QString templatename;
protected slots:
  /** No descriptions */
  void slotOk();
private slots:
  void slotTemplateClicked(QIconViewItem *item);
  /** slot to recent view */
  void slotRecentClicked(QIconViewItem *item);
};

#endif
