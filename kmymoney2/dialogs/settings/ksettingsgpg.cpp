/***************************************************************************
                             ksettingsgpg.cpp
                             --------------------
    copyright            : (C) 2005 by Thomas Baumgart
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

#include <qgroupbox.h>
#include <qcheckbox.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kled.h>
#include <klineedit.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "ksettingsgpg.h"
#include <kmymoney/kgpgfile.h>

#define RECOVER_KEY_ID  "0xD2B08440"

KSettingsGpg::KSettingsGpg(QWidget* parent, const char* name) :
  KSettingsGpgDecl(parent, name),
  m_checkCount(0)
{
  m_idGroup->setEnabled(KGPGFile::GPGAvailable());
  m_recoveryGroup->setEnabled(KGPGFile::keyAvailable(RECOVER_KEY_ID));

  m_userKeyFound->off();
  m_recoverKeyFound->off();

  connect(kcfg_WriteDataEncrypted, SIGNAL(toggled(bool)), this, SLOT(slotStatusChanged(bool)));
  connect(kcfg_GpgRecipient, SIGNAL(textChanged(const QString&)), this, SLOT(slotIdChanged(const QString&)));
}

KSettingsGpg::~KSettingsGpg()
{
}

void KSettingsGpg::slotIdChanged(const QString& /*txt*/)
{
  // this looks a bit awkward. Here's why: KGPGFile::keyAvailable() starts
  // an external task and processes UI events while it waits for the external
  // process to finish. Thus, the first time we get here, the external process
  // is started and the user my press a second key which calls this routine
  // again.
  //
  // The second invocation is counted, but the check is not started until the
  // first one finishes. Once the external process finishes, we check if we
  // were called in the meantime and restart the check.
  if(++m_checkCount == 1) {
    while(1) {
      if(kcfg_GpgRecipient->text().stripWhiteSpace().length() > 0) {
        m_userKeyFound->setState((KLed::State) (KGPGFile::keyAvailable(kcfg_GpgRecipient->text()) ? KLed::On : KLed::Off));
        if(m_checkCount > 1) {
          m_checkCount = 1;
          continue;
        }
      } else {
        m_userKeyFound->setState(KLed::Off);
      }
      break;
    }
    --m_checkCount;
  }
}

void KSettingsGpg::slotStatusChanged(bool state)
{
  if(state && !KGPGFile::GPGAvailable())
    state = false;

  m_idGroup->setEnabled(state);
  m_recoveryGroup->setEnabled(state);

  if(state) {
    m_recoverKeyFound->setState((KLed::State) (KGPGFile::keyAvailable(RECOVER_KEY_ID) ? KLed::On : KLed::Off));
    if(kcfg_GpgRecipient->text().isEmpty())
      m_userKeyFound->setState(KLed::Off);
    else
      m_userKeyFound->setState((KLed::State) (KGPGFile::keyAvailable(kcfg_GpgRecipient->text()) ? KLed::On : KLed::Off));
  } else {
    m_recoverKeyFound->setState(KLed::Off);
    m_userKeyFound->setState(KLed::Off);
  }
}

#include "ksettingsgpg.moc"
