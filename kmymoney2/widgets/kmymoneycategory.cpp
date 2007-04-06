/***************************************************************************
                          kmymoneycategory.cpp  -  description
                             -------------------
    begin                : Mon Jul 10 2006
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

// ----------------------------------------------------------------------------
// QT Includes

#include <qrect.h>
#include <qpainter.h>
#include <qpalette.h>
#include <qlayout.h>
#include <qtimer.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <klocale.h>
#include <kpushbutton.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kguiitem.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneycategory.h"
#include <kmymoney/mymoneyfile.h>
#include "kmymoneyaccountcompletion.h"


KMyMoneyCategory::KMyMoneyCategory(QWidget* parent, const char * name, bool splitButton) :
  KMyMoneyCombo(true, parent, name),
  m_splitButton(0),
  m_frame(0)
{
  if(splitButton) {
    m_frame = new QFrame(0);
    m_frame->setFocusProxy(this);
    QHBoxLayout* layout = new QHBoxLayout(m_frame);
    // make sure not to use our own overridden version of reparent() here
    KMyMoneyCombo::reparent(m_frame, getWFlags() & ~WType_Mask, QPoint(0, 0), true);
    if(parent)
      m_frame->reparent(parent, QPoint(0, 0), true);

    // create button
    KGuiItem splitButtonItem("",
        QIconSet(KGlobal::iconLoader()->loadIcon("split_transaction", KIcon::Small,
        KIcon::SizeSmall)), "", "");
    m_splitButton = new KPushButton(splitButtonItem, m_frame, "splitButton");

    layout->addWidget(this, 5);
    layout->addWidget(m_splitButton);
  }

  m_completion = new kMyMoneyAccountCompletion(this, 0);
  connect(m_completion, SIGNAL(itemSelected(const QCString&)), this, SLOT(slotItemSelected(const QCString&)));
  connect(this, SIGNAL(textChanged(const QString&)), m_completion, SLOT(slotMakeCompletion(const QString&)));
}

KMyMoneyCategory::~KMyMoneyCategory()
{
  // make sure to wipe out the frame, button and layout
  if(m_frame && !m_frame->parentWidget())
    m_frame->deleteLater();
}

KPushButton* KMyMoneyCategory::splitButton(void) const
{
  return m_splitButton;
}

void KMyMoneyCategory::setPalette(const QPalette& palette)
{
  if(m_frame)
    m_frame->setPalette(palette);
  KMyMoneyCombo::setPalette(palette);
}

void KMyMoneyCategory::reparent(QWidget *parent, WFlags w, const QPoint& pos, bool showIt)
{
  if(m_frame)
    m_frame->reparent(parent, w, pos, showIt);
  else
    KMyMoneyCombo::reparent(parent, w, pos, showIt);
}

kMyMoneyAccountSelector* KMyMoneyCategory::selector(void) const
{
  return dynamic_cast<kMyMoneyAccountSelector*>(KMyMoneyCombo::selector());
}

void KMyMoneyCategory::setCurrentText(const QCString& id)
{
  if(!id.isEmpty())
    setCurrentText(MyMoneyFile::instance()->accountToCategory(id));
  else
    setCurrentText();
  setSuppressObjectCreation(false);
}

void KMyMoneyCategory::slotItemSelected(const QCString& id)
{
  setCurrentText(id);

  m_completion->hide();

  if(m_id != id) {
    m_id = id;
    emit itemSelected(id);
  }
}

void KMyMoneyCategory::focusInEvent(QFocusEvent *ev)
{
  KMyMoneyCombo::focusInEvent(ev);

  // make sure, we get a clean state before we automagically move the focus to
  // some other widget (like for 'split transaction'). We do this by delaying
  // the emission of the focusIn signal until the next run of the event loop.
  QTimer::singleShot(0, this, SIGNAL(focusIn()));
}

void KMyMoneyCategory::setSplitTransaction(void)
{
  setCurrentText(i18n("Split transaction (category replacement)", "Split transaction"));
  setSuppressObjectCreation(true);
}

bool KMyMoneyCategory::isSplitTransaction(void) const
{
  return currentText() == i18n("Split transaction (category replacement)", "Split transaction");
}

KMyMoneySecurity::KMyMoneySecurity(QWidget* parent, const char * name) :
  KMyMoneyCategory(parent, name, false)
{
}

KMyMoneySecurity::~KMyMoneySecurity()
{
}

void KMyMoneySecurity::setCurrentText(const QCString& id)
{
  if(!id.isEmpty())
    KMyMoneyCategory::setCurrentText(MyMoneyFile::instance()->account(id).name());
  else
    KMyMoneyCategory::setCurrentText();
}

#include "kmymoneycategory.moc"
