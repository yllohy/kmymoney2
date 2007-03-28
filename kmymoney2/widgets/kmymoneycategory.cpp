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






// -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF --
// -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF --
// -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF --
// -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF --
// -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF --
// -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF --
// -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF --
// -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF --
// -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF -- -- EOF --

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

#include <qapplication.h>
#include <qvbox.h>

#include <kmessagebox.h>


// #include "../dialogs/knewaccountdlg.h"
#include <kmymoney/mymoneyobjectcontainer.h>

kMyMoneyCategory::kMyMoneyCategory(QWidget *parent, const char *name, const KMyMoneyUtils::categoryTypeE /*categoryType*/)
  : KLineEdit(parent,name)
{
  m_inCreation = false;
  m_completion = new kMyMoneyAccountCompletion(this, 0);
  m_completion->hide();
  m_displayOnly = false;

  // show all but investment accounts
  MyMoneyObjectContainer objects;
  AccountSet set(&objects);
  set.addAccountType(MyMoneyAccount::Checkings);
  set.addAccountType(MyMoneyAccount::Savings);
  set.addAccountType(MyMoneyAccount::Cash);
  set.addAccountType(MyMoneyAccount::AssetLoan);
  set.addAccountType(MyMoneyAccount::CertificateDep);
  set.addAccountType(MyMoneyAccount::MoneyMarket);
  set.addAccountType(MyMoneyAccount::Asset);
  set.addAccountType(MyMoneyAccount::Currency);

  set.addAccountGroup(MyMoneyAccount::Asset);
  set.removeAccountType(MyMoneyAccount::Investment);
  set.removeAccountType(MyMoneyAccount::Stock);
  set.addAccountGroup(MyMoneyAccount::Liability);
  set.addAccountGroup(MyMoneyAccount::Income);
  set.addAccountGroup(MyMoneyAccount::Expense);

  set.load(m_completion->selector());

  connect(this, SIGNAL(textChanged(const QString&)), m_completion, SLOT(slotMakeCompletion(const QString&)));
  connect(m_completion, SIGNAL(itemSelected(const QCString&)), this, SLOT(slotSelectAccount(const QCString&)));
}

kMyMoneyCategory::~kMyMoneyCategory()
{
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
}

void kMyMoneyCategory::loadAccount(const QCString& id)
{
  blockSignals(true);
  m_id = QCString();
  m_displayOnly = false;
  if(!id.isEmpty()) {
    try {
      MyMoneyAccount acc = MyMoneyFile::instance()->account(id);
      setText(MyMoneyFile::instance()->accountToCategory(id));
      m_id = id;
      m_completion->setSelected(id);
    } catch(MyMoneyException *e) {
      qDebug("Account with id %s not found anymore", id.data());
      delete e;
    }
  } else {
    KLineEdit::setText(QString());
  }
  blockSignals(false);
}

void kMyMoneyCategory::loadText(const QString& text)
{
  m_displayOnly = true;
  m_id = QCString();
  blockSignals(true);
  KLineEdit::setText(text);
  blockSignals(false);
}

void kMyMoneyCategory::focusInEvent(QFocusEvent *ev)
{
  KLineEdit::focusInEvent(ev);
  emit signalFocusIn();
}

void kMyMoneyCategory::checkForNewCategory(void)
{
  bool newAccount = true;

  if(!text().isEmpty()) {
    if(!m_id.isEmpty()) {
      MyMoneyAccount acc = MyMoneyFile::instance()->account(m_id);
      QString txt = text();
      int pos = text().findRev(':');
      if(pos != -1) {
        txt = txt.mid(pos+1);
      }
      if(acc.name() == txt)
        newAccount = false;
    }
  } else {
    slotSelectAccount(QCString());
    newAccount = false;
  }

  if(newAccount && !m_displayOnly) {
    MyMoneyAccount acc;
    m_inCreation = true;

    if(KMessageBox::questionYesNo(this,
      QString("<p>")+i18n("The category <b>%1</b> currently does not exist. "
           "Do you want to create it?").arg(text()), i18n("Create category"),
      KStdGuiItem::yes(), KStdGuiItem::no(), "CreateNewCategories") == KMessageBox::Yes) {
        acc.setName(text());
        // FIXME maybe add some logic that notifies the user (and through the ML us)
        // that no slot is connected to this signal.
        emit newCategory(acc);

    }
    slotSelectAccount(acc.id());

    m_inCreation = false;
  }
}

void kMyMoneyCategory::focusOutEvent(QFocusEvent *ev)
{
  m_completion->hide();

  if(!m_inCreation)
    checkForNewCategory();

  if(!m_inCreation && !m_id.isEmpty()) {
    slotSelectAccount(m_id);
  }

  // now call base class
  KLineEdit::focusOutEvent(ev);

  // force update of hint
  if(text().isEmpty())
    repaint();
}

bool kMyMoneyCategory::eventFilter(QObject* o, QEvent* e)
{
  // filter out mouse wheel events here as they distract
  // the account completion logic
  // if(m_completion->isVisible() && (e->type() == QEvent::Wheel)) {
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

        case Qt::Key_Tab:
          break;
      }
    }
  }
  return rc;
}

void kMyMoneyCategory::slotSelectAccount(const QCString& id)
{
  if(!id.isEmpty())
    setText(MyMoneyFile::instance()->accountToCategory(id));
  else
    setText("");

  m_completion->hide();

  if(m_id != id) {
    m_id = id;
    emit categoryChanged(id);
  }
}

void kMyMoneyCategory::removeAccount(const QCString& id)
{
  m_completion->selector()->removeItem(id);
}

void kMyMoneyCategory::connectNotify(const char * /* signal */)
{
}

void kMyMoneyCategory::drawContents( QPainter *p)
{
  KLineEdit::drawContents(p);

  if(text().isEmpty() && !m_hint.isEmpty() && !hasFocus()) {
    const int innerMargin = 1;

    // the following 5 lines are taken from QLineEdit::drawContents()
    QRect cr = contentsRect();
    QFontMetrics fm = fontMetrics();
    QRect lineRect( cr.x() + innerMargin, cr.y() + (cr.height() - fm.height() + 1) / 2,
                    cr.width() - 2*innerMargin, fm.height() );
    QPoint topLeft = lineRect.topLeft() - QPoint(0, -fm.ascent());

    p->save();
    QFont f = p->font();
    f.setItalic(true);
    f.setWeight(QFont::Light);
    p->setFont(f);
    p->setPen(palette().disabled().text());

    p->drawText(topLeft, m_hint);

    p->restore();
  }
}

#include "kmymoneycategory.moc"
