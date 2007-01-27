/***************************************************************************
                          registersearchline.h
                             -------------------
    begin                : Sun Jan 14 2006
    copyright            : (C) 2006 by Thomas Baumgart
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

#ifndef REGISTERSEARCHLINE_H
#define REGISTERSEARCHLINE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qhbox.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <klineedit.h>

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoney/register.h>
#include <kmymoney/export.h>

namespace KMyMoneyRegister {

/**
  * This class makes it easy to add a search line for filtering the items
  * in a register based on simple text.  Inspired by the idea of the kdelibs
  * class KListViewSearchLine.
  *
  * @author Thomas Baumgart
  */
class KMYMONEY_EXPORT RegisterSearchLine : public KLineEdit
{
  Q_OBJECT
public:
  /**
    * Constructs a RegisterSearchLine with @a reg being the register to be
    * filtered.
    *
    * If @a reg is null then the widget will be disabled until a register
    * is set with setRegister().
    */
  RegisterSearchLine(QWidget* parent = 0, Register* reg = 0, const char* name = 0);

  /**
    * Constructs a RegisterSearchLine
    *
    * The widget will be disabled until a register is set with setRegister().
    */
  RegisterSearchLine(QWidget* parent = 0, const char* name = 0);

  /**
    * Destroys the object
    */
  ~RegisterSearchLine();

  /**
    * Sets the KMyMoneyRegister that is filtered by this search line.
    * If @a reg is null then the widget will be disabled.
    *
    * @see KMyMoneyRegister()
    */
  void setRegister(Register* reg);

protected:
  virtual bool itemMatches(const RegisterItem* item, const QString& s) const;

public slots:
  virtual void updateSearch(const QString& s = QString::null);
  virtual void reset(void);

protected slots:
  void queueSearch(const QString& search);
  void activateSearch(void);
  void slotStatusChanged(int);

private slots:
  void itemAdded(RegisterItem* item) const;
  void registerDestroyed(void);

private:
  void init(Register* reg);

private:
  class RegisterSearchLinePrivate;
  RegisterSearchLinePrivate* d;
};

/**
  * Creates a widget containing a RegisterSearchLine, a label with the text
  * "Search" and a button to clear the search. Modelled after KListViewSearchLineWidget.
  *
  * @author Thomas Baumgart
  */
class KMYMONEY_EXPORT RegisterSearchLineWidget : public QHBox
{
  Q_OBJECT
public:
  /**
    * Creates a RegisterSearchLineWidget for @a reg with @a parent as the
    * parent and with @a name.
    */
  RegisterSearchLineWidget(Register* reg = 0, QWidget* parent = 0, const char* name = 0);

  /**
    * Destroys the object
    */
  ~RegisterSearchLineWidget();

  /**
    * Returns a pointer to the searchline
    */
  RegisterSearchLine* searchLine() const;

  /**
    * Creates the search line.  This can be useful to reimplement in cases where
    * a RegisterSearchLine subclass is used.
    */
  virtual RegisterSearchLine* createSearchLine(Register* reg);

protected slots:
  /**
    * Creates the widgets inside of the widget.  This is called from the
    * constructor via a single shot timer so that it it guaranteed to run
    * after construction is complete.  This makes it suitable for overriding in
    * subclasses.
    */
  virtual void createWidgets(void);

private slots:
  void positionInToolBar(void);

private:
  class RegisterSearchLineWidgetPrivate;
  RegisterSearchLineWidgetPrivate* d;
};

} // namespace

#endif
