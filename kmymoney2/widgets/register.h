/***************************************************************************
                             register.h
                             ----------
    begin                : Fri Mar 10 2006
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

#ifndef REGISTER_H
#define REGISTER_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qtable.h>
#include <qvaluelist.h>
#include <qvaluevector.h>
#include <qwidgetlist.h>
#include <qmap.h>
#include <qpair.h>
#include <qevent.h>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoney/mymoneyaccount.h>
#include <kmymoney/registeritem.h>
#include <kmymoney/transaction.h>
#include <kmymoney/transactioneditorcontainer.h>
#include <kmymoney/selectedtransaction.h>
#include <kmymoney/transactionsortoption.h>

class MyMoneyObjectContainer;
class RegisterToolTip;

namespace KMyMoneyRegister {

typedef enum {
  UnknownSort = 0,      //< unknown sort criteria
  PostDateSort = 1,     //< sort by post date
  EntryDateSort,        //< sort by entry date
  PayeeSort,            //< sort by payee name
  ValueSort,            //< sort by value
  NoSort,               //< sort by number field
  EntryOrderSort,       //< sort by entry order
  TypeSort,             //< sort by CashFlowDirection
  CategorySort,         //< sort by Category
  ReconcileStateSort,   //< sort by reconciliation state
  SecuritySort,         //< sort by security (only useful for investment accounts)
  // insert new values in front of this line
  MaxSortFields
} TransactionSortField;

typedef enum {
  Ascending = 0,        //< sort in ascending order
  Descending            //< sort in descending order
} SortDirection;

class Register;
class RegisterItem;
class ItemPtrVector;

const QString& sortOrderToText(TransactionSortField idx);
TransactionSortField textToSortOrder(const QString& text);


class QWidgetContainer : public QMap<QString, QWidget*>
{
public:
  QWidgetContainer() {};

  QWidget* haveWidget(const QString& name) const {
    QWidgetContainer::const_iterator it_w;
    it_w = find(name);
    if(it_w != end())
      return *it_w;
    return 0;
  }

  void removeOrphans(void) {
    QWidgetContainer::iterator it_w;
    for(it_w = begin(); it_w != end(); ) {
      if((*it_w) && (*it_w)->parent())
        ++it_w;
      else {
        delete (*it_w);
        remove(it_w);
        it_w = begin();
      }
    }
  }

};

class GroupMarker : public RegisterItem
{
public:
  GroupMarker(Register* parent);
  bool isSelectable(void) const { return false; }
  bool canHaveFocus(void) const { return false; }
  int numRows(void) const { return 1; }

  bool isErronous(void) const { return false; }

  void paintRegisterCell(QPainter* painter, int row, int col, const QRect& r, bool selected, const QColorGroup& cg);
  void paintFormCell(QPainter* /* painter */, int /* row */, int /* col */, const QRect& /* r */, bool /* selected */, const QColorGroup& /* cg */) {}

  int rowHeightHint(void) const;

protected:
  QString                  m_txt;
  QPixmap                  m_bg;
  int                      m_lastCol;
};


class FancyDateGroupMarker : public GroupMarker
{
public:
  FancyDateGroupMarker(Register* parent, const QDate& date, const QString& txt);

  virtual const QDate& sortPostDate(void) const { return m_date; }
  virtual const QDate& sortEntryDate(void) const { return m_date; }
private:
  QDate                    m_date;
};

class SimpleDateGroupMarker : public FancyDateGroupMarker
{
public:
  SimpleDateGroupMarker(Register* parent, const QDate& date, const QString& txt);
  void paintRegisterCell(QPainter* painter, int row, int col, const QRect& r, bool selected, const QColorGroup& cg);
  int rowHeightHint(void) const;
};

class TypeGroupMarker : public GroupMarker
{
public:
  TypeGroupMarker(Register* parent, CashFlowDirection dir, MyMoneyAccount::accountTypeE accType);
  CashFlowDirection sortType(void) const { return m_dir; }
private:
  CashFlowDirection        m_dir;
};

class PayeeGroupMarker : public GroupMarker
{
public:
  PayeeGroupMarker(Register* parent, const QString& name);
  const QString& sortPayee(void) const { return m_txt; }
};

class CategoryGroupMarker : public GroupMarker
{
public:
  CategoryGroupMarker(Register* parent, const QString& category);
  const QString& sortCategory(void) const { return m_txt; }
  const QString& sortSecurity(void) const { return m_txt; }
};

class ReconcileGroupMarker : public GroupMarker
{
public:
  ReconcileGroupMarker(Register* parent, MyMoneySplit::reconcileFlagE state);
  virtual MyMoneySplit::reconcileFlagE sortReconcileState(void) const { return m_state; }
private:
  MyMoneySplit::reconcileFlagE  m_state;
};


class ItemPtrVector : public QValueVector<RegisterItem *>
{
public:
  ItemPtrVector() {}

  void sort(void) { std::sort(begin(), end(), item_cmp); }

protected:
  /**
    * sorter's compare routine. Returns true if i1 < i2
    */
  static bool item_cmp(RegisterItem* i1, RegisterItem* i2);
};


class Register : public TransactionEditorContainer
{
  Q_OBJECT

  // friend class QHeader;
  // friend class QTableHeader;
  friend class Item;

public:
  Register(QWidget *parent = 0, const char *name = 0);
  virtual ~Register();

  /**
    * add the item @a p to the register
    */
  void addItem(RegisterItem* p);

  /**
    * insert the item @a p into the register after item @a q
    */
  void insertItemAfter(RegisterItem* p, RegisterItem* q);

  /**
    * remove the item @p from the register
    */
  void removeItem(RegisterItem* p);

  /**
    * This method returns a list of pointers to all selected items
    * in the register
    *
    * @retval QValueList<RegisterItem*>
    */
  QValueList<RegisterItem*> selectedItems(void) const;

  void selectedTransactions(QValueList<SelectedTransaction>& list) const;

  QString text(int row, int col) const;
  QWidget* createEditor(int row, int col, bool initFromCell) const;
  void setCellContentFromEditor(int row, int col);
  QWidget* cellWidget(int row, int col) const;
  void endEdit(int row, int col, bool accept, bool replace);
  void paintCell(QPainter* painter, int row, int col, const QRect& r, bool selected, const QColorGroup& cg);

  void resizeData(int) {};
  QTableItem* item(int, int) { return 0; }
  void setItem(int, int, QTableItem*) {}
  void clearCell(int, int) {}
  void clearCellWidget(int, int);

  /**
    * Override the QTable member function to avoid display of focus
    */
  void paintFocus(QPainter*, const QRect& ) {}

  /**
    * Override the QTable member function to avoid functionality
    */
  void updateCell(int row, int col) {}

  RegisterItem* focusItem(void) const { return m_focusItem; }
  void setFocusItem(RegisterItem* focusItem);
  void selectItem(RegisterItem* item);

  /**
    * Clears all items in the register. All objects
    * added to the register will be deleted.
    */
  void clear(void);

  void updateRegister(bool forceUpdateRowHeight = false);

  void adjustColumn(int col);

  void setupRegister(const MyMoneyAccount& account, bool showAccountColumn = false);

  void setSortOrder(const QString& order);
  const QValueList<TransactionSortField>& sortOrder(void) const { return m_sortOrder; }
  TransactionSortField primarySortKey(void) const;
  void sortItems(void);

  /**
    * This member returns the last visible column that is used by the register
    * after it has been setup using setupRegister().
    *
    * @return last actively used column (base 0)
    */
  Column lastCol(void) const { return m_lastCol; }

  RegisterItem* firstItem(void) const;
  RegisterItem* nextItem(RegisterItem*) const;
  RegisterItem* lastItem(void) const;
  RegisterItem* prevItem(RegisterItem*) const;
  RegisterItem* itemAtRow(int row) const;

  void resize(int col);

  void forceUpdateLists(void) { m_listsDirty = true; }

  void ensureItemVisible(RegisterItem* item);

  void arrangeEditWidgets(QMap<QString, QWidget*>& editWidgets, Transaction* t);
  void removeEditWidgets(QMap<QString, QWidget*>& editWidgets);
  void tabOrder(QWidgetList& tabOrderWidgets, KMyMoneyRegister::Transaction* t) const;

  int rowHeightHint(void) const;

  void clearSelection(void);

  bool markErronousTransactions(void) const { return (m_markErronousTransactions & 0x01) != 0; }

  /**
    * This method creates a specifc transaction according to the
    * transaction passed in @a transaction.
    *
    * @param parent pointer to register where the created object should be added
    * @param objects pointer to MyMoneyObjectContainer to be used
    * @param transaction the transaction which should be used to create the object
    * @param split the split of the transaction which should be used to create the object
    *
    * @return pointer to created object (0 upon failure)
    */
  static Transaction* transactionFactory(Register *parent, MyMoneyObjectContainer* objects, const MyMoneyTransaction& transaction, const MyMoneySplit& split);

  const MyMoneyAccount& account(void) const { return m_account; }

protected:
  void drawContents(QPainter *p, int cx, int cy, int cw, int ch);

  void contentsMouseReleaseEvent( QMouseEvent *e );
  void unselectItems(int from = -1, int to = -1) { doSelectItems(from, to, false); }
  void selectItems(int from, int to) { doSelectItems(from, to, true); }
  void doSelectItems(int from, int to, bool selected);
  void repaintItems(RegisterItem* first = 0, RegisterItem* last = 0);
  int selectedItemsCount(void) const;

  void focusOutEvent(QFocusEvent*);
  void focusInEvent(QFocusEvent*);
  void keyPressEvent(QKeyEvent*);

  int rowToIndex(int row) const;
  void setupItemIndex(int rowCount);

  /**
    * This method determines the register item that is one page
    * further down or up in the ledger from the previous focus item.
    * The height to scroll is determined by visibleHeight()
    *
    * @param key Qt::Page_Up or Qt::Page_Down depending on the direction to scroll
    *
    * @return pointer to selectable register item
    */
  RegisterItem* scrollPage(int key);

  /**
    * This method determines the pointer to a RegisterItem
    * based on the item's @a id. If @a id is empty, this method
    * returns @a m_lastItem.
    *
    * @param id id of the item to be searched
    * @return pointer to RegisterItem or 0 if not found
    */
  RegisterItem* itemById(const QCString& id) const;

  void insertWidget(int row, int col, QWidget* w);

  /**
    * Override logic and use standard QFrame behaviour
    */
  bool focusNextPrevChild(bool next);

  bool eventFilter(QObject* o, QEvent* e);

protected slots:
  void resize(void);

  void selectItem(int row, int col, int button, const QPoint & mousePos );
  void slotEnsureItemVisible(void);
  void slotDoubleClicked(int, int, int, const QPoint&);

  void slotToggleErronousTransactions(void);

signals:
  void selectionChanged(void);
  void selectionChanged(const QValueList<KMyMoneyRegister::SelectedTransaction>& list);
  /**
    * This signal is emitted when the focus and selection changes to @p item.
    *
    * @param item pointer to transaction that received the focus and was selected
    */
  void focusChanged(KMyMoneyRegister::Transaction* item);

  /**
    * This signal is emitted when the focus changes but the selection remains
    * the same. This usually happens when the focus is changed using the keyboard.
    */
  void focusChanged(void);

  void aboutToSelectItem(KMyMoneyRegister::RegisterItem* item);
  void editTransaction(void);
  void headerClicked(void);

  /**
    * This signal is sent out when the user clicks on the ReconcileStateColumn and
    * only a single transaction is selected.
    */
  void reconcileStateColumnClicked(KMyMoneyRegister::Transaction* item);

  /**
    * This signal is sent out, if an item without a transaction id has been selected.
    */
  void emptyItemSelected(void);

  /**
    * This signal is sent out, if the user selects an item with the right mouse button
    */
  void openContextMenu(void);

protected:
  ItemPtrVector                m_items;
  QValueVector<RegisterItem*>  m_itemIndex;
  RegisterItem*                m_selectAnchor;
  RegisterItem*                m_focusItem;
  RegisterItem*                m_ensureVisibleItem;
  RegisterItem*                m_firstItem;
  RegisterItem*                m_lastItem;
  RegisterItem*                m_firstErronous;
  RegisterItem*                m_lastErronous;

  int                          m_markErronousTransactions;
  int                          m_rowHeightHint;

  MyMoneyAccount               m_account;

private:
  bool                         m_listsDirty;
  bool                         m_ignoreNextButtonRelease;
  bool                         m_needInitialColumnResize;
  Qt::ButtonState              m_buttonState;
  Column                       m_lastCol;
  QValueList<TransactionSortField> m_sortOrder;
  QMap<QPair<int, int>, QWidget*> m_cellWidgets;
  RegisterToolTip*             m_tooltip;
  QRect                        m_lastRepaintRect;
};

} // namespace

#endif
// vim:cin:si:ai:et:ts=2:sw=2: