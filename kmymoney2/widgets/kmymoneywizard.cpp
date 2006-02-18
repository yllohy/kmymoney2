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
  m_widget(widget)
{
  d = new KMyMoneyWizardPagePrivate(widget, name);
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

KMyMoneyWizardPage* KMyMoneyWizardPage::nextPage(void)
{
  return 0;
}

bool KMyMoneyWizardPage::isLastPage(void)
{
  return false;
}

bool KMyMoneyWizardPage::isComplete(void)
{
  return true;
}



KMyMoneyWizard::KMyMoneyWizard(QWidget *parent, const char *name, bool modal, WFlags f) :
  QDialog(parent, name, modal, f)
{
  // enable the little grip in the right corner
  setSizeGripEnabled(true);

  // create buttons
  m_cancelButton = new KPushButton(this, i18n("&Cancel"));
  m_backButton = new KPushButton(this, i18n("&Back"));
  m_nextButton = new KPushButton(this, i18n("&Next"));
  m_finishButton = new KPushButton(this, i18n("&Finish"));
  m_helpButton = new KPushButton(this, i18n("&Help"));

  bool useIcons = KGlobalSettings::showIconsOnPushButtons();

  if ( useIcons )
  {
    KGuiItem back = KStdGuiItem::back( KStdGuiItem::UseRTL );
    KGuiItem forward = KStdGuiItem::forward( KStdGuiItem::UseRTL );

    m_backButton->setIconSet( back.iconSet() );
    m_nextButton->setIconSet( forward.iconSet() );

    m_finishButton->setIconSet( SmallIconSet( "apply" ) );
    m_cancelButton->setIconSet( SmallIconSet( "button_cancel" ) );
    m_helpButton->setIconSet( SmallIconSet( "help" ) );

    m_backButton->setText( i18n( "&Back" ) );
    m_nextButton->setText( i18n( "&Next" ) );
    m_finishButton->setText( i18n( "&Finish" ) );
    m_cancelButton->setText( i18n( "&Cancel" ) );
    m_helpButton->setText( i18n( "&Help" ) );
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
  m_pageLayout->addItem(new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding));

  m_pageLayout->addLayout(m_buttonLayout);
  hboxLayout->addLayout(m_pageLayout);
  m_wizardLayout->addLayout(hboxLayout);

  resize(QSize(600, 400).expandedTo(minimumSizeHint()));
  clearWState(WState_Polished);

  m_titleLabel->setText("Title");
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

void KMyMoneyWizard::selectStep(unsigned int step)
{
  if(step < 1 || step > m_steps.count())
    return;

  m_stepLabel->setText(i18n("Step %1 of %2").arg(step).arg(m_steps.count()));
  QValueList<QLabel*>::iterator it_l;
  QFont f = m_steps[0]->font();
  for(it_l = m_steps.begin(); it_l != m_steps.end(); ++it_l) {
    f.setBold(false);
    if(--step == 0) {
      f.setBold(true);
    }
    (*it_l)->setFont(f);
  }
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
  }
  completeStateChanged();
}

void KMyMoneyWizard::backButtonClicked(void)
{
  KMyMoneyWizardPage* oldPage = m_history.back();
  m_history.pop_back();
  oldPage->resetPage();
  switchPage(oldPage);
}

void KMyMoneyWizard::nextButtonClicked(void)
{
  KMyMoneyWizardPage* oldPage = m_history.back();
  KMyMoneyWizardPage* newPage = oldPage->nextPage();
  m_history.append(newPage);
  newPage->resetPage();
  switchPage(oldPage);
}

void KMyMoneyWizard::completeStateChanged(void)
{
  KMyMoneyWizardPage* currentPage = m_history.back();
  bool lastPage = currentPage->isLastPage();

  m_finishButton->setShown(lastPage);
  m_nextButton->setShown(!lastPage);

  if(lastPage)
    m_finishButton->setEnabled(currentPage->isComplete());
  else
    m_nextButton->setEnabled(currentPage->isComplete());

  m_backButton->setEnabled(m_history.count() > 1);
}

#include "kmymoneywizard.moc"

