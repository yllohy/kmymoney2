/***************************************************************************
                             ksettingsgpg.cpp
                             --------------------
    copyright            : (C) 2005, 2008 by Thomas Baumgart
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
#include <qregexp.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kled.h>
#include <klineedit.h>
#include <keditlistbox.h>
#include <kcombobox.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "ksettingsgpg.h"
#include <kmymoney/kgpgfile.h>

#define RECOVER_KEY_ID      "0xD2B08440"
#define RECOVER_KEY_ID_FULL "59B0F826D2B08440"

KSettingsGpg::KSettingsGpg(QWidget* parent, const char* name) :
  KSettingsGpgDecl(parent, name),
  m_checkCount(0),
  m_needCheckList(true),
  m_listOk(false)
{
  setEnabled(KGPGFile::GPGAvailable());

  // don't show the widget in which the master key is actually kept
  kcfg_GpgRecipient->hide();

  connect(kcfg_WriteDataEncrypted, SIGNAL(toggled(bool)), this, SLOT(slotStatusChanged(bool)));
  connect(m_masterKeyCombo, SIGNAL(activated(int)), this, SLOT(slotIdChanged()));
  connect(kcfg_GpgRecipientList, SIGNAL(changed()), this, SLOT(slotIdChanged()));
  connect(kcfg_GpgRecipientList, SIGNAL(added(const QString&)), this, SLOT(slotKeyListChanged()));
  connect(kcfg_GpgRecipientList, SIGNAL(removed(const QString&)), this, SLOT(slotKeyListChanged()));

  // Initial state setup
  slotStatusChanged(kcfg_WriteDataEncrypted->isChecked());
}

KSettingsGpg::~KSettingsGpg()
{
}

void KSettingsGpg::slotKeyListChanged(void)
{
  m_needCheckList = true;
  slotIdChanged();
}

void KSettingsGpg::slotIdChanged(void)
{
  // this looks a bit awkward. Here's why: KGPGFile::keyAvailable() starts
  // an external task and processes UI events while it waits for the external
  // process to finish. Thus, the first time we get here, the external process
  // is started and the user may press a second key which calls this routine
  // again.
  //
  // The second invocation is counted, but the check is not started until the
  // first one finishes. Once the external process finishes, we check if we
  // were called in the meantime and restart the check.
  if(++m_checkCount == 1) {
    while(1) {
      // first we check the current edit field if filled
      bool keysOk = true;
      if(!kcfg_GpgRecipientList->currentText().isEmpty()) {
        keysOk = KGPGFile::keyAvailable(kcfg_GpgRecipientList->currentText());
      }

      // if it is available, then scan the current list if we need to
      if(keysOk) {
        if(m_needCheckList) {
          QStringList keys = kcfg_GpgRecipientList->items();
          QStringList::const_iterator it_s;
          for(it_s = keys.begin(); keysOk && it_s != keys.end(); ++it_s) {
            if(!KGPGFile::keyAvailable(*it_s))
              keysOk = false;
          }
          m_listOk = keysOk;
          m_needCheckList = false;

        } else {
          keysOk = m_listOk;
        }
      }

      // did we receive some more requests to check?
      if(m_checkCount > 1) {
        m_checkCount = 1;
        continue;
      }

      // if we have a master key, we store it in the hidden widget
      if(m_masterKeyCombo->currentItem() != 0) {
        QRegExp keyExp(".* \\((.*)\\)");
        if(keyExp.search(m_masterKeyCombo->currentText()) != -1) {
          kcfg_GpgRecipient->setText(keyExp.cap(1));
        }
      }

      m_userKeysFound->setState(static_cast<KLed::State>(keysOk && (kcfg_GpgRecipientList->items().count() != 0) ? KLed::On : KLed::Off));
      break;
    }

    --m_checkCount;
  }
}

void KSettingsGpg::show(void)
{
  QString masterKey;

  if(m_masterKeyCombo->currentItem() != 0) {
    QRegExp keyExp(".* \\((.*)\\)");
    if(keyExp.search(m_masterKeyCombo->currentText()) != -1) {
      masterKey = keyExp.cap(1);
    }
  } else
    masterKey = kcfg_GpgRecipient->text();

  // fill the secret key combobox with a fresh list
  m_masterKeyCombo->clear();
  QStringList keyList;
  KGPGFile::secretKeyList(keyList);

  for(QStringList::iterator it = keyList.begin(); it != keyList.end(); ++it) {
    QStringList fields = QStringList::split(":", *it);
    if(fields[0] != RECOVER_KEY_ID_FULL) {
      // replace parenthesis in name field with brackets
      QString name = fields[1];
      name.replace('(', "[");
      name.replace(')', "]");
      name = QString("%1 (0x%2)").arg(name).arg(fields[0]);
      m_masterKeyCombo->insertItem(name);
      if(name.contains(masterKey))
        m_masterKeyCombo->setCurrentItem(name);
    }
  }

  // if we don't have at least one secret key, we turn off encryption
  if(keyList.isEmpty()) {
    setEnabled(false);
    kcfg_WriteDataEncrypted->setChecked(false);
  }

  slotStatusChanged(kcfg_WriteDataEncrypted->isChecked());
  KSettingsGpgDecl::show();
}

void KSettingsGpg::slotStatusChanged(bool state)
{
  if(state && !KGPGFile::GPGAvailable())
    state = false;

  m_idGroup->setEnabled(state);
  kcfg_EncryptRecover->setEnabled(state);
  m_masterKeyCombo->setEnabled(state);
  kcfg_GpgRecipientList->setEnabled(state);

  if(state) {
    m_recoverKeyFound->setState((KLed::State) (KGPGFile::keyAvailable(RECOVER_KEY_ID) ? KLed::On : KLed::Off));
    kcfg_EncryptRecover->setEnabled(m_recoverKeyFound);
    slotIdChanged();

  } else {
    m_recoverKeyFound->setState(KLed::Off);
    m_userKeysFound->setState(KLed::Off);
  }
}

#include "ksettingsgpg.moc"
