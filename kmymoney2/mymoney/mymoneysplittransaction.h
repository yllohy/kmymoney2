/***************************************************************************
                          mymoneysplittransaction.h  -  description
                             -------------------
    begin                : Wed Jan 9 2002
    copyright            : (C) 2000-2002 by Thomas Baumgart
    email                : ipwizard@users.sourceforge.net
                             Javier Campos Morales <javi_c@users.sourceforge.net>
                             Felix Rodriguez <frodriguez@users.sourceforge.net>
                             John C <thetacoturtle@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MYMONEYSPLITTRANSACTION_H
#define MYMONEYSPLITTRANSACTION_H

#include "mymoneytransactionbase.h"

class MyMoneyTransaction;

/**Objects of this class represent a single
part of a splitted transaction 
  *@author Thomas Baumgart
  */

class MyMoneySplitTransaction : public MyMoneyTransactionBase  {
public: 
	MyMoneySplitTransaction();
  MyMoneySplitTransaction(MyMoneyTransaction* parent);
	~MyMoneySplitTransaction();

  /**
    * Returns true for MyMoneySplitTransactions
    *
    * @return always true
    */
  virtual bool isSplit(void) { return true; };

  // modify the parent
  void setParent(MyMoneyTransaction&);

protected:
  // set the dirty flag of the parent
  void setDirty(const bool flag);


private:
  // backward pointer to master transaction
  MyMoneyTransaction* m_parent;

};

#endif
