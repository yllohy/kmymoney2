/***************************************************************************
                       mymoneystoragegnc  -  description
                          -------------------
 begin                : Wed Mar 3 2004
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
/*
The main class of this module, MyMoneyStorageGNC, contains only a readFile()
function, which controls the import of data from an XML file created by the
current GnuCash version (1.8.8).
 
The XML is processed in class XmlReader, which is an implementation of the Qt
SAX2 reader class. 
 
Data in the input file is processed as a set of objects which fortunately,
though perhaps not surprisingly, have almost a one-for-one correspondence with
KMyMoney objects. These objects are bounded by start and end XML elements, and
may contain both nested objects (described as sub objects in the code), and data
items, also delimited by start and end elements. For example:
<gnc:account> * start of sub object within file
  <act:name>Account Name</act:name> * data string with start and end elements
  ...
</gnc:account> * end of sub objects
 
A GnuCash file may consist of more than one 'book', or set of data. It is not
clear how we could currently implement this, so only the first book in a file is
processed. This should satisfy most user situations.
 
GnuCash is somewhat inconsistent in its division of the major sections of the
file. For example, multiple price history entries are delimited by <gnc:pricedb>
elements, while each account starts with  its own top-level element. In general,
the 'container' elements are ignored.
 
XmlReader
 
This is an implementation of the Qt QXmlDefaultHandler class, which provides
three main function calls in addition to start and end of document. The
startElement() and endElement() calls are self-explanatory, the characters()
function provides data strings. Thus in the above example, the sequence of calls
would be
  startElement() for gnc:account
  startElement() for act:name
   characters() for 'Account Name'
  endElement() for act:name
  ...
  endElement() for gnc:account
 
Objects
 
Since the processing requirements of XML for most elements are very similar, the
common code is implemented in a GncObject class, from which the others are
derived, with virtual function calls to cater for any differences. The
'grandfather' object, GncFile representing the file (or more correctly, 'book')
as a whole, is created in the startDocument() function call.
 
The constructor function of each object is responsible for providing two lists
for the XmlReader to scan, a list of element names which represent sub objects
(called sub elements in the code), and a similar list of names representing data
elements. In addition, an array of variables (m_v) is provided and initialized,
to contain the actual data strings.
 
Implementation
 
Since objects may be nested, a stack is used, with the top element pointing to
the 'current object'. The startDocument() call creates the first, GncFile,
object at the top of the stack.
 
As each startElement() call occurs, the two element lists created by the current
object are scanned. 
If this element represents the start of a sub object, the current object's subEl()
function is called to create an instance of the appropriate type. This is then
pushed to the top of the stack, and the new object's initiate() function is
called. This is used to process any XML attributes attached to the element;
GnuCash makes little use of these.
If this represents the start of a data element, a pointer (m_dataPointer) is set
to point to an entry in the array (m_v) in which a subsequent characters() call
can store the actual data.
 
When an endElement() call occurs, a check is made to see if it matches the
element name which started the current object. If so, the object's terminate()
function is called. If the object represents a similar KMM object, this will
normally result in a call to a conversion routine in the main
(MyMoneyStorageGNC) class to convert the data to native format and place it in
storage. The stack is then popped, and the parent (now current) object notified
by a call to its endSubEl() function. Again depending on the type of object,
this will either delete the instance, or save it in its own storage for later
processing.
For example, a GncSplit object makes little sense outside the context of its
transaction, so will be saved by the transaction. A GncTransaction object on the
other hand will be converted, along with its attendant splits, and then deleted
by its parent.
 
Since at any one time an object will only be processing either a subobject or a
data element, a single object variable, m_state, is used to determine the actual
type. In effect, it acts as the current index into either the subElement or
dataElement list. As an object variable, it will be saved on the stack across
subobject processing.
 
Exceptions and Problems
 
Fatal exceptions are processed via the standard MyMoneyException method. 
Due to differences in implementation between GnuCash and KMM, it is not always
possible to provide an absolutely correct conversion. When such a problem
situation is recognized, a message, along with any relevant variable data, is
passed to the main class, and used to produce a report when processing
terminates. The GncMessages and GncMessageArg classes implement this.
 
Anonymizer
 
When debugging problems, it is often useful to have a trace of what is happening
within the module. However, in view of the sensitive nature of personal finance
data, most users will be reluctant to provide this. Accordingly, an anonymize
(hide()) function is provided to handle data strings. These may either be passed
through asis (non-personal data), blanked out (non-critical but possibly personal
data), replaced with a generated version (required, but possibly personal), or
randomized (monetary amounts). The action for each data item is determined in
the object's constructor function along with the creation of the data element
list.
This module will later be used as the basis of a file anonymizer, which will
enable users to safely provide us with a copy of their GnuCash files, and will
allow us to test the structure, if not the data content, of the file.
*/

#ifndef MYMONEYSTORAGEGNC_H
#define MYMONEYSTORAGEGNC_H
// system includes
#include <stdlib.h>

// ----------------------------------------------------------------------------
// QT Includes

#include <qdatastream.h>
class QIODevice;
#include <qmessagebox.h>
#include <qobject.h>
#include <qvaluelist.h>
#include <qptrlist.h>
#include <qptrstack.h>
#include <qxml.h>
#include <qdatetime.h>

// ----------------------------------------------------------------------------
// Project Includes
#ifndef _GNCFILEANON
#include "imymoneyserialize.h" // not used any more, but call interface requires it
#include "imymoneystorageformat.h"
#endif // _GNCFILEANON

// not sure what these are for, but leave them in
#define VERSION_0_60_XML  0x10000010    // Version 0.5 file version info
#define VERSION_0_61_XML  0x10000011    // use 8 bytes for MyMoneyMoney objects
#define GNUCASH_ID_KEY "GNUCASH_ID"

typedef QMap<QCString, QCString> map_accountIds;
typedef map_accountIds::iterator map_accountIds_iter;
typedef map_accountIds::const_iterator map_accountIds_citer;

class MyMoneyStorageGNC;

/** GncObject is the base class for the various objects in the gnucash file
    Beyond the first level XML objects, elements will be of one of three types:
     1. Sub object elements, which require creation of another object to process
     2. Data object elements, which are only followed by data to be stored in a variable (m_v array)
     3. Ignored objects, data not needed and not included herein
*/
class GncObject {
public:
  GncObject();
  ; // to save delete loop when finished
  virtual ~GncObject() {}; // make sure to have impl  of all virtual rtns to avoid vtable errors?
protected:
  friend class XmlReader;
  friend class MyMoneyStorageGNC;

  // check for sub object element; if it is, create the object
  GncObject *isSubElement (const QString &elName, const QXmlAttributes& elAttrs);
  // check for data element; if so, set data pointer
  bool isDataElement (const QString &elName, const QXmlAttributes& elAttrs);
  // process start element for 'this'; normally for attribute checking; other initialization done in constructor
  virtual void initiate (const QString&, const QXmlAttributes&) { return ;};
  // a sub object has completed; process the data it gathered
  virtual void endSubEl(GncObject *) {m_dataPtr = 0; return ;};
  // store data for data element
  void storeData (const QString& pData) // NB - data MAY come in chunks, and may need to be anonymized
  {if (m_dataPtr != 0)
    m_dataPtr->append (hide (pData, m_anonClass)); return ;}
  // following is provided only for a future file anonymizer
  QString getData () const { return ((m_dataPtr != 0) ? *m_dataPtr : "");};
  void resetDataPtr() {m_dataPtr = 0;};
  // process end element for 'this'; usually to convert to KMM format
  virtual void terminate() { return ;};

  // some gnucash elements have version attribute; check it
  static void checkVersion (const QString&, const QXmlAttributes&);
  // get name of element processed by 'this'
  const QString getElName () const { return (m_elementName);};
  // return gnucash counts (not always accurate!)
  int gncCommodityCount() const { return (m_gncCommodityCount);};
  int gncAccountCount () const { return (m_gncAccountCount);};
  int gncTransactionCount () const { return (m_gncTransactionCount);};
  int gncScheduleCount () const { return (m_gncScheduleCount);};
  // pass 'main' pointer to object
  void setPm (MyMoneyStorageGNC *pM) {pMain = pM;};
  // debug only
  void debugDump();

  // called by isSubElement to create appropriate sub object
  virtual GncObject *startSubEl() { return (0);};
  // called by isDataElement to set variable pointer
  virtual void dataEl(const QXmlAttributes&) {m_dataPtr = m_v.at(m_state); m_anonClass = m_anonClassList[m_state];};
  // return gnucash data string variable pointer
  virtual const QString var (int i) const { return (*(m_v.at(i)));};
  // anonymize data
  virtual QString hide (QString, unsigned int);
  
  MyMoneyStorageGNC *pMain;    // pointer to 'main' class
  // used at start of each transaction so same money hide factor is applied to all splits
  void adjustHideFactor();
  
  QString m_elementName; // save 'this' element's name
  const QString *m_subElementList; // list of sub object element names for 'this'
  unsigned int m_subElementListCount; // count of above
  const QString *m_dataElementList; // ditto for data elements
  unsigned int m_dataElementListCount;
  QString *m_dataPtr; // pointer to m_v variable for current data item
  mutable QPtrList<QString> m_v; // storage for variable pointers
  
  unsigned int m_state; // effectively, the index to subElementList or dataElementList, whichever is currently in use
  
  const unsigned int *m_anonClassList;
  enum anonActions {ASIS, SUPPRESS, NXTACC, NXTEQU, NXTPAY, NXTSCHD, MAYBEQ, MONEY1, MONEY2}; // anonymize actions - see hide()
  unsigned int m_anonClass; // class of current data item for anonymizer
  static double m_moneyHideFactor; // a per-transaction factor
  
  static int m_gncCommodityCount; // to hold count data from gnc file
  static int m_gncAccountCount;
  static int m_gncTransactionCount;
  static int m_gncScheduleCount;
};

// *****************************************************************************
// This is the 'grandfather' object representing the gnucash file as a whole
class GncFile : public GncObject {
public:
  GncFile ();
  ~GncFile();
private:
  enum iSubEls {BOOK, COUNT, CMDTY, PRICE, ACCT, TX, TEMPLATES, SCHEDULES, END_FILE_SELS };
  virtual GncObject *startSubEl();
  virtual void endSubEl(GncObject *);

  bool m_processingTemplates; // gnc uses same transaction element for ordinary and template tx's; this will distinguish
  bool m_bookFound;  // to  detect multi-book files
};
// The following are 'utility' objects, which occur within several other object types
// ****************************************************************************
// commodity specification. consists of
//  cmdty:space - either ISO4217 if this cmdty is a currency, or, usually, the name of a stock exchange
//  cmdty:id - ISO4217 currency symbol, or 'ticker symbol'
class GncCmdtySpec : public GncObject {
public:
  GncCmdtySpec ();
  ~GncCmdtySpec ();
protected:
  friend class MyMoneyStorageGNC;
  friend class GncTransaction;
  const bool isCurrency() const { return (*m_v.at(CMDTYSPC) == QString("ISO4217"));};
  const QString id() const { return (*m_v.at(CMDTYID));};
  const QString space() const { return (*m_v.at(CMDTYSPC));};
private:
  // data elements
  enum CmdtySpecDataEls {CMDTYSPC, CMDTYID, END_CmdtySpec_DELS};
  virtual QString hide (QString, unsigned int);
};
// *********************************************************************
// date; maybe one of two types, ts:date which is date/time, gdate which is date only
// we do not preserve time data (at present)
class GncDate : public GncObject {
public:
  GncDate ();
  ~GncDate();
protected:
  friend class MyMoneyStorageGNC;
  friend class GncPrice;
  friend class GncTransaction;
  friend class GncSplit;
  friend class GncSchedule;
  const QDate date() const { return (QDate::fromString(m_v.at(TSDATE)->section(' ', 0, 0), Qt::ISODate));};
private:
  // data elements
  enum DateDataEls {TSDATE, GDATE, END_Date_DELS};
  virtual void dataEl(const QXmlAttributes&) {m_dataPtr = m_v.at(TSDATE); m_anonClass = GncObject::ASIS;}
  ; // treat both date types the same
};
// ************* GncKvp********************************************
// Key/value pairs, which are introduced by the 'slot' element
// Consist of slot:key (the 'name' of the kvp), and slot:value (the data value)
// the slot value also contains a slot type (string, integer, etc) implemented as an XML attribute
// kvp's may be nested
class GncKvp : public GncObject {
public:
  GncKvp ();
  ~GncKvp();
protected:
  friend class MyMoneyStorageGNC;

  const QString key() const { return (var(KEY));};
  const QString value() const { return (var(VALUE));};
  const QString type() const { return (m_kvpType);};
  const unsigned int kvpCount() const { return (m_kvpList.count());};
  const GncKvp *getKvp(unsigned int i) const { return (static_cast<GncKvp *>(m_kvpList.at(i)));};
private:
  // subsidiary objects/elements
  enum KvpSubEls {KVP, END_Kvp_SELS };
  virtual GncObject *startSubEl();
  virtual void endSubEl(GncObject *);
  // data elements
  enum KvpDataEls {KEY, VALUE, END_Kvp_DELS };
  virtual void dataEl (const QXmlAttributes&);
  mutable QPtrList<GncObject> m_kvpList;
  QString m_kvpType;  // type is an XML attribute
};

/** Following are the main objects within the gnucash file, which correspond largely one-for-one
    with similar objects in the kmymoney structure, apart from schedules which gnc splits between
    template (transaction data) and schedule (date data)
*/ 
//********************************************************************
class GncCountData : public GncObject {
public:
  GncCountData ();
  ~GncCountData ();
private:
  virtual void initiate (const QString&, const QXmlAttributes&);
  virtual void terminate();
  QString m_countType; // type of element being counted
};
//********************************************************************
class GncCommodity : public GncObject {
public:
  GncCommodity ();
  ~GncCommodity();
protected:
  friend class MyMoneyStorageGNC;
  // access data values
  const bool isCurrency() const { return (var(SPACE) == QString("ISO4217"));};
  const QString space() const { return (var(SPACE));};
  const QString id() const { return (var(ID));};
  const QString name() const { return (var(NAME));};
  const QString fraction() const { return (var(FRACTION));};
private:
  virtual void terminate();
  // data elements
  enum {SPACE, ID, NAME, FRACTION, END_Commodity_DELS};
};
// ************* GncPrice********************************************
class GncPrice : public GncObject {
public:
  GncPrice ();
  ~GncPrice();
protected:
  friend class MyMoneyStorageGNC;
  // access data values
  const GncCmdtySpec *commodity() const { return (m_vpCommodity);};
  const GncCmdtySpec *currency() const { return (m_vpCurrency);};
  const QString value() const { return (var(VALUE));};
  const QDate priceDate () const { return (m_vpPriceDate->date());};
private:
  virtual void terminate();
  // sub object elements
  enum PriceSubEls {CMDTY, CURR, PRICEDATE, END_Price_SELS };
  virtual GncObject *startSubEl();
  virtual void endSubEl(GncObject *);
  // data elements
  enum PriceDataEls {VALUE, END_Price_DELS };
  GncCmdtySpec *m_vpCommodity, *m_vpCurrency;
  GncDate *m_vpPriceDate;
};
// ************* GncAccount********************************************
class GncAccount : public GncObject {
public:
  GncAccount ();
  ~GncAccount();
protected:
  friend class MyMoneyStorageGNC;
  // access data values
  GncCmdtySpec *commodity() const { return (m_vpCommodity);};
  const QString id () const { return (var(ID));};
  const QString name () const { return (var(NAME));};
  const QString desc () const { return (var(DESC));};
  const QString type () const { return (var(TYPE));};
  const QString parent () const { return (var(PARENT));};
private:
  // subsidiary objects/elements
  enum AccountSubEls {CMDTY, KVP, END_Account_SELS };
  virtual GncObject *startSubEl();
  virtual void endSubEl(GncObject *);
  virtual void terminate();
  // data elements
  enum AccountDataEls {ID, NAME, DESC, TYPE, PARENT, END_Account_DELS };
  GncCmdtySpec *m_vpCommodity;
  QPtrList<GncObject> kvpList;
};
// ************* GncSplit********************************************
class GncSplit : public GncObject {
public:
  GncSplit ();
  ~GncSplit();
protected:
  friend class MyMoneyStorageGNC;
  // access data values
  const QString id() const { return (var(ID));};
  const QString memo() const { return (var(MEMO));};
  const QString recon() const { return (var(RECON));};
  const QString value() const { return (var(VALUE));};
  const QString qty() const { return (var(QTY));};
  const QString acct() const { return (var(ACCT));};
const QDate reconDate() const {QDate x = QDate(); return (m_vpDateReconciled == NULL ? x : m_vpDateReconciled->date());};
private:
  // subsidiary objects/elements
  enum TransactionSubEls {RECDATE, END_Split_SELS };
  virtual GncObject *startSubEl();
  virtual void endSubEl(GncObject *);
  // data elements
  enum SplitDataEls {ID, MEMO, RECON, VALUE, QTY, ACCT, END_Split_DELS };
  GncDate *m_vpDateReconciled;
};
// ************* GncTransaction********************************************
class GncTransaction : public GncObject {
public:
  GncTransaction (bool processingTemplates);
  ~GncTransaction();
protected:
  friend class MyMoneyStorageGNC;
  // access data values
  const QString id() const { return (var(ID));};
  const QString no() const { return (var(NO));};
  const QString desc() const { return (var(DESC));};
  const QString currency() const { return (m_vpCurrency->id());};
  const QDate dateEntered() const { return (m_vpDateEntered->date());};
  const QDate datePosted() const { return (m_vpDatePosted->date());};
  const bool isTemplate() const { return (m_template);};
  const unsigned int splitCount() const { return (m_splitList.count());};
  const GncObject *getSplit (unsigned int i) const { return (m_splitList.at(i));};
private:
  // subsidiary objects/elements
  enum TransactionSubEls {CURRCY, POSTED, ENTERED, SPLIT, END_Transaction_SELS };
  virtual GncObject *startSubEl();
  virtual void endSubEl(GncObject *);
  virtual void terminate();
  // data elements
  enum TransactionDataEls {ID, NO, DESC, END_Transaction_DELS };
  GncCmdtySpec *m_vpCurrency;
  GncDate *m_vpDateEntered, *m_vpDatePosted;
  mutable QPtrList<GncObject> m_splitList;
  bool m_template; // true if this is a template for scheduled transaction
};
// ************* GncTemplateSplit********************************************
class GncTemplateSplit : public GncObject {
public:
  GncTemplateSplit ();
  ~GncTemplateSplit();
protected:
  friend class MyMoneyStorageGNC;
  // access data values
  const QString id() const { return (var(ID));};
  const QString memo() const { return (var(MEMO));};
  const QString recon() const { return (var(RECON));};
  const QString value() const { return (var(VALUE));};
  const QString qty() const { return (var(QTY));};
  const QString acct() const { return (var(ACCT));};
  const unsigned int kvpCount() const { return (m_kvpList.count());};
  const GncKvp *getKvp(unsigned int i) const { return (static_cast<GncKvp *>(m_kvpList.at(i)));};
private:
  // subsidiary objects/elements
  enum TemplateSplitSubEls {KVP, END_TemplateSplit_SELS };
  virtual GncObject *startSubEl();
  virtual void endSubEl(GncObject *);
  // data elements
  enum TemplateSplitDataEls {ID, MEMO, RECON, VALUE, QTY, ACCT, END_TemplateSplit_DELS };
  mutable QPtrList<GncObject> m_kvpList;
};
// ************* GncSchedule********************************************
class GncFreqSpec;
class GncSchedule : public GncObject {
public:
  GncSchedule ();
  ~GncSchedule();
protected:
  friend class MyMoneyStorageGNC;
  // access data values
  const QString name() const { return (var(NAME));};
  const QString autoCreate() const { return (var(AUTOC));};
  const QString autoCrNotify() const { return (var(AUTOCN));};
  const QString autoCrDays() const { return (var(AUTOCD));};
  const QString advCrDays() const { return (var(ADVCD));};
  const QString advCrRemindDays() const { return (var(ADVRD));};
  const QString instanceCount() const { return (var(INSTC));};
  const QString numOccurs() const { return (var(NUMOCC));};
  const QString remOccurs() const { return (var(REMOCC));};
  const QString templId() const { return (var(TEMPLID));};
  const QDate startDate () const 
    {QDate x = QDate(); return (m_vpStartDate == NULL ? x : m_vpStartDate->date());};
  const QDate lastDate () const 
    {QDate x = QDate(); return (m_vpLastDate == NULL ? x : m_vpLastDate->date());};
  const QDate endDate() const 
    {QDate x = QDate(); return (m_vpEndDate == NULL ? x : m_vpEndDate->date());};
  const GncFreqSpec *getFreqSpec() const { return (m_vpFreqSpec);};
private:
  // subsidiary objects/elements
  enum ScheduleSubEls {STARTDATE, LASTDATE, ENDDATE, FREQ, END_Schedule_SELS };
  virtual GncObject *startSubEl();
  virtual void endSubEl(GncObject *);
  virtual void terminate();
  // data elements
  enum ScheduleDataEls {NAME, AUTOC, AUTOCN, AUTOCD, ADVCD, ADVRD, INSTC,
                        NUMOCC, REMOCC, TEMPLID, END_Schedule_DELS };
  GncDate *m_vpStartDate, *m_vpLastDate, *m_vpEndDate;
  GncFreqSpec *m_vpFreqSpec;
};
// ************* GncFreqSpec********************************************
class GncFreqSpec : public GncObject {
public:
  GncFreqSpec ();
  ~GncFreqSpec();
protected:
  friend class MyMoneyStorageGNC;
  // access data values (only interval type used at present)
  const QString intervalType() const { return (var(INTVT));};
private:
  // subsidiary objects/elements
  enum FreqSpecSubEls {COMPO, END_FreqSpec_SELS };
  virtual GncObject *startSubEl();
  virtual void endSubEl(GncObject *);
  // data elements
  enum FreqSpecDataEls {INTVT, MONTHLY, DAILY, WEEKLY, INTVI, INTVO, INTVD, END_FreqSpec_DELS};
  virtual void terminate();
  mutable QPtrList<GncObject> m_fsList;
};

// ****************************************************************************************
/**
                                    XML Reader
 The XML reader is an implementation of the Qt SAX2 XML parser. It determines the type
 of object represented by the XMl, and calls the appropriate object functions
*/ 
// *****************************************************************************************
class XmlReader : public QXmlDefaultHandler {
protected:
  friend class MyMoneyStorageGNC;
  XmlReader (MyMoneyStorageGNC *pM) : pMain(pM) {}; // keep pointer to 'main'
  void processFile (QIODevice*); // main entry point of reader
  //  define xml content handler functions
  bool startDocument ();
  bool startElement (const QString&, const QString&, const QString&, const QXmlAttributes&);
  bool endElement (const QString&, const QString&, const QString&);
  bool characters (const QString &);
  bool endDocument();
private:
  QXmlInputSource *m_source;
  QXmlSimpleReader *m_reader;
  QPtrStack<GncObject> m_os; // stack of sub objects
  GncObject *m_co;           // current object, for ease of coding (=== os.top)
  MyMoneyStorageGNC *pMain;  // the 'main' pointer, to pass on to objects
#ifdef _GNCFILEANON
  int lastType; // 0 = start element, 1 = data, 2 = end element
  int indentCount;
#endif // GNCFILENON
};

/**
  * private classes to define messages to be held in list for final report
  */
class GncMessageArgs {
protected:
  friend class MyMoneyStorageGNC;
  QString source; // 'type of message
  unsigned int code; // to identify actual message
  QValueList<QString> args; // variable arguments
};

class GncMessages {
protected:
  friend class MyMoneyStorageGNC;
  static QString text (const QString, const unsigned int); // returns text of identified message
  static const unsigned int argCount (const QString, const unsigned int); // returns no. of args required
private:
  typedef struct {
    const QString source;
    const unsigned int code;
    QString text;
  }
  messText;
  static messText texts [];
};

/**
                   MyMoneyStorageGNC -  Main class for this module
  Controls overall operation of the importer
  */

#ifndef _GNCFILEANON
class MyMoneyStorageGNC : public IMyMoneyStorageFormat {
#else
class MyMoneyStorageGNC {
#endif // GNCFIELANON
public:
  MyMoneyStorageGNC();
  virtual ~MyMoneyStorageGNC();
  /**
    * A parameter is used to determine the direction.
    *
    * @param pDoc: pointer to the entire DOM document
    * @param userInfo: DOM Element to write the user information to.  
    *
    * @return void
    *
    * @see
    */
#ifndef _GNCFILEANON
  void readFile (QIODevice *, IMyMoneySerialize *); // main entry point, IODevice is gnucash file
  void writeFile (QIODevice*, IMyMoneySerialize*) { return ;}; // dummy entry needed by kmymoneywiew. we will not be writing
#else
  void readFile (QString, QString);
#endif // _GNCFILEANON

protected:
  friend class GncObject; // pity we can't just say GncObject. And compiler doesn't like multiple friends on one line...
  friend class GncFile; // there must be a better way...
  friend class GncDate;
  friend class GncCmdtySpec;
  friend class GncKvp;
  friend class GncCommodity;
  friend class GncPrice;
  friend class GncAccount;
  friend class GncTransaction;
  friend class GncSplit;
  friend class GncTemplateTransaction;
  friend class GncTemplateSplit;
  friend class GncSchedule;
  friend class GncFreqSpec;
  friend class XmlReader;
#ifndef _GNCFILEANON
  /** functions to convert gnc objects to our equivalent */
  void convertCommodity (const GncCommodity *);
  void convertPrice (const GncPrice *);
  void convertAccount (const GncAccount *);
  void convertTransaction (const GncTransaction *);
  void convertSplit (const GncSplit *);
  void saveTemplateTransaction (GncTransaction *t) {m_templateList.append (t);};
  void convertSchedule (const GncSchedule *);
  void convertFreqSpec (const GncFreqSpec *);
#else  
    /** functions to convert gnc objects to our equivalent */
  void convertCommodity (const GncCommodity *) {return;};
  void convertPrice (const GncPrice *) {return;};
  void convertAccount (const GncAccount *) {return;};
  void convertTransaction (const GncTransaction *) {return;};
  void convertSplit (const GncSplit *) {return;};
  void saveTemplateTransaction (GncTransaction *t)  {return;};
  void convertSchedule (const GncSchedule *) {return;};
  void convertFreqSpec (const GncFreqSpec *) {return;};
#endif // _GNCFILEANON
/** to post messages for final report */
  void postMessage (const QString, const unsigned int, ...); // will have variable number of args
  void setProgressCallback (void(*callback)(int, int, const QString&));
  void signalProgress (int current, int total, const QString& = "");
  /** user options */
  /**
              Scheduled Transactions
    Due to differences in implementation, it is not always possible to import scheduled
    transactions correctly. Though best efforts are made, it may be that some
    imported transactions cause problems within kmymoney.
    An attempt is made within the importer to identify potential problem transactions,
    and setting this option will cause them to be dropped from the file.
    A report of which were dropped, and why, will be produced.
       m_dropSuspectSchedules - drop suspect scheduled transactions
  */
  bool m_dropSuspectSchedules;
  /**
                Investments
    In kmymoney, all accounts representing investments (stocks, shares, bonds, etc.) must
    have an associated investment account (e.g. a broker account). The stock account holds
    the share balance, the investment account a money balance.
    Gnucash does not do this, so we cannot automate this function. If you have investments,
    you must select one of the following options.
       0 - create a separate investment account for each stock with the same name as the stock
       1 - create a single investment account to hold all stocks - you will be asked for a name
       2 - create multiple investment accounts - you will be asked for a name for each stock 
       N.B. :- option 2 doesn't really work quite as desired at present
  */
  unsigned int m_investmentOption;
  /*          Debug Options
    If you don't know what these are, best leave them alone.
       gncdebug - produce general debug messages
       xmldebug - produce a trace of the gnucash file XML
       bAnonymize - hide personal data (account names, payees, etc., randomize money amounts)
  */
  bool gncdebug; // general debug messages
  bool xmldebug; // xml trace
  bool bAnonymize; // anonymize input
  static double m_fileHideFactor; // an overall anonymization factor to be applied to all items
  bool developerDebug;
private:
  void setOptions (); // to set user options, with a dialog eventually
  void setFileHideFactor ();
#ifndef _GNCFILEANON
  MyMoneyTransaction convertTemplateTransaction (const QString, const GncTransaction *);
  void convertTemplateSplit (const QString, const GncTemplateSplit *);
#endif // _GNCFILEANON
  // wind up when all done
  void terminate();
  const QString buildReportSection (const QString);
  bool writeReportToFile (const QValueList<QString>);
  // main storage
#ifndef _GNCFILEANON
  IMyMoneyStorage *m_storage;
#else
  QTextStream oStream;
#endif // _GNCFILEANON
  XmlReader *xr;
  /** to hold the callback pointer for the progress bar */
  void (*m_progressCallback)(int, int, const QString&);
  /** counters for reporting */
  int m_commodityCount;
  int m_priceCount;
  int m_accountCount;
  int m_transactionCount;
  int m_templateCount;
  int m_scheduleCount;
#ifndef _GNCFILEANON
  // counters for error reporting
  int m_ccCount, m_orCount, m_scCount;
  // currency counter
  QMap<QString, unsigned int> m_currencyCount;
  /**
    * Map gnucash vs. Kmm ids for accounts, equities, schedules
    */
  QMap<QCString, QCString> m_mapIds;
  QMap<QCString, QCString> m_mapEquities;
  QMap<QCString, QCString> m_mapSchedules;
  /**
    * A list of stock accounts (gnc ids) which will be held till the end 
      so we can implement the user's investment option
    */
  QValueList<QString> m_stockList;
  /**
    * Temporary storage areas for transaction processing
    */
  QString m_defaultPayee; // kmm is unhappy without a payee though gnc doesn't necessarily have one
  QCString m_txCommodity; // save commodity for current transaction
  QString m_txPayeeId;    // gnc has payee at tx level, we need it at split level
  QDate m_txDatePosted;   // ditto for post date
  /** In kmm, the order of splits is critical to some operations. These
    * areas will hold the splits until we've read them all */
  QValueList<MyMoneySplit> m_splitList, m_liabilitySplitList, m_otherSplitList;
  bool m_potentialTransfer;       // to determine whether this might be a transfer
  /** Schedules are processed through 3 different functions, any of which may set this flag */
  bool m_suspectSchedule;
  /**
  * A holding area for template txs while we're waiting for the schedules
  */
  QPtrList<GncTransaction> m_templateList;
  /**
    * To hold message data till final report
    */
  QPtrList<GncMessageArgs> m_messageList;
  GncMessages *m_messageTexts;
  /**
    * Internal utility functions
    */
  const QString createPayee (const QString); // create a payee and return it's id
  const QCString createOrphanAccount (const QString); // create unknown account and return the id
  QDate incrDate (QDate lastDate, unsigned char interval, unsigned int intervalCount); // for date calculations
  MyMoneyAccount checkConsistency (MyMoneyAccount& parent, MyMoneyAccount& child); // gnucash is sometimes TOO flexible
  void checkInvestmentOption (QString stockId); // implement user investment option
#endif // _GNCFILEANON
};

#endif // MYMONEYSTORAGEGNC_H
