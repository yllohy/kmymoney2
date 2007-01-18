/***************************************************************************
                          registersearchline.cpp
                             -------------------
    copyright            : (C) 2006 by Thomas Baumgart
    email                : ipwizard@users.sourceforge.net
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

#include <qapplication.h>
#include <qlabel.h>
#include <qtoolbutton.h>
#include <qtimer.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <ktoolbar.h>
#include <ktoolbarbutton.h>
#include <kiconloader.h>
#include <klocale.h>

// ----------------------------------------------------------------------------
// Project Includes

#include <registersearchline.h>

using namespace KMyMoneyRegister;

class RegisterSearchLine::RegisterSearchLinePrivate
{
public:
  RegisterSearchLinePrivate() :
    reg(0),
    queuedSearches(0) {}

  Register* reg;
  QString search;
  int queuedSearches;
};

RegisterSearchLine::RegisterSearchLine(QWidget* parent, Register* reg, const char* name) :
  KLineEdit(parent, name)
{
  d = new RegisterSearchLinePrivate;
  d->reg = reg;
  connect(this, SIGNAL(textChanged(const QString&)), this, SLOT(queueSearch(const QString&)));

  if(reg) {
    connect(reg, SIGNAL(destroyed()), this, SLOT(registerDestroyed()));
    connect(reg, SIGNAL(itemAdded(RegisterItem*)), this, SLOT(itemAdded(RegisterItem*)));
  } else {
    setEnabled(false);
  }
}

RegisterSearchLine::RegisterSearchLine(QWidget* parent, const char* name) :
    KLineEdit(parent, name)
{
  d = new RegisterSearchLinePrivate;
  setEnabled(false);
}

RegisterSearchLine::~RegisterSearchLine()
{
  delete d;
}

void RegisterSearchLine::setRegister(Register* reg)
{
  if(d->reg) {
    disconnect(d->reg, SIGNAL(destroyed()), this, SLOT(registerDestroyed()));
    disconnect(d->reg, SIGNAL(itemAdded(RegisterItem*)), this, SLOT(itemAdded(RegisterItem*)));
  }

  d->reg = reg;

  if(reg) {
    connect(reg, SIGNAL(destroyed()), this, SLOT(registerDestroyed()));
    connect(reg, SIGNAL(itemAdded(RegisterItem*)), this, SLOT(itemAdded(RegisterItem*)));
  }

  setEnabled(reg != 0);
}

void RegisterSearchLine::queueSearch(const QString& search)
{
  d->queuedSearches++;
  d->search = search;
  QTimer::singleShot(200, this, SLOT(activateSearch()));
}

void RegisterSearchLine::activateSearch(void)
{
  --(d->queuedSearches);
  if(d->queuedSearches == 0)
    updateSearch(d->search);
}

void RegisterSearchLine::updateSearch(const QString& s)
{
  if(!d->reg)
    return;

  d->search = s.isNull() ? text() : s;

  // keep track of the current focus item
  RegisterItem* focusItem = d->reg->focusItem();

  bool enabled = d->reg->isUpdatesEnabled();
  d->reg->setUpdatesEnabled(false);

  RegisterItem* p = d->reg->firstItem();
  for(; p; p = p->nextItem()) {
    p->setVisible(p->matches(s));
  }
  d->reg->suppressAdjacentMarkers();
  d->reg->updateAlternate();
  d->reg->setUpdatesEnabled(enabled);

  // if focus item is still visible, then make sure we have
  // it on screen
  if(focusItem && focusItem->isVisible()) {
    d->reg->ensureItemVisible(focusItem);
  } else
    d->reg->updateContents();
}

bool RegisterSearchLine::itemMatches(const RegisterItem* item, const QString& s) const
{
  return item->matches(s);
}

void RegisterSearchLine::itemAdded(RegisterItem* item) const
{
  item->setVisible(itemMatches(item, text()));
}

void RegisterSearchLine::registerDestroyed(void)
{
  d->reg = 0;
  setEnabled(false);
}


class RegisterSearchLineWidget::RegisterSearchLineWidgetPrivate
{
  public:
  RegisterSearchLineWidgetPrivate() :
    reg(0),
    searchLine(0),
    clearButton(0) {}

  Register* reg;
  RegisterSearchLine* searchLine;
  QToolButton* clearButton;
};


RegisterSearchLineWidget::RegisterSearchLineWidget(Register* reg, QWidget* parent, const char* name) :
  QHBox(parent, name)
{
  d = new RegisterSearchLineWidgetPrivate;
  d->reg = reg;
  setSpacing(5);
  QTimer::singleShot(0, this, SLOT(createWidgets()));
}

RegisterSearchLineWidget::~RegisterSearchLineWidget()
{
  delete d;
}

RegisterSearchLine* RegisterSearchLineWidget::createSearchLine(Register* reg)
{
  if(!d->searchLine)
    d->searchLine = new RegisterSearchLine(this, reg);
  return d->searchLine;
}

void RegisterSearchLineWidget::createWidgets(void)
{
  // positionInToolBar();
  if(!d->clearButton) {
    d->clearButton = new QToolButton(this);
    QIconSet icon = SmallIconSet(QApplication::reverseLayout() ? "clear_left" : "locationbar_erase");
    d->clearButton->setIconSet(icon);
  }

  d->clearButton->show();

  QLabel *label = new QLabel(i18n("S&earch:"), this, "kde toolbar widget");

  d->searchLine = createSearchLine(d->reg);
  d->searchLine->show();

  label->setBuddy(d->searchLine);
  label->show();

  connect(d->clearButton, SIGNAL(clicked()), d->searchLine, SLOT(clear()));
}


RegisterSearchLine* RegisterSearchLineWidget::searchLine(void) const
{
  return d->searchLine;
}

void RegisterSearchLineWidget::positionInToolBar(void)
{
  KToolBar *toolBar = dynamic_cast<KToolBar *>(parent());

  if(toolBar) {

    // Here we have The Big Ugly.  Figure out how many widgets are in the
    // and do a hack-ish iteration over them to find this widget so that we
    // can insert the clear button before it.

    int widgetCount = toolBar->count();

    for(int index = 0; index < widgetCount; index++) {
      int id = toolBar->idAt(index);
      if(toolBar->getWidget(id) == this) {
        toolBar->setItemAutoSized(id);
        if(!d->clearButton) {
          QString icon = QApplication::reverseLayout() ? "clear_left" : "locationbar_erase";
          d->clearButton = new KToolBarButton(icon, 2005, toolBar);
        }
        toolBar->insertWidget(2005, d->clearButton->width(), d->clearButton, index+1);
        break;
      }
    }
  }

  if(d->searchLine)
    d->searchLine->show();
}

#include "registersearchline.moc"
