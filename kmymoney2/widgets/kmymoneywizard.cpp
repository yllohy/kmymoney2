/***************************************************************************
                             kmymoneywizard.cpp
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

// ----------------------------------------------------------------------------
// QT Includes

#include <qlayout.h>
#include <qlabel.h>
#include <qpoint.h>
#include <qfont.h>
#include <qframe.h>
#include <qtooltip.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <klocale.h>
#include <kpushbutton.h>
#include <kstdguiitem.h>
#include <kglobalsettings.h>
#include <kiconloader.h>

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoney/kmymoneywizard.h>
#include "kmymoneywizard_p.h"
#include <kmymoney/kmymoneytitlelabel.h>
#include <kmymoney/kguiutils.h>

KMyMoneyWizardPagePrivate::KMyMoneyWizardPagePrivate(QObject* parent, const char* name) :
  QObject(parent, name)
{
}

void KMyMoneyWizardPagePrivate::emitCompleteStateChanged(void)
{
  emit completeStateChanged();
}


KMyMoneyWizardPage::KMyMoneyWizardPage(unsigned int step, QWidget* widget, const char* name) :
  m_step(step),
  m_widget(widget),
  d(new KMyMoneyWizardPagePrivate(widget, name))
{
  m_mandatoryGroup = new kMandatoryFieldGroup(widget);
  QObject::connect(m_mandatoryGroup, SIGNAL(stateChanged()), object(), SIGNAL(completeStateChanged()));
  widget->hide();
}

QObject* KMyMoneyWizardPage::object(void) const
{
  return d;
}

void KMyMoneyWizardPage::completeStateChanged(void) const
{
  d->emitCompleteStateChanged();
}

void KMyMoneyWizardPage::resetPage(void)
{
}

void KMyMoneyWizardPage::enterPage(void)
{
}

void KMyMoneyWizardPage::leavePage(void)
{
}

KMyMoneyWizardPage* KMyMoneyWizardPage::nextPage(void) const
{
  return 0;
}

bool KMyMoneyWizardPage::isLastPage(void) const
{
  return nextPage() == 0;
}

bool KMyMoneyWizardPage::isComplete(void) const
{
  if(!isLastPage())
    QToolTip::add(wizard()->m_nextButton, i18n("Continue with next page"));
  else
    QToolTip::add(wizard()->m_finishButton, i18n("Finish wizard"));
  return m_mandatoryGroup->isEnabled();
}

KMyMoneyWizard::KMyMoneyWizard(QWidget *parent, const char *name, bool modal, WFlags f) :
  QDialog(parent, name, modal, f),
  m_step(0)
{
  // enable the little grip in the right corner
  setSizeGripEnabled(true);

  // create buttons
  m_cancelButton = new KPushButton(i18n("&Cancel"), this);
  m_backButton = new KPushButton(i18n("&Back"), this);
  m_nextButton = new KPushButton(i18n("&Next"), this);
  m_finishButton = new KPushButton(i18n("&Finish"), this);
  m_helpButton = new KPushButton(i18n("&Help"), this);

  if ( KGlobalSettings::showIconsOnPushButtons() )
  {
    m_backButton->setIconSet( KStdGuiItem::back( KStdGuiItem::UseRTL ).iconSet() );
    m_nextButton->setIconSet( KStdGuiItem::forward( KStdGuiItem::UseRTL ).iconSet() );
    m_finishButton->setIconSet( SmallIconSet( "apply" ) );
    m_cancelButton->setIconSet( SmallIconSet( "button_cancel" ) );
    m_helpButton->setIconSet( SmallIconSet( "help" ) );
  }

  // create button layout
  m_buttonLayout = new QHBoxLayout;
  m_buttonLayout->addWidget(m_helpButton);
  m_buttonLayout->addStretch(1);
  m_buttonLayout->addWidget(m_backButton);
  m_buttonLayout->addWidget(m_nextButton);
  m_buttonLayout->addWidget(m_finishButton);
  m_buttonLayout->addWidget(m_cancelButton);

  // create wizard layout
  m_wizardLayout = new QVBoxLayout(this, 6, 0, "wizardLayout");
  m_titleLabel = new KMyMoneyTitleLabel(this, "titleLabel");
  m_wizardLayout->addWidget(m_titleLabel);

  QHBoxLayout* hboxLayout = new QHBoxLayout(0, 0, 6, "hboxLayout");

  // create stage layout and frame
  m_stepFrame = new QFrame(this, "stepFrame");
  m_stepFrame->setPaletteBackgroundColor(KGlobalSettings::highlightColor());
  m_stepLayout = new QVBoxLayout(m_stepFrame, 11, 6, "stepLayout");
  m_stepLayout->addWidget(new QLabel("", m_stepFrame));
  m_stepLayout->addItem(new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding));
  m_stepLabel = new QLabel(m_stepFrame, "stepLabel");
  m_stepLabel->setAlignment(Qt::AlignHCenter);
  m_stepLayout->addWidget(m_stepLabel);
  hboxLayout->addWidget(m_stepFrame);

  // create page layout
  m_pageLayout = new QVBoxLayout(0, 0, 6, "pageLayout");

  // the page will be inserted later dynamically above this line
  QFrame* line = new QFrame( this, "line" );
  line->setFrameShadow( QFrame::Sunken );
  line->setFrameShape( QFrame::HLine );
  m_pageLayout->addWidget( line );
  m_pageLayout->addLayout(m_buttonLayout);

  // now glue everything together
  hboxLayout->addLayout(m_pageLayout);
  m_wizardLayout->addLayout(hboxLayout);

  resize(QSize(770, 520).expandedTo(minimumSizeHint()));
  clearWState(WState_Polished);

  m_titleLabel->setText("No Title specified");
  m_titleLabel->setRightImageFile("pics/titlelabel_background.png");

  m_finishButton->hide();

  connect(m_backButton, SIGNAL(clicked()), this, SLOT(backButtonClicked()));
  connect(m_nextButton, SIGNAL(clicked()), this, SLOT(nextButtonClicked()));
  connect(m_cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
  connect(m_finishButton, SIGNAL(clicked()), this, SLOT(accept()));
}

void KMyMoneyWizard::setTitle(const QString& txt)
{
  m_titleLabel->setText(txt);
}

void KMyMoneyWizard::addStep(const QString& text)
{
  QLabel* step = new QLabel(text, m_stepFrame);
  step->setFrameStyle(QFrame::Panel | QFrame::Raised);
  step->setAlignment(Qt::AlignHCenter);
  step->setFrameStyle(QFrame::Box | QFrame::Sunken);
  step->setMargin(2);
  m_steps.append(step);
  m_stepLayout->insertWidget(m_steps.count(), step);

  QFont font(step->font());
  font.setBold(true);
  QFontMetrics fm(font);
  int w = fm.width(text)+30;
  if(m_stepFrame->minimumWidth() < w) {
    m_stepFrame->setMinimumWidth(w);
  }
}

void KMyMoneyWizard::setStepHidden(unsigned int step, bool hidden)
{
  if((step < 1) || (step > m_steps.count()))
    return;

  m_steps[--step]->setHidden(hidden);
  updateStepCount();
}

void KMyMoneyWizard::selectStep(unsigned int step)
{
  if((step < 1) || (step > m_steps.count()))
    return;

  m_step = step;
  QValueList<QLabel*>::iterator it_l;
  QFont f = m_steps[0]->font();
  for(it_l = m_steps.begin(); it_l != m_steps.end(); ++it_l) {
    f.setBold(false);
    (*it_l)->setFrameStyle(QFrame::NoFrame);
    if(--step == 0) {
      f.setBold(true);
      (*it_l)->setFrameStyle(QFrame::Box | QFrame::Sunken);
    }
    (*it_l)->setFont(f);
  }
  updateStepCount();
}

void KMyMoneyWizard::updateStepCount(void)
{
  QValueList<QLabel*>::iterator it_l;
  int stepCount = 0;
  int hiddenAdjust = 0;
  int step = 0;
  for(it_l = m_steps.begin(); it_l != m_steps.end(); ++it_l) {
    if(!(*it_l)->isHidden())
      ++stepCount;
    else if(step < m_step)
      hiddenAdjust++;
    ++step;
  }
  m_stepLabel->setText(i18n("Step %1 of %2").arg(m_step - hiddenAdjust).arg(stepCount));
}

void KMyMoneyWizard::setFirstPage(KMyMoneyWizardPage* page)
{
  page->resetPage();
  m_history.clear();
  m_history.append(page);
  switchPage(0);
}

void KMyMoneyWizard::switchPage(KMyMoneyWizardPage* oldPage)
{
  if(oldPage) {
    oldPage->widget()->hide();
    m_pageLayout->remove(oldPage->widget());
    disconnect(oldPage->object(), SIGNAL(completeStateChanged()), this, SLOT(completeStateChanged()));
  }
  KMyMoneyWizardPage* newPage = m_history.back();
  if(newPage) {
    m_pageLayout->insertWidget(0, newPage->widget());
    connect(newPage->object(), SIGNAL(completeStateChanged()), this, SLOT(completeStateChanged()));
    newPage->widget()->show();
    selectStep(newPage->step());
    if(newPage->isLastPage()) {
      m_nextButton->setDefault(false);
      m_finishButton->setDefault(true);
    } else {
      m_finishButton->setDefault(false);
      m_nextButton->setDefault(true);
    }
    QWidget* w = newPage->initialFocusWidget();
    if(w)
      w->setFocus();
  }
  completeStateChanged();
}

void KMyMoneyWizard::backButtonClicked(void)
{
  KMyMoneyWizardPage* oldPage = m_history.back();
  m_history.pop_back();
  oldPage->leavePage();
  oldPage->resetPage();
  switchPage(oldPage);
}

void KMyMoneyWizard::nextButtonClicked(void)
{
  // make sure it is really complete. Some widgets only change state during focusOutEvent,
  // so we just create such an animal by changing the focus to the next button and
  // check again for copmpleness
  m_nextButton->setFocus();
  KMyMoneyWizardPage* oldPage = m_history.back();
  if(oldPage->isComplete()) {
    KMyMoneyWizardPage* newPage = oldPage->nextPage();
    m_history.append(newPage);
    newPage->enterPage();
    newPage->resetPage();
    switchPage(oldPage);
  }
}

void KMyMoneyWizard::completeStateChanged(void)
{
  KMyMoneyWizardPage* currentPage = m_history.back();
  bool lastPage = currentPage->isLastPage();

  m_finishButton->setShown(lastPage);
  m_nextButton->setShown(!lastPage);

  KPushButton* button;

  button = lastPage ? m_finishButton : m_nextButton;

  bool rc = currentPage->isComplete();
  button->setEnabled(rc);

  m_backButton->setEnabled(m_history.count() > 1);
}

void KMyMoneyWizard::accept(void)
{
  // make sure it is really complete. Some widgets only change state during focusOutEvent,
  // so we just create such an animal by changing the focus to the finish button and
  // check again for completeness.
  m_finishButton->setFocus();
  KMyMoneyWizardPage* page = m_history.back();
  if(page->isComplete())
    QDialog::accept();
}

#include "kmymoneywizard.moc"

