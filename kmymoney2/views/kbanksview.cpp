/***************************************************************************
                          KAccountsView.cpp
                             -------------------
    copyright            : (C) 2000 by Michael Edwardes
    email                : mte@users.sourceforge.net
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

#include <qheader.h>
#include <qtooltip.h>
#include <qiconview.h>
#include <qpixmap.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kglobal.h>
#include <klocale.h>
#include <klistview.h>
#include <kconfig.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kbanksview.h"
#include "kbanklistitem.h"
#include "../mymoney/mymoneyfile.h"

static const char* const assetIconImage[] = {
"32 32 9 1",
". c None",
"# c #000000",
"a c #0000ff",
"d c #008183",
"c c #838183",
"f c #c5c2c5",
"g c #ff0000",
"e c #ffff00",
"b c #ffffff",
"................................",
"............#...................",
"...........#a#..................",
"..........#a#a#.................",
".........#a#b#a#................",
"........#a#bbb#a#...............",
".......#a#bbbbb#a#..............",
"......#a#bbbbbbb#a#.............",
".....#a#bbbbbbbbb#a#............",
"....#a#bbbbbbbbbbb#a#...........",
"...#a#bbbbbbbbbbbbb#a#..........",
"..#a#bbbbbbbbbbbbbbb#a#.........",
".#a#bbbbbbbbbbbbbbbbb#a#........",
"#a##bb####bbbbb####bb##a#.......",
".#c#bb##d#bbbbb##d#bb#c#c.......",
".cc#bb#dd#bbbbb#dd#bb#cccc......",
"...#bb#dd#bbbbb#dd#bb#c.c.......",
"...#bb####bbbbb####bb#c.........",
"...#bbbbbb#####bbbbbb#c.........",
"...#bbbbbb#efe#bbbbbb#c.........",
"...#bbbbbb#fef#bbb#######.......",
"...#bbbbbb#efe#bb#ggggggg#......",
"...#bbbbbb#fef#b#ggggggggg#.....",
"...#bbbbbb#######ggggggggg#####.",
"...#bbbbbb##gggggggggggggggggg#.",
"...#########gg###ggggggg###ggg#c",
"....ccccccc#g##c##ggggg##c##gg#c",
"...........###ccc#######ccc####c",
"............c##f##ccccc##f##cccc",
".............c###cc.....###cc...",
"...............ccc.......ccc....",
"................................"};

static const char* const cashIconImage[] = {
"32 32 6 1",
". c None",
"# c #000000",
"a c #838100",
"c c #838183",
"d c #c5c2c5",
"b c #ffff00",
"................................",
".......................#####....",
".....................###aba##...",
"....................##bababab##.",
"....................#babababab#c",
"...........#........#aba###aba#c",
"..........#c#.......##abababa##c",
"........##dbd##.....#b##bab##b#c",
"......##.d.#.d.##...#ab#####ba#c",
"...#####db#c#bdbd##.##abbbbba##c",
".###aba###ddd##d.d.##b##dad##b#c",
"##bababab##bdbd##ddbd#######ba#c",
"#babababab#d.d.dd##d.d.##bbba##c",
"#aba###aba#bdbdbdbd##ddbd####b#c",
"##abababa##d.d.d.d.dd##d.d.##a#c",
"#b##bab##b##dbdbdbdbdbd##ddbd##c",
"#ab#####ba#d##dd.d.d.d.dd###.d.#",
"##abbbbba##bdd##dbdbdbdd##ddd##c",
"#b##dad##b##.d.d##dddd##dd.##dd#",
"#ab#####ba#d##dbdd####dbd##dd##c",
"##abbbbba###dd##.d.#dd.##dd##dd#",
"#b##dad##b#d##dd##dbd##dd##dd##c",
"#ab#####ba##dd##dd###dd##dd##dd#",
"##abbbbba##d##dd##d#d##dd##dd##c",
"#b##dad##b##dd##dd###dd##dd##ccc",
"#ab#####ba#c##dd##d#d##dd##ccc..",
"##abbbbba##ccc##dd###dd##ccc....",
".c##dad##ccc..cc##d#d##ccc......",
"..c#####cc......cc###ccc........",
"....ccccc.........c#cc..........",
"...................cc...........",
"................................"};

static const char* const creditCardIconImage[] = {
"32 32 7 1",
". c None",
"a c #000000",
"b c #000083",
"e c #0000ff",
"d c #008183",
"c c #838183",
"# c #ffffff",
"................................",
"................................",
"................................",
"................................",
"................................",
"...............................#",
".aaaaaaaaaaaaaaaaaaaaaaaaaaaaa..",
"a#############################a.",
"a#bbbbbbbbbbbbbbbbbbbbbbbbbbb#ac",
"a#bbbbbbbbbbbbbbbbbbbbbbbbbbb#ac",
"a#bbbbbbbbbbbbbbbbbbbbbbbbbbb#ac",
"a#bbbbbbbbbbbbbbbbbbbbbbbbbbb#ac",
"a#bbbbbbbbbbbbbbbbbbbbbbbbbbb#ac",
"a#dededededededededededededed#ac",
"a#ededededededededededededede#ac",
"a#dededededededededededededed#ac",
"a#ededededededededededededede#ac",
"a#ded##e##d##e##d##e##d##eded#ac",
"a#ede##d##e##d##e##d##e##dede#ac",
"a#dededededededededededededed#ac",
"a#ededededededededededededede#ac",
"a#ded##################ededed#ac",
"a#ededededededededededededede#ac",
"a#dededededededededededededed#ac",
"a#############################ac",
".aaaaaaaaaaaaaaaaaaaaaaaaaaaaac.",
"..cccccccccccccccccccccccccccc..",
"................................",
"................................",
"................................",
"................................",
"................................"};

static const char* const checkingsIconImage[] = {
"32 32 6 1",
". c None",
"# c #000000",
"b c #0000ff",
"a c #008183",
"c c #838183",
"d c #ffffff",
"........###.....................",
"........#ab##...................",
"........#baba##.................",
"........#ababab##...............",
"........#babababa##.............",
"........#ababababab##...........",
".........#abababababa##.........",
".........#babababababab##.......",
".........#abababababababa##.....",
"..........#abababababababab##...",
"..........#babababababababab#...",
"..........#ababababababababa#...",
"...........##bababababababab#...",
"..........##c##ababababababa#...",
"........##d##cc##bababababab#...",
"......##ddddd##cc##ababababab#..",
"....##ddddddddd##cc##babababa#..",
"..##ddd##dddddddd##cc##ababab#..",
".#ddd##dd##dddddddd##cc##baba##.",
".##dddd##dd##dddddddd##cc###ba#.",
".#c##dddd##dd##dddd#ddd##ccc###.",
"###cc##dddd##dd##ddd##ddd##cc#b#",
"#ba##cc##dddd##dd##ddd#dddd###a#",
"###ab##cc##dddd##dd##dddd##cc#b#",
"ccc##aa##cc##dddd##dddd##cc##aa#",
"...cc##ab##cc##dddddd##cc##ab##c",
".....cc##aa##cc##dd##cc##aa##cc.",
".......cc##ab##cc##cc##ab##cc...",
".........cc##aa##cc##aa##cc.....",
"...........cc##ab##ab##cc.......",
".............cc##aa##cc.........",
"...............cc##cc..........."};

static const char* const investmentIconImage[] = {
"32 32 5 1",
". c None",
"# c #000000",
"a c #838183",
"b c #c5c2c5",
"c c #ffff00",
"................................",
"#a..............................",
"#a..............................",
"#a..####........................",
"#a..#bc#b.......................",
"#a..#cb#b.......................",
"#a..#bc#b.......................",
"#a..#cb#b####...................",
"#a..#bc#b#bc#...................",
"#a..#cb#b#cb#b..................",
"#a..#bc#b#bc#b..................",
"#a..#cb#b#cb#b####..............",
"#a..#bc#b#bc#b#bc#..............",
"#a..#cb#b#cb#b#cb#b.............",
"#a..#bc#b#bc#b#bc#b.............",
"#a..#cb#b#cb#b#cb#b####.........",
"#a..#bc#b#bc#b#bc#b#bc#b........",
"#a..#cb#b#cb#b#cb#b#cb#b........",
"#a..#bc#b#bc#b#bc#b#bc#b........",
"#a..#cb#b#cb#b#cb#b#cb#b####....",
"#a..#bc#b#bc#b#bc#b#bc#b#bc#....",
"#a..#cb#b#cb#b#cb#b#cb#b#cb#b...",
"#a..#bc#b#bc#b#bc#b#bc#b#bc#b...",
"#a..#cb#b#cb#b#cb#b#cb#b#cb#b...",
"#a..#bc#b#bc#b#bc#b#bc#b#bc#b...",
"#a..####b####b####b####b####b...",
"#a..............................",
"#a..............................",
"###############################.",
"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa.",
"................................",
"................................"};

static const char* const loanIconImage[] = {
"32 32 8 1",
". c None",
"# c #000000",
"b c #0000ff",
"a c #008183",
"c c #838183",
"e c #c5c2c5",
"f c #ffff00",
"d c #ffffff",
".....##########################.",
".....#abababababababababababab#c",
".....#bababdddddddddddddddbaba#c",
".....#ababaaaaaaaaaaaaaaaaabab#c",
".....#babababababababababababa#c",
".....#dddddddddddddddddddddddd#c",
".....#dddddddddddddddccdccdccd#c",
".....#dddddddddddddddccdccdccd#c",
".....#dddddddddddddddddddddddd#c",
".....#dddcc###dccdccdccdccdccd#c",
".....#ddd##efe##cdccdccdccdccd#c",
".....#d##dededed##dddddddddddd#c",
".....##efef###eefe##dccdccdccd#c",
"...##dede##dee##eded##cdccdccd#c",
".##efef##efefefe##eefe##dddddd#c",
"#eede##eededededee##eded##dccd#c",
".##efef##efefefefefe##eefe##cd#c",
"#cc##dede##eededededee###ded#d#c",
".##cc##efee##efefefee##eee##cd#c",
"#cc##cc##dede##eeee##eed##cc#d#c",
".##cc##cc##efee####efe##cc##cd#c",
"#cc##cc##cc##ded#eed##cc##cc#d#c",
".##cc##cc##cc##efe##cc##cc##c##c",
".cc##cc##cc##cc###cc##cc##cc#ccc",
"...cc##cc##cc##c#c##cc##cc##c...",
".....cc##cc##cc###cc##cc##ccc...",
".......cc##cc##c#c##cc##ccc.....",
".........cc##cc###cc##ccc.......",
"...........cc##c#c##ccc.........",
".............cc###ccc...........",
"...............c#cc.............",
"................cc.............."};


KAccountsView::KAccountsView(QWidget *parent, const char *name)
 : KBankViewDecl(parent,name)
{
  KConfig *config = KGlobal::config();
  config->setGroup("List Options");
  m_bViewNormalAccountsView = config->readBoolEntry("NormalAccountsView", true);

  accountListView->setRootIsDecorated(true);
  accountListView->setAllColumnsShowFocus(true);

  accountListView->addColumn(i18n("Account"));
  accountListView->addColumn(i18n("Type"));
  accountListView->addColumn(i18n("Balance"));



  accountListView->setMultiSelection(false);

  accountListView->setColumnWidthMode(0, QListView::Maximum);
  accountListView->setColumnWidthMode(1, QListView::Maximum);
  accountListView->setColumnWidthMode(2, QListView::Maximum);

  accountListView->setColumnAlignment(2, Qt::AlignRight);

  accountListView->setResizeMode(QListView::AllColumns);

  accountListView->header()->setResizeEnabled(true);


  QFont defaultFont = QFont("helvetica", 12);
  accountListView->header()->setFont(config->readFontEntry("listHeaderFont", &defaultFont));

  connect(accountListView, SIGNAL(selectionChanged(QListViewItem*)),
    this, SLOT(slotSelectionChanged(QListViewItem*)));

  // somehow, the rightButtonClicked signal does not make it, we use
  // rightButtonPressed instead to show the context menu
  // connect(accountListView, SIGNAL(rightButtonClicked(QListViewItem* , const QPoint&, int)),
  //   this, SLOT(slotListRightMouse(QListViewItem*, const QPoint&, int)));
  connect(accountListView, SIGNAL(rightButtonPressed(QListViewItem* , const QPoint&, int)),
    this, SLOT(slotListRightMouse(QListViewItem*, const QPoint&, int)));
  connect(accountIconView, SIGNAL(rightButtonPressed(QIconViewItem*, const QPoint&)),
    this, SLOT(slotIconRightMouse(QIconViewItem*, const QPoint&)));

  connect(accountListView, SIGNAL(doubleClicked(QListViewItem*, const QPoint&, int)),
    this, SLOT(slotListDoubleClicked(QListViewItem*, const QPoint&, int)));
  connect(accountIconView, SIGNAL(doubleClicked(QIconViewItem*)),
    this, SLOT(slotIconDoubleClicked(QIconViewItem*)));

  m_bSelectedAccount=false;
  m_bSelectedInstitution=false;
  // m_bSignals=true;

  accountIconView->clear();
  accountIconView->setSorting(-1);
  accountListView->setSorting(0);

  // never show a horizontal scroll bar
  //accountListView->setHScrollBarMode(QScrollView::AlwaysOff);

  MyMoneyFile::instance()->attach(MyMoneyFile::NotifyClassAccountHierarchy, this);
  MyMoneyFile::instance()->attach(MyMoneyFile::NotifyClassAccount, this);
  MyMoneyFile::instance()->attach(MyMoneyFile::NotifyClassInstitution, this);

}

KAccountsView::~KAccountsView()
{
  MyMoneyFile::instance()->detach(MyMoneyFile::NotifyClassAccountHierarchy, this);
  MyMoneyFile::instance()->detach(MyMoneyFile::NotifyClassAccount, this);
  MyMoneyFile::instance()->detach(MyMoneyFile::NotifyClassInstitution, this);
}

void KAccountsView::slotListDoubleClicked(QListViewItem* item, const QPoint& pos, int c)
{
  KAccountListItem* accountItem = static_cast<KAccountListItem*> (item);
  if(accountItem)
  {

    // Only emit the signal if its an account
    MyMoneyFile *file = MyMoneyFile::instance();

    try
    {
      MyMoneyAccount account = file->account(accountItem->accountID());

      // this signal will only be emitted for asset and liability accounts
      if(!file->isStandardAccount(account.id())) {
        switch(file->accountGroup(account.accountType())) {
          case MyMoneyAccount::Asset:
          case MyMoneyAccount::Liability:
            m_bSelectedAccount=true;
            m_bSelectedInstitution=false;
            m_selectedAccount = accountItem->accountID();
            emit accountDoubleClick();
            break;

          default:
            break;
        }
      }
    }
    catch (MyMoneyException *e)
    {
      // Probably clicked on the institution in normal view
      delete e;
    }
  }
}

void KAccountsView::slotIconDoubleClicked(QIconViewItem* item)
{
  KAccountIconItem *accountItem = static_cast<KAccountIconItem*> (item);

  if(accountItem) {
    MyMoneyFile *file = MyMoneyFile::instance();

    try {
      MyMoneyAccount account = file->account(accountItem->accountID());
      m_bSelectedAccount=true;
      m_bSelectedInstitution=false;
      m_selectedAccount = accountItem->accountID();
      emit accountDoubleClick();
    } catch (MyMoneyException *e) {
      delete e;
    }
  }
}

void KAccountsView::slotIconRightMouse(QIconViewItem* item, const QPoint&)
{
  KAccountIconItem *accountItem = static_cast<KAccountIconItem*> (item);
  if (accountItem) {
    try {
      m_bSelectedAccount=true;
      m_bSelectedInstitution=false;
      m_selectedAccount = accountItem->accountID();

      emit accountRightMouseClick();

    } catch (MyMoneyException *e) {
      delete e;
    }
  }
}

void KAccountsView::slotListRightMouse(QListViewItem* item, const QPoint& , int col)
{
  if (item==0 || col==-1) {
    emit rightMouseClick();

  } else {
    KAccountListItem* accountItem = static_cast<KAccountListItem*> (item);
    if (accountItem) {
      try {
        MyMoneyFile *file = MyMoneyFile::instance();
        MyMoneyAccount account = file->account(accountItem->accountID());
        
        m_bSelectedAccount=true;
        m_bSelectedInstitution=false;
        m_selectedAccount = accountItem->accountID();
        emit accountRightMouseClick();

      } catch (MyMoneyException *e) {
        m_bSelectedAccount=false;
        m_bSelectedInstitution=true;
        m_selectedInstitution = accountItem->accountID();

        emit bankRightMouseClick();
      }
    }
  }
}

QCString KAccountsView::currentAccount(bool& success)
{
  success=m_bSelectedAccount;
  return (success) ? m_selectedAccount : QCString("");
}

QCString KAccountsView::currentInstitution(bool& success)
{
  success=m_bSelectedInstitution;
  return (success) ? m_selectedInstitution : QCString("");
}

void KAccountsView::update(const QCString& id)
{
  if(id == MyMoneyFile::NotifyClassAccountHierarchy
  || (id == MyMoneyFile::NotifyClassInstitution && m_bViewNormalAccountsView == true))
    refresh(id);
  if(id == MyMoneyFile::NotifyClassAccount)
    refreshTotalProfit();
}

void KAccountsView::refreshView(void)
{
  refresh(m_selectedAccount);
}

void KAccountsView::refreshTotalProfit(void)
{
  KConfig *config = KGlobal::config();
  config->setGroup("List Options");
  QFont defaultFont = QFont("helvetica", 12);

  MyMoneyMoney totalProfit;
  MyMoneyFile* file = MyMoneyFile::instance();

  MyMoneyAccount liabilityAccount = file->liability();
  MyMoneyAccount assetAccount = file->asset();

  totalProfit = file->totalBalance(assetAccount.id()) +
                file->totalBalance(liabilityAccount.id());

  QString s(i18n("Total Profits: "));
  s += totalProfit.formatMoney();


  totalProfitsLabel->setFont(config->readFontEntry("listCellFont", &defaultFont));
  totalProfitsLabel->setText(s);
}

const QPixmap KAccountsView::accountImage(const MyMoneyAccount::accountTypeE type) const
{
  QPixmap rc;
  switch(type) {
    default:
    case MyMoneyAccount::Checkings:
    case MyMoneyAccount::Savings:
      rc = QPixmap(checkingsIconImage);
      break;

    case MyMoneyAccount::CreditCard:
      rc = QPixmap(creditCardIconImage);
      break;

    case MyMoneyAccount::Asset:
      rc = QPixmap(assetIconImage);
      break;

    case MyMoneyAccount::Cash:
      rc = QPixmap(cashIconImage);
      break;
  }

  return rc;
}

void KAccountsView::refresh(const QCString& selectAccount)
{
  KConfig *config = KGlobal::config();
  config->setGroup("List Options");
  QFont defaultFont = QFont("helvetica", 12);
  accountListView->header()->setFont(config->readFontEntry("listHeaderFont", &defaultFont));
  m_bViewNormalAccountsView = config->readBoolEntry("NormalAccountsView", true);

  clear();

  m_selectedAccount = selectAccount;

  MyMoneyFile *file = MyMoneyFile::instance();

  QValueList<MyMoneyAccount> accountList;
  accountList = file->accountList();
  m_accountMap.clear();

  QValueList<MyMoneyAccount>::ConstIterator it_a;
  for(it_a = accountList.begin(); it_a != accountList.end(); ++it_a)
    m_accountMap[(*it_a).id()] = *it_a;

  if (m_bViewNormalAccountsView) {
    accountListView->header()->setLabel(0, i18n("Institution"));

    try {
      QValueList<MyMoneyInstitution> list = file->institutionList();
      QValueList<MyMoneyInstitution>::ConstIterator institutionIterator;
      for (institutionIterator = list.begin();

            institutionIterator != list.end();
            ++institutionIterator) {

        KAccountListItem *topLevelInstitution = new KAccountListItem(accountListView,
                      *institutionIterator);

        QCStringList accountList = (*institutionIterator).accountList();
        for ( QCStringList::ConstIterator it = accountList.begin();
              it != accountList.end();
              ++it ) {
          KAccountListItem *accountItem = new KAccountListItem(topLevelInstitution,
              m_accountMap[*it]);
          KAccountIconItem* accountIcon = new KAccountIconItem(accountIconView,
              m_accountMap[*it],
              accountImage(m_accountMap[*it].accountType()));

          QCStringList subAccounts = m_accountMap[*it].accountList();
          if (subAccounts.count() >= 1) {
            showSubAccounts(subAccounts, accountItem, i18n(""));
          }
        }
      }

    } catch (MyMoneyException *e) {
      qDebug("Exception in assets account refresh (normal view): %s", e->what().latin1());
      delete e;
    }

  } else {       // Show new 'advanced' view
    accountListView->header()->setLabel(0, i18n("Account"));
    // Do all 4 account roots
    try {
      // Asset
      MyMoneyAccount assetAccount = file->asset();
      KAccountListItem *assetTopLevelAccount = new KAccountListItem(accountListView,
            assetAccount);

      for ( QCStringList::ConstIterator it = assetAccount.accountList().begin();
            it != assetAccount.accountList().end();
            ++it ) {
        KAccountListItem *accountItem = new KAccountListItem(assetTopLevelAccount,
            m_accountMap[*it]);

        KAccountIconItem* accountIcon = new KAccountIconItem(accountIconView,
            m_accountMap[*it],
            accountImage(m_accountMap[*it].accountType()));

        QCStringList subAccounts = m_accountMap[*it].accountList();
        if (subAccounts.count() >= 1) {
          showSubAccounts(subAccounts, accountItem, i18n("Asset"));
        }
      }

      // Liability
      MyMoneyAccount liabilityAccount = file->liability();
      KAccountListItem *liabilityTopLevelAccount = new KAccountListItem(accountListView,
            liabilityAccount);

      for ( QCStringList::ConstIterator it = liabilityAccount.accountList().begin();
            it != liabilityAccount.accountList().end();
            ++it ) {
        KAccountListItem *accountItem = new KAccountListItem(liabilityTopLevelAccount,
            m_accountMap[*it]);

        KAccountIconItem* accountIcon = new KAccountIconItem(accountIconView,
            m_accountMap[*it],
            accountImage(m_accountMap[*it].accountType()));

        QCStringList subAccounts = m_accountMap[*it].accountList();
        if (subAccounts.count() >= 1) {
          showSubAccounts(subAccounts, accountItem, i18n("Liability"));
        }
      }

      // Income
      MyMoneyAccount incomeAccount = file->income();
      KAccountListItem *incomeTopLevelAccount = new KAccountListItem(accountListView,
            incomeAccount);

      for ( QCStringList::ConstIterator it = incomeAccount.accountList().begin();
            it != incomeAccount.accountList().end();
            ++it ) {
        KAccountListItem *accountItem = new KAccountListItem(incomeTopLevelAccount,
            m_accountMap[*it]);

        QCStringList subAccounts = m_accountMap[*it].accountList();
        if (subAccounts.count() >= 1) {
          showSubAccounts(subAccounts, accountItem, i18n("Income"));
        }
      }

      // Expense
      MyMoneyAccount expenseAccount = file->expense();
      KAccountListItem *expenseTopLevelAccount = new KAccountListItem(accountListView,
            expenseAccount);

      for ( QCStringList::ConstIterator it = expenseAccount.accountList().begin();
            it != expenseAccount.accountList().end();
            ++it ) {
        KAccountListItem *accountItem = new KAccountListItem(expenseTopLevelAccount,
            m_accountMap[*it]);

        QCStringList subAccounts = m_accountMap[*it].accountList();

        if (subAccounts.count() >= 1) {
          showSubAccounts(subAccounts, accountItem, i18n("Expense"));
        }
      }

    } catch (MyMoneyException *e) {
      qDebug("Exception in assets account refresh: %s", e->what().latin1());
      delete e;
    }
  }

  m_accountMap.clear();

  refreshTotalProfit();

/*
  if (m_bSelectedBank || m_bSelectedAccount)
    accountListView->setSelected(item, true);
*/
}

void KAccountsView::showSubAccounts(const QCStringList& accounts, KAccountListItem *parentItem, const QString& group)
{
  for ( QCStringList::ConstIterator it = accounts.begin(); it != accounts.end(); ++it ) {
    KAccountListItem *accountItem  = new KAccountListItem(parentItem,
          m_accountMap[*it]);

    QCStringList subAccounts = m_accountMap[*it].accountList();
    if (subAccounts.count() >= 1) {
      showSubAccounts(subAccounts, accountItem, group);
    }
  }
}

void KAccountsView::clear(void)
{
  accountListView->clear();
  accountIconView->clear();
  m_bSelectedAccount = false;
  m_bSelectedInstitution=false;
}

void KAccountsView::resizeEvent(QResizeEvent* e)
{
  //accountListView->setColumnWidth(0, 400);
  //accountListView->setColumnWidth(1,150);
  //int totalWidth=accountListView->width();
  // accountListView->setColumnWidth(2, totalWidth-550-5);

  // call base class resizeEvent()
  //KBankViewDecl::resizeEvent(e);
}

void KAccountsView::slotSelectionChanged(QListViewItem *item)
{
  KAccountListItem *accountItem = (KAccountListItem*)item;
  if (accountItem)
  {
    MyMoneyFile *file = MyMoneyFile::instance();

    try
    {
      MyMoneyAccount account = file->account(accountItem->accountID());
      m_bSelectedAccount=true;
      m_selectedAccount = accountItem->accountID();
      //emit accountSelected();
    }
    catch (MyMoneyException *e)
    {
      // Probably clicked on the institution in normal view
      m_bSelectedAccount=false;
      m_bSelectedInstitution=true;
      // FIXME: Change KAccountListItem::accountID to id.
      m_selectedInstitution = accountItem->accountID();

      delete e;
    }
  }
}

void KAccountsView::show()
{
  emit signalViewActivated();

  QWidget::show();
}
