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

#include <klocale.h>
#include <kmessagebox.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneycategory.h"
#include "../mymoney/mymoneyfile.h"
#include "../widgets/kmymoneyaccountcompletion.h"
#include "../dialogs/knewaccountdlg.h"

kMyMoneyCategory::kMyMoneyCategory(QWidget *parent, const char *name, const KMyMoneyUtils::categoryTypeE categoryType)
  : KLineEdit(parent,name)
{
  m_inCreation = false;
  m_accountSelector = new kMyMoneyAccountCompletion(this, 0);
  m_accountSelector->hide();
  m_displayOnly = false;

  // show all but investment accounts
  QValueList<int> typeList;
  typeList << MyMoneyAccount::Checkings;
  typeList << MyMoneyAccount::Savings;
  typeList << MyMoneyAccount::Cash;
  typeList << MyMoneyAccount::AssetLoan;
  typeList << MyMoneyAccount::CertificateDep;
  // typeList << MyMoneyAccount::Investment;
  typeList << MyMoneyAccount::MoneyMarket;
  typeList << MyMoneyAccount::Asset;
  typeList << MyMoneyAccount::Currency;
  typeList << MyMoneyAccount::CreditCard;
  typeList << MyMoneyAccount::Loan;
  typeList << MyMoneyAccount::Liability;
  typeList << MyMoneyAccount::Income;
  typeList << MyMoneyAccount::Expense;
  m_accountSelector->loadList(typeList);

  connect(this, SIGNAL(textChanged(const QString&)), m_accountSelector, SLOT(slotMakeCompletion(const QString&)));
  connect(m_accountSelector, SIGNAL(itemSelected(const QCString&)), this, SLOT(slotSelectAccount(const QCString&)));
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
  m_id = QCString();
  m_displayOnly = false;
  if(!id.isEmpty()) {
    try {
      MyMoneyAccount acc = MyMoneyFile::instance()->account(id);
      setText(MyMoneyFile::instance()->accountToCategory(id));
      m_id = id;
      m_accountSelector->setSelected(id);
    } catch(MyMoneyException *e) {
      qDebug("Account with id %s not found anymore", id.data());
      delete e;
    }
  }
}

void kMyMoneyCategory::loadText(const QString& text)
{
  m_displayOnly = true;
  m_id = QCString();
  KLineEdit::setText(text);
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
    m_inCreation = true;

    if(KMessageBox::questionYesNo(this,
          QString("<p>")+i18n("The category <b>%1</b> currently does not exist. "
                "Do you want to create it?").arg(text()), i18n("Create category"),
                KStdGuiItem::yes(), KStdGuiItem::no(), "CreateNewCategories") == KMessageBox::Yes) {
      MyMoneyAccount acc;
      int rc;
      acc.setName(text());

      KNewAccountDlg dlg(acc, false, true);
      rc = dlg.exec();
      if(rc == QDialog::Accepted) {
        try {
          MyMoneyAccount parentAccount;
          acc = dlg.account();
          parentAccount = dlg.parentAccount();
          MyMoneyFile::instance()->addAccount(acc, parentAccount);
          slotSelectAccount(acc.id());

        } catch(MyMoneyException *e) {
          KMessageBox::detailedSorry(0, i18n("Unable to add category"),
              (e->what() + " " + i18n("thrown in") + " " + e->file()+ ":%1").arg(e->line()));
          delete e;
          slotSelectAccount(QCString());
        }
      } else
        slotSelectAccount(QCString());

    } else
      slotSelectAccount(QCString());
    m_inCreation = false;
  }
}

void kMyMoneyCategory::focusOutEvent(QFocusEvent *ev)
{
  m_accountSelector->hide();

  if(!m_inCreation)
    checkForNewCategory();

  if(!m_inCreation && !m_id.isEmpty()) {
    slotSelectAccount(m_id);
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

  if(m_id != id) {
    m_id = id;
    emit categoryChanged(id);
  }
  m_accountSelector->hide();
}
