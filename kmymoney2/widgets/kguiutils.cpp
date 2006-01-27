/***************************************************************************
                         kguiutils.cpp  -  description
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

// ----------------------------------------------------------------------------
// QT Includes
 // No need for QDateEdit, QSpinBox, etc., since these always return values
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qwidget.h>
#include <qhbox.h>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kguiutils.h"
#include "../kmymoneysettings.h"
#include "kmymoneyedit.h"

 /**************************************************************************
  *                                                                        *
  * The MandatoryFieldGroup code is courtesy of                            *
  * Mark Summerfield in Qt Quarterly                                       *
  * http://doc.trolltech.com/qq/qq11-mandatoryfields.html                  *
  *                                                                        *
  **************************************************************************/

void kMandatoryFieldGroup::add(QWidget *widget)
{
  if (!widgets.contains(widget)) {
    if (widget->inherits("QCheckBox"))
      connect((QCheckBox*)widget->qt_cast("QCheckBox"),
               SIGNAL(clicked()),
               this, SLOT(changed()));
    else if (widget->inherits("QComboBox"))
      connect((QComboBox*)widget->qt_cast("QComboBox"),
               SIGNAL(highlighted(int)),
               this, SLOT(changed()));
    else if (widget->inherits("QLineEdit"))
      connect((QLineEdit*)widget->qt_cast("QLineEdit"),
               SIGNAL(textChanged(const QString&)),
               this, SLOT(changed()));
    else if (widget->isA("kMyMoneyEdit")) {
      connect((kMyMoneyEdit*)widget,
               SIGNAL(textChanged(const QString&)),
               this, SLOT(changed()));
      ((kMyMoneyEdit*)widget)->setPaletteBackgroundColor(KMyMoneySettings::requiredFieldColor());
    }
    else {
      qWarning("MandatoryFieldGroup: unsupported class %s",
               widget->className());
      return;
    }
    widget->setPaletteBackgroundColor(KMyMoneySettings::requiredFieldColor());
    widgets.append(widget);
    changed();
  }
}


void kMandatoryFieldGroup::remove(QWidget *widget)
{
  widget->setPaletteBackgroundColor(widget->colorGroup().background());
  widgets.remove(widget);
  changed();
}


void kMandatoryFieldGroup::setOkButton(QPushButton *button)
{
  if (okButton && okButton != button)
    okButton->setEnabled(true);
  okButton = button;
  changed();
}


void kMandatoryFieldGroup::changed()
{
  if (!okButton)
    return;
  bool enable = true;
  QValueList<QWidget *>::ConstIterator i;
  for (i = widgets.begin(); i != widgets.end(); ++i) {
    QWidget *widget = *i;
    if (widget->inherits("QCheckBox")) {
      if (((QCheckBox*)widget->qt_cast("QCheckBox"))->state() ==
            QButton::NoChange) {
        enable = false;
        break;
            }
            else
              continue;
    }
    if (widget->inherits("QComboBox")) {
      if (((QComboBox*)widget->qt_cast("QComboBox"))->currentText()
            .isEmpty()) {
        enable = false;
        break;
            }
            else
              continue;
    }
    if (widget->inherits("QLineEdit")) {
      if (((QLineEdit*)widget->qt_cast("QLineEdit"))->text()
            .isEmpty()) {
        enable = false;
        break;
            }
            else
              continue;
    }
    if (widget->isA("kMyMoneyEdit")) {
      if (!((kMyMoneyEdit*)widget)->isValid()) {
        enable = false;
        break;
            }
            else
              continue;
    }
  }

  okButton->setEnabled(enable);
}


void kMandatoryFieldGroup::clear()
{
  QValueList<QWidget *>::Iterator i;
  for (i = widgets.begin(); i != widgets.end(); ++i)
    (*i)->setPaletteBackgroundColor((*i)->colorGroup().background());
  widgets.clear();
  if (okButton) {
    okButton->setEnabled(true);
    okButton = 0;
  }
}



