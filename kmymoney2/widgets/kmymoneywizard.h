/***************************************************************************
                             kmymoneywizard.h
                             -------------------
    copyright            : (C) 2006 by Thomas Baumagrt
    email                : ipwizard@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KMYMONEYWIZARD_H
#define KMYMONEYWIZARD_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qdialog.h>
#include <qvaluelist.h>
class QVBoxLayout;
class QHBoxLayout;
class QLabel;
class QFrame;

// ----------------------------------------------------------------------------
// KDE Includes

class KPushButton;

// ----------------------------------------------------------------------------
// Project Includes

class KMyMoneyTitleLabel;
class KMyMoneyWizard;
class KMyMoneyWizardPagePrivate;

/**
  * @author Thomas Baumgart (C) 2006
  *
  * This class represents the base class for wizard pages for the
  * KMyMoneyWizard. One cannot create a wizard page directly, but
  * must derive from it. The KMyMoneyWizardPage class provides the
  * necessary functionality to work in concert with KMyMoneyWizard.
  *
  * Therefore, few steps are necessary to use this class. They seem to
  * be awkward at some stage, but I wanted to be able to use Qt designer
  * to actually design the widget for the page. That's why some things
  * aren't implemented in a more straight fashion than one would
  * normally do this.
  *
  * The first step is to derive a specific base page for the specific wizard.
  * In this example we use the name NewUser as template for the specific wizard.
  * This class provides a 'back'-pointer to the actual wizard object
  * for all pages.
  *
  * @code
  * class KNewUserPage : public KMyMoneyWizardPage
  * {
  * public:
  *   KNewUserPage(unsigned int step, QWidget* widget, KNewUserWizard* parent, const char* name);
  *
  * protected:
  *   KNewUserWizard*    m_wizard;
  * }
  * @endcode
  *
  * The implementation of this class is rather straight-forward:
  *
  * @code
  * KNewUserPage::KNewUserPage(unsigned int step, QWidget* widget, KNewUserWizard* parent, const char* name) :
  *   KMyMoneyWizardPage(step, widget, name),
  *   m_wizard(parent)
  * {
  * }
  * @endcode
  *
  * For each page of the wizard, you will have to create a @p ui file with
  * Qt designer.
  * Let's assume we call the first page of the wizard 'General' and go
  * from there.
  * We also assume, that the wizard has more than one page.
  * The ui designer generated class should have the name KNewUserGeneralDecl
  * as all other dialogs. The class definition of KNewUserGeneral will
  * look like this:
  *
  * @code
  * class KNewUserGeneral : public KNewUserGeneralDecl, public KNewUserPage
  * {
  *   Q_OBJECT
  * public:
  *   KNewUserGeneral(KNewUserWizard* parent, const char* name = 0);
  *   KMyMoneyWizardPage* nextPage(void);
  *   bool isLastPage(void) { return false; }
  *
  * protected:
  *   KNewUserWizard*    m_wizard;
  * }
  * @endcode
  *
  * The implementation depends heavily on the logic of your code. If you only
  * fill some widgets, it could be as simple as:
  *
  * @code
  * KNewUserGeneral::KNewUserGeneral(KNewUserWizard* parent, const char* name) :
  *   KNewUserGeneralDecl(parent),
  *   KNewUserPage(1, this, parent, name)
  * {
  * }
  * KMyMoneyWizardPage* KNewUserGeneral::nextPage(void)
  * {
  *   return m_wizard->m_personalPage;
  * }
  * @endcode
  *
  * A note on the first parameter to KNewUserPage in the above example: it ties
  * this page to be part of step 1 (see KMyMoneyWizard::addStep() for details).
  *
  * Depending on the actual logic of the page, you would want to override the
  * following methods: resetPage, nextPage, isLastPage and isComplete.
  *
  * @note The implementation of this class is heavily based on ideas found at
  *       http://doc.trolltech.com/4.1/dialogs-complexwizard.html
  */
class KMyMoneyWizardPage
{
public:
  /**
    * This method is called by the wizard whenever a page is entered
    * (either in forward or backward direction). The default
    * implementation does nothing.
    */
  virtual void resetPage(void);

  /**
    * This method returns a pointer to the next page that should be
    * shown when the user presses the 'Next' button.
    *
    * @return pointer to next wizard page
    */
  virtual KMyMoneyWizardPage* nextPage(void);

  /**
    * This returns, if the current page is the last page of the wizard.
    * The default implementation returns @p false if nextPage() returns 0,
    * @p true otherwise.
    *
    * @retval false more pages follow
    * @retval true this is the last page of the wizard
    */
  virtual bool isLastPage(void);

  /**
    * This returns, if all necessary data for this page has been
    * filled. It is used to enabled the 'Next' or 'Finish' button.
    * The button is only enabled, if this method returns @p true,
    * which is the default implementation.
    *
    * @retval false more data required from the user before we can proceed
    * @retval true all data available, we allow to switch to the next page
    */
  virtual bool isComplete(void);

  /**
    * This method returns the step to which this page belongs.
    * It is required by the KMyMoneyWizard and is not intended
    * to be used by application code.
    *
    * @return step of wizard this page belongs to
    */
  unsigned int step(void) const { return m_step; }

  /**
    * This method returns a pointer to the widget of the page.
    * It is required by the KMyMoneyWizard and is not intended
    * to be used by application code.
    *
    * @return pointer to widget of page
    */
  QWidget* widget(void) const { return m_widget; }

  /**
    * This method returns a pointer to the QObject used for
    * the signal/slot mechanism.
    * It is required by the KMyMoneyWizard and is not intended
    * to be used by application code.
    */
  QObject* object(void) const;

protected:
  /**
    * Constructor (kept protected, so that one cannot create such an object directly)
    */
  KMyMoneyWizardPage(unsigned int step, QWidget* widget, const char* name = 0);

  /**
    * This method must be called by the implementation when the
    * data in the fields of the wizard change and the state of
    * completeness changed.
    *
    * @note If you do not override isComplete() then there is no need
    * to call this method.
    */
  void completeStateChanged(void) const;

private:
  unsigned int     m_step;
  QWidget*         m_widget;
  KMyMoneyWizardPagePrivate* d;
};


/**
  * @author Thomas Baumgart (C) 2006
  *
  * This is a base class for implementation of the KMyMoneyWizard. It provides
  * the following layout of a wizard:
  *
  * @code
  * +-wizardLayout-----------------------------------------------+
  * |                                                            |
  * +------------------------------------------------------------+
  * |+-stepLayout--++-------------------------------------------+|
  * ||             ||+-pageLayout------------------------------+||
  * ||             |||                                         |||
  * ||             |||                                         |||
  * ||             |||                                         |||
  * ||             |||                                         |||
  * ||             |||                                         |||
  * ||             |||                                         |||
  * ||             ||+-----------------------------------------+||
  * ||             |||+-buttonLayout--------------------------+|||
  * ||             ||||                                       ||||
  * ||             |||+---------------------------------------+|||
  * ||             ||+-----------------------------------------+||
  * |+-------------++-------------------------------------------+|
  * +------------------------------------------------------------+
  * @endcode
  *
  * The top bar is filled with a KMyMoneyTitleLabel as known from
  * KMyMoney's views. To the left there is an area in the same color
  * as the title bar showing the steps for this wizard. Each such step
  * can consist of one or more wizard pages. At the bottom of this area
  * the text "Step x of y" is shown and updated. To the right of this
  * part, the actual wizard page is shown. At the bottom of the page
  * the class inserts a standard button widget consisting of a Help,
  * Back, Next/Finish and Cancel button.
  *
  * The wizard serves as container for the wizard pages. In order to access
  * the data filled into the pages, one would have to provide getter methods.
  *
  * Here is an example how this object could be used. Please also see the
  * example described with the KMyMoneyWizardPage class.
  *
  * @code
  *
  * class KNewUserGeneral;
  * class KNewUserPersonal;
  *
  * class KNewUserWizard : public KMyMoneyWizard
  * {
  *   Q_OBJECT
  * public:
  *   KNewUserWizard(QWidget* parent = 0, const char* name = 0, bool modal = false, WFlags flags = 0);
  *
  * private:
  *   KNewUserGeneral*  m_generalPage;
  *   KNewUserPersonal* m_personalPage;
  *   KNewUserFinal*    m_finalPage;
  *   // add more pages here
  *
  *   friend class KNewUserGeneral;
  *   friend class KNewUserPersonal;
  *   friend class KNewUserFinal;
  *   // add more pages here
  * };
  * @endcode
  *
  * The implementation is also easy and looks like this:
  *
  * @code
  * KNewUserWizard::KNewUserWizard(QWidget* parent, const char* name, bool modal, WFlags flags) :
  *   KMyMoneyWizard(parent, name, modal, flags)
  * {
  *   setTitle("KMyMoney New User Setup");
  *   addStep("General Data");
  *   addStep("Personal Data");
  *   addStep("Finish");
  *
  *   m_generalPage = new KNewUserGeneral(this);
  *   m_personalPage = new KNewUserPersonal(this);
  *   m_finalPage = new KNewUserFinal(this);
  *
  *   setFirstPage(m_testPage1);
  * }
  * @endcode
  *
  * Don't forget to call setFirstPage() to get things started.
  *
  * The code to use this whole structure would then look something like this:
  *
  * @code
  *     KNewUserWizard* wizard = new KNewUserWizard(this, "NewUserWizard");
  *     int rc = wizard->exec();
  * @endcode
  *
  * The return code of exec() is either @p QDialog::Accepted or
  * @p QDialog::Rejected.
  *
  * @note The implementation of this class is heavily based on ideas found at
  *       http://doc.trolltech.com/4.1/dialogs-complexwizard.html
  */
class KMyMoneyWizard : public QDialog
{
  Q_OBJECT
public:
  /**
    * Modify the title of the wizard to be @p txt.
    *
    * @param txt The text that should be used as title
    */
  void setTitle(const QString& txt);

  /**
    * Add step @p text to the wizard
    *
    * @param text Text to be shown for this step
    */
  void addStep(const QString& text);

  QValueList<KMyMoneyWizardPage*> historyPages() const { return m_history; }

protected:
  /**
    * Constructor (kept protected, so that one cannot create such an object directly)
    */
  KMyMoneyWizard(QWidget *parent = 0, const char *name = 0, bool modal = false, WFlags f = 0);

  /**
    * This method sets up the first page after creation of the object
    *
    * @param page pointer to first page of wizard
    */
  void setFirstPage(KMyMoneyWizardPage* page);

private slots:
  void backButtonClicked(void);
  void nextButtonClicked(void);
  void completeStateChanged(void);

private:
  /**
    * Switch to page which is currently the top of the history stack.
    * @p oldPage is a pointer to the current page or 0 if no page
    * is shown.
    *
    * @param oldPage pointer to currently displayed page
    */
  void switchPage(KMyMoneyWizardPage* oldPage);

  /**
    * This method selects the step given by @p step.
    *
    * @param step step to be selected
    */
  void selectStep(unsigned int step);

  /*
   * The buttons
   */
  KPushButton*          m_cancelButton;
  KPushButton*          m_backButton;
  KPushButton*          m_nextButton;
  KPushButton*          m_finishButton;
  KPushButton*          m_helpButton;

  /*
   * The layouts
   */
  QVBoxLayout*          m_wizardLayout;
  QVBoxLayout*          m_stepLayout;
  QVBoxLayout*          m_pageLayout;
  QHBoxLayout*          m_buttonLayout;

  /*
   * Some misc. widgets required
   */
  QFrame*               m_stepFrame;
  QLabel*               m_stepLabel;

  /*
   * The list of labels
   */
  QValueList<QLabel*>   m_steps;

  /*
   * The title bar
   */
  KMyMoneyTitleLabel*   m_titleLabel;

  /*
   * The history stack
   */
  QValueList<KMyMoneyWizardPage*> m_history;
};

#endif