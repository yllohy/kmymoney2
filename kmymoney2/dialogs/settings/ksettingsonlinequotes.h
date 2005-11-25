/***************************************************************************
                          kmymoneyonlinequoteconfig.h  -  description
                             -------------------
    begin                : Thu Dec 30 2004
    copyright            : (C) 2004 by Thomas Baumgart
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

#ifndef KSETTINGSONLINEQUOTES_H
#define KSETTINGSONLINEQUOTES_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoney2/dialogs/settings/ksettingsonlinequotesdecl.h"
#include "kmymoney2/converter/webpricequote.h"

class KSettingsOnlineQuotes : public KSettingsOnlineQuotesDecl
{
  Q_OBJECT
public:
  KSettingsOnlineQuotes(QWidget* parent = 0, const char *name = 0);
  virtual ~KSettingsOnlineQuotes() {};

  void writeConfig(void) {}
  void readConfig(void) {}
  void resetConfig(void);

protected slots:
  void slotUpdateEntry(void);
  void slotLoadWidgets(QListViewItem* item);
  void slotEntryChanged(void);
  void slotNewEntry(void);
  void slotEntryRenamed(QListViewItem* item, const QString& text, int col);

protected:
  void loadList(const bool updateResetList = false);

private:
  QValueList<WebPriceQuoteSource>  m_resetList;
  WebPriceQuoteSource              m_currentItem;
};

#endif
