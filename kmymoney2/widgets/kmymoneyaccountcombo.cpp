/***************************************************************************
                         kmymoneyaccountbutton  -  description
                            -------------------
   begin                : Mon May 31 2004
   copyright            : (C) 2000-2004 by Michael Edwardes
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

// ----------------------------------------------------------------------------
// QT Includes

#include <qdrawutil.h>
#include <qpainter.h>
#include <qstyle.h>
#include <qapplication.h>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyaccountcombo.h"
#include "kmymoneyaccountcompletion.h"

kMyMoneyAccountCombo::kMyMoneyAccountCombo( QWidget* parent, const char* name ) :
    KPushButton( parent, name )
{
  m_selector = new kMyMoneyAccountCompletion(this, "selector");
  connect(this, SIGNAL(clicked()), this, SLOT(slotButtonPressed()));
  connect(m_selector, SIGNAL(accountSelected(const QCString&)), this, SIGNAL(accountSelected(const QCString&)));
}

kMyMoneyAccountCombo::~kMyMoneyAccountCombo()
{
}

void kMyMoneyAccountCombo::slotButtonPressed(void)
{
  m_selector->loadList();
  m_selector->show();
}

void kMyMoneyAccountCombo::setSelected(const MyMoneyAccount& acc)
{
  m_selector->setSelected(acc.id());
  setText(acc.name());
}

void kMyMoneyAccountCombo::paintEvent( QPaintEvent * )
{
  // copied the following code from qcombobox.cpp and
  // modified it to the local needs. (ipwizard)

  QPainter p( this );
  const QColorGroup & g = colorGroup();
  p.setPen(g.text());

  QStyle::SFlags flags = QStyle::Style_Default;
  if (isEnabled())
    flags |= QStyle::Style_Enabled;
  if (hasFocus())
    flags |= QStyle::Style_HasFocus;

  if ( width() < 5 || height() < 5 ) {
    qDrawShadePanel( &p, rect(), g, FALSE, 2,
        &g.brush( QColorGroup::Button ) );
    return;
  }

  bool reverse = QApplication::reverseLayout();

  if (style().styleHint(QStyle::SH_GUIStyle) == Qt::MotifStyle) {      // motif 1.x style
    int dist, buttonH, buttonW;
    dist     = 8;
    buttonH  = 7;
    buttonW  = 11;
    int xPos;
    int x0;
    int w = width() - dist - buttonW - 1;
    if ( reverse ) {
      xPos = dist + 1;
      x0 = xPos + 4;
    } else {
      xPos = w;
      x0 = 4;
    }
    qDrawShadePanel( &p, rect(), g, FALSE,
        style().pixelMetric(QStyle::PM_DefaultFrameWidth, this),
        &g.brush( QColorGroup::Button ) );
    qDrawShadePanel( &p, xPos, (height() - buttonH)/2,
        buttonW, buttonH, g, FALSE,
        style().pixelMetric(QStyle::PM_DefaultFrameWidth, this) );
    QRect clip( x0, 2, w - 2 - 4 - 5, height() - 4 );
    QString str = text();
    if ( !str.isNull() ) {
      p.drawText( clip, AlignCenter | SingleLine, str );
    }

    const QPixmap *pix = pixmap();
    QIconSet *iconSet = this->iconSet();
    if (pix || iconSet) {
      QPixmap pm = ( pix ? *pix : iconSet->pixmap() );
      p.setClipRect( clip );
      p.drawPixmap( 4, (height()-pm.height())/2, pm );
      p.setClipping( FALSE );
    }

    if ( hasFocus() )
      p.drawRect( xPos - 5, 4, width() - xPos + 1 , height() - 8 );
  } else {
    style().drawComplexControl( QStyle::CC_ComboBox, &p, this, rect(), g,
          flags, QStyle::SC_All & ~QStyle::SC_ComboBoxEditField,
          (true ?                        // d->arrowDown
            QStyle::SC_ComboBoxArrow :
            QStyle::SC_None ));

    QRect re = style().querySubControlMetrics( QStyle::CC_ComboBox, this,
                QStyle::SC_ComboBoxEditField );
    re = QStyle::visualRect(re, this);
    p.setClipRect( re );

    QString str = text();
    const QPixmap *pix = pixmap();
    if ( !str.isNull() ) {
      p.save();
      p.setFont(font());
      QFontMetrics fm(font());
      int x = re.x(), y = re.y() + fm.ascent();
      if( pix )
        x += pix->width() + 5;
      p.drawText( x, y, str );
      p.restore();
    }
    if ( pix ) {
      p.fillRect( re.x(), re.y(), pix->width() + 4, re.height(),
      colorGroup().brush( QColorGroup::Base ) );
      p.drawPixmap( re.x() + 2, re.y() +
        ( re.height() - pix->height() ) / 2, *pix );
    }
  }
#if 0
   else {
    style().drawComplexControl( QStyle::CC_ComboBox, &p, this, rect(), g,
              flags, QStyle::SC_All,
              (d->arrowDown ?
              QStyle::SC_ComboBoxArrow :
              QStyle::SC_None ));

    QRect re = style().querySubControlMetrics( QStyle::CC_ComboBox, this,
                QStyle::SC_ComboBoxEditField );
    re = QStyle::visualRect(re, this);
    p.setClipRect( re );

    if ( !d->ed ) {
      QListBoxItem * item = d->listBox()->item( d->current );
      if ( item ) {
        int itemh = item->height( d->listBox() );
        p.translate( re.x(), re.y() + (re.height() - itemh)/2  );
        item->paint( &p );
      }
    } else if ( d->listBox() && d->listBox()->item( d->current ) ) {
      QListBoxItem * item = d->listBox()->item( d->current );
      const QPixmap *pix = item->pixmap();
      if ( pix ) {
        p.fillRect( re.x(), re.y(), pix->width() + 4, re.height(),
              colorGroup().brush( QColorGroup::Base ) );
        p.drawPixmap( re.x() + 2, re.y() +
                ( re.height() - pix->height() ) / 2, *pix );
      }
    }
    p.setClipping( FALSE );
  }
#endif
}
