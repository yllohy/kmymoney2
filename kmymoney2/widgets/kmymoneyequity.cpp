/***************************************************************************
                          kmymoneyequity.cpp  -  description
                             -------------------
    begin                : Sun Aug 11 2002
    copyright            : (C) 2000-2002 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
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
#include <qvbox.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <klocale.h>
#include <kmessagebox.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyequity.h"
#include "../mymoney/mymoneyfile.h"
#include "../widgets/kmymoneyequitycompletion.h"
#include "../dialogs/knewequityentrydlg.h"

kMyMoneyEquity::kMyMoneyEquity(QWidget *parent, const char *name)
  : KLineEdit(parent,name)
{
  m_inCreation = false;
  m_equitySelector = new kMyMoneyEquityCompletion(this, 0);
  m_equitySelector->hide();
  connect(this, SIGNAL(textChanged(const QString&)), m_equitySelector, SLOT(slotMakeCompletion(const QString&)));
  connect(m_equitySelector, SIGNAL(itemSelected(const QCString&)), this, SLOT(slotSelectEquity(const QCString&)));
}

kMyMoneyEquity::~kMyMoneyEquity()
{
}

void kMyMoneyEquity::keyPressEvent( QKeyEvent * ev)
{
  KLineEdit::keyPressEvent(ev);
}

void kMyMoneyEquity::loadEquity(const QCString& id)
{
  try {
    MyMoneyEquity equity = MyMoneyFile::instance()->equity(id);
    setText(equity.tradingSymbol());
    m_id = id;
    m_equitySelector->setSelected(id);
  } catch(MyMoneyException *e) {
    qDebug("Equity with id %s not found anymore", id.data());
    delete e;
  }
}

void kMyMoneyEquity::focusInEvent(QFocusEvent *ev)
{
  KLineEdit::focusInEvent(ev);
  emit signalFocusIn();
}

void kMyMoneyEquity::focusOutEvent(QFocusEvent *ev)
{
  m_equitySelector->hide();
  if(!m_inCreation && !m_id.isEmpty()) {
    slotSelectEquity(m_id);
  }

  // now call base class
  KLineEdit::focusOutEvent(ev);
}

bool kMyMoneyEquity::eventFilter(QObject* o, QEvent* e)
{
  // filter out mouse wheel events here as they distract
  // the account completion logic
  // if(m_equitySelector->isVisible() && (e->type() == QEvent::Wheel)) {
  if(e->type() == QEvent::Wheel) {
    qDebug("Eat wheel event");
    QWheelEvent *w = static_cast<QWheelEvent *> (e);
    w->accept();
    return true;
  }

  bool rc = KLineEdit::eventFilter(o, e);

  if(rc == false) {
    if(e->type() == QEvent::KeyPress) {
      QKeyEvent *k = static_cast<QKeyEvent *> (e);
      bool newEquity = true;
      switch(k->key()) {
        case Qt::Key_Return:
        case Qt::Key_Enter:
          emit signalEnter();
          rc = true;
          break;

        case Qt::Key_Escape:
          emit signalEsc();
          rc = true;
          break;

        case Qt::Key_Tab:
          if(!m_id.isEmpty()) {
            MyMoneyEquity equity = MyMoneyFile::instance()->equity(m_id);
            if(equity.tradingSymbol() == text())
              newEquity = false;
          }
          if(newEquity) {
            m_inCreation = true;

            if(KMessageBox::questionYesNo(this,
                  i18n("The equity \"%1\" currently does not exist. "
                       "Do you want to create it?").arg(text())) == KMessageBox::Yes) {


              KNewEquityEntryDlg *pDlg = new KNewEquityEntryDlg(this, 0);
              pDlg->setSymbolName(text());
              rc = pDlg->exec();
              if(rc == QDialog::Accepted) {
                //create the new Equity object, and assign an ID.
                MyMoneyEquity newEquity;
                //fill in the fields.
                newEquity.setTradingSymbol(pDlg->symbolName());
                newEquity.setName(pDlg->name());
                try {
                  MyMoneyFile::instance()->addEquity(newEquity);
                  slotSelectEquity(newEquity.id());
                } catch(MyMoneyException *e) {
                  qWarning("Cannot add equity %s to storage", newEquity.name().data());
                  delete e;
                  rc = QDialog::Rejected;
                }
              }

              if(rc != QDialog::Accepted) {
                rc = true;
              }
            } else {
              rc = true;
            }

            m_inCreation = false;
          }
          break;
      }
    }
  }
  return rc;
}

void kMyMoneyEquity::slotSelectEquity(const QCString& id)
{
  KLineEdit::setText(MyMoneyFile::instance()->equity(id).tradingSymbol());
  if(m_id != id) {
    emit equityChanged(id);
    m_id = id;
  }
  m_equitySelector->hide();
}
