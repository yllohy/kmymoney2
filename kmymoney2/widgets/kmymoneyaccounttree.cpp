/***************************************************************************
                         kmymoneyaccounttree.cpp  -  description
                            -------------------
   begin                : Sat Jan 1 2005
   copyright            : (C) 2005 by Thomas Baumgart
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

#include <qpoint.h>
#include <qevent.h>
#include <qdragobject.h>
#include <qtimer.h>
#include <qcursor.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kmessagebox.h>
#include <klocale.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyaccounttree.h"
#include "../mymoney/mymoneyfile.h"

// FIXME we should really make kaccountlistitem a widget not a view (ipwizard)
#include "../views/kbanklistitem.h"

kMyMoneyAccountTree::kMyMoneyAccountTree(QWidget* parent, const char* name) :
  KListView(parent, name)
{
  setDragEnabled(true);
  setAcceptDrops(true);
  setItemsMovable(false);
  setDropVisualizer(false);
  setDropHighlighter(true);

  connect(this, SIGNAL(dropped(QDropEvent*,QListViewItem*,QListViewItem*)), this, SLOT(slotObjectDropped(QDropEvent*,QListViewItem*,QListViewItem*)));
}

bool kMyMoneyAccountTree::acceptDrag(QDropEvent* event) const
{
  bool rc;

  if(rc = (acceptDrops() && event->source() == viewport())) {
    QPoint vp = contentsToViewport(event->pos());
    QListViewItem *item = itemAt( vp );
    KAccountListItem* p = dynamic_cast<KAccountListItem*>(item);
    rc = false;
    if(p) {
      QCString id(event->encodedData("text/plain"));
      MyMoneyAccount accTo, accFrom;
      try {
        accTo = MyMoneyFile::instance()->account(p->accountID());
        accFrom = MyMoneyFile::instance()->account(id);
        // it does not make sense to reparent an account to oneself
        // or to reparent it to it's current parent
        if(accTo.id() != accFrom.id()
        && accFrom.parentAccountId() != accTo.id()) {

          rc = accTo.accountGroup() == accFrom.accountGroup();
          if(rc) {
            if(accTo.accountType() == MyMoneyAccount::Investment
            && accFrom.accountType() != MyMoneyAccount::Stock)
              rc = false;
            else if(accFrom.accountType() == MyMoneyAccount::Stock
            && accTo.accountType() != MyMoneyAccount::Investment)
              rc = false;
          } else {
            if(accFrom.accountGroup() == MyMoneyAccount::Income
            && accTo.accountGroup() == MyMoneyAccount::Expense)
              rc = true;

            if(accFrom.accountGroup() == MyMoneyAccount::Expense
            && accTo.accountGroup() == MyMoneyAccount::Income)
              rc = true;

          }
        }

      } catch(MyMoneyException *e) {
        delete e;
        try {
          MyMoneyFile::instance()->institution(p->accountID());
          rc = true;
        } catch(MyMoneyException *e) {
          delete e;
          rc = p->accountID().isEmpty();
        }
      }

    }
  }

  // QWidget::setCursor(rc ? Qt::pointingHandCursor : Qt::forbiddenCursor);

  return rc;
}

void kMyMoneyAccountTree::startDrag()
{
  QListViewItem* item = currentItem();
  KAccountListItem* p = dynamic_cast<KAccountListItem*>(item);
  if(!p)
    return;

  try {
    MyMoneyAccount acc = MyMoneyFile::instance()->account(p->accountID());
    QTextDrag* drag = new QTextDrag(acc.id(), viewport());
    drag->setSubtype("plain");

    if (drag->dragMove() && drag->target() != viewport())
      emit moved();

  } catch(MyMoneyException *e) {
    // we end up here if the item is an institution also.
    // drag and drop for institutions is not wanted
    delete e;
  }
  return;
}

void kMyMoneyAccountTree::slotObjectDropped(QDropEvent* event, QListViewItem* parent, QListViewItem* after)
{
  KAccountListItem* newParent;

  if(after)
    newParent = dynamic_cast<KAccountListItem*>(after);
  else
    newParent = dynamic_cast<KAccountListItem*>(parent);

  QCString id(event->encodedData("text/plain"));
  try {
    MyMoneyAccount accTo, accFrom;
    accTo = MyMoneyFile::instance()->account(newParent->accountID());
    accFrom = MyMoneyFile::instance()->account(id);
    if(KMessageBox::questionYesNo(this, QString("<p>")+i18n("Do you really want to move <b>%1</b> to be a sub-account of <b>%2</b>?").arg(accFrom.name()).arg(accTo.name()), i18n("Moving account")) == KMessageBox::Yes) {
      try {
        MyMoneyFile::instance()->reparentAccount(accFrom, accTo);
      } catch(MyMoneyException *e) {
        QString detail = i18n("%1 caught in %2 at line %3").arg(e->what()).arg(e->file()).arg(e->line());
        KMessageBox::detailedError(this, i18n("Cannot move account"), detail, i18n("Error"));
      }
    }
  } catch(MyMoneyException *e) {
    delete e;
    // might have been a bank that we dropped on. let's check
    try {
      MyMoneyInstitution institution = MyMoneyFile::instance()->institution(newParent->accountID());
      MyMoneyAccount acc = MyMoneyFile::instance()->account(id);
      acc.setInstitutionId(institution.id());
      try {
        MyMoneyFile::instance()->modifyAccount(acc);
      } catch(MyMoneyException *e) {
        QString detail = i18n("%1 caught in %2 at line %3").arg(e->what()).arg(e->file()).arg(e->line());
        KMessageBox::detailedError(this, i18n("Cannot move account to institution"), detail, i18n("Error"));
      }

    } catch(MyMoneyException *e) {
      delete e;
      // we also end up here, if the id of the new parent is empty.
      // the only case this happens is that we will be removed from the institution
      MyMoneyAccount acc = MyMoneyFile::instance()->account(id);
      acc.setInstitutionId(QCString());
      try {
        MyMoneyFile::instance()->modifyAccount(acc);
      } catch(MyMoneyException *e) {
        QString detail = i18n("%1 caught in %2 at line %3").arg(e->what()).arg(e->file()).arg(e->line());
        KMessageBox::detailedError(this, i18n("Cannot remove account from institution"), detail, i18n("Error"));
      }
    }
  }
}
