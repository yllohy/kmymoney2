/***************************************************************************
                         knewinvestmentwizard  -  description
                            -------------------
   begin                : Sat Dec 4 2004
   copyright            : (C) 2004 by Thomas Baumgart
   email                : kmymoney2-developers
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

#ifndef KNEWINVESTMENTWIZARD_H
#define KNEWINVESTMENTWIZARD_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "../dialogs/knewinvestmentwizarddecl.h"
#include "../mymoney/mymoneyaccount.h"
#include "../mymoney/mymoneysecurity.h"

/**
  * This class contains the implementation of the new investment wizard.
  *
  * @author Thomas Baumgart
  */
class KNewInvestmentWizard : public KNewInvestmentWizardDecl
{
  Q_OBJECT
public:
  /**
    * Use this constructor for the creation of a new investment
    */
  KNewInvestmentWizard( QWidget *parent = 0, const char *name = 0 );

  /**
    * Use this constructor for the modification of an existing investment
    */
  KNewInvestmentWizard( const MyMoneyAccount& acc, QWidget *parent = 0, const char *name = 0 );

  /**
    * Use this constructor for the modification of an existing security
    */
  KNewInvestmentWizard( const MyMoneySecurity& sec, QWidget *parent = 0, const char *name = 0 );

  ~KNewInvestmentWizard();

  /**
    * Depending on the constructor used, this method either
    * creates all necessary objects for the investment or updates
    * them.
    *
    * @param parentId id of parent account for the investment
    */
  void createObjects(const QCString& parentId);

protected slots:
  void next(void);
  void slotCheckPage(void);

private:
  void init(void);

private:
  MyMoneyAccount    m_account;
  MyMoneySecurity   m_security;
  bool              m_createAccount;
};

#endif
