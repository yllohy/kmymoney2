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

#ifndef KMYMONEYONLINEQUOTECONFIG_H
#define KMYMONEYONLINEQUOTECONFIG_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyonlinequoteconfigdecl.h"

/**
  * @author Thomas Baumgart
  */

/**
  * This is a helper class to store information about an online source
  * for stock prices or currency exchange rates.
  */
class kMyMoneyOnlineQuoteSource
{
public:
  kMyMoneyOnlineQuoteSource() {};
  kMyMoneyOnlineQuoteSource(const QString& name);
  ~kMyMoneyOnlineQuoteSource() {};

  void writeConfig(void);
  void renameConfig(const QString& name);

  QString    m_name;
  QString    m_date;
  QString    m_price;
  QString    m_url;
  QString    m_sym;
};

class kMyMoneyOnlineQuoteConfig : public kMyMoneyOnlineQuoteConfigDecl
{
  Q_OBJECT
public:
  kMyMoneyOnlineQuoteConfig(QWidget* parent, const char *name);
  virtual ~kMyMoneyOnlineQuoteConfig() {};

  void writeConfig(void) {};
  void readConfig(void) {};
  void resetConfig(void);

  static QStringList quoteList(void);

protected slots:
  void slotUpdateEntry(void);
  void slotLoadWidgets(QListViewItem* item);
  void slotEntryChanged(void);
  void slotNewEntry(void);
  void slotEntryRenamed(QListViewItem* item, const QString& text, int col);

protected:
  /**
    * This member returns a list of the names of the quote sources
    * currently defined.
    *
    * @return QStringList of quote source names
    */
  QStringList quoteSourceList(void);

  /**
    * This method converts the simple name to the group name
    * used in the configuration file.
    *
    * Example: @p Yahoo is converted to @p Online-Quote-Source-Yahoo .
    *
    * @param name name of the source
    *
    * @return converted name
    */
  const QString groupName(const QString& name) const;

  void loadList(const bool updateResetList = false);

private:
  QValueList<kMyMoneyOnlineQuoteSource>  m_resetList;
  kMyMoneyOnlineQuoteSource              m_currentItem;
};

#endif
