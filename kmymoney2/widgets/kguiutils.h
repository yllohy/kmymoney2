/***************************************************************************
                         kguiutils.h  -  description
                            -------------------
   begin                : Fri Jan 27 2006
   copyright            : (C) 2006 Tony Bloomfield
   email                : Tony Bloomfield <tonybloom@users.sourceforge.net>
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KGUIUTILS_H
#define KGUIUTILS_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qobject.h>
#include <qvaluelist.h>
class QWidget;

// ----------------------------------------------------------------------------
// KDE Includes

#include <kpushbutton.h>
#include <kcombobox.h>

// ----------------------------------------------------------------------------
// Project Includes

/**
  * @author Tony Bloomfield
  */
class kMandatoryFieldGroup : public QObject
{
  Q_OBJECT

public:
  kMandatoryFieldGroup(QObject *parent) :
    QObject(parent), okButton(0), m_enabled(true) {}

  /**
    * This method adds a widget to the list of mandatory fields for the current dialog
    *
    * @param widget pointer to the widget to be added
    */
  void add(QWidget *widget);

  /**
    * This method removes a widget form the list of mandatory fields for the current dialog
    *
    * @param widget pointer to the widget to be removed
    */
  void remove(QWidget *widget);

  /**
    * This method designates the button to be enabled when all mandatory fields
    * have been completed
    *
    * @param button pointer to the 'ok' button
    */
  void setOkButton(QPushButton *button);

  /**
    * This method returns if all requirements for the mandatory group
    * have been fulfilled (@p true) or not (@p false).
    */
  bool isEnabled(void) const { return m_enabled; }

public slots:
  void clear(void);

  /**
    * Force update of ok button
    */
  void changed(void);

signals:
  void stateChanged(void);
  void stateChanged(bool state);

private:
  QValueList<QWidget *> widgets;
  QPushButton*          okButton;
  bool                  m_enabled;
};

#endif // KGUIUTILS_H
