/***************************************************************************
                          kcategoriesview.cpp  -  description
                             -------------------
    begin                : Sun Jan 20 2002
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
#include <klistview.h>
#include <klocale.h>
#include <kglobal.h>
#if QT_VERSION > 300
#include <kstandarddirs.h>
#else
#include <kstddirs.h>
#endif

#include <kmessagebox.h>
#include <kconfig.h>

#include <qheader.h>
#include <qpushbutton.h>
#include <qlabel.h>

#include "kcategoriesview.h"
#include "../dialogs/kcategorylistitem.h"
//#include "../dialogs/knewcategorydlg.h"
#include "../views/kmymoneyfile.h"
#include "../dialogs/knewaccountdlg.h"

KCategoriesView::KCategoriesView(QWidget *parent, const char *name )
  : kCategoriesViewDecl(parent,name)
{
  categoryListView->setRootIsDecorated(true);
  categoryListView->addColumn(i18n("Category"));
  categoryListView->addColumn(i18n("Type"));
  categoryListView->setMultiSelection(false);
  categoryListView->setColumnWidthMode(0, QListView::Manual);
  categoryListView->header()->setResizeEnabled(false);
  categoryListView->setAllColumnsShowFocus(true);

  // never show a horizontal scroll bar
  categoryListView->setHScrollBarMode(QScrollView::AlwaysOff);

  categoryListView->setSorting(-1);

  readConfig();

  refresh();
	
	connect(categoryListView, SIGNAL(selectionChanged(QListViewItem*)),
	  this, SLOT(slotSelectionChanged(QListViewItem*)));
	connect(buttonEdit, SIGNAL(clicked()), this, SLOT(slotEditClicked()));
	connect(buttonNew, SIGNAL(clicked()), this, SLOT(slotNewClicked()));
  connect(buttonDelete, SIGNAL(clicked()), this, SLOT(slotDeleteClicked()));
}

KCategoriesView::~KCategoriesView()
{
  writeConfig();
}

void KCategoriesView::refresh(void)
{
  KConfig *config = KGlobal::config();
  config->setGroup("List Options");
  QFont defaultFont = QFont("helvetica", 12);
  categoryListView->header()->setFont(config->readFontEntry("listHeaderFont", &defaultFont));

  categoryListView->clear();

  MyMoneyFile *file = KMyMoneyFile::instance()->file();

  try
  {
    MyMoneyAccount expenseAccount = file->expense();
    MyMoneyAccount incomeAccount = file->income();

    // Income
    KCategoryListItem *incomeTopLevelAccount = new KCategoryListItem(categoryListView,
                      incomeAccount.name(), incomeAccount.id(), i18n("Income"));

    for ( QCStringList::ConstIterator it = file->income().accountList().begin();
          it != file->income().accountList().end();
          ++it )
    {
      KCategoryListItem *accountItem = new KCategoryListItem(incomeTopLevelAccount,
          file->account(*it).name(), file->account(*it).id(), i18n("Income"));

      QCStringList subAccounts = file->account(*it).accountList();
      if (subAccounts.count() >= 1)
      {
        showSubAccounts(subAccounts, accountItem, file, i18n("Income"));
      }
    }

    // Expense
    KCategoryListItem *expenseTopLevelAccount = new KCategoryListItem(categoryListView,
                      expenseAccount.name(), expenseAccount.id(), i18n("Expense"));

    for ( QCStringList::ConstIterator it = file->expense().accountList().begin();
          it != file->expense().accountList().end();
          ++it )
    {
      KCategoryListItem *accountItem = new KCategoryListItem(expenseTopLevelAccount,
          file->account(*it).name(), file->account(*it).id(), i18n("Expense"));

      QCStringList subAccounts = file->account(*it).accountList();
      if (subAccounts.count() >= 1)
      {
        showSubAccounts(subAccounts, accountItem, file, i18n("Expense"));
      }
    }

    categoryListView->setOpen(incomeTopLevelAccount, true);
    categoryListView->setOpen(expenseTopLevelAccount, true);
  }
  catch (MyMoneyException *e)
  {
    qDebug("Exception in assets account refresh: %s", e->what().latin1());
    delete e;
  }
}

void KCategoriesView::showSubAccounts(QCStringList accounts, KCategoryListItem *parentItem, MyMoneyFile *file,
  const QString& typeName)
{
  for ( QCStringList::ConstIterator it = accounts.begin(); it != accounts.end(); ++it )
  {
    KCategoryListItem *accountItem  = new KCategoryListItem(parentItem,
          file->account(*it).name(), file->account(*it).id(), typeName);

    QCStringList subAccounts = file->account(*it).accountList();
    if (subAccounts.count() >= 1)
    {
      showSubAccounts(subAccounts, accountItem, file, typeName);
    }
  }
}

void KCategoriesView::show()
{
  refresh();
  emit signalViewActivated();
  QWidget::show();
}

void KCategoriesView::resizeEvent(QResizeEvent* e)
{
  categoryListView->setColumnWidth(0, categoryListView->width()-105);
  categoryListView->setColumnWidth(1, 100);

  // call base class resizeEvent()
  kCategoriesViewDecl::resizeEvent(e);
}

void KCategoriesView::slotNewClicked()
{
  MyMoneyAccount account;

  KNewAccountDlg dialog(account, false, true, this, "hi", i18n("Create A New Category"));

  if (dialog.exec())
  {
    try
    {
      MyMoneyFile* file = KMyMoneyFile::instance()->file();

      MyMoneyAccount newAccount = dialog.account();
      MyMoneyAccount parentAccount = dialog.parentAccount();
      file->addAccount(newAccount, parentAccount);
      categoryListView->clear();
      refresh();
    }
    catch (MyMoneyException *e)
    {
      QString message("Unable to add account: ");
      message += e->what();
      KMessageBox::information(this, message);
      delete e;
      return;
    }
  }

}

void KCategoriesView::slotDeleteClicked()
{
  KCategoryListItem *item = (KCategoryListItem *)categoryListView->selectedItem();
  if (!item)
    return;

  QString prompt = i18n("Delete this category item: ");
  prompt += item->text(0);

  if ((KMessageBox::questionYesNo(this, prompt))==KMessageBox::Yes)
  {
    try
    {
      MyMoneyFile *file = KMyMoneyFile::instance()->file();

      file->removeAccount(file->account(item->accountID()));
      categoryListView->clear();
      refresh();
    }
    catch (MyMoneyException *e)
    {
      QString message(i18n("Unable to remove category: "));
      message += e->what();
      KMessageBox::error(this, message);
      delete e;
    }
  }
}

void KCategoriesView::slotSelectionChanged(QListViewItem* item)
{
  KCategoryListItem *kitem = (KCategoryListItem *)item;
  if (!kitem)
  {
    buttonEdit->setEnabled(false);
    buttonDelete->setEnabled(false);
  }
  else
  {
    buttonEdit->setEnabled(true);
    buttonDelete->setEnabled(true);
  }
}

void KCategoriesView::slotEditClicked()
{
  KCategoryListItem *item = (KCategoryListItem *)categoryListView->selectedItem();
  if (!item)
    return;

  try
  {
    MyMoneyFile* file = KMyMoneyFile::instance()->file();
    MyMoneyAccount account = file->account(item->accountID());

    KNewAccountDlg dlg(account, true, true, this, "hi", i18n("Edit an Account"));

    if (dlg.exec())
    {
      file->modifyAccount(dlg.account());
    }
  }
  catch (MyMoneyException *e)
  {
    QString errorString = i18n("Cannot edit category: ");
    errorString += e->what();
    KMessageBox::error(this, errorString);
    delete e;
  }
}

void KCategoriesView::readConfig(void)
{
  KConfig *config = KGlobal::config();
  config->setGroup("Last Use Settings");
  m_lastCat = config->readEntry("KCategoriesView_LastCategory");
}

void KCategoriesView::writeConfig(void)
{
  KCategoryListItem *item = (KCategoryListItem *)categoryListView->selectedItem();
  if (!item)
    return;

  KConfig *config = KGlobal::config();
  config->setGroup("Last Use Settings");
  config->writeEntry("KCategoriesView_LastCategory", item->text(0));
  config->sync();
}
