/***************************************************************************
                          kmymoneyfile.h  -  description
                             -------------------
    begin                : Mon Jun 10 2002
    copyright            : (C) 2000-2002 by Michael Edwardes
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

#ifndef KMYMONEYFILE_H
#define KMYMONEYFILE_H

#include "../mymoney/mymoneyfile.h"
#include "../mymoney/storage/mymoneyseqaccessmgr.h"

/**
  *@author Michael Edwardes
  */

class KMyMoneyFile {
  static KMyMoneyFile *m_instance;
  static MyMoneyFile *m_file;
  static MyMoneySeqAccessMgr *m_storage;

  KMyMoneyFile();
  KMyMoneyFile(const QString&);

public:
  ~KMyMoneyFile();
  static KMyMoneyFile *instance();
  static MyMoneyFile* file();
  static MyMoneySeqAccessMgr* storage();
  static void reset();
};

#endif
