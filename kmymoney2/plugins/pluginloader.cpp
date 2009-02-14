/***************************************************************************
                          pluginloader.cpp
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

#include <qstringlist.h>
#include <qcheckbox.h>
#include <qlayout.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <ktrader.h>
#include <kparts/componentfactory.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kconfig.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyplugin.h"
#include "pluginloader.h"

namespace KMyMoneyPlugin {

//---------------------------------------------------------------------
//
// PluginLoader::Info
//
//---------------------------------------------------------------------
struct PluginLoader::Info::Private {
  QString m_name;
  QString m_comment;
  QString m_library;
  Plugin* m_plugin;
  bool m_shouldLoad;
};

PluginLoader::Info::Info(const QString& name, const QString& comment, const QString& library, bool shouldLoad)
{
  d=new Private;
  d->m_name=name;
  d->m_comment=comment;
  d->m_library=library;
  d->m_plugin=0;
  d->m_shouldLoad=shouldLoad;
}

PluginLoader::Info::~Info()
{
  delete d;
}

const QString& PluginLoader::Info::name() const
{
  return d->m_name;
}

const QString& PluginLoader::Info::comment() const
{
  return d->m_comment;
}

const QString& PluginLoader::Info::library() const
{
  return d->m_library;
}

Plugin* PluginLoader::Info::plugin() const
{
  return d->m_plugin;
}

void PluginLoader::Info::setPlugin(Plugin* plugin)
{
  d->m_plugin=plugin;
}

bool PluginLoader::Info::shouldLoad() const
{
  return d->m_shouldLoad;
}

void PluginLoader::Info::setShouldLoad(bool value)
{
  d->m_shouldLoad=value;
}

//---------------------------------------------------------------------
//
// PluginLoader
//
//---------------------------------------------------------------------
static PluginLoader* s_instance = 0;

struct PluginLoader::Private
{
  QObject*   m_parent;
  PluginList m_pluginList;
};

PluginLoader::PluginLoader(QObject* parent)
{
  Q_ASSERT( s_instance == 0 );
  s_instance = this;

  d=new Private;
  d->m_parent = parent;

  KTrader::OfferList offers = KTrader::self()->query("KMyMoneyPlugin");
  KConfig* config = KGlobal::config();
  config->setGroup( QString("KMyMoneyPlugin/EnabledPlugin") );

  KTrader::OfferList::ConstIterator iter;
  for(iter = offers.begin(); iter != offers.end(); ++iter) {

    KService::Ptr service = *iter;
    QString name    = service->name();
    QString comment = service->comment();
    QString library = service->library();

    if (library.isEmpty() || name.isEmpty() ) {
        kdWarning() << "KMyMoneyPlugin::PluginLoader: Plugin had an empty name or library file - this should not happen." << endl;
        continue;
    }

    bool load = config->readBoolEntry( name, true );

    Info* info = new Info( name, comment, library, load );
    d->m_pluginList.append( info );
  }
}

PluginLoader::~PluginLoader()
{
  delete d;
}

void PluginLoader::loadPlugins()
{
  for( PluginList::Iterator it = d->m_pluginList.begin(); it != d->m_pluginList.end(); ++it ) {
    loadPlugin( *it );
  }
  emit replug();
}

void PluginLoader::loadPlugin( Info* info )
{
  if ( info->plugin() == 0 && info->shouldLoad() ) {
    Plugin *plugin = 0;
    int error;
    plugin = KParts::ComponentFactory
             ::createInstanceFromLibrary<Plugin>(info->library().local8Bit().data(),
                                                 d->m_parent, info->name(), QStringList(), &error);

    if (plugin)
      kdDebug() << "KMyMoneyPlugin::PluginLoader: Loaded plugin " << plugin->name()<< endl;
    else
    {
      kdWarning() << "KMyMoneyPlugin::PluginLoader:: createInstanceFromLibrary returned 0 for "
                  << info->name()
                  << " (" << info->library() << ")"
                  << " with error number "
                  << error << endl;
      if (error == KParts::ComponentFactory::ErrNoLibrary)
        kdWarning() << "KLibLoader says: "
                    << KLibLoader::self()->lastErrorMessage() << endl;
    }
    info->setPlugin(plugin);
  }
  if ( info->plugin() ) // Do not emit if we had trouble loading the plugin.
    emit PluginLoader::instance()->plug( info );
}

const PluginLoader::PluginList& PluginLoader::pluginList()
{
  return d->m_pluginList;
}

PluginLoader* PluginLoader::instance()
{
  Q_ASSERT( s_instance != 0);
  return s_instance;
}


//---------------------------------------------------------------------
//
// ConfigWidget
//
//---------------------------------------------------------------------
ConfigWidget* PluginLoader::configWidget( QWidget* parent )
{
  return new ConfigWidget( parent );
}

class PluginCheckBox :public QCheckBox
{
public:
  PluginCheckBox( PluginLoader::Info* info, QWidget* parent )
    : QCheckBox( QString("%1 (%2)").arg(info->name()).arg(info->comment()), parent ), info( info )
    {
      setChecked( info->shouldLoad() );
    }
  PluginLoader::Info* info;
};

struct ConfigWidget::Private
{
  QValueList< PluginCheckBox* > _boxes;
};

ConfigWidget::ConfigWidget( QWidget* parent )
  :QScrollView( parent, "KMyMoneyPlugin::PluginLoader::ConfigWidget" )
{
  d=new Private;
  QWidget* top = new QWidget( viewport() );
  addChild( top );
  setResizePolicy( AutoOneFit );

  QVBoxLayout* lay = new QVBoxLayout( top, KDialog::marginHint(), KDialog::spacingHint() );

  PluginLoader::PluginList list = PluginLoader::instance()->d->m_pluginList;
  for( PluginLoader::PluginList::Iterator it = list.begin(); it != list.end(); ++it ) {
    PluginCheckBox* cb = new PluginCheckBox( *it, top );
    lay->addWidget( cb );
    d->_boxes.append( cb );
  }

  lay->addStretch(10);
}

ConfigWidget::~ConfigWidget()
{
  delete d;
}

void ConfigWidget::apply()
{
  KConfig* config = KGlobal::config();
  config->setGroup( QString( "KMyMoneyPlugin/EnabledPlugin" ) );
  bool changes = false;

  for( QValueList<PluginCheckBox*>::Iterator it = d->_boxes.begin(); it != d->_boxes.end(); ++it ) {
    bool orig = (*it)->info->shouldLoad();
    bool load = (*it)->isChecked();
    if ( orig != load ) {
      changes = true;
      config->writeEntry( (*it)->info->name(), load );
      (*it)->info->setShouldLoad(load);
      if ( load ) {
        PluginLoader::instance()->loadPlugin( (*it)->info);
      }
      else {
        if ( (*it)->info->plugin() ) // Do not emit if we had trouble loading plugin.
          emit PluginLoader::instance()->unplug( (*it)->info);
      }
    }
  }
  emit PluginLoader::instance()->replug();
}

} // namespace

#include "pluginloader.moc"
