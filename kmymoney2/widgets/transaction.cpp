/***************************************************************************
                          transaction.cpp  -  description
                             -------------------
    begin                : Tue Jun 13 2006
    copyright            : (C) 2000-2006 by Thomas Baumgart
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

#include <qstring.h>
#include <qpainter.h>
#include <qwidgetlist.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kdebug.h>

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoney/transaction.h>
#include <kmymoney/mymoneytransaction.h>
#include <kmymoney/mymoneysplit.h>
#include <kmymoney/mymoneyfile.h>
#include <kmymoney/mymoneypayee.h>
#include <kmymoney/register.h>
#include <kmymoney/kmymoneycategory.h>
#include <kmymoney/kmymoneydateinput.h>
#include <kmymoney/transactionform.h>
#include <kmymoney/kmymoneylineedit.h>
#include <kmymoney/kmymoneyedit.h>
#include <kmymoney/transactioneditor.h>
#include <kmymoney/investtransactioneditor.h>
#include <kmymoney/kmymoneyutils.h>

#include "../kmymoneyglobalsettings.h"

using namespace KMyMoneyRegister;
using namespace KMyMoneyTransactionForm;

static unsigned char attentionSign[] = {
  0x89,0x50,0x4E,0x47,0x0D,0x0A,0x1A,0x0A,
  0x00,0x00,0x00,0x0D,0x49,0x48,0x44,0x52,
  0x00,0x00,0x00,0x14,0x00,0x00,0x00,0x14,
  0x08,0x06,0x00,0x00,0x00,0x8D,0x89,0x1D,
  0x0D,0x00,0x00,0x00,0x04,0x73,0x42,0x49,
  0x54,0x08,0x08,0x08,0x08,0x7C,0x08,0x64,
  0x88,0x00,0x00,0x00,0x19,0x74,0x45,0x58,
  0x74,0x53,0x6F,0x66,0x74,0x77,0x61,0x72,
  0x65,0x00,0x77,0x77,0x77,0x2E,0x69,0x6E,
  0x6B,0x73,0x63,0x61,0x70,0x65,0x2E,0x6F,
  0x72,0x67,0x9B,0xEE,0x3C,0x1A,0x00,0x00,
  0x02,0x05,0x49,0x44,0x41,0x54,0x38,0x8D,
  0xAD,0x95,0xBF,0x4B,0x5B,0x51,0x14,0xC7,
  0x3F,0x2F,0xBC,0x97,0x97,0x97,0x97,0x77,
  0xF3,0xF2,0x1C,0xA4,0x54,0x6B,0x70,0x10,
  0x44,0x70,0x2A,0x91,0x2E,0x52,0x02,0x55,
  0x8A,0xB5,0xA3,0xAB,0x38,0x08,0x66,0xCC,
  0xEE,0xE0,0xE2,0x20,0xB8,0x38,0xB8,0xB8,
  0xF8,0x1F,0x38,0x29,0xA5,0x29,0x74,0x90,
  0x0E,0x0D,0x0E,0x22,0x1D,0x44,0xA8,0xD0,
  0xD4,0xB4,0x58,0x4B,0x09,0xF9,0xF1,0x4A,
  0x3B,0xD4,0xD3,0xE1,0x55,0xD3,0x34,0xAF,
  0x49,0x6C,0x3D,0xF0,0x85,0x7B,0xCF,0xFD,
  0x9E,0xEF,0x3D,0xE7,0xFE,0xD4,0x44,0x84,
  0xDB,0xB4,0x48,0x2F,0xA4,0x94,0xAB,0xE5,
  0x52,0xAE,0x96,0xEB,0x49,0x51,0x44,0x3A,
  0x02,0x18,0x88,0xC7,0xF1,0xE3,0x71,0x7C,
  0x60,0xA0,0x1B,0xBF,0x6B,0x86,0x49,0xC5,
  0x46,0x3E,0x47,0x34,0x9F,0x23,0x9A,0x54,
  0x6C,0xFC,0x57,0x86,0x40,0xC6,0x4B,0xE1,
  0x37,0xCA,0x48,0xA3,0x8C,0x78,0x29,0x7C,
  0x20,0xD3,0x31,0xA6,0xD3,0xA0,0x52,0x1C,
  0x6D,0x6F,0x72,0xD9,0x28,0x23,0xFE,0x07,
  0x64,0x7B,0x93,0x4B,0xA5,0x38,0xFA,0x27,
  0x41,0x60,0x6E,0x74,0x84,0x7A,0xE5,0x1D,
  0x92,0x54,0x88,0xE7,0x22,0xD5,0x12,0x32,
  0x3A,0x42,0x1D,0x98,0xBB,0x91,0x20,0x60,
  0xDA,0x36,0x17,0xFB,0x7B,0xC8,0xC1,0x4B,
  0x04,0x02,0xBC,0x7E,0x81,0xEC,0xEF,0x21,
  0xB6,0xCD,0x05,0x60,0xF6,0x2C,0x68,0x9A,
  0x2C,0xCF,0x4C,0xE1,0x4B,0x05,0x39,0x3F,
  0x69,0x0A,0xBE,0x7F,0x83,0x48,0x05,0x99,
  0x99,0xC2,0x37,0x4D,0x96,0x7B,0x12,0x04,
  0xFA,0x2D,0x8B,0xC6,0xE9,0x61,0x10,0x2C,
  0x15,0xC4,0x8A,0x21,0x86,0x8E,0xFC,0xF8,
  0x12,0xF4,0x4F,0x0F,0x11,0xCB,0xA2,0x01,
  0xF4,0x77,0x3D,0x36,0x4E,0x82,0xF5,0xA5,
  0x05,0x8C,0xE1,0x74,0xD3,0x37,0x34,0x18,
  0x20,0xF2,0x8B,0x3D,0x9C,0x86,0xA5,0x05,
  0x0C,0x27,0xC1,0x7A,0xC7,0x63,0x03,0x8C,
  0x2B,0x07,0xBF,0x5A,0x6A,0x66,0x27,0x15,
  0x64,0x3A,0x8B,0x3C,0x7A,0xD8,0xEA,0xAB,
  0x96,0x10,0xE5,0xE0,0x03,0xE3,0x7F,0xCD,
  0x50,0x39,0x6C,0xAD,0xAD,0x10,0x53,0xAA,
  0x75,0xD2,0xF4,0xBD,0x00,0x2D,0x5C,0x05,
  0x6B,0x2B,0xC4,0x94,0xC3,0xD6,0xEF,0xFE,
  0x6B,0x41,0x4D,0xD3,0x66,0xFB,0x3C,0xC6,
  0x16,0xE7,0xDB,0x97,0x61,0xE2,0x3E,0x3C,
  0xC8,0xB4,0x15,0xC7,0xE2,0x3C,0x91,0x3E,
  0x8F,0x31,0x4D,0xD3,0x66,0x5B,0x4A,0x06,
  0x8C,0x84,0xCD,0x59,0x61,0xA7,0xB5,0xAC,
  0x2B,0x9C,0x1C,0x04,0x08,0x1B,0x2B,0xEC,
  0x20,0x09,0x9B,0x33,0xC0,0xB8,0xDE,0x65,
  0x43,0x27,0x9F,0x9D,0xA4,0x1E,0x16,0xF0,
  0xF9,0x6D,0xB0,0xC3,0x86,0x1E,0xB4,0xC3,
  0x38,0xD9,0x49,0xEA,0x86,0x4E,0xFE,0xEA,
  0x29,0xF4,0x2C,0x8B,0xDA,0x71,0x31,0x9C,
  0xFC,0xF5,0x23,0x32,0x34,0x88,0xDC,0xBD,
  0x13,0x5C,0xBF,0x30,0xCE,0x71,0x11,0xB1,
  0x2C,0x6A,0x80,0xA7,0xDB,0x36,0xAB,0x4F,
  0xA6,0x89,0xBA,0x49,0x38,0xFF,0xD4,0xBE,
  0x4E,0x00,0xAF,0x9E,0x81,0x08,0xD4,0xEA,
  0x01,0xFE,0x34,0x37,0x09,0x4F,0x1F,0x13,
  0xDD,0x7D,0xCE,0xAA,0x96,0x72,0x29,0x7C,
  0xFB,0xCE,0x44,0xB8,0xD4,0xCD,0x2C,0x66,
  0x52,0xD4,0x6E,0xFB,0x0B,0xF8,0x09,0x63,
  0x63,0x31,0xE4,0x85,0x76,0x2E,0x0E,0x00,
  0x00,0x00,0x00,0x49,0x45,0x4E,0x44,0xAE,
  0x42,0x60,0x82
};

Transaction::Transaction(Register *parent, const MyMoneyTransaction& transaction, const MyMoneySplit& split, int uniqueId) :
  RegisterItem(parent),
  m_transaction(transaction),
  m_split(split),
  m_form(0),
  m_uniqueId(m_transaction.id()),
  m_formRowHeight(-1),
  m_selected(false),
  m_focus(false),
  m_erronous(false),
  m_inEdit(false),
  m_inRegisterEdit(false),
  m_showBalance(true),
  m_reducedIntensity(false)
{
  MyMoneyFile* file = MyMoneyFile::instance();

  // load the account
  if(!m_split.accountId().isEmpty())
    m_account = file->account(m_split.accountId());

  // load the payee
  if(!m_split.payeeId().isEmpty()) {
    m_payee = file->payee(m_split.payeeId()).name();
  }
  if(parent->account().isIncomeExpense()) {
    m_payeeHeader = m_split.shares().isNegative() ? i18n("From") : i18n("Pay to");
  } else {
    m_payeeHeader = m_split.shares().isNegative() ? i18n("Pay to") : i18n("From");
  }

  // load the currency
  if(!m_transaction.id().isEmpty())
    m_splitCurrencyId = m_account.currencyId();

  // check if transaction is errnous or not
  m_erronous = !m_transaction.splitSum().isZero();

  if(!m_uniqueId.isEmpty()) {
    m_uniqueId += "-";
    QString id;
    id.setNum(uniqueId);
    m_uniqueId += id.rightJustify(3, '0');
  }
}

void Transaction::setFocus(bool focus, bool updateLens)
{
  if(focus != m_focus) {
    m_focus = focus;
  }
  if(updateLens) {
    if(KMyMoneyGlobalSettings::ledgerLens()
    || !KMyMoneyGlobalSettings::transactionForm()
    || KMyMoneyGlobalSettings::showRegisterDetailed()
    || m_parent->m_ledgerLensForced) {
      if(focus)
        setNumRowsRegister(numRowsRegister(true));
      else
        setNumRowsRegister(numRowsRegister(KMyMoneyGlobalSettings::showRegisterDetailed()));
    }
  }
}

void Transaction::markAttachment(QPainter* painter, int /* row */, int /* col */, const QRect& r)
{
  static QPixmap clip;

  const int m = 2;  // margin
  int h = m_parent->rowHeightHint() - (2*m);
  int lx = r.topRight().x() - h;
  if(isErronous())
    lx -= h;
  QRect cr(QPoint(lx - m, m), QSize(h, h));

  painter->save();
  if(clip.isNull()) {
    clip = KGlobal::iconLoader()->loadIcon("attach", KIcon::Small, KIcon::SizeSmall, KIcon::DefaultState);
    if(clip.height() > h) {
      clip.resize(h, h);
    }
  }

  painter->drawPixmap(QPoint(lx - m, m + (h - clip.height())/2 ), clip);
  painter->restore();
}

void Transaction::markAsErronous(QPainter* painter, int /* row */, int /* col */, const QRect& r)
{
  const int m = 2;  // margin
  int h = m_parent->rowHeightHint() - (2*m);
  QRect cr(QPoint(r.topRight().x() - h - m, m), QSize(h, h));

  painter->save();
  QPixmap attention;
  attention.loadFromData(attentionSign, sizeof(attentionSign), 0, 0);

  if(attention.height() > h) {
    attention.resize(h, h);
  }
  painter->drawPixmap(QPoint(r.topRight().x() - h - m, m + (h - attention.height())/2 ), attention);
  painter->restore();

}

bool Transaction::paintRegisterCellSetup(QPainter* painter, int& row, int& col, QRect& cellRect, QRect& textRect, QColorGroup& cg, QBrush& brush)
{
  if(m_reducedIntensity)
    cg = m_parent->palette().disabled();

  if(m_alternate)
    cg.setColor(QColorGroup::Base, KMyMoneyGlobalSettings::listColor());
  else
    cg.setColor(QColorGroup::Base, KMyMoneyGlobalSettings::listBGColor());

  cellRect.setX(0);
  cellRect.setY(0);
  cellRect.setWidth(m_parent->columnWidth(col));
  cellRect.setHeight(m_parent->rowHeight(m_startRow + row));

  textRect = cellRect;
  textRect.setX(2);
  textRect.setWidth(textRect.width()-4);

  if(m_selected) {
    brush = QBrush(cg.highlight());
    painter->setPen(cg.highlightedText());
  } else {
    brush = QBrush(cg.base());
    painter->setPen(cg.text());
  }

  // do we need to switch to the error color?
  if(m_erronous && m_parent->markErronousTransactions()) {
    painter->setPen(KMyMoneyGlobalSettings::listErronousTransactionColor());
  }

  // do we need to switch to the negative balance color?
  if(col == BalanceColumn) {
    bool showNegative = m_balance.isNegative();
    if(m_account.accountGroup() == MyMoneyAccount::Liability && !m_balance.isZero() )
      showNegative = !showNegative;
    if(showNegative)
      painter->setPen(KMyMoneyGlobalSettings::listNegativeValueColor());
  }
  return true;
}

void Transaction::paintRegisterCellFocus(QPainter* painter, int row, int col, const QRect& r, const QColorGroup& cg)
{

  if(m_focus) {
    QPen oldPen = painter->pen();
    QPen newPen = oldPen;
    newPen.setWidth(0);

    painter->setFont(KMyMoneyGlobalSettings::listCellFont());
    painter->setPen(newPen);
    painter->setPen(cg.foreground());
    painter->setPen(Qt::DotLine);
    // for the first Row, we need to paint the top
    QPoint start, end;
#if 0
    if(row == 0) {
      start = QPoint(r.x(), r.y() + 1);
      end = QPoint(r.x() + r.width(), r.y() + 1);
      if(col == 0) {
        start.rx()++;
      } else if(col == m_parent->lastCol()) {
        end.rx()--;
      }
      // painter->drawLine(start, end);
      painter->drawWinFocusRect(QRect(start, end));
    }
    // for the last Row, we need to paint the bottom
    if(row == numRows() - 1) {
      start = QPoint(r.x(), r.y() + r.height() - 1);
      end = QPoint(r.x() + r.width(), r.y() + r.height() - 1);
      if(col == 0) {
        start.rx()++;
      } else if(col == m_parent->lastCol()) {
        end.rx()--;
      }
      // painter->drawLine(start, end);
      painter->drawWinFocusRect(QRect(start, end));
    }
    // for the first col, we need to paint the left
    if(col == 0) {
      start = QPoint(r.x() + 1, r.y());
      end = QPoint(r.x() + 1, r.y() + r.height());
      if(row == 0) {
        start.ry()++;
      } else if(row == numRows()-1) {
        end.ry()--;
      }
      //painter->drawLine(start, end);
      painter->drawWinFocusRect(QRect(start, end));
    }
    // for the last col, we need to paint the left
    if(col == m_parent->lastCol()) {
      start = QPoint(r.x() + r.width() - 1, r.y());
      end = QPoint(r.x() + r.width() - 1, r.y() + r.height());
      if(row == 0) {
        start.ry()++;
      } else if(row == numRows()-1) {
        end.ry()--;
      }
      //painter->drawLine(start, end);
      painter->drawWinFocusRect(QRect(start, end));
    }
#endif
    if(row == 0) {
      start = QPoint(r.x(), r.y());
      end = QPoint(r.x() + r.width(), r.y() + 1);
      if(col == 0) {
        start.rx()++;
      } else if(col == m_parent->lastCol()) {
        end.rx()--;
      }
      // painter->drawLine(start, end);
      painter->drawWinFocusRect(QRect(start, end));
    }
    // for the last Row, we need to paint the bottom
    if(row == numRowsRegister() - 1) {
      start = QPoint(r.x(), r.y() + r.height() - 2);
      end = QPoint(r.x() + r.width(), r.y() + r.height() - 2);
      if(col == 0) {
        start.rx()++;
      } else if(col == m_parent->lastCol()) {
        end.rx()--;
      }
      // painter->drawLine(start, end);
      painter->drawWinFocusRect(QRect(start, end));
    }
    // for the first col, we need to paint the left
    if(col == 0) {
      start = QPoint(r.x() + 1, r.y());
      end = QPoint(r.x() + 1, r.y() + r.height());
      if(row == 0) {
        start.ry()++;
      } else if(row == numRowsRegister()-1) {
        end.ry()--;
      }
      //painter->drawLine(start, end);
      painter->drawWinFocusRect(QRect(start, end));
    }
    // for the last col, we need to paint the left
    if(col == m_parent->lastCol()) {
      start = QPoint(r.x() + r.width() - 1, r.y());
      end = QPoint(r.x() + r.width() - 1, r.y() + r.height());
      if(row == 0) {
        start.ry()++;
      } else if(row == numRowsRegister()-1) {
        end.ry()--;
      }
      //painter->drawLine(start, end);
      painter->drawWinFocusRect(QRect(start, end));
    }
    painter->setPen(oldPen);
  }
}

void Transaction::registerCellText(QString& txt, int row, int col)
{
  int align = 0;
  registerCellText(txt, align, row, col, 0);
}

void Transaction::paintRegisterCell(QPainter* painter, int row, int col, const QRect& r, bool /* selected */, const QColorGroup& _cg)
{
  QColorGroup cg(_cg);
  QRect cellRect(r);
  QRect textRect;
  QBrush backgroundBrush;

  painter->save();

  if(paintRegisterCellSetup(painter, row, col, cellRect, textRect, cg, backgroundBrush)) {
    // construct the text for the cell
    int align = Qt::AlignVCenter;
    QString txt;
    if(m_transaction != MyMoneyTransaction() && !m_inRegisterEdit) {
      registerCellText(txt, align, row, col, painter);
    }

    paintRegisterCellBackground(painter, row, col, cellRect, backgroundBrush);

    // and paint it
    paintRegisterCellText(painter, row, col, textRect, cg, align, txt);

    // paint the grid
    paintRegisterGrid(painter, row, col, cellRect, cg);

    // possible icons
    paintRegisterIcons(painter, row, col, cellRect, cg);

    // and the focus
    paintRegisterCellFocus(painter, row, col, cellRect, cg);
  }

  painter->restore();
}

void Transaction::paintRegisterIcons(QPainter* painter, int row, int col, const QRect& /*r*/, const QColorGroup& /*cg*/)
{
  if(row == 0 && col == DetailColumn && painter) {
    if(m_erronous || !m_transaction.value("kmm-attachment").isEmpty()) {
      QRect cellRect;
      cellRect.setX(0);
      cellRect.setY(0);
      cellRect.setWidth(m_parent->columnWidth(col));
      cellRect.setHeight(m_parent->rowHeight(m_startRow + row));
      if(m_erronous) {
        markAsErronous(painter, row, col, cellRect);
      }
      if(!m_transaction.value("kmm-attachment").isEmpty()) {
        markAttachment(painter, row, col, cellRect);
      }
    }
  }
}
void Transaction::paintRegisterCellBackground(QPainter* painter, int row, int col, const QRect& r, const QBrush& backgroundBrush)
{
  Q_UNUSED(row);
  Q_UNUSED(col);

  // fill the background
  painter->fillRect(r, backgroundBrush);
}

void Transaction::paintRegisterGrid(QPainter* painter, int row, int col, const QRect& r, const QColorGroup& _cg) const
{
  Q_UNUSED(_cg);

  // if a grid is selected, we paint it right away
  if (KMyMoneyGlobalSettings::showGrid()) {
    painter->setPen(KMyMoneyGlobalSettings::listGridColor());
    if(col != 0)
      painter->drawLine(r.x(), 0, r.x(), r.height()-1);
    if(row == numRowsRegister()-1)
      painter->drawLine(r.x(), r.height()-1, r.width(), r.height()-1);
  }
}

void Transaction::paintRegisterCellText(QPainter* painter, int row, int col, const QRect& r, const QColorGroup& _cg, int align, const QString& txt)
{
  Q_UNUSED(row);
  Q_UNUSED(col);
  Q_UNUSED(r);
  Q_UNUSED(_cg);

  // make sure, we clear the cell
  if(txt.isEmpty())
    painter->drawText(r, align, " ");
  else
    painter->drawText(r, align, txt);
}

int Transaction::formRowHeight(int /*row*/)
{
  if(m_formRowHeight < 0) {
    m_formRowHeight = formRowHeight();
  }
  return m_formRowHeight;
}

int Transaction::formRowHeight(void) const
{
  if(m_formRowHeight < 0) {
    // determine the height of the objects in the table
    kMyMoneyDateInput dateInput;
    KMyMoneyCategory category(0,0,true);

    return QMAX(dateInput.sizeHint().height(), category.sizeHint().height());
  }
  return m_formRowHeight;
}

void Transaction::setupForm(TransactionForm* form)
{
  m_form = form;
  form->verticalHeader()->setUpdatesEnabled(false);
  form->horizontalHeader()->setUpdatesEnabled(false);

  form->setNumRows(numRowsForm());
  form->setNumCols(numColsForm());

  // Force all cells to have some text (so that paintCell is called for each cell)
  for(int r = 0; r < numRowsForm(); ++r) {
    for(int c = 0; c < numColsForm(); ++c) {
      form->setText(r, c, "x");
      if(r == 0 && form->columnWidth(c) == 0) {
        form->setColumnWidth(c, 10);
      }
    }
  }
  form->horizontalHeader()->setUpdatesEnabled(true);
  form->verticalHeader()->setUpdatesEnabled(true);

  loadTab(form);
}

void Transaction::paintFormCell(QPainter* painter, int row, int col, const QRect& /*r*/, bool /*selected*/, const QColorGroup& _cg)
{
  if(!m_form)
    return;

  QRect cellRect = m_form->cellRect(row, col);

  QRect textRect(cellRect);
  textRect.setX(1);
  textRect.setY(1);
  textRect.setWidth(textRect.width()-2);
  textRect.setHeight(textRect.height()-2);

  painter->fillRect(cellRect, _cg.background());
  painter->setPen(_cg.text());

  QString txt;
  int align = Qt::AlignVCenter;
  bool editField = formCellText(txt, align, row, col, painter);

  // if we have an editable field and don't currently edit the transaction
  // show the background in a different color
  if(editField && !m_inEdit) {
    painter->fillRect(textRect, _cg.base());
  }

  // make sure, we clear the cell
  // in case of an editable field and edit mode, we just clear the field
  if(txt.isEmpty() || (editField && m_inEdit))
    painter->drawText(textRect, align, " ");
  else
    painter->drawText(textRect, align, txt);

}

void Transaction::setupPalette(const QPalette& palette, QMap<QString, QWidget*>& editWidgets)
{
  QMap<QString, QWidget*>::iterator it_w;
  for(it_w = editWidgets.begin(); it_w != editWidgets.end(); ++it_w) {
    if(*it_w) {
      (*it_w)->setPalette(palette);
    }
  }
}

void Transaction::setupFormPalette(QMap<QString, QWidget*>& editWidgets)
{
  setupPalette(m_parent->palette(), editWidgets);
}

void Transaction::setupRegisterPalette(QMap<QString, QWidget*>& editWidgets)
{
  // make sure, we're using the right palette
  QPalette palette = m_parent->palette();

  // use the highlight color as background
  palette.setColor(QPalette::Active, QColorGroup::Background, palette.color(QPalette::Active, QColorGroup::Highlight));

  setupPalette(palette, editWidgets);
}

QWidget* Transaction::focusWidget(QWidget* w) const
{
  if(w) {
    while(w->focusProxy())
      w = w->focusProxy();
  }
  return w;
}

void Transaction::arrangeWidget(QTable* tbl, int row, int col, QWidget* w) const
{
  if(w) {
    tbl->setCellWidget(row, col, w);
    // remove the widget from the QTable's eventFilter so that all
    // events will be directed to the edit widget
    w->removeEventFilter(tbl);
  } else
    qDebug("No widget for %d,%d", row, col);
}

bool Transaction::haveNumberField(void) const
{
  bool rc = true;
  switch(m_account.accountType()) {
    case MyMoneyAccount::Savings:
    case MyMoneyAccount::Cash:
    case MyMoneyAccount::Loan:
    case MyMoneyAccount::AssetLoan:
    case MyMoneyAccount::Asset:
    case MyMoneyAccount::Liability:
    case MyMoneyAccount::Equity:
      rc = KMyMoneyGlobalSettings::alwaysShowNrField();
      break;

    case MyMoneyAccount::Checkings:
    case MyMoneyAccount::CreditCard:
    // the next case is used for the editor when the account
    // is unknown (eg. when creating new schedules)
    case MyMoneyAccount::UnknownAccountType:
      break;

    default:
      rc = false;
      break;
  }
  return rc;
}

bool Transaction::maybeTip(const QPoint& cpos, int row, int col, QRect& r, QString& msg)
{
  if(col != DetailColumn)
    return false;

  if(!m_erronous && m_transaction.splitCount() < 3)
    return false;

  int h = m_parent->rowHeightHint();

  // check for detail column in row 0 of the transaction for a possible exclamation mark
  r = m_parent->cellGeometry(m_startRow + 0, col);
  // qDebug("r is %d,%d,%d,%d", r.x(), r.y(), r.width(), r.height());
  r.setBottomLeft(QPoint(r.x() + (r.width() - h), r.y() + h));
  // qDebug("r is %d,%d,%d,%d", r.x(), r.y(), r.width(), r.height());
  // qDebug("p is in r = %d", r.contains(cpos));
  if(r.contains(cpos) && m_erronous) {
    if(m_transaction.splits().count() < 2) {
      msg = QString("<qt>%1</qt>").arg(i18n("Transaction is missing a category assignment."));
    } else {
      const MyMoneySecurity& sec = MyMoneyFile::instance()->security(m_account.currencyId());
      msg = QString("<qt>%1</qt>").arg(i18n("The transaction has a missing assignment of <b>%1</b>.").arg(m_transaction.splitSum().abs().formatMoney(m_account, sec)));
    }
    return true;
  }

  // check for detail column in row 1 of the transaction for a possible exclamation mark
  r = m_parent->cellGeometry(m_startRow + 1, col);
  if(row == 1 && r.contains(cpos) && m_transaction.splitCount() > 2) {
    MyMoneyFile* file = MyMoneyFile::instance();
    QValueList<MyMoneySplit>::const_iterator it_s;
    QString txt;
    const MyMoneySecurity& sec = file->security(m_transaction.commodity());
    MyMoneyMoney factor(1, 1);
    if(!m_split.value().isNegative())
      factor = -factor;

    for(it_s = m_transaction.splits().begin(); it_s != m_transaction.splits().end(); ++it_s) {
      if(*it_s == m_split)
        continue;
      const MyMoneyAccount& acc = file->account((*it_s).accountId());
      QString category = file->accountToCategory(acc.id());
      QString amount = ((*it_s).value() * factor).formatMoney(acc, sec);

      txt += QString("<tr><td><nobr>%1</nobr></td><td align=right><nobr>%2</nobr></td></tr>").arg(category, amount);
    }
    msg = QString("<table>%1</table>").arg(txt);
    return true;
  }

  return false;
}

QString Transaction::reconcileState(bool text) const
{
  QString txt = KMyMoneyUtils::reconcileStateToString(m_split.reconcileFlag(), text);

  if((text == true)
  && (txt == i18n("Unknown"))
  && (m_transaction == MyMoneyTransaction()))
    txt = QString();
  return txt;
}

void Transaction::startEditMode(void)
{
  m_inEdit = true;
  // only update the number of lines displayed if we edit inside the register
  if(m_inRegisterEdit)
    setNumRowsRegister(numRowsRegister(true));
}

void Transaction::leaveEditMode(void)
{
  m_inEdit = false;
  m_inRegisterEdit = false;
  setFocus(hasFocus(), true);
}

void Transaction::singleLineMemo(QString& txt, const MyMoneySplit& split) const
{
  txt = split.memo();
  // remove empty lines
  txt.replace("\n\n", "\n");
  // replace '\n' with ", "
  txt.replace('\n', ", ");
}

int Transaction::rowHeightHint(void) const
{
  return m_inEdit ? formRowHeight()-4 : RegisterItem::rowHeightHint();
}


bool Transaction::matches(const QString& txt) const
{
  if(txt.isEmpty() || m_transaction.splitCount() == 0)
    return true;

  MyMoneyFile* file = MyMoneyFile::instance();
  QString s(txt);
  s.replace(MyMoneyMoney::thousandSeparator(), QString());

  const QValueList<MyMoneySplit>&list = m_transaction.splits();
  QValueList<MyMoneySplit>::const_iterator it_s;
  for(it_s = list.begin(); it_s != list.end(); ++it_s) {
    // check if the text is contained in one of the fields
    // memo, number, payee, account
    if((*it_s).memo().contains(txt, false)
       || (*it_s).number().contains(txt, false))
      return true;

    if(!(*it_s).payeeId().isEmpty()) {
      const MyMoneyPayee& payee = file->payee((*it_s).payeeId());
      if(payee.name().contains(txt, false))
        return true;
    }
    const MyMoneyAccount& acc = file->account((*it_s).accountId());
    if(acc.name().contains(txt, false))
      return true;

    if(!s.isEmpty()) {
      // check if any of the value field matches if a value has been entered
      QString r = (*it_s).value().formatMoney(m_account.fraction(), false);
      if(r.contains(s, false))
        return true;
      const MyMoneyAccount& acc = file->account((*it_s).accountId());
      r = (*it_s).shares().formatMoney(acc.fraction(), false);
      if(r.contains(s, false))
        return true;
    }
  }

  return false;
}

void Transaction::setShowBalance(bool showBalance)
{
  m_showBalance = showBalance;
}

void Transaction::setVisible(bool visible)
{
  if(visible != isVisible()) {
    RegisterItem::setVisible(visible);
    RegisterItem* p;
    Transaction* t;
    if(!visible) {
      // if we are hidden, we need to inform all previous transactions
      // about it so that they don't show the balance
      p = prevItem();
      while(p) {
        t = dynamic_cast<Transaction*>(p);
        if(t) {
          if(!t->m_showBalance)
            break;
          t->m_showBalance = false;
        }
        p = p->prevItem();
      }
    } else {
      // if we are shown, we need to check if the next transaction
      // is visible and change the display of the balance
      p = this;
      do {
        p = p->nextItem();
        t = dynamic_cast<Transaction*>(p);
      } while(!t && p);

      // if the next transaction is visible or I am the last one
      if((t && t->m_showBalance) || !t) {
        m_showBalance = true;
        p = prevItem();
        while(p && p->isVisible()) {
          t = dynamic_cast<Transaction*>(p);
          if(t) {
            if(t->m_showBalance)
              break;
            t->m_showBalance = true;
          }
          p = p->prevItem();
        }
      }
    }
  }
}

void Transaction::setSelected(bool selected)
{
  if(!selected || (selected && isVisible()))
    m_selected = selected;
}

StdTransaction::StdTransaction(Register *parent, const MyMoneyTransaction& transaction, const MyMoneySplit& split, int uniqueId) :
  Transaction(parent, transaction, split, uniqueId),
  m_showAccountRow(false)
{
  try {
    m_categoryHeader = i18n("Category");
    switch(transaction.splitCount()) {
      default:
        m_category = i18n("Split transaction (category replacement)", "Split transaction");
        break;

      case 0: // the empty transaction
      case 1:
        break;

      case 2:
        setupFormHeader(m_transaction.splitByAccount(m_split.accountId(), false).accountId());
        break;
    }
  } catch(MyMoneyException *e) {
    kdDebug(2) << "Problem determining the category for transaction '" << m_transaction.id() << "'. Reason: " << e->what()  << "\n";
    delete e;
  }
  m_rowsForm = 6;

  if(KMyMoneyUtils::transactionType(m_transaction) == KMyMoneyUtils::InvestmentTransaction) {
    MyMoneySplit split = KMyMoneyUtils::stockSplit(m_transaction);
    m_payee = MyMoneyFile::instance()->account(split.accountId()).name();
    QString addon;
    if(split.action() == MyMoneySplit::ActionBuyShares) {
      if(split.value().isNegative()) {
        addon = i18n("Sell");
      } else {
        addon = i18n("Buy");
      }
    } else if(split.action() == MyMoneySplit::ActionDividend) {
        addon = i18n("Dividend");
    } else if(split.action() == MyMoneySplit::ActionYield) {
        addon = i18n("Yield");
    }
    if(!addon.isEmpty()) {
      m_payee += QString(" (%1)").arg(addon);
    }
    m_payeeHeader = i18n("Activity");
    m_category = i18n("Investment transaction");
  }

  // setup initial size
  setNumRowsRegister(numRowsRegister(KMyMoneyGlobalSettings::showRegisterDetailed()));

  emit parent->itemAdded(this);
}

void StdTransaction::setupFormHeader(const QString& id)
{
  m_category = MyMoneyFile::instance()->accountToCategory(id);
  switch(MyMoneyFile::instance()->account(id).accountGroup()) {
    case MyMoneyAccount::Asset:
    case MyMoneyAccount::Liability:
      m_categoryHeader = m_split.shares().isNegative() ? i18n("Transfer to") : i18n("Transfer from");
      break;

    default:
      m_categoryHeader = i18n("Category");
      break;
  }
}

KMyMoneyRegister::Action StdTransaction::actionType(void) const
{
  KMyMoneyRegister::Action action=ActionNone;

  // if at least one split is referencing an income or
  // expense account, we will not call it a transfer
  QValueList<MyMoneySplit>::const_iterator it_s;

  for(it_s = m_transaction.splits().begin(); it_s != m_transaction.splits().end(); ++it_s) {
    if((*it_s).accountId() == m_split.accountId())
      continue;
    MyMoneyAccount acc = MyMoneyFile::instance()->account((*it_s).accountId());
    if(acc.accountGroup() == MyMoneyAccount::Income
    || acc.accountGroup() == MyMoneyAccount::Expense) {
      // otherwise, we have to determine between deposit and withdrawal
      action = m_split.shares().isNegative() ? ActionWithdrawal : ActionDeposit;
      break;
    }
  }
    // otherwise, it's a transfer
  if(it_s == m_transaction.splits().end())
    action = ActionTransfer;

  return action;
}

void StdTransaction::loadTab(TransactionForm* form)
{
  TabBar* bar = form->tabBar();
  bar->setSignalEmission(TabBar::SignalNever);
  for(int i = 0; i < bar->count(); ++i) {
    bar->setTabEnabled(bar->tabAt(i)->identifier(), true);
  }

  if(m_transaction.splitCount() > 0) {
    bar->setCurrentTab(actionType());
  }
  bar->setSignalEmission(TabBar::SignalAlways);
}

void StdTransaction::setupForm(TransactionForm* form)
{
  Transaction::setupForm(form);

  QTableItem* memo = form->item(3, ValueColumn1);
  memo->setSpan(3, 1);
}

bool StdTransaction::showRowInForm(int row) const
{
  return row == 0 ? m_showAccountRow : true;
}

void StdTransaction::setShowRowInForm(int row, bool show)
{
  if(row == 0)
    m_showAccountRow = show;
}

bool StdTransaction::formCellText(QString& txt, int& align, int row, int col, QPainter* /* painter */)
{
  // if(m_transaction != MyMoneyTransaction()) {
    switch(row) {
      case 0:
        switch(col) {
          case LabelColumn1:
            align |= Qt::AlignLeft;
            txt = i18n("Account");
            break;
        }
        break;

      case 1:
        switch(col) {
          case LabelColumn1:
            align |= Qt::AlignLeft;
            txt = m_payeeHeader;
            break;

          case ValueColumn1:
            align |= Qt::AlignLeft;
            txt = m_payee;
            break;

          case LabelColumn2:
            align |= Qt::AlignLeft;
            if(haveNumberField())
              txt = i18n("Number");
            break;

          case ValueColumn2:
            align |= Qt::AlignRight;
            if(haveNumberField())
              txt = m_split.number();
            break;
        }
        break;

      case 2:
        switch(col) {
          case LabelColumn1:
            align |= Qt::AlignLeft;
            txt = m_categoryHeader;
            break;

          case ValueColumn1:
            align |= Qt::AlignLeft;
            txt = m_category;
            if(m_transaction != MyMoneyTransaction()) {
              if(txt.isEmpty() && !m_split.value().isZero())
                txt = i18n("*** UNASSIGNED ***");
            }
            break;

          case LabelColumn2:
            align |= Qt::AlignLeft;
            txt = i18n("Date");
            break;

          case ValueColumn2:
            align |= Qt::AlignRight;
            if(m_transaction != MyMoneyTransaction())
              txt = KGlobal::locale()->formatDate(m_transaction.postDate(), true);
            break;
        }
        break;

      case 3:
        switch(col) {
          case LabelColumn1:
            align |= Qt::AlignLeft;
            txt = i18n("Memo");
            break;

          case ValueColumn1:
            align &= ~Qt::AlignVCenter;
            align |= Qt::AlignTop;
            align |= Qt::AlignLeft;
            if(m_transaction != MyMoneyTransaction())
              txt = m_split.memo().section('\n', 0, 2);
            break;

          case LabelColumn2:
            align |= Qt::AlignLeft;
            txt = i18n("Amount");
            break;

          case ValueColumn2:
            align |= Qt::AlignRight;
            if(m_transaction != MyMoneyTransaction()) {
              txt = (m_split.value(m_transaction.commodity(), m_splitCurrencyId).abs()).formatMoney(m_account.fraction());
            }
            break;
        }
        break;

      case 5:
        switch(col) {
          case LabelColumn2:
            align |= Qt::AlignLeft;
            txt = i18n("Status");
            break;

          case ValueColumn2:
            align |= Qt::AlignRight;
            txt = reconcileState();
            break;
        }
    }
  // }
  if(col == ValueColumn2 && row == 1) {
    return haveNumberField();
  }
  return (col == ValueColumn1 && row < 4) || (col == ValueColumn2 && row > 0 && row != 4);
}

void StdTransaction::registerCellText(QString& txt, int& align, int row, int col, QPainter* painter)
{
  switch(row) {
    case 0:
      switch(col) {
        case NumberColumn:
          align |= Qt::AlignLeft;
          if(haveNumberField())
            txt = m_split.number();
          break;

        case DateColumn:
          align |= Qt::AlignLeft;
          txt = KGlobal::locale()->formatDate(m_transaction.postDate(), true);
          break;

        case DetailColumn:
          align |= Qt::AlignLeft;
          txt = m_payee;
          if(txt.isEmpty() && m_rowsRegister < 3) {
            singleLineMemo(txt, m_split);
          }
          if(txt.isEmpty() && m_rowsRegister < 2) {
            if(m_account.accountType() != MyMoneyAccount::Income
            && m_account.accountType() != MyMoneyAccount::Expense) {
              txt = m_category;
              if(txt.isEmpty() && !m_split.value().isZero()) {
                txt = i18n("*** UNASSIGNED ***");
                if(painter)
                  painter->setPen(KMyMoneyGlobalSettings::listErronousTransactionColor());
              }
            }
          }
          break;

        case ReconcileFlagColumn:
          align |= Qt::AlignHCenter;
          txt = reconcileState(false);
          break;

        case PaymentColumn:
          align |= Qt::AlignRight;
          if(m_split.value().isNegative()) {
            txt = (-m_split.value(m_transaction.commodity(), m_splitCurrencyId)).formatMoney(m_account.fraction());
          }
          break;

        case DepositColumn:
          align |= Qt::AlignRight;
          if(!m_split.value().isNegative()) {
            txt = m_split.value(m_transaction.commodity(), m_splitCurrencyId).formatMoney(m_account.fraction());
          }
          break;

        case BalanceColumn:
          align |= Qt::AlignRight;
          if(m_showBalance)
            txt = m_balance.formatMoney(m_account.fraction());
          else
            txt = "----";
          break;

        case AccountColumn:
          // txt = m_objects->account(m_transaction.splits()[0].accountId()).name();
          txt = MyMoneyFile::instance()->account(m_split.accountId()).name();
          break;

        default:
          break;
      }
      break;

    case 1:
      switch(col) {
        case DetailColumn:
          align |= Qt::AlignLeft;
          txt = m_category;
          if(txt.isEmpty() && !m_split.value().isZero()) {
            txt = i18n("*** UNASSIGNED ***");
            if(painter)
              painter->setPen(KMyMoneyGlobalSettings::listErronousTransactionColor());
          }
          break;

        default:
          break;
      }
      break;

    case 2:
      switch(col) {
        case DetailColumn:
          align |= Qt::AlignLeft;
          singleLineMemo(txt, m_split);
          break;

        default:
          break;
      }
      break;
  }
}

int StdTransaction::registerColWidth(int col, const QFontMetrics& cellFontMetrics)
{
  QString txt;
  int firstRow = 0, lastRow = 0;

  int nw = 0;
  for(int i = firstRow; i <= lastRow; ++i) {
    int align;
    registerCellText(txt, align, i, col, 0);
    int w = cellFontMetrics.width(txt+"   ");
    if(w > nw)
      nw = w;
  }
  return nw;
}

void StdTransaction::arrangeWidgetsInForm(QMap<QString, QWidget*>& editWidgets)
{
  if(!m_form || !m_parent)
    return;

  setupFormPalette(editWidgets);

  arrangeWidget(m_form, 0, ValueColumn1, editWidgets["account"]);
  arrangeWidget(m_form, 1, LabelColumn1, editWidgets["cashflow"]);
  arrangeWidget(m_form, 1, ValueColumn1, editWidgets["payee"]);
  arrangeWidget(m_form, 2, ValueColumn1, editWidgets["category"]->parentWidget());
  arrangeWidget(m_form, 3, ValueColumn1, editWidgets["memo"]);
  if(haveNumberField()) {
    arrangeWidget(m_form, 1, LabelColumn2, editWidgets["number-label"]);
    arrangeWidget(m_form, 1, ValueColumn2, editWidgets["number"]);
  }
  arrangeWidget(m_form, 2, LabelColumn2, editWidgets["date-label"]);
  arrangeWidget(m_form, 2, ValueColumn2, editWidgets["postdate"]);
  arrangeWidget(m_form, 3, ValueColumn2, editWidgets["amount"]);
  arrangeWidget(m_form, 5, ValueColumn2, editWidgets["status"]);
  arrangeWidget(m_form, 2, LabelColumn1, editWidgets["category-label"]);

  // get rid of the hints. we don't need them for the form
  QMap<QString, QWidget*>::iterator it;
  for(it = editWidgets.begin(); it != editWidgets.end(); ++it) {
    KMyMoneyCombo* combo = dynamic_cast<KMyMoneyCombo*>(*it);
    kMyMoneyLineEdit* edit = dynamic_cast<kMyMoneyLineEdit*>(*it);
    KMyMoneyPayeeCombo* payee = dynamic_cast<KMyMoneyPayeeCombo*>(*it);
    if(combo)
      combo->setHint(QString());
    if(edit)
      edit->setHint(QString());
    if(payee)
      payee->setHint(QString());
  }

  // drop the tabbar on top of the original
  KMyMoneyTransactionForm::TransactionForm* form = dynamic_cast<KMyMoneyTransactionForm::TransactionForm*>(m_form);
  TabBar* w = dynamic_cast<TabBar*>(editWidgets["tabbar"]);
  if(w) {
    w->reparent(form->tabBar(), QPoint(0, 0), true);
  }
}

void StdTransaction::tabOrderInForm(QWidgetList& tabOrderWidgets) const
{
  // account
  tabOrderWidgets.append(focusWidget(m_form->cellWidget(0, ValueColumn1)));

  // cashflow direction
  tabOrderWidgets.append(focusWidget(m_form->cellWidget(1, LabelColumn1)));
  // payee
  tabOrderWidgets.append(focusWidget(m_form->cellWidget(1, ValueColumn1)));
  // make sure to have the category field and the split button as seperate tab order widgets
  // ok, we have to have some internal knowledge about the KMyMoneyCategory object, but
  // it's one of our own widgets, so we actually don't care. Just make sure, that we don't
  // go haywire when someone changes the KMyMoneyCategory object ...
  QWidget* w = m_form->cellWidget(2, ValueColumn1);
  tabOrderWidgets.append(focusWidget(w));
  w = dynamic_cast<QWidget*>(w->child("splitButton"));
  if(w)
    tabOrderWidgets.append(w);
  // memo
  tabOrderWidgets.append(focusWidget(m_form->cellWidget(3, ValueColumn1)));
  // number
  if(haveNumberField()) {
    if((w = focusWidget(m_form->cellWidget(1, ValueColumn2))))
      tabOrderWidgets.append(w);
  }
  // date
  tabOrderWidgets.append(focusWidget(m_form->cellWidget(2, ValueColumn2)));
  // amount
  tabOrderWidgets.append(focusWidget(m_form->cellWidget(3, ValueColumn2)));
  // state
  tabOrderWidgets.append(focusWidget(m_form->cellWidget(5, ValueColumn2)));
}

void StdTransaction::arrangeWidgetsInRegister(QMap<QString, QWidget*>& editWidgets)
{
  if(!m_parent)
    return;

  setupRegisterPalette(editWidgets);

  if(haveNumberField())
    arrangeWidget(m_parent, m_startRow+0, NumberColumn, editWidgets["number"]);
  arrangeWidget(m_parent, m_startRow + 0, DateColumn, editWidgets["postdate"]);
  arrangeWidget(m_parent, m_startRow + 1, DateColumn, editWidgets["status"]);
  arrangeWidget(m_parent, m_startRow + 0, DetailColumn, editWidgets["payee"]);
  arrangeWidget(m_parent, m_startRow + 1, DetailColumn, editWidgets["category"]->parentWidget());
  arrangeWidget(m_parent, m_startRow + 2, DetailColumn, editWidgets["memo"]);
  arrangeWidget(m_parent, m_startRow + 0, PaymentColumn, editWidgets["payment"]);
  arrangeWidget(m_parent, m_startRow + 0, DepositColumn, editWidgets["deposit"]);

  // increase the height of the row containing the memo widget
  m_parent->setRowHeight(m_startRow+2, m_parent->rowHeightHint() * 3);
}

void StdTransaction::tabOrderInRegister(QWidgetList& tabOrderWidgets) const
{
  QWidget* w;
  // number
  if(haveNumberField()) {
    if((w = focusWidget(m_parent->cellWidget(m_startRow + 0, NumberColumn))))
      tabOrderWidgets.append(w);
  }

  // date
  tabOrderWidgets.append(focusWidget(m_parent->cellWidget(m_startRow + 0, DateColumn)));
  // payee
  tabOrderWidgets.append(focusWidget(m_parent->cellWidget(m_startRow + 0, DetailColumn)));
  // make sure to have the category field and the split button as seperate tab order widgets
  // ok, we have to have some internal knowledge about the KMyMoneyCategory object, but
  // it's one of our own widgets, so we actually don't care. Just make sure, that we don't
  // go haywire when someone changes the KMyMoneyCategory object ...
  w = m_parent->cellWidget(m_startRow + 1, DetailColumn);
  tabOrderWidgets.append(focusWidget(w));
  w = dynamic_cast<QWidget*>(w->child("splitButton"));
  if(w)
    tabOrderWidgets.append(w);
  // memo
  tabOrderWidgets.append(focusWidget(m_parent->cellWidget(m_startRow + 2, DetailColumn)));
  // payment
  tabOrderWidgets.append(focusWidget(m_parent->cellWidget(m_startRow + 0, PaymentColumn)));
  // deposit
  tabOrderWidgets.append(focusWidget(m_parent->cellWidget(m_startRow + 0, DepositColumn)));
  // status
  tabOrderWidgets.append(focusWidget(m_parent->cellWidget(m_startRow + 1, DateColumn)));
}

int StdTransaction::numRowsRegister(bool expanded) const
{
  int numRows = 1;
  if(expanded) {
    numRows = 3;
    if(!m_inEdit) {
      if(m_payee.isEmpty()) {
        numRows--;
      }
      if(m_split.memo().isEmpty()) {
        numRows--;
      }
      // For income and expense accounts that only have
      // two splits we only show one line, because the
      // account name is already contained in the account column.
      if(m_account.accountType() == MyMoneyAccount::Income
      || m_account.accountType() == MyMoneyAccount::Expense) {
        if(numRows > 2 && m_transaction.splitCount() == 2)
          numRows = 1;
      }
    }
  }
  return numRows;
}

TransactionEditor* StdTransaction::createEditor(TransactionEditorContainer* regForm, const KMyMoneyRegister::SelectedTransactions& list, const QDate& lastPostDate)
{
  m_inRegisterEdit = regForm == m_parent;
  return new StdTransactionEditor(regForm, this, list, lastPostDate);
}

InvestTransaction::InvestTransaction(Register *parent, const MyMoneyTransaction& transaction, const MyMoneySplit& split, int uniqueId) :
  Transaction(parent, transaction, split, uniqueId)
{
  // dissect the transaction into its type, splits, currency, security etc.
  InvestTransactionEditor::dissectTransaction(m_transaction, m_split,
                     m_assetAccountSplit,
                     m_feeSplits,
                     m_interestSplits,
                     m_security,
                     m_currency,
                     m_transactionType);

  QValueList<MyMoneySplit>::ConstIterator it_s;
  for(it_s = m_feeSplits.begin(); it_s != m_feeSplits.end(); ++it_s) {
    m_feeAmount += (*it_s).value();
  }
  for(it_s = m_interestSplits.begin(); it_s != m_interestSplits.end(); ++it_s) {
    m_interestAmount += (*it_s).value();
  }

  // check the count of the fee splits and setup the text
  switch(m_feeSplits.count()) {
    case 0:
      break;

    case 1:
      m_feeCategory = MyMoneyFile::instance()->accountToCategory(m_feeSplits[0].accountId());
      break;

    default:
      m_feeCategory = i18n("Split transaction (category replacement)", "Split transaction");
      break;
  }

  // check the count of the interest splits and setup the text
  switch(m_interestSplits.count()) {
    case 0:
      break;

    case 1:
      m_interestCategory = MyMoneyFile::instance()->accountToCategory(m_interestSplits[0].accountId());
      break;

    default:
      m_interestCategory = i18n("Split transaction (category replacement)", "Split transaction");
      break;
  }

  m_rowsForm = 7;

  // setup initial size
  setNumRowsRegister(numRowsRegister(KMyMoneyGlobalSettings::showRegisterDetailed()));

  emit parent->itemAdded(this);
}

void InvestTransaction::setupForm(TransactionForm* form)
{
  Transaction::setupForm(form);

  QTableItem* memo = form->item(5, 1);
  memo->setSpan(2, 1);
}

void InvestTransaction::activity(QString& txt, MyMoneySplit::investTransactionTypeE type) const
{
  switch(type) {
    case MyMoneySplit::AddShares:
      txt = i18n("Add shares");
      break;
    case MyMoneySplit::RemoveShares:
      txt = i18n("Remove shares");
      break;
    case MyMoneySplit::BuyShares:
      txt = i18n("Buy shares");
      break;
    case MyMoneySplit::SellShares:
      txt = i18n("Sell shares");
      break;
    case MyMoneySplit::Dividend:
      txt = i18n("Dividend");
      break;
    case MyMoneySplit::ReinvestDividend:
      txt = i18n("Reinvest Dividend");
      break;
    case MyMoneySplit::Yield:
      txt = i18n("Yield");
      break;
    case MyMoneySplit::SplitShares:
      txt = i18n("Split shares");
      break;
    default:
      txt = i18n("Unknown");
      break;
  }
}

bool InvestTransaction::formCellText(QString& txt, int& align, int row, int col, QPainter* /* painter */)
{
  bool fieldEditable = false;

  switch(row) {
    case 0:
      switch(col) {
        case LabelColumn1:
          align |= Qt::AlignLeft;
          txt = i18n("Activity");
          break;

        case ValueColumn1:
          align |= Qt::AlignLeft;
          fieldEditable = true;
          activity(txt, m_transactionType);
          break;

        case LabelColumn2:
          align |= Qt::AlignLeft;
          txt = i18n("Date");
          break;

        case ValueColumn2:
          align |= Qt::AlignRight;
          fieldEditable = true;
          if(m_transaction != MyMoneyTransaction())
            txt = KGlobal::locale()->formatDate(m_transaction.postDate(), true);
          break;
      }
      break;

    case 1:
      switch(col) {
        case LabelColumn1:
          align |= Qt::AlignLeft;
          txt = i18n("Security");
          break;

        case ValueColumn1:
          align |= Qt::AlignLeft;
          fieldEditable = true;
          if(m_account.isInvest())
            txt = m_security.name();
          break;

        case LabelColumn2:
          align |= Qt::AlignLeft;
          if(haveShares()) {
            txt = i18n("Shares");
          } else if(haveSplitRatio()) {
            txt = i18n("Ratio");
          }
          break;

        case ValueColumn2:
          align |= Qt::AlignRight;
          if((fieldEditable = haveShares()) == true) {
            txt = m_split.shares().abs().formatMoney("", MyMoneyMoney::denomToPrec(m_security.smallestAccountFraction()));
          } else if(haveSplitRatio()) {
            txt = QString("1 / %1").arg(m_split.shares().abs().formatMoney("", -1));
          }
          break;
      }
      break;

    case 2:
      switch(col) {
        case LabelColumn1:
          align |= Qt::AlignLeft;
          if(haveAssetAccount())
            txt = i18n("Account");
          break;

        case ValueColumn1:
          align |= Qt::AlignLeft;
          if((fieldEditable = haveAssetAccount()) == true) {
            txt = MyMoneyFile::instance()->accountToCategory(m_assetAccountSplit.accountId());
          }
          break;

        case LabelColumn2:
          align |= Qt::AlignLeft;
          if(havePrice())
            txt = i18n("Price/share");
          break;

        case ValueColumn2:
          align |= Qt::AlignRight;
          if((fieldEditable = havePrice()) == true && !m_split.shares().isZero()) {
            txt = m_split.price().formatMoney("", KMyMoneyGlobalSettings::pricePrecision());
          }
          break;
      }
      break;

    case 3:
      switch(col) {
        case LabelColumn1:
          align |= Qt::AlignLeft;
          if(haveFees())
            txt = i18n("Fees");
          break;

        case ValueColumn1:
          align |= Qt::AlignLeft;
          if((fieldEditable = haveFees()) == true) {
            txt = m_feeCategory;
          }
          break;

        case LabelColumn2:
          align |= Qt::AlignLeft;
          if(haveFees() && !m_feeCategory.isEmpty())
            txt = i18n("Amount");
          break;

        case ValueColumn2:
          align |= Qt::AlignRight;
          if(haveFees()) {
            if((fieldEditable = !m_feeCategory.isEmpty()) == true) {
              txt = m_feeAmount.abs().formatMoney(m_currency);
            }
          }
          break;
      }
      break;

    case 4:
      switch(col) {
        case LabelColumn1:
          align |= Qt::AlignLeft;
          if(haveInterest())
            txt = i18n("Interest");
          break;

        case ValueColumn1:
          align |= Qt::AlignLeft;
          if((fieldEditable = haveInterest()) == true) {
            txt = m_interestCategory;
          }
          break;

        case LabelColumn2:
          align |= Qt::AlignLeft;
          if(haveInterest() && !m_interestCategory.isEmpty())
            txt = i18n("Amount");
          break;

        case ValueColumn2:
          align |= Qt::AlignRight;
          if(haveInterest()) {
            if((fieldEditable = !m_interestCategory.isEmpty()) == true) {
              txt = m_interestAmount.abs().formatMoney(m_currency);
            }
          }
          break;
      }
      break;

    case 5:
      switch(col) {
        case LabelColumn1:
          align |= Qt::AlignLeft;
          txt = i18n("Memo");
          break;

        case ValueColumn1:
          align &= ~Qt::AlignVCenter;
          align |= Qt::AlignTop;
          align |= Qt::AlignLeft;
          fieldEditable = true;
          if(m_transaction != MyMoneyTransaction())
            txt = m_split.memo().section('\n', 0, 2);
          break;

        case LabelColumn2:
          align |= Qt::AlignLeft;
          if(haveAmount())
            txt = i18n("Total");
          break;

        case ValueColumn2:
          align |= Qt::AlignRight;
          if((fieldEditable = haveAmount()) == true) {
            txt = m_assetAccountSplit.value().abs().formatMoney(m_currency);
          }
      }
      break;

    case 6:
      switch(col) {
        case LabelColumn2:
          align |= Qt::AlignLeft;
          txt = i18n("Status");
          break;

        case ValueColumn2:
          align |= Qt::AlignRight;
          fieldEditable = true;
          txt = reconcileState();
          break;
      }
  }

  return fieldEditable;
}

void InvestTransaction::registerCellText(QString& txt, int& align, int row, int col, QPainter* /* painter */)
{
  switch(row) {
    case 0:
      switch(col) {
        case DateColumn:
          align |= Qt::AlignLeft;
          txt = KGlobal::locale()->formatDate(m_transaction.postDate(), true);
          break;

        case DetailColumn:
          align |= Qt::AlignLeft;
          activity(txt, m_transactionType);
          break;

        case SecurityColumn:
          align |= Qt::AlignLeft;
          if(m_account.isInvest())
            txt = m_security.name();
          break;

        case ReconcileFlagColumn:
          align |= Qt::AlignHCenter;
          txt = reconcileState(false);
          break;

        case QuantityColumn:
          align |= Qt::AlignRight;
          if(haveShares())
            txt = m_split.shares().abs().formatMoney("", MyMoneyMoney::denomToPrec(m_security.smallestAccountFraction()));
          else if(haveSplitRatio()) {
            txt = QString("1 / %1").arg(m_split.shares().abs().formatMoney("", -1));
          }
          break;

        case PriceColumn:
          align |= Qt::AlignRight;
          if(havePrice() && !m_split.shares().isZero()) {
            txt = m_split.price().formatMoney(m_currency.tradingSymbol(), KMyMoneyGlobalSettings::pricePrecision());
          }
          break;

        case ValueColumn:
          align |= Qt::AlignRight;
          if(haveAmount()) {
            txt = m_assetAccountSplit.value().abs().formatMoney(m_currency);

          } else if(haveInterest()) {
            txt = m_interestAmount.abs().formatMoney(m_currency);
          }
          break;

        case BalanceColumn:
          align |= Qt::AlignRight;
          if(m_showBalance)
            txt = m_balance.formatMoney("", MyMoneyMoney::denomToPrec(m_security.smallestAccountFraction()));
          else
            txt = "----";
          break;

        default:
          break;
      }
      break;

    case 1:
      switch(col) {
        case DetailColumn:
          align |= Qt::AlignLeft;
          if(haveAssetAccount() && !m_assetAccountSplit.accountId().isEmpty()) {
            txt = MyMoneyFile::instance()->accountToCategory(m_assetAccountSplit.accountId());
          } else if(haveInterest() && m_interestSplits.count()) {
            txt = m_interestCategory;
          } else if(haveFees() && m_feeSplits.count()) {
            txt = m_feeCategory;
          } else
            singleLineMemo(txt, m_split);
          break;

        case QuantityColumn:
          align |= Qt::AlignRight;
          if(haveAssetAccount() && !m_assetAccountSplit.accountId().isEmpty()) {
            // txt = m_interestAmount.abs().formatMoney(m_currency);
          } else if(haveInterest() && m_interestSplits.count()) {
            txt = m_interestAmount.abs().formatMoney(m_currency);
          } else if(haveFees() && m_feeSplits.count()) {
            txt = m_feeAmount.abs().formatMoney(m_currency);
          }
          break;

        default:
          break;
      }
      break;

    case 2:
      switch(col) {
        case DetailColumn:
          align |= Qt::AlignLeft;
          if(haveAssetAccount() && !m_assetAccountSplit.accountId().isEmpty()
          && haveInterest() && m_interestSplits.count()) {
            txt = m_interestCategory;
          } else if(haveFees() && m_feeSplits.count()) {
            txt = m_feeCategory;
          } else
            singleLineMemo(txt, m_split);
          break;

        case QuantityColumn:
          align |= Qt::AlignRight;
          if(haveAssetAccount() && !m_assetAccountSplit.accountId().isEmpty()
          && haveInterest() && m_interestSplits.count()) {
            txt = m_interestAmount.abs().formatMoney(m_currency);
          } else if(haveFees() && m_feeSplits.count()) {
            txt = m_feeAmount.abs().formatMoney(m_currency);
          }
          break;

        default:
          break;
      }
      break;

    case 3:
      switch(col) {
        case DetailColumn:
          align |= Qt::AlignLeft;
          if(haveAssetAccount() && !m_assetAccountSplit.accountId().isEmpty()
          && haveInterest() && m_interestSplits.count()
          && haveFees() && m_feeSplits.count()) {
            txt = m_feeCategory;
          } else
            singleLineMemo(txt, m_split);
          break;

        case QuantityColumn:
          align |= Qt::AlignRight;
          if(haveAssetAccount() && !m_assetAccountSplit.accountId().isEmpty()
          && haveInterest() && m_interestSplits.count()
          && haveFees() && m_feeSplits.count()) {
            txt = m_feeAmount.abs().formatMoney(m_currency);
          }
          break;

        default:
          break;
      }
      break;

    case 4:
      switch(col) {
        case DetailColumn:
          align |= Qt::AlignLeft;
          singleLineMemo(txt, m_split);
          break;

        default:
          break;
      }
      break;
  }
}

int InvestTransaction::registerColWidth(int col, const QFontMetrics& cellFontMetrics)
{
  QString txt;
  MyMoneyMoney amount;
  int nw = 0;

  // for now just check all rows in that column
  for(int row = 0; row < m_rowsRegister; ++row) {
    int w;
    Transaction::registerCellText(txt, row, col);
    w = cellFontMetrics.width(txt+"  ");
    nw = QMAX(nw, w);
  }

  // TODO the optimized way would be to base the size on the contents of a single row
  //      as we do it in StdTransaction::registerColWidth()
#if 0
  switch(col) {
    default:
      break;

    case PriceColumn:
      if(havePrice()) {
        txt = (m_split.value() / m_split.shares()).formatMoney("", KMyMoneyGlobalSettings::pricePrecision());
        nw = cellFontMetrics.width(txt+"  ");
      }
      break;
  }
#endif
  return nw;
}

void InvestTransaction::arrangeWidgetsInForm(QMap<QString, QWidget*>& editWidgets)
{
  if(!m_form || !m_parent)
    return;

  setupFormPalette(editWidgets);

  // arrange the edit widgets
  arrangeWidget(m_form, 0, ValueColumn1, editWidgets["activity"]);
  arrangeWidget(m_form, 0, ValueColumn2, editWidgets["postdate"]);
  arrangeWidget(m_form, 1, ValueColumn1, editWidgets["security"]);
  arrangeWidget(m_form, 1, ValueColumn2, editWidgets["shares"]);
  arrangeWidget(m_form, 2, ValueColumn1, editWidgets["asset-account"]);
  arrangeWidget(m_form, 2, ValueColumn2, editWidgets["price"]);
  arrangeWidget(m_form, 3, ValueColumn1, editWidgets["fee-account"]->parentWidget());
  arrangeWidget(m_form, 3, ValueColumn2, editWidgets["fee-amount"]);
  arrangeWidget(m_form, 4, ValueColumn1, editWidgets["interest-account"]->parentWidget());
  arrangeWidget(m_form, 4, ValueColumn2, editWidgets["interest-amount"]);
  arrangeWidget(m_form, 5, ValueColumn1, editWidgets["memo"]);
  arrangeWidget(m_form, 5, ValueColumn2, editWidgets["total"]);
  arrangeWidget(m_form, 6, ValueColumn2, editWidgets["status"]);

  // arrange dynamic labels
  arrangeWidget(m_form, 1, LabelColumn2, editWidgets["shares-label"]);
  arrangeWidget(m_form, 2, LabelColumn1, editWidgets["asset-label"]);
  arrangeWidget(m_form, 2, LabelColumn2, editWidgets["price-label"]);
  arrangeWidget(m_form, 3, LabelColumn1, editWidgets["fee-label"]);
  arrangeWidget(m_form, 3, LabelColumn2, editWidgets["fee-amount-label"]);
  arrangeWidget(m_form, 4, LabelColumn1, editWidgets["interest-label"]);
  arrangeWidget(m_form, 4, LabelColumn2, editWidgets["interest-amount-label"]);
  arrangeWidget(m_form, 5, LabelColumn2, editWidgets["total-label"]);

  // get rid of the hints. we don't need them for the form
  QMap<QString, QWidget*>::iterator it;
  for(it = editWidgets.begin(); it != editWidgets.end(); ++it) {
    KMyMoneyCombo* combo = dynamic_cast<KMyMoneyCombo*>(*it);
    kMyMoneyLineEdit* lineedit = dynamic_cast<kMyMoneyLineEdit*>(*it);
    kMyMoneyEdit* edit = dynamic_cast<kMyMoneyEdit*>(*it);
    KMyMoneyPayeeCombo* payee = dynamic_cast<KMyMoneyPayeeCombo*>(*it);
    if(combo)
      combo->setHint(QString());
    if(edit)
      edit->setHint(QString());
    if(lineedit)
      lineedit->setHint(QString());
    if(payee)
      payee->setHint(QString());
  }
}

void InvestTransaction::tabOrderInForm(QWidgetList& tabOrderWidgets) const
{
  // activity
  tabOrderWidgets.append(focusWidget(m_form->cellWidget(0, ValueColumn1)));

  // date
  tabOrderWidgets.append(focusWidget(m_form->cellWidget(0, ValueColumn2)));

  // security
  tabOrderWidgets.append(focusWidget(m_form->cellWidget(1, ValueColumn1)));

  // shares
  tabOrderWidgets.append(focusWidget(m_form->cellWidget(1, ValueColumn2)));

  // account
  tabOrderWidgets.append(focusWidget(m_form->cellWidget(2, ValueColumn1)));

  // price
  tabOrderWidgets.append(focusWidget(m_form->cellWidget(2, ValueColumn2)));

  // make sure to have the fee category field and the split button as seperate tab order widgets
  // ok, we have to have some internal knowledge about the KMyMoneyCategory object, but
  // it's one of our own widgets, so we actually don't care. Just make sure, that we don't
  // go haywire when someone changes the KMyMoneyCategory object ...
  QWidget* w = m_form->cellWidget(3, ValueColumn1);
  tabOrderWidgets.append(focusWidget(w));
  w = dynamic_cast<QWidget*>(w->child("splitButton"));
  if(w)
    tabOrderWidgets.append(w);

  // fee amount
  tabOrderWidgets.append(focusWidget(m_form->cellWidget(3, ValueColumn2)));

  // the same applies for the interest categories
  w = m_form->cellWidget(4, ValueColumn1);
  tabOrderWidgets.append(focusWidget(w));
  w = dynamic_cast<QWidget*>(w->child("splitButton"));
  if(w)
    tabOrderWidgets.append(w);

  // interest amount
  tabOrderWidgets.append(focusWidget(m_form->cellWidget(4, ValueColumn2)));

  // memo
  tabOrderWidgets.append(focusWidget(m_form->cellWidget(5, ValueColumn1)));

  // total
  tabOrderWidgets.append(focusWidget(m_form->cellWidget(5, ValueColumn2)));

  // state
  tabOrderWidgets.append(focusWidget(m_form->cellWidget(6, ValueColumn2)));
}

void InvestTransaction::arrangeWidgetsInRegister(QMap<QString, QWidget*>& editWidgets)
{
  if(!m_parent)
    return;

  setupRegisterPalette(editWidgets);

  arrangeWidget(m_parent, m_startRow + 0, DateColumn, editWidgets["postdate"]);
  arrangeWidget(m_parent, m_startRow + 0, SecurityColumn, editWidgets["security"]);
  arrangeWidget(m_parent, m_startRow + 0, DetailColumn, editWidgets["activity"]);
  arrangeWidget(m_parent, m_startRow + 1, DetailColumn, editWidgets["asset-account"]);
  arrangeWidget(m_parent, m_startRow + 2, DetailColumn, editWidgets["interest-account"]->parentWidget());
  arrangeWidget(m_parent, m_startRow + 3, DetailColumn, editWidgets["fee-account"]->parentWidget());
  arrangeWidget(m_parent, m_startRow + 4, DetailColumn, editWidgets["memo"]);
  arrangeWidget(m_parent, m_startRow + 0, QuantityColumn, editWidgets["shares"]);
  arrangeWidget(m_parent, m_startRow + 0, PriceColumn, editWidgets["price"]);
  arrangeWidget(m_parent, m_startRow + 2, QuantityColumn, editWidgets["interest-amount"]);
  arrangeWidget(m_parent, m_startRow + 3, QuantityColumn, editWidgets["fee-amount"]);
  arrangeWidget(m_parent, m_startRow + 0, ValueColumn, editWidgets["total"]);
  arrangeWidget(m_parent, m_startRow + 1, DateColumn, editWidgets["status"]);

  // increase the height of the row containing the memo widget
  m_parent->setRowHeight(m_startRow+4, m_parent->rowHeightHint() * 3);
}

void InvestTransaction::tabOrderInRegister(QWidgetList& tabOrderWidgets) const
{
  QWidget* w;

  // date
  tabOrderWidgets.append(focusWidget(m_parent->cellWidget(m_startRow + 0, DateColumn)));
  // security
  tabOrderWidgets.append(focusWidget(m_parent->cellWidget(m_startRow + 0, SecurityColumn)));
  // activity
  tabOrderWidgets.append(focusWidget(m_parent->cellWidget(m_startRow + 0, DetailColumn)));
  // shares
  tabOrderWidgets.append(focusWidget(m_parent->cellWidget(m_startRow + 0, QuantityColumn)));
  // price
  tabOrderWidgets.append(focusWidget(m_parent->cellWidget(m_startRow + 0, PriceColumn)));
  // asset account
  tabOrderWidgets.append(focusWidget(m_parent->cellWidget(m_startRow + 1, DetailColumn)));

  // make sure to have the category fields and the split button as seperate tab order widgets
  // ok, we have to have some internal knowledge about the KMyMoneyCategory object, but
  // it's one of our own widgets, so we actually don't care. Just make sure, that we don't
  // go haywire when someone changes the KMyMoneyCategory object ...
  w = m_parent->cellWidget(m_startRow + 2, DetailColumn);    // interest account
  tabOrderWidgets.append(focusWidget(w));
  w = dynamic_cast<QWidget*>(w->child("splitButton"));
  if(w)
    tabOrderWidgets.append(w);

  // interest amount
  tabOrderWidgets.append(focusWidget(m_parent->cellWidget(m_startRow + 2, QuantityColumn)));

  w = m_parent->cellWidget(m_startRow + 3, DetailColumn);    // fee account
  tabOrderWidgets.append(focusWidget(w));
  w = dynamic_cast<QWidget*>(w->child("splitButton"));
  if(w)
    tabOrderWidgets.append(w);

  // fee amount
  tabOrderWidgets.append(focusWidget(m_parent->cellWidget(m_startRow + 3, QuantityColumn)));

  // memo
  tabOrderWidgets.append(focusWidget(m_parent->cellWidget(m_startRow + 4, DetailColumn)));

  // status
  tabOrderWidgets.append(focusWidget(m_parent->cellWidget(m_startRow + 1, DateColumn)));
}

int InvestTransaction::numRowsRegister(bool expanded) const
{
  int numRows = 1;
  if(expanded) {
    if(!m_inEdit) {
      if(haveAssetAccount() && !m_assetAccountSplit.accountId().isEmpty())
        ++numRows;
      if(haveInterest() && m_interestSplits.count())
        ++numRows;
      if(haveFees() && m_feeSplits.count())
        ++numRows;
      if(!m_split.memo().isEmpty())
        ++numRows;
    } else
      numRows = 5;
  }
  return numRows;
}

bool InvestTransaction::haveShares(void) const
{
  bool rc = true;
  switch(m_transactionType) {
    case MyMoneySplit::Dividend:
    case MyMoneySplit::Yield:
    case MyMoneySplit::SplitShares:
      rc = false;
      break;

    default:
      break;
  }
  return rc;
}

bool InvestTransaction::haveFees(void) const
{
  bool rc = true;
  switch(m_transactionType) {
    case MyMoneySplit::AddShares:
    case MyMoneySplit::RemoveShares:
    case MyMoneySplit::SplitShares:
      rc = false;
      break;

    default:
      break;
  }
  return rc;
}

bool InvestTransaction::haveInterest(void) const
{
  bool rc = false;
  switch(m_transactionType) {
    case MyMoneySplit::BuyShares:
    case MyMoneySplit::SellShares:
    case MyMoneySplit::Dividend:
    case MyMoneySplit::ReinvestDividend:
    case MyMoneySplit::Yield:
      rc = true;
      break;

    default:
      break;
  }
  return rc;
}

bool InvestTransaction::havePrice(void) const
{
  bool rc = false;
  switch(m_transactionType) {
    case MyMoneySplit::BuyShares:
    case MyMoneySplit::SellShares:
    case MyMoneySplit::ReinvestDividend:
      rc = true;
      break;

    default:
      break;
  }
  return rc;
}

bool InvestTransaction::haveAmount(void) const
{
  bool rc = false;
  switch(m_transactionType) {
    case MyMoneySplit::BuyShares:
    case MyMoneySplit::SellShares:
    case MyMoneySplit::Dividend:
    case MyMoneySplit::Yield:
      rc = true;
      break;

    default:
      break;
  }
  return rc;
}

bool InvestTransaction::haveAssetAccount(void) const
{
  bool rc = true;
  switch(m_transactionType) {
    case MyMoneySplit::AddShares:
    case MyMoneySplit::RemoveShares:
    case MyMoneySplit::SplitShares:
    case MyMoneySplit::ReinvestDividend:
      rc = false;
      break;

    default:
      break;
  }
  return rc;
}

bool InvestTransaction::haveSplitRatio(void) const
{
  return m_transactionType == MyMoneySplit::SplitShares;
}

void InvestTransaction::splits(MyMoneySplit& assetAccountSplit, QValueList<MyMoneySplit>& interestSplits, QValueList<MyMoneySplit>& feeSplits) const
{
  assetAccountSplit = m_assetAccountSplit;
  interestSplits = m_interestSplits;
  feeSplits = m_feeSplits;
}

TransactionEditor* InvestTransaction::createEditor(TransactionEditorContainer* regForm, const KMyMoneyRegister::SelectedTransactions& list, const QDate& lastPostDate)
{
  m_inRegisterEdit = regForm == m_parent;
  return new InvestTransactionEditor(regForm, this, list, lastPostDate);
}

