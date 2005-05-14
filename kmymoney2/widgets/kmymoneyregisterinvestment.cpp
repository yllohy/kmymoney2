/***************************************************************************
                          kmymoneyregisterinvestment.cpp  -  description
                             -------------------
    begin                : Mon Jul 12 2004
    copyright            : (C) 2004 by Thomas Baumgart
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

#include "klocale.h"

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyregisterinvestment.h"
#include "../mymoney/mymoneyfile.h"
#include "../views/kledgerview.h"
#include "../views/kledgerviewinvestments.h"

kMyMoneyRegisterInvestment::kMyMoneyRegisterInvestment(QWidget *parent, const char *name )
  : kMyMoneyRegister(4, parent, name)
{
  setNumCols(7);
  setCurrentCell(0, 1);
  horizontalHeader()->setClickEnabled(true);
  horizontalHeader()->setLabel(0, i18n("Date"));
  horizontalHeader()->setLabel(1, i18n("Security"));
  horizontalHeader()->setLabel(2, i18n("Activity"));
  horizontalHeader()->setLabel(3, i18n("C"));
  horizontalHeader()->setLabel(4, i18n("Amount"));
  horizontalHeader()->setLabel(5, i18n("Price"));
  horizontalHeader()->setLabel(6, i18n("Value"));
  setLeftMargin(0);
  verticalHeader()->hide();
  setColumnStretchable(0, false);
  setColumnStretchable(1, false);
  setColumnStretchable(2, false);
  setColumnStretchable(3, false);
  setColumnStretchable(4, false);
  setColumnStretchable(5, false);
  setColumnStretchable(6, false);

  horizontalHeader()->setResizeEnabled(false);
  horizontalHeader()->setMovingEnabled(false);

  // never show horizontal scroll bars
  setHScrollBarMode(QScrollView::AlwaysOff);
}

kMyMoneyRegisterInvestment::~kMyMoneyRegisterInvestment()
{
}

void kMyMoneyRegisterInvestment::paintCell(QPainter *p, int row, int col, const QRect& r,
                                 bool selected, const QColorGroup& cg)
{
  QCString splitCurrency;
  MyMoneyAccount acc;
  MyMoneySecurity security;
  QString currency;
  MyMoneyMoney tmp;
  MyMoneyFile* file = MyMoneyFile::instance();
  KLedgerViewInvestments* myParent = dynamic_cast<KLedgerViewInvestments*> (m_parent);
  if(myParent == 0) {
    qDebug("Internal failure at %s(%d)", __FILE__, __LINE__);
    return;
  }

  setTransactionRow(row);

  int align = Qt::AlignVCenter;
  QString txt(" ");
  if(m_transaction != 0) {
    try {
      security = file->security(m_transaction->commodity());
      currency = security.tradingSymbol();
    } catch(MyMoneyException *e) {
      currency = "???";
      delete e;
    }
    try {
      m_feeSplit = MyMoneySplit();
      m_interestSplit = MyMoneySplit();
      m_accountSplit = MyMoneySplit();
      // find the split that references the stock account
      QValueList<MyMoneySplit>::ConstIterator it_s;
      for(it_s = m_transaction->splits().begin(); it_s != m_transaction->splits().end(); ++it_s) {
        acc = file->account((*it_s).accountId());
        if(acc.accountType() == MyMoneyAccount::Stock) {
          m_security = MyMoneyFile::instance()->security(acc.currencyId());
          m_split = *it_s;
        } else if(acc.accountGroup() == MyMoneyAccount::Expense) {
          m_feeSplit = *it_s;
        } else if(acc.accountGroup() == MyMoneyAccount::Income) {
          m_interestSplit = *it_s;
        } else if(acc.accountGroup() == MyMoneyAccount::Asset
                || acc.accountGroup() == MyMoneyAccount::Liability) {
          m_accountSplit = *it_s;
        }
      }

      KLedgerView::investTransactionTypeE transactionType = myParent->transactionType(*m_transaction, m_split);

      int prec;

      switch (col) {
        case 0:
          align |= Qt::AlignLeft;
          switch(m_transactionRow) {
            case 0:
              txt = KGlobal::locale()->formatDate(m_transaction->postDate(), true);
              break;
          }
          break;

        case 1:
          align |= Qt::AlignLeft;
          switch(m_transactionRow) {
            case 0:
              try {
                acc = file->account(m_split.accountId());
                security = file->security(acc.currencyId());
                txt = security.tradingSymbol();
              } catch(MyMoneyException *e) {
                delete e;
              }
              break;

            case 1:
              align |= Qt::AlignRight;
              switch(transactionType) {
                case KLedgerView::BuyShares:
                case KLedgerView::SellShares:
                case KLedgerView::ReinvestDividend:
                  txt = i18n("Fees");
                  break;
                case KLedgerView::Dividend:
                case KLedgerView::Yield:
                  txt = i18n("Interest");
                  break;
                case KLedgerView::AddShares:
                case KLedgerView::RemoveShares:
                default:
                  txt = " ";
                  break;
              }
              break;

            case 2:
              align |= Qt::AlignRight;
              switch(transactionType) {
                case KLedgerView::BuyShares:
                case KLedgerView::SellShares:
                case KLedgerView::Dividend:
                case KLedgerView::Yield:
                  txt = i18n("Account");
                  break;
                case KLedgerView::ReinvestDividend:
                  txt = i18n("Interest");
                  break;
                case KLedgerView::AddShares:
                case KLedgerView::RemoveShares:
                default:
                  txt = " ";
                  break;
              }
              break;
          }
          break;

        case 2:
          align |= Qt::AlignLeft;
          switch(m_transactionRow) {
            case 0:
              switch(transactionType) {
                case KLedgerView::BuyShares:
                  txt = i18n("Buy Shares");
                  break;
                case KLedgerView::SellShares:
                  txt = i18n("Sell Shares");
                  break;
                case KLedgerView::ReinvestDividend:
                  txt = i18n("Reinvest Dividend");
                  break;
                case KLedgerView::Dividend:
                  txt = i18n("Dividend");
                  break;
                case KLedgerView::Yield:
                  txt = i18n("Yield");
                  break;
                case KLedgerView::AddShares:
                  txt = i18n("Add Shares");
                  break;
                case KLedgerView::RemoveShares:
                  txt = i18n("Remove Shares");
                  break;
                default:
                  txt = " ";
                  break;
              }
              break;

            case 1:
              switch(transactionType) {
                case KLedgerView::BuyShares:
                case KLedgerView::SellShares:
                case KLedgerView::ReinvestDividend:
                  acc = file->account(m_feeSplit.accountId());
                  txt = acc.name();
                  break;
                case KLedgerView::Dividend:
                case KLedgerView::Yield:
                  acc = file->account(m_interestSplit.accountId());
                  txt = acc.name();
                  break;
                case KLedgerView::AddShares:
                case KLedgerView::RemoveShares:
                default:
                  txt = " ";
                  break;
              }
              break;

            case 2:
              switch(transactionType) {
                case KLedgerView::BuyShares:
                case KLedgerView::SellShares:
                case KLedgerView::Dividend:
                case KLedgerView::Yield:
                  acc = file->account(m_accountSplit.accountId());
                  txt = acc.name();
                  break;
                case KLedgerView::ReinvestDividend:
                  acc = file->account(m_interestSplit.accountId());
                  txt = acc.name();
                  break;
                case KLedgerView::AddShares:
                case KLedgerView::RemoveShares:
                default:
                  txt = " ";
                  break;
              }
              break;

            case 3:
              txt = m_split.memo();
              break;

          }
          break;

        case 3:
          switch(m_transactionRow) {
            case 0:
              align |= Qt::AlignHCenter;
              switch(m_split.reconcileFlag()) {
                case MyMoneySplit::Cleared:
                  txt = i18n("C");
                  break;
                case MyMoneySplit::Reconciled:
                case MyMoneySplit::Frozen:
                  txt = i18n("R");
                  break;
                case MyMoneySplit::NotReconciled:
                  break;
              }
              break;
          }
          break;

        case 4:
          switch(m_transactionRow) {
            case 0:
              align |= Qt::AlignRight;
              switch(transactionType) {
                case KLedgerView::BuyShares:
                case KLedgerView::SellShares:
                case KLedgerView::ReinvestDividend:
                case KLedgerView::AddShares:
                case KLedgerView::RemoveShares:
                  prec = MyMoneyMoney::denomToPrec(m_security.smallestAccountFraction());
                  txt = m_split.shares().abs().formatMoney("", prec);
                  break;
                case KLedgerView::Dividend:
                case KLedgerView::Yield:
                default:
                  txt = " ";
                  break;
              }
              break;

            case 1:
              align |= Qt::AlignRight;
              switch(transactionType) {
                case KLedgerView::BuyShares:
                case KLedgerView::SellShares:
                case KLedgerView::ReinvestDividend:
                  txt = m_feeSplit.value().abs().formatMoney(currency);
                  break;
                case KLedgerView::Dividend:
                case KLedgerView::Yield:
                  txt = m_interestSplit.value().abs().formatMoney();
                  break;
                case KLedgerView::AddShares:
                case KLedgerView::RemoveShares:
                default:
                  txt = " ";
                  break;
              }
              break;
          }
          break;

        case 5:
          switch(m_transactionRow) {
            case 0:
              align |= Qt::AlignRight;
              switch(transactionType) {
                case KLedgerView::BuyShares:
                case KLedgerView::SellShares:
                case KLedgerView::ReinvestDividend:
                  txt = (m_split.value(QCString(), QCString())/m_split.shares()).abs().formatMoney(currency);
                  break;
                case KLedgerView::AddShares:
                case KLedgerView::RemoveShares:
                case KLedgerView::Dividend:
                case KLedgerView::Yield:
                default:
                  txt = " ";
                  break;
              }
              break;

          }
          break;

        case 6:
          switch(m_transactionRow) {
            case 0:
              align |= Qt::AlignRight;
              switch(transactionType) {
                case KLedgerView::BuyShares:
                case KLedgerView::SellShares:
                case KLedgerView::Dividend:
                case KLedgerView::Yield:
                  txt = m_accountSplit.value(QCString(), QCString()).abs().formatMoney(currency);
                  break;
                case KLedgerView::ReinvestDividend:
                  txt = m_interestSplit.value(QCString(), QCString()).abs().formatMoney(currency);
                  break;
                case KLedgerView::AddShares:
                case KLedgerView::RemoveShares:
                default:
                  txt = " ";
                  break;
              }
              break;

          }
          break;
      }
    } catch(MyMoneyException *e) {
      delete e;
    }
  }

  // do general stuff
  kMyMoneyRegister::paintCell(p, row, col, r, selected, cg, txt, align);
}

void kMyMoneyRegisterInvestment::adjustColumn(int col)
{
  QHeader *topHeader = horizontalHeader();
  QFontMetrics cellFontMetrics(m_cellFont);
  MyMoneyFile* file = MyMoneyFile::instance();

  int w = topHeader->fontMetrics().width( topHeader->label( col ) ) + 10;
  if ( topHeader->iconSet( col ) )
    w += topHeader->iconSet( col )->pixmap().width();
  w = QMAX( w, 20 );

  // check for date column
  if(col == 0) {
    QString txt = KGlobal::locale()->formatDate(QDate(6999,12,29), true);
    int nw = cellFontMetrics.width(txt+"  ");
    w = QMAX( w, nw );
  }

  // scan through the transactions
  for ( int i = (numRows()/m_rpt)-1; i >= 0; --i ) {
    QString txt;
    KMyMoneyTransaction *t = m_parent->transaction(i);
    MyMoneyMoney amount;
    MyMoneyAccount acc;
    MyMoneySecurity security;
    MyMoneySplit split, feeSplit, interestSplit, accountSplit;
    int nw = 0;

    if(t != NULL) {
      MyMoneySplit split = t->splitById(t->splitId());

      // find the split that references the stock account
      QValueList<MyMoneySplit>::ConstIterator it_s;
      for(it_s = t->splits().begin(); it_s != t->splits().end(); ++it_s) {
        acc = file->account((*it_s).accountId());
        if(acc.accountType() == MyMoneyAccount::Stock) {
          split = *it_s;
        } else if(acc.accountGroup() == MyMoneyAccount::Expense) {
          feeSplit = *it_s;
        } else if(acc.accountGroup() == MyMoneyAccount::Income) {
          interestSplit = *it_s;
        } else if(acc.accountGroup() == MyMoneyAccount::Asset
                || acc.accountGroup() == MyMoneyAccount::Liability) {
          accountSplit = *it_s;
        }
      }
      switch(col) {
        default:
          break;

        case 1:
          acc = file->account(split.accountId());
          security = file->security(acc.currencyId());
          txt = security.tradingSymbol();
          nw = cellFontMetrics.width(txt+"  ");
          nw = QMAX(nw, cellFontMetrics.width(i18n("Fees")+"  "));
          nw = QMAX(nw, cellFontMetrics.width(i18n("Interest")+"  "));
          nw = QMAX(nw, cellFontMetrics.width(i18n("Account")+"  "));
          break;

        case 4:
          amount = split.shares().abs() > feeSplit.value().abs() ? split.shares().abs() : feeSplit.value().abs();
          amount = amount > interestSplit.value().abs() ? amount : interestSplit.value().abs();
          txt = amount.formatMoney();
          nw = cellFontMetrics.width(txt+" WWW  ");
          break;

        case 5:
          if(!split.shares().isZero()) {
            amount = (split.value() / split.shares()).abs();
            txt = amount.formatMoney();
          }
          nw = cellFontMetrics.width(txt+"  ");
          break;

        case 6:
          amount = accountSplit.value().abs();
          txt = amount.formatMoney();
          nw = cellFontMetrics.width(txt+" WWW  ");
          break;
      }
      w = QMAX( w, nw );
    }
  }
  setColumnWidth( col, w );
}

// This must be implemented here, as QTable::eventFilter is not virtual :-(

bool kMyMoneyRegisterInvestment::eventFilter(QObject* o, QEvent* e)
{
  return kMyMoneyRegister::eventFilter(o, e);
}
