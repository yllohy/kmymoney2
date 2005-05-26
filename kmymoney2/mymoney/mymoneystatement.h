/***************************************************************************
                          mymoneystatement.h
                          -------------------
    begin                : Mon Aug 30 2004
    copyright            : (C) 2000-2004 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
                           Ace Jones <acejones@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MYMONEYSTATEMENT_H
#define MYMONEYSTATEMENT_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qstring.h>
#include <qvaluelist.h>
#include <qdatetime.h>
#include <kmymoney/export.h>

class QDomElement;
class QDomDocument;
  
/**
Represents the electronic analog of the paper bank statement just like we used to get in the regular mail.  This class is designed to be easy to extend and easy to create with minimal dependencies.  So the header file should include as few project files as possible (preferrably NONE).

@author ace jones
*/
struct MyMoneyStatement
{
  enum EType { etNone = 0, etCheckings, etSavings, etInvestment, etCreditCard, etEnd };
  
  struct Transaction
  {
    QDate m_datePosted;
    QString m_strPayee;
    QString m_strMemo;
    QString m_strNumber;
    QString m_strBankID;
    double m_moneyAmount;
    
    // the following members are only used when m_eType==etInvestment
    enum EAction { eaNone = 0, eaBuy, eaSell, eaReinvestDividend, eaCashDividend, eaEnd };
    EAction m_eAction;
    double m_dShares;
    QString m_strSecurity;  // should be security ID followed by name, e.g. "DIS The Disney Corporation"
  };
  
  struct Price
  {
    QDate m_date;
    QString m_strSecurity;
    double m_moneyAmount;
  };
  
  QString m_strAccountName;
  QString m_strAccountNumber;
  QString m_strCurrency;
  QDate m_dateBegin;
  QDate m_dateEnd;
  double m_moneyClosingBalance;
  EType m_eType;
  
  QValueList<Transaction> m_listTransactions;
  QValueList<Price> m_listPrices;
  
  void write(QDomElement&,QDomDocument*) const;
  bool read(const QDomElement&);
  
  KMYMONEY_EXPORT static bool isStatementFile(const QString&);
  KMYMONEY_EXPORT static bool readXMLFile( MyMoneyStatement&, const QString& );
  KMYMONEY_EXPORT static void writeXMLFile( const MyMoneyStatement&, const QString& );
};

#endif
