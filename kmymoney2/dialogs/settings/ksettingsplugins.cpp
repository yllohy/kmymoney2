/***************************************************************************
                          ksettingsplugins.cpp
                             -------------------
    begin                : Thu Feb 12 2009
    copyright            : (C) 2009 Cristian Onet
    email                : onet.cristian@gmail.com
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

#include <qlayout.h>
#include <qstring.h>
#include <qlabel.h>

// ----------------------------------------------------------------------------
// KDE Includes
#include <klocale.h>
#include <kdialog.h>

// ----------------------------------------------------------------------------
// Project Includes
#include "kmymoney2/plugins/pluginloader.h"
#include "ksettingsplugins.h"

class KSettingsPlugins::Private
{
public:
  Private() : pluginsNumber(0), pluginConfig(0) {}

  QLabel*                       pluginsNumber;
  KMyMoneyPlugin::ConfigWidget* pluginConfig;
};

KSettingsPlugins::KSettingsPlugins(QWidget* parent)
  : QWidget(parent), d(new Private)
{
  QVBoxLayout *layout = new QVBoxLayout(this);
  d->pluginsNumber    = new QLabel(this);
  d->pluginConfig     = KMyMoneyPlugin::PluginLoader::instance()->configWidget(this);

  layout->addWidget(d->pluginsNumber);
  layout->addWidget(d->pluginConfig);
  layout->setSpacing(KDialog::spacingHint());
}

KSettingsPlugins::~KSettingsPlugins()
{
  delete d;
}

void KSettingsPlugins::initPlugins(int pluginsNumber)
{
  d->pluginsNumber->setText(i18n("%1 plugin(s) found").arg(pluginsNumber));
}

void KSettingsPlugins::slotApplyPlugins()
{
  d->pluginConfig->apply();
}

#include "ksettingsplugins.moc"
