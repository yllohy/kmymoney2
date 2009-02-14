/***************************************************************************
                          pluginloader.h
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

#ifndef PLUGINLOADER_H
#define PLUGINLOADER_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// ----------------------------------------------------------------------------
// QT Includes

#include <qwidget.h>
#include <qscrollview.h>
// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoney/export.h>

namespace KMyMoneyPlugin
{
  class Plugin;
  class ConfigWidget;

  class KMYMONEY_EXPORT PluginLoader : public QObject
  {
    Q_OBJECT
  public:
    class KMYMONEY_EXPORT Info
    {
    public:
      Info( const QString& name, const QString& comment, const QString& library, bool shouldLoad );
      ~Info();
      const QString& name() const;

      const QString& comment() const;

      const QString& library() const;

      Plugin* plugin() const;
      void setPlugin(Plugin*);

      bool shouldLoad() const;
      void setShouldLoad(bool);

    private:
      struct Private;
      Private* d;
    };

    PluginLoader(QObject* parent);
    virtual ~PluginLoader();
    void loadPlugins();
    static PluginLoader* instance();
    ConfigWidget* configWidget(QWidget* parent);

    typedef QValueList<Info*> PluginList;

    const PluginList& pluginList();

    void loadPlugin( Info* );

  signals:
    void plug(KMyMoneyPlugin::PluginLoader::Info*);
    void unplug(KMyMoneyPlugin::PluginLoader::Info*);
    void replug();

  private:
    friend class ConfigWidget;
    friend class PluginCheckBox;

    struct Private;
    Private* d;
  };

  class KMYMONEY_EXPORT ConfigWidget : public QScrollView
  {
    Q_OBJECT
  public:
    ConfigWidget( QWidget* parent );
    ~ConfigWidget();
  public slots:
    void apply();
  private:
    struct Private;
    Private* d;
  };
}

#endif /* PLUGINLOADER_H */
