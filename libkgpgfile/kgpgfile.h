/***************************************************************************
                             kgpgfile.h
                             -------------------
    begin                : Fri Jan 23 2004
    copyright            : (C) 2004,2005 by Thomas Baumgart
    email                : thb@net-bembel.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KGPGFILE_H
#define KGPGFILE_H

#include <qfile.h>
#include <qobject.h>
#include <qstring.h>
#include <qstringlist.h>

/**
  * @author Thomas Baumgart
  */

class KShellProcess;
class KProcess;

/**
  * A class for reading and writing data to/from an
  * encrypted e.g. file.
  *
  * This class presents a QFile based object to the application
  * but reads/writes data from/to the file through an instance of GPG.
  *
  * @code
  *
  *  +------------------+   write  +-----------+     stdin +-------+     +--------+
  *  |                  |--------->|\          |---------->|       |---->|        |
  *  | Application code |   read   | QFile     |    stdout |  GPG  |     |  File  |
  *  |                  |<---------|/          |<----------|       |<----|        |
  *  +------------------+          |  KGPGFile |           +-------+     +--------+
  *                |        control|           |
  *                +-------------->|           |
  *                                +-----------+
  * @endcode
  *
  * The @p write interface contains methods as writeBlock() and putch(), the @p read
  * interface the methods readBlock(), getch() and ungetch(). The @p control interface
  * special methods only available with KGPGFile e.g. addRecipient(), keyAvailable() and
  * GPGAvailable(). Other, more general methods such as open(), close() and flush() are
  * not shown in the above picture.
  */
class KGPGFile : public QObject, public QFile
{
  Q_OBJECT

public:
  KGPGFile(const QString& fname = "",
           const QString& homedir = "~/.gnupg",
           const QString& options = "");

  ~KGPGFile();

  virtual bool open(int mode);
  virtual void close(void);
  virtual void flush(void);

  virtual Offset size(void) const { return 0; };

  virtual Q_LONG readBlock(char *data, Q_ULONG maxlen);
  virtual Q_LONG writeBlock(const char *data, Q_ULONG maxlen);
  virtual QByteArray readAll(void);

  virtual int getch(void);
  virtual int putch(int c);
  virtual int ungetch(int c);

  /**
    * Adds a recipient for whom the file should be encrypted.
    * At least one recipient must be specified using this
    * method before the file can be written to. @p recipient
    * must contain a valid name as defined by GPG. See the
    * GPG documentation for more information.
    *
    * @param recipient recipients identification (e.g. e-mail address)
    */
  void addRecipient(const QCString& recipient);

  /**
    * sets the name of the file to @p fn. This method must be
    * called prior to open().
    */
  void setName(const QString& fn);
  void setComment(const QString& txt);

  const QCString errmsg(void) const { return m_errmsg; };
  const int exitStatus(void) const { return m_exitStatus; };

  /**
    * Checks whether GPG is available or not
    *
    * @retval true GPG can be started and returns a version number
    * @retval false GPG is not available
    */
  static const bool GPGAvailable(void);

  /**
    * Checks whether a key for a given user-id @p name exists.
    *
    * @param name the user-id to be checked. @p name can be
    *             any reference understood by GPG (e.g. an e-mail
    *             address or a key-id)
    * @retval true key for user-id @p name was found
    * @retval false key for user-id @p not available
    */
  static const bool keyAvailable(const QString& name);

  /**
    * This function returns a list of the secret keys contained
    * in the keyring. Each list item is devided into two fields
    * separated by a colon (':'). The first field contains the
    * key id, the second field the name. The list may contain
    * multiple entries with the same key-id and different names.
    *
    * Example of an entry in the list:
    *
    *    "9C59DB40B75DD3BA:Thomas Baumgart <ipwizard@users.sourceforge.net>"
    */
  static void secretKeyList(QStringList& list);

#ifdef KMM_DEBUG
  void dumpUngetBuffer(void);
  void dumpBuffer(char *s, int len) const;
#endif

protected slots:
  void slotGPGExited(KProcess *);
  void slotDataFromGPG(KProcess *, char *buf, int len);
  void slotErrorFromGPG(KProcess *, char *buf, int len);
  void slotSendDataToGPG(KProcess *);

private:
  void init(void);
  const bool startProcess(const QStringList& args);
  Q_LONG _writeBlock(const char *data, Q_ULONG maxlen);
  bool open(int mode, const QString&, bool skipPasswd);

private:
  QString m_fn;
  QString m_pubring;
  QString m_secring;
  QString m_options;
  QString m_comment;
  QString m_homedir;

  KShellProcess* m_process;

  QValueList<QCString> m_recipient;
  QCString m_ungetchBuffer;
  QCString m_errmsg;
  int      m_exitStatus;
  Q_LONG  m_readRemain;
  char*   m_ptrRemain;
  bool    m_needExitLoop;
};

#endif
