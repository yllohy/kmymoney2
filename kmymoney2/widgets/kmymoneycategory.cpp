/***************************************************************************
                          kmymoneycategory.cpp  -  description
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

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneycategory.h"
#include "../mymoney/mymoneyfile.h"
#include "../widgets/kmymoneyaccountcompletion.h"

kMyMoneyCategory::kMyMoneyCategory(QWidget *parent, const char *name, const KMyMoneyUtils::categoryTypeE categoryType)
  : KLineEdit(parent,name)
{
#ifndef OWN_COMPLETION  
  // make sure, the completion object exists
  if(compObj() == 0)
    completionObject();

  compObj()->setOrder(KCompletion::Sorted);
  setCompletionMode(KGlobalSettings::CompletionPopupAuto);

  // set the standard value for substring completion, as we
  // fake that with every key entered
  setKeyBinding(SubstringCompletion, KShortcut("Ctrl+T"));

  loadList(categoryType);
#else
/*
  m_accountFrame = new QVBox(0,0,WType_Popup);
  m_accountFrame->setFrameStyle(QFrame::Box | QFrame::Plain);
  m_accountFrame->setLineWidth(1);
  m_accountFrame->hide();
  m_accountSelector = new kMyMoneyAccountCompletion(m_accountFrame, 0);
*/
  m_accountSelector = new kMyMoneyAccountCompletion(this, 0);
  m_accountSelector->hide();
  connect(this, SIGNAL(textChanged(const QString&)), m_accountSelector, SLOT(slotMakeCompletion(const QString&)));
  connect(m_accountSelector, SIGNAL(accountSelected(const QCString&)), this, SLOT(slotSelectAccount(const QCString&)));

#endif
}

kMyMoneyCategory::~kMyMoneyCategory()
{
}

void kMyMoneyCategory::toggleAccountSelector(void)
{
#if 0
  int w = m_accountFrame->sizeHint().width();
  int h = m_accountFrame->sizeHint().height();
  if(w < width())
    w = width();

  if(m_accountFrame->isVisible()) {
    qDebug("account selector hide");
    m_accountFrame->hide();
  } else {
    qDebug("account selector show");
    
    QPoint tmpPoint = mapToGlobal(QPoint(width(),height()));

    // usually, the datepicker widget is shown underneath the dateEdit widget
    // if it does not fit on the screen, we show it above this widget

    if(tmpPoint.y() + h > QApplication::desktop()->height()) {
      tmpPoint.setY(tmpPoint.y() - h - height());
    }

#if 0
    if (m_qtalignment == Qt::AlignRight) {
      m_accountFrame->setGeometry(tmpPoint.x(), tmpPoint.y(), w, h);
    } else {
      tmpPoint.setX(tmpPoint.x() - w);
      m_accountFrame->setGeometry(tmpPoint.x(), tmpPoint.y(), w, h);
    }
#else
    tmpPoint.setX(tmpPoint.x() - w);
    m_accountFrame->setGeometry(tmpPoint.x(), tmpPoint.y(), w, h);
#endif
    QSize hint = m_accountFrame->sizeHint();
    qDebug("selector size hint is (%d,%d)", hint.width(), hint.height());
    m_accountSelector->loadList((KMyMoneyUtils::categoryTypeE) 0x0f);
    m_accountFrame->show();
  }
#endif

  if(m_accountSelector->isVisible()) {
    qDebug("account selector hide");
    m_accountSelector->hide();
  } else {
    qDebug("account selector show");
#if 0
    QPoint tmpPoint = mapToGlobal(QPoint(width(),height()));

    // usually, the datepicker widget is shown underneath the dateEdit widget
    // if it does not fit on the screen, we show it above this widget

    if(tmpPoint.y() + h > QApplication::desktop()->height()) {
      tmpPoint.setY(tmpPoint.y() - h - height());
    }

    tmpPoint.setX(tmpPoint.x() - w);
    m_accountSelector->setGeometry(tmpPoint.x(), tmpPoint.y(), w, h);
    QSize hint = m_accountSelector->sizeHint();
    qDebug("selector size hint is (%d,%d)", hint.width(), hint.height());
#endif
    m_accountSelector->show();
  }

}

void kMyMoneyCategory::keyPressEvent( QKeyEvent * ev)
{
  bool oldColon = text().find(':');
  
  KLineEdit::keyPressEvent(ev);
  
  if(ev->isAccepted()) {
    // check if the name contains one or more colons. We
    // wipe out the stuff to the left of the right most colon
    // to see only the last part of the category/account hierarchy,
    // but only if the colon was there before. Otherwise, we just
    // wipe out the colon.
    int pos = text().findRev(':');
    if(pos != -1 && oldColon) {
      setText(text().mid(pos+1));
      
    } else if(pos != -1) {
      // it was just entered, so we take it away again ;-)
      setText(text().left(pos-1)+text().mid(pos+1));
    }
  }
#ifndef OWN_COMPLETION
  if(ev->isAccepted()) {
    // if the key was accepted by KLineEdit, we fake a substring completion
    // which we set previously to Ctrl+T.
    QKeyEvent evc(QEvent::KeyPress, Qt::Key_T, 0, Qt::ControlButton);
    KLineEdit::keyPressEvent(&evc);
  }
#endif
}

void kMyMoneyCategory::loadText(const QString& text)
{
  m_text = text;
  setText(text);
}

void kMyMoneyCategory::resetText(void)
{
  setText(m_text);
}


void kMyMoneyCategory::addCategories(QStringList& strList, const QCString& id, const QString& leadIn)
{
  MyMoneyFile *file = MyMoneyFile::instance();
  QString name;

  MyMoneyAccount account = file->account(id);

  QCStringList accList = account.accountList();
  QCStringList::ConstIterator it_a;

  for(it_a = accList.begin(); it_a != accList.end(); ++it_a) {
    account = file->account(*it_a);
    strList << leadIn + account.name();
    addCategories(strList, *it_a, leadIn + account.name() + ":");
  }
}

void kMyMoneyCategory::loadList(const KMyMoneyUtils::categoryTypeE type)
{
#ifndef OWN_COMPLETION
  QStringList strList;

  try {
    MyMoneyFile *file = MyMoneyFile::instance();

    // read all account items from the MyMoneyFile objects and add them to the listbox
    m_accountList = file->accountList();

    if(type & KMyMoneyUtils::liability)
      addCategories(strList, file->liability().id(), "");

    if(type & KMyMoneyUtils::asset)
      addCategories(strList, file->asset().id(), "");

    if(type & KMyMoneyUtils::expense)
      addCategories(strList, file->expense().id(), "");

    if(type & KMyMoneyUtils::income)
      addCategories(strList, file->income().id(), "");

  } catch (MyMoneyException *e) {
    qDebug("Exception '%s' thrown in %s, line %ld caught in kMyMoneyCategory::loadList",
      e->what().latin1(), e->file().latin1(), e->line());
    delete e;
  }

  // construct the list of completion items
  compObj()->setItems(strList);
  compObj()->setIgnoreCase(true);
#endif
}

void kMyMoneyCategory::focusInEvent(QFocusEvent *ev)
{
  KLineEdit::focusInEvent(ev);
  emit signalFocusIn();
}

void kMyMoneyCategory::focusOutEvent(QFocusEvent *ev)
{
  m_accountSelector->hide();
#ifndef OWN_COMPLETION
  // if the current text is not in the list of
  // possible completions, we have a new category
  // and signal that to the outside world.
  if(!text().isEmpty() && compObj()->items().contains(text()) == 0) {
    emit newCategory(text());
  }
#endif

  if(!(text().isEmpty() && m_text.isEmpty())
  && text() != m_text) {
    emit categoryChanged(text());
  }
  // now call base class
  KLineEdit::focusOutEvent(ev);
}

bool kMyMoneyCategory::eventFilter(QObject* o, QEvent* e)
{
  // filter out mouse wheel events here as they distract
  // the account completion logic
  // if(m_accountSelector->isVisible() && (e->type() == QEvent::Wheel)) {
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

      }
    }
  }
  return rc;
}

void kMyMoneyCategory::slotSelectAccount(const QCString& id)
{
  setText(MyMoneyFile::instance()->accountToCategory(id));
  m_accountSelector->hide();
}
