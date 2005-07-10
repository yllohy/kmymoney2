/***************************************************************************
                          kgpgfile.h  -  description
                             -------------------
    begin                : Fri Jan 23 2004
    copyright            : (C) 2004 by Thomas Baumgart
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
#include <qmutex.h>

/**
  * @author Thomas Baumgart
  */

class KShellProcess;
class KProcess;

/**
  * A class for reading and writing data to/from an
  * encrypted e.g. file.
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
  virtual Offset at(void) { return 0; };
  virtual bool at(Offset) { return false; };
  virtual bool atEnd(void);

  virtual Q_LONG readBlock(char *data, Q_ULONG maxlen);
  virtual Q_LONG writeBlock(const char *data, Q_ULONG maxlen);

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

  void dumpUngetBuffer(void);

protected slots:
  void slotGPGExited(KProcess *);
  void slotDataFromGPG(KProcess *, char *buf, int len);
  void slotErrorFromGPG(KProcess *, char *buf, int len);
  void slotSendDataToGPG(KProcess *);

private:
  void init(void);
  Q_LONG _writeBlock(const char *data, Q_ULONG maxlen);
  const bool startProcess(const QStringList& args);

private:
  QString m_fn;
  QString m_pubring;
  QString m_secring;
  QString m_options;
  QString m_comment;
  QString m_homedir;

  Q_LONG  m_readRemain;
  char*   m_ptrRemain;

  bool    m_needExitLoop;

  KShellProcess* m_process;

  QValueList<QCString> m_recipient;
  QMutex   m_ungetMutex;
  QCString m_ungetchBuffer;
  QCString m_errmsg;
  int      m_exitStatus;
};

#endif
