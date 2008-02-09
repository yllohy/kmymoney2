/***************************************************************************
                          kaccounttemplateselector.h  -  description
                             -------------------
    begin                : Tue Feb 5 2008
    copyright            : (C) 2008 by Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KACCOUNTTEMPLATESELECTOR_H
#define KACCOUNTTEMPLATESELECTOR_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoney2/widgets/kaccounttemplateselectordecl.h>
class KAccountTemplateSelectorPrivate;
class MyMoneyTemplate;

/**
 * @author Thomas Baumgart <ipwizard@users.sourceforge.net>
 */
class KAccountTemplateSelector : public KAccountTemplateSelectorDecl
{
  Q_OBJECT
  public:
    KAccountTemplateSelector(QWidget* parent = 0, const char* name = 0);
    ~KAccountTemplateSelector();
    QValueList<MyMoneyTemplate> selectedTemplates(void) const;

    void loadTemplateList(void);

  private slots:
    void slotLoadHierarchy(void);

  private:
    KAccountTemplateSelectorPrivate* d;
};

#endif
