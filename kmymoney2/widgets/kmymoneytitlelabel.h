/***************************************************************************
                          kmymoneytitlelabel.h
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

#ifndef KTITLELABEL_H
#define KTITLELABEL_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qlabel.h>
#include <qimage.h>
#include <qcolor.h>
class QPixmap;
class QCString;

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

/**
@author ace jones
*/
class kMyMoneyTitleLabel : public QLabel
{
Q_OBJECT
Q_PROPERTY( QCString leftImageFile READ leftImageFile WRITE setLeftImageFile DESIGNABLE true )
Q_PROPERTY( QCString rightImageFile READ rightImageFile WRITE setRightImageFile DESIGNABLE true )
Q_PROPERTY( QColor bgColor READ bgColor WRITE setBgColor DESIGNABLE true )
Q_PROPERTY( QString text READ text WRITE setText DESIGNABLE true )
public:
    kMyMoneyTitleLabel(QWidget *parent = 0, const char *name = 0);
    ~kMyMoneyTitleLabel();

    void setBgColor(const QColor& _color) { m_bgColor = _color; }
    void setLeftImageFile(const QCString& _file);
    void setRightImageFile(const QCString& _file);

    QCString leftImageFile(void) const { return m_leftImageFile; }
    QCString rightImageFile(void) const { return m_rightImageFile; }
    QColor bgColor(void) const { return m_bgColor; }
    QString text(void) const { return m_text; }

public slots:
    virtual void setText(const QString& txt);

protected:
    void updatePixmap(void);
    virtual void resizeEvent ( QResizeEvent * );
    void drawContents(QPainter *);

private:
    QImage m_leftImage;
    QImage m_rightImage;
    QColor m_bgColor;
    QColor m_textColor;
    QColor m_shadowColor;
    QString m_text;

    QCString m_leftImageFile;
    QCString m_rightImageFile;
};

#endif
