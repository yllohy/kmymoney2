/****************************************************************************
** KBanksView meta object code from reading C++ file 'kbanksview.h'
**
** Created: Tue Jan 22 20:22:35 2002
**      by: The Qt MOC ($Id: kbanksview.moc.cpp,v 1.1 2002/01/22 20:32:14 mte Exp $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#if !defined(Q_MOC_OUTPUT_REVISION)
#define Q_MOC_OUTPUT_REVISION 9
#elif Q_MOC_OUTPUT_REVISION != 9
#error "Moc format conflict - please regenerate all moc files"
#endif

#include "kbanksview.h"
#include <qmetaobject.h>
#include <qapplication.h>



const char *KBanksView::className() const
{
    return "KBanksView";
}

QMetaObject *KBanksView::metaObj = 0;

void KBanksView::initMetaObject()
{
    if ( metaObj )
	return;
    if ( qstrcmp(KBankViewDecl::className(), "KBankViewDecl") != 0 )
	badSuperclassWarning("KBanksView","KBankViewDecl");
    (void) staticMetaObject();
}

#ifndef QT_NO_TRANSLATION

QString KBanksView::tr(const char* s)
{
    return qApp->translate( "KBanksView", s, 0 );
}

QString KBanksView::tr(const char* s, const char * c)
{
    return qApp->translate( "KBanksView", s, c );
}

#endif // QT_NO_TRANSLATION

QMetaObject* KBanksView::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    (void) KBankViewDecl::staticMetaObject();
#ifndef QT_NO_PROPERTIES
#endif // QT_NO_PROPERTIES
    typedef void (KBanksView::*m1_t0)(QListViewItem*,const QPoint&,int);
    typedef void (QObject::*om1_t0)(QListViewItem*,const QPoint&,int);
    typedef void (KBanksView::*m1_t1)(QListViewItem*,const QPoint&,int);
    typedef void (QObject::*om1_t1)(QListViewItem*,const QPoint&,int);
    typedef void (KBanksView::*m1_t2)(QListViewItem*);
    typedef void (QObject::*om1_t2)(QListViewItem*);
    m1_t0 v1_0 = &KBanksView::slotListDoubleClicked;
    om1_t0 ov1_0 = (om1_t0)v1_0;
    m1_t1 v1_1 = &KBanksView::slotListRightMouse;
    om1_t1 ov1_1 = (om1_t1)v1_1;
    m1_t2 v1_2 = &KBanksView::slotSelectionChanged;
    om1_t2 ov1_2 = (om1_t2)v1_2;
    QMetaData *slot_tbl = QMetaObject::new_metadata(3);
    QMetaData::Access *slot_tbl_access = QMetaObject::new_metaaccess(3);
    slot_tbl[0].name = "slotListDoubleClicked(QListViewItem*,const QPoint&,int)";
    slot_tbl[0].ptr = (QMember)ov1_0;
    slot_tbl_access[0] = QMetaData::Protected;
    slot_tbl[1].name = "slotListRightMouse(QListViewItem*,const QPoint&,int)";
    slot_tbl[1].ptr = (QMember)ov1_1;
    slot_tbl_access[1] = QMetaData::Protected;
    slot_tbl[2].name = "slotSelectionChanged(QListViewItem*)";
    slot_tbl[2].ptr = (QMember)ov1_2;
    slot_tbl_access[2] = QMetaData::Protected;
    typedef void (KBanksView::*m2_t0)(const MyMoneyBank,bool);
    typedef void (QObject::*om2_t0)(const MyMoneyBank,bool);
    typedef void (KBanksView::*m2_t1)(const MyMoneyAccount,bool);
    typedef void (QObject::*om2_t1)(const MyMoneyAccount,bool);
    typedef void (KBanksView::*m2_t2)();
    typedef void (QObject::*om2_t2)();
    typedef void (KBanksView::*m2_t3)();
    typedef void (QObject::*om2_t3)();
    typedef void (KBanksView::*m2_t4)();
    typedef void (QObject::*om2_t4)();
    m2_t0 v2_0 = &KBanksView::bankRightMouseClick;
    om2_t0 ov2_0 = (om2_t0)v2_0;
    m2_t1 v2_1 = &KBanksView::accountRightMouseClick;
    om2_t1 ov2_1 = (om2_t1)v2_1;
    m2_t2 v2_2 = &KBanksView::accountDoubleClick;
    om2_t2 ov2_2 = (om2_t2)v2_2;
    m2_t3 v2_3 = &KBanksView::bankSelected;
    om2_t3 ov2_3 = (om2_t3)v2_3;
    m2_t4 v2_4 = &KBanksView::accountSelected;
    om2_t4 ov2_4 = (om2_t4)v2_4;
    QMetaData *signal_tbl = QMetaObject::new_metadata(5);
    signal_tbl[0].name = "bankRightMouseClick(const MyMoneyBank,bool)";
    signal_tbl[0].ptr = (QMember)ov2_0;
    signal_tbl[1].name = "accountRightMouseClick(const MyMoneyAccount,bool)";
    signal_tbl[1].ptr = (QMember)ov2_1;
    signal_tbl[2].name = "accountDoubleClick()";
    signal_tbl[2].ptr = (QMember)ov2_2;
    signal_tbl[3].name = "bankSelected()";
    signal_tbl[3].ptr = (QMember)ov2_3;
    signal_tbl[4].name = "accountSelected()";
    signal_tbl[4].ptr = (QMember)ov2_4;
    metaObj = QMetaObject::new_metaobject(
	"KBanksView", "KBankViewDecl",
	slot_tbl, 3,
	signal_tbl, 5,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    metaObj->set_slot_access( slot_tbl_access );
#ifndef QT_NO_PROPERTIES
#endif // QT_NO_PROPERTIES
    return metaObj;
}

#include <qobjectdefs.h>
#include <qsignalslotimp.h>

// SIGNAL bankRightMouseClick
void KBanksView::bankRightMouseClick( const MyMoneyBank t0, bool t1 )
{
    // No builtin function for signal parameter type const MyMoneyBank,bool
    QConnectionList *clist = receivers("bankRightMouseClick(const MyMoneyBank,bool)");
    if ( !clist || signalsBlocked() )
	return;
    typedef void (QObject::*RT0)();
    typedef void (QObject::*RT1)(const MyMoneyBank);
    typedef void (QObject::*RT2)(const MyMoneyBank,bool);
    RT0 r0;
    RT1 r1;
    RT2 r2;
    QConnectionListIt it(*clist);
    QConnection   *c;
    QSenderObject *object;
    while ( (c=it.current()) ) {
	++it;
	object = (QSenderObject*)c->object();
	object->setSender( this );
	switch ( c->numArgs() ) {
	    case 0:
#ifdef Q_FP_CCAST_BROKEN
		r0 = reinterpret_cast<RT0>(*(c->member()));
#else
		r0 = (RT0)*(c->member());
#endif
		(object->*r0)();
		break;
	    case 1:
#ifdef Q_FP_CCAST_BROKEN
		r1 = reinterpret_cast<RT1>(*(c->member()));
#else
		r1 = (RT1)*(c->member());
#endif
		(object->*r1)(t0);
		break;
	    case 2:
#ifdef Q_FP_CCAST_BROKEN
		r2 = reinterpret_cast<RT2>(*(c->member()));
#else
		r2 = (RT2)*(c->member());
#endif
		(object->*r2)(t0, t1);
		break;
	}
    }
}

// SIGNAL accountRightMouseClick
void KBanksView::accountRightMouseClick( const MyMoneyAccount t0, bool t1 )
{
    // No builtin function for signal parameter type const MyMoneyAccount,bool
    QConnectionList *clist = receivers("accountRightMouseClick(const MyMoneyAccount,bool)");
    if ( !clist || signalsBlocked() )
	return;
    typedef void (QObject::*RT0)();
    typedef void (QObject::*RT1)(const MyMoneyAccount);
    typedef void (QObject::*RT2)(const MyMoneyAccount,bool);
    RT0 r0;
    RT1 r1;
    RT2 r2;
    QConnectionListIt it(*clist);
    QConnection   *c;
    QSenderObject *object;
    while ( (c=it.current()) ) {
	++it;
	object = (QSenderObject*)c->object();
	object->setSender( this );
	switch ( c->numArgs() ) {
	    case 0:
#ifdef Q_FP_CCAST_BROKEN
		r0 = reinterpret_cast<RT0>(*(c->member()));
#else
		r0 = (RT0)*(c->member());
#endif
		(object->*r0)();
		break;
	    case 1:
#ifdef Q_FP_CCAST_BROKEN
		r1 = reinterpret_cast<RT1>(*(c->member()));
#else
		r1 = (RT1)*(c->member());
#endif
		(object->*r1)(t0);
		break;
	    case 2:
#ifdef Q_FP_CCAST_BROKEN
		r2 = reinterpret_cast<RT2>(*(c->member()));
#else
		r2 = (RT2)*(c->member());
#endif
		(object->*r2)(t0, t1);
		break;
	}
    }
}

// SIGNAL accountDoubleClick
void KBanksView::accountDoubleClick()
{
    activate_signal( "accountDoubleClick()" );
}

// SIGNAL bankSelected
void KBanksView::bankSelected()
{
    activate_signal( "bankSelected()" );
}

// SIGNAL accountSelected
void KBanksView::accountSelected()
{
    activate_signal( "accountSelected()" );
}
