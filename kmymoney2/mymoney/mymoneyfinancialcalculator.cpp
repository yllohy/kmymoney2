/***************************************************************************
                          mymoneyfinancialcalculator.cpp  -  description
                             -------------------
    begin                : Tue Oct 21 2003
    copyright            : (C) 2000-2003 by Michael Edwardes
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

#include <math.h>
#include <stdio.h>

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes


#include "mymoneyfinancialcalculator.h"
#include "mymoneyexception.h"

static inline long double dabs(const long double x)
{
  return (x >= 0.0) ? x : -x;
}

const long double MyMoneyFinancialCalculator::rnd(const long double x) const
{
  long double r;
  char  buf[50];

  if(m_prec > 0) {
    sprintf (buf, "%.*Lf", m_prec, x);
    sscanf (buf, "%Lf", &r);
  } else
    r = x;
  return r;
}

MyMoneyFinancialCalculator::MyMoneyFinancialCalculator()
{
  setPrec();
  setPF();
  setCF();
  setBep();
  setDisc();
  
  setNpp(0.0);
  setIr(0.0);
  setPv(0.0);
  setPmt(0.0);
  setFv(0.0);

  // clear the mask
  m_mask = 0;
}

MyMoneyFinancialCalculator::~MyMoneyFinancialCalculator()
{
}

void MyMoneyFinancialCalculator::setPrec(const unsigned short prec)
{
  m_prec = prec;
}

void MyMoneyFinancialCalculator::setPF(const unsigned short PF)
{
  m_PF = PF;
}

void MyMoneyFinancialCalculator::setCF(const unsigned short CF)
{
  m_CF = CF;
}

void MyMoneyFinancialCalculator::setBep(const bool bep)
{
  m_bep = bep;
}

void MyMoneyFinancialCalculator::setDisc(const bool disc)
{
  m_disc = disc;
}

void MyMoneyFinancialCalculator::setIr(const long double ir)
{
  m_ir = ir;
  m_mask |= IR_SET;
}

void MyMoneyFinancialCalculator::setPv(const long double pv)
{
  m_pv = pv;
  m_mask |= PV_SET;
}

void MyMoneyFinancialCalculator::setPmt(const long double pmt)
{
  m_pmt = pmt;
  m_mask |= PMT_SET;
}

void MyMoneyFinancialCalculator::setNpp(const long double npp)
{
  m_npp = npp;
  m_mask |= NPP_SET;
}

void MyMoneyFinancialCalculator::setFv(const long double fv)
{
  m_fv = fv;
  m_mask |= FV_SET;
}

const long double MyMoneyFinancialCalculator::numPayments(void)
{
  const unsigned short mask = PV_SET | IR_SET | PMT_SET | FV_SET;

  if((m_mask & mask) != mask)
    throw new MYMONEYEXCEPTION("Not all parameters set for calculation of numPayments");

  long double eint = eff_int();
  long double CC = _C(eint);

  CC = (CC - m_fv) / (CC + m_pv);
  m_npp = (CC > 0.0) ? logl (CC) / logl (eint +1.0) : 0.0;

  m_mask |= NPP_SET;
  return m_npp;
}

const long double MyMoneyFinancialCalculator::payment(void)
{
  const unsigned short mask = PV_SET | IR_SET | NPP_SET | FV_SET;

  if((m_mask & mask) != mask)
    throw new MYMONEYEXCEPTION("Not all parameters set for calculation of payment");

  long double eint = eff_int();
  long double AA = _A(eint);
  long double BB = _B(eint);

  m_pmt = -floor((m_fv + m_pv * (AA + 1.0)) / (AA * BB));

  m_mask |= PMT_SET;
  return m_pmt;
}

const long double MyMoneyFinancialCalculator::presentValue(void)
{
  const unsigned short mask = PMT_SET | IR_SET | NPP_SET | FV_SET;

  if((m_mask & mask) != mask)
    throw new MYMONEYEXCEPTION("Not all parameters set for calculation of payment");

  long double eint = eff_int();
  long double AA = _A(eint);
  long double CC = _C(eint);
  
  m_pv = roundl(-(m_fv + (AA * CC)) / (AA + 1.0));

  m_mask |= PV_SET;
  return m_pv;
}

const long double MyMoneyFinancialCalculator::futureValue(void)
{
  const unsigned short mask = PMT_SET | IR_SET | NPP_SET | PV_SET;

  if((m_mask & mask) != mask)
    throw new MYMONEYEXCEPTION("Not all parameters set for calculation of payment");

  long double eint = eff_int();
  long double AA = _A(eint);
  long double CC = _C(eint);
  m_fv = roundl(-(m_pv + AA * (m_pv + CC)));

  m_mask |= FV_SET;
  return m_fv;
}

const long double MyMoneyFinancialCalculator::interestRate(void)
{
  long double eint;
  long double a, dik;
  const long double ratio = 1e4;
  int ri;

  if (m_pmt == 0.0) {
    eint = powl ((dabs (m_fv) / dabs (m_pv)), (1.0 / m_npp)) - 1.0;
  } else {
    if ((m_pmt * m_fv) < 0.0) {
      if(m_pv)
        a = -1.0;
      else
        a = 1.0;
      eint =
        dabs ((m_fv + a * m_npp * m_pmt) /
              (3.0 *
               ((m_npp - 1.0) * (m_npp - 1.0) * m_pmt + m_pv -
                m_fv)));
    } else {
      if ((m_pv * m_pmt) < 0.0) {
        eint = dabs ((m_npp * m_pmt + m_pv + m_fv) / (m_npp * m_pv));
      } else {
        a = dabs (m_pmt / (dabs(m_pv) + dabs(m_fv)));
        eint = a + 1.0 / (a * m_npp * m_npp * m_npp);
      }
    }
    do {
      dik = _fi(eint) / _fip(eint);
      eint -= dik;
      (void) modfl(ratio * (dik / eint), &a);
      ri = static_cast<unsigned> (a);
    }
    while (ri);
  }
  m_mask |= IR_SET;
  m_ir = rnd(nom_int(eint) * 100.0);
  return m_ir;
}

const long double MyMoneyFinancialCalculator::_fi(const long double eint) const
{
  return _A(eint) * (m_pv + _C(eint)) + m_pv + m_fv;
}

const long double MyMoneyFinancialCalculator::_fip(const long double eint) const
{
  double AA = _A(eint);
  double CC = _C(eint);
  double D = (AA + 1.0) / (eint + 1.0);

  return m_npp *(m_pv + CC) * D - (AA * CC) / eint;
}

const long double MyMoneyFinancialCalculator::_A(const long double eint) const
{
  return powl ((eint + 1.0), m_npp) - 1.0;
}

const long double MyMoneyFinancialCalculator::_B(const long double eint) const
{
  if(eint == 0.0)
    throw new MYMONEYEXCEPTION("Zero interest");

  if(m_bep == false)
    return static_cast<long double>(1.0) / eint;

  return (eint + 1.0) / eint;
}

const long double MyMoneyFinancialCalculator::_C(const long double eint) const
{
  return m_pmt * _B(eint);
}

const long double MyMoneyFinancialCalculator::eff_int(void) const
{
  long double nint = m_ir / 100.0;
  long double eint;

  if(m_disc) {              // periodically compound?
    if(m_CF == m_PF) {      // same frequency?
      eint = nint / static_cast<long double>(m_CF);
      
    } else {
      eint = powl((static_cast<long double>(1.0) + nint / static_cast<long double>(m_CF)),
                  (static_cast<long double>(m_CF) / static_cast<long double>(m_PF))) - 1.0;
      
    }
    
  } else {
    eint = expl(nint / static_cast<long double>(m_PF)) - 1.0;
  }
  
  return eint;
}

const long double MyMoneyFinancialCalculator::nom_int(const long double eint) const
{
  long double nint;

  if(m_disc) {
    if(m_CF == m_PF) {
      nint = m_CF * eint;
      
    } else {
      nint = m_CF * (powl ((eint + 1.0), (static_cast<long double>(m_PF) / static_cast<long double>(m_CF))) - 1.0);
    }
  } else
    nint = logl (powl (eint + 1.0, m_PF));

  return nint;
}
