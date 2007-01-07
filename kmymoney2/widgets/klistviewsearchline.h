/* This file is part of the KDE libraries
   Copyright (c) 2003 Scott Wheeler <wheeler@kde.org>
   Adapted to be used with KMyMoney under KDE 3.2 .. 3.4

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KLISTVIEWSEARCHLINE_H
#define KLISTVIEWSEARCHLINE_H

#include <klineedit.h>
#include <qhbox.h>
#include <kmymoney/export.h>

class KListView;
class QListViewItem;
class QToolButton;

/**
 * This class makes it easy to add a search line for filtering the items in a
 * listview based on a simple text search.
 *
 * No changes to the application other than instantiating this class with an
 * appropriate KListView should be needed.
 *
 * @since 3.3
 */

class KMYMONEY_EXPORT KListViewSearchLine : public KLineEdit
{
    Q_OBJECT

public:

    /**
     * Constructs a KListViewSearchLine with \a listView being the KListView to
     * be filtered.
     *
     * If \a listView is null then the widget will be disabled until a listview
     * is set with setListView().
     */
    KListViewSearchLine(QWidget *parent = 0, KListView *listView = 0, const char *name = 0);

    /**
     * Constructs a KListViewSearchLine without any KListView to filter. The
     * KListView object has to be set later with setListView(). 
     */
    KListViewSearchLine(QWidget *parent, const char *name);

    /**
     * Destroys the KListViewSearchLine.
     */
    virtual ~KListViewSearchLine();

    /**
     * Returns true if the search is case sensitive.  This defaults to false.
     *
     * @see setCaseSensitive()
     */
    bool caseSensitive() const;

    /**
     * Returns the current list of columns that will be searched.  If the
     * returned list is empty all visible columns will be searched.
     *
     * @see setSearchColumns
     */
    QValueList<int> searchColumns() const;

    /**
     * If this is true (the default) then the parents of matched items will also
     * be shown.
     *
     * @see setKeepParentsVisible()
     */
    bool keepParentsVisible() const;

    /**
     * Returns the listview that is currently filtered by the search.
     *
     * @see setListView()
     */
    KListView *listView() const;

public slots:
    /**
     * Updates search to only make visible the items that match \a s.  If
     * \a s is null then the line edit's text will be used.
     */
    virtual void updateSearch(const QString &s = QString::null);

    /**
     * Make the search case sensitive or case insensitive.
     *
     * @see caseSenstive()
     */
    void setCaseSensitive(bool cs);

    /**
     * When a search is active on a list that's organized into a tree view if
     * a parent or ancesestor of an item is does not match the search then it
     * will be hidden and as such so too will any children that match.
     *
     * If this is set to true (the default) then the parents of matching items
     * will be shown.
     *
     * @see keepParentsVisible
     */
    void setKeepParentsVisible(bool v);

    /**
     * Sets the list of columns to be searched.  The default is to search all,
     * visible columns which can be restored by passing \a columns as an empty
     * list.
     *
     * @see searchColumns
     */
    void setSearchColumns(const QValueList<int> &columns);

    /**
     * Sets the KListView that is filtered by this search line.  If \a lv is null
     * then the widget will be disabled.
     *
     * @see listView()
     */
    void setListView(KListView *lv);

protected:

    /**
     * Returns true if \a item matches the search \a s.  This will be evaluated
     * based on the value of caseSensitive().  This can be overridden in
     * subclasses to implement more complicated matching schemes.
     */
    virtual bool itemMatches(const QListViewItem *item, const QString &s) const;

    /**
    * Re-implemented for internal reasons.  API not affected.
    *
    * See QLineEdit::mousePressEvent().
    */
    virtual QPopupMenu *createPopupMenu();

protected slots:
    /**
     * When keys are pressed a new search string is created and a timer is
     * activated.  The most recent search is activated when this timer runs out
     * if another key has not yet been pressed.
     *
     * This method makes @param search the most recent search and starts the
     * timer.
     *
     * Together with activateSearch() this makes it such that searches are not
     * started until there is a short break in the users typing.
     *
     * @see activateSearch()
     */
    void queueSearch(const QString &search);

    /**
     * When the timer started with queueSearch() expires this slot is called.
     * If there has been another timer started then this slot does nothing.
     * However if there are no other pending searches this starts the list view
     * search.
     *
     * @see queueSearch()
     */
    void activateSearch();

private:

    /**
     * This is used in case parent items of matching items shouldn't be
     * visible.  It hides all items that don't match the search string.
     */
    void checkItemParentsNotVisible();

    /**
     * This is used in case parent items of matching items should be visible.
     * It makes a recursive call to all children.  It returns true if at least
     * one item in the subtree with the given root item is visible.
     */
    bool checkItemParentsVisible(QListViewItem *item, QListViewItem *highestHiddenParent = 0);

private slots:
    void itemAdded(QListViewItem *item) const;
    void listViewDeleted();
    void searchColumnsMenuActivated(int);

private:
    class KListViewSearchLinePrivate;
    KListViewSearchLinePrivate *d;
};

/**
 * Creates a widget featuring a KListViewSearchLine, a label with the text
 * "Search" and a button to clear the search.
 *
 * @since 3.4
 */
class KMYMONEY_EXPORT KListViewSearchLineWidget : public QHBox
{
    Q_OBJECT

public:
    /**
     * Creates a KListViewSearchLineWidget for \a listView with \a parent as the
     * parent with and \a name.
     */
    KListViewSearchLineWidget(KListView *listView = 0, QWidget *parent = 0,
                              const char *name = 0);

    /**
     * Destroys the KListViewSearchLineWidget
     */
    ~KListViewSearchLineWidget();

    /**
     * Creates the search line.  This can be useful to reimplement in cases where
     * a KListViewSearchLine subclass is used.
     */
    virtual KListViewSearchLine *createSearchLine(KListView *listView);

    /**
     * Returns a pointer to the search line.
     */
    KListViewSearchLine *searchLine() const;

protected slots:
    /**
     * Creates the widgets inside of the widget.  This is called from the
     * constructor via a single shot timer so that it it guaranteed to run
     * after construction is complete.  This makes it suitable for overriding in
     * subclasses.
     */
    virtual void createWidgets();

private slots:
    void positionInToolBar();

private:
    class KListViewSearchLineWidgetPrivate;
    KListViewSearchLineWidgetPrivate *d;
};

#endif
