/***************************************************************************
                          kmymoneytitlelabel.cpp
                             -------------------
    begin                : Sun Feb 05 2005
    copyright            : (C) 2005 by Ace Jones
    email                : acejones@users.sourceforge.net
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

#include <qpixmap.h>
#include <qvariant.h>
#include <qstyle.h>
#include <qpainter.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kglobal.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <kglobalsettings.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneytitlelabel.h"

kMyMoneyTitleLabel::kMyMoneyTitleLabel(QWidget *parent, const char *name)
 :  QLabel(parent, name),
    m_bgColor( KGlobalSettings::highlightColor() ),
    m_textColor( KGlobalSettings::highlightedTextColor() )
{
  QFont f = font();
  f.setPointSize(14);
  f.setStyleHint( QFont::SansSerif, QFont::PreferAntialias );
  setFont(f);
}

kMyMoneyTitleLabel::~kMyMoneyTitleLabel()
{
}

void kMyMoneyTitleLabel::setLeftImageFile(const QCString& _file)
{
  m_leftImageFile = _file;
  QString lfullpath = KGlobal::dirs()->findResource("appdata",QString(m_leftImageFile));
  m_leftImage.load(lfullpath);
  m_leftImage.setAlphaBuffer(true);
}

void kMyMoneyTitleLabel::setRightImageFile(const QCString& _file)
{
  m_rightImageFile = _file;
  QString rfullpath = KGlobal::dirs()->findResource("appdata",QString(m_rightImageFile));
  m_rightImage.load(rfullpath);
  m_rightImage.setAlphaBuffer(true);
  setMinimumHeight( m_rightImage.height() );
  setMaximumHeight( m_rightImage.height() );
}

void kMyMoneyTitleLabel::resizeEvent ( QResizeEvent * )
{
  QRect cr = contentsRect();
  QImage output( cr.width(), cr.height(), 32 );
  output.fill( m_bgColor.rgb() );

  bitBlt ( &output, cr.width() - m_rightImage.width(), 0, &m_rightImage, 0, 0, m_rightImage.width(), m_rightImage.height(), 0 );
  bitBlt ( &output, 0, 0, &m_leftImage, 0, 0, m_leftImage.width(), m_leftImage.height(), 0 );

  QPixmap pix;
  pix.convertFromImage(output);
  setPixmap(pix);
  setMinimumWidth( m_rightImage.width() );
}

void kMyMoneyTitleLabel::drawContents(QPainter *p)
{
  // first draw pixmap
  QLabel::drawContents(p);

  // then draw text on top
  style().drawItem( p, contentsRect(), alignment(), colorGroup(), isEnabled(),
                          0, QString("   ")+m_text, -1, &m_textColor );
}

void kMyMoneyTitleLabel::setText(const QString& txt)
{
  m_text = txt;
  update();
}
