/***************************************************************************
                          kgpgfile.cpp  -  description
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

#include <qfile.h>
#include <qdir.h>
#include <qstring.h>

#include <kapplication.h>
#include <klocale.h>
#include <kprocess.h>
#include <kpassdlg.h>
#include <klibloader.h>

#include "kgpgfile.h"

#if 0
class KGPGFileFactory : public KLibFactory
{
public:
    KGPGFileFactory() : KLibFactory() {}
    ~KGPGFileFactory(){}
    QObject *createObject( QObject *, const char *, const char*, const QStringList & )
    {
        return new KGPGFile;
    }
};

extern "C" {
    void *init_libkgpgfile()
    {
        return new KGPGFileFactory;
    }
}
#endif

KGPGFile::KGPGFile(const QString& fn, const QString& homedir, const QString& options) :
  m_options(options),
  m_homedir(homedir),
  m_readRemain(0),
  m_needExitLoop(false)
{
  setName(fn);
  m_exitStatus = -2;
  m_ungetchBuffer = QCString();
  m_comment = "created by KGPGFile";
  // qDebug("ungetchbuffer %d", m_ungetchBuffer.length());
}

KGPGFile::~KGPGFile()
{
  close();
}

void KGPGFile::init(void)
{
  setFlags(IO_Sequential);
  setStatus(IO_Ok);
  setState(0);
}

void KGPGFile::setName(const QString& fn)
{
  m_fn = fn;
  if(fn[0] == '~') {
    m_fn = QDir::homeDirPath()+fn.mid(1);

  } else if(QDir::isRelativePath(m_fn)) {
    QDir dir(fn);
    m_fn = dir.absPath();
  }
  // qDebug("setName: '%s'", m_fn.data());
}

void KGPGFile::flush(void)
{
  // no functionality
}

bool KGPGFile::atEnd(void)
{
  if(!isOpen())
    return false;

  if(isReadable()) {
    int ch = getch();
    bool result = ch == EOF;
    ungetch(ch);
    return result;
  }
  return false;
}

void KGPGFile::addRecipient(const QCString& recipient)
{
  m_recipient << recipient;
}

bool KGPGFile::open(int mode)
{
  m_errmsg.resize(1);

  if(isOpen())
    return false;

  if(m_fn.isEmpty())
    return false;

  init();
  setMode(mode);

  if(!(isReadable() || isWritable()))
    return false;

  if(isWritable()) {
    if(m_recipient.count() == 0)
      return false;
    if(!checkAccess(m_fn, W_OK))
      return false;
  }

  QStringList args;

  args << "--homedir" << m_homedir
       << "-q"
       << "--batch";

  if(isWritable()) {
    args << "-ea"
         << "-z" << "6"
         << "--comment" << QString("\"") + m_comment + QString("\"")
         << "-o" << m_fn;
    QValueList<QCString>::Iterator it;
    for(it = m_recipient.begin(); it != m_recipient.end(); ++it)
      args << "-r" << *it;

    // some versions of GPG had trouble to replace a file
    // so we delete it first
    QFile::remove(m_fn);
  } else {
    args << "-da"
         << "--passphrase-fd" << "0"
         << m_fn;
  }

  QCString pwd;
  if(isReadable()) {
    if(KPasswordDialog::getPassword(pwd, i18n("Enter passphrase"), 0) == QDialog::Rejected)
      return false;
  }

  if(!startProcess(args))
    return false;

  if(!m_process)
    return false;

  if(isReadable()) {
    if(_writeBlock(pwd.data(), pwd.length()) == -1) {
      // qDebug("Sending passphrase failed");
      return false;
    }
    m_process->closeStdin();
  }

  setState( IO_Open );
  ioIndex = 0;
  // qDebug("File open");
  return true;
}

const bool KGPGFile::startProcess(const QStringList& args)
{
  // now start the KProcess with GPG
  m_process = new KShellProcess();
  *m_process << "gpg";
  *m_process << args;

  connect(m_process, SIGNAL(processExited(KProcess *)),
          this, SLOT(slotGPGExited(KProcess *)));

  connect(m_process, SIGNAL(receivedStdout(KProcess*, char*, int)),
          this, SLOT(slotDataFromGPG(KProcess*, char*, int)));

  connect(m_process, SIGNAL(receivedStderr(KProcess*, char*, int)),
          this, SLOT(slotErrorFromGPG(KProcess*, char*, int)));

  connect(m_process, SIGNAL(wroteStdin(KProcess *)),
          this, SLOT(slotSendDataToGPG(KProcess *)));

  if(!m_process->start(KProcess::NotifyOnExit, (KProcess::Communication)(KProcess::Stdin|KProcess::Stdout|KProcess::Stderr))) {
    delete m_process;
    m_process = 0;
    return false;
  }

  // let the process settle and see if it starts and survives ;-)
  kapp->processEvents(100);
  return true;
}

void KGPGFile::close(void)
{
  if(!isOpen())
    return;

  // finish the KProcess and clean up things
  if(m_process) {
    if(isWritable()) {
      if(m_process->isRunning()) {
        m_process->closeStdin();
        // now wait for GPG to finish
        m_needExitLoop = true;
        qApp->enter_loop();
      } else
        m_process->kill();

    } else if(isReadable()) {
      if(m_process->isRunning()) {
        m_process->closeStdout();
        // now wait for GPG to finish
        m_needExitLoop = true;
        qApp->enter_loop();
      } else
        m_process->kill();
    }
  }
  // m_ungetMutex.lock();
  m_ungetchBuffer = QCString();
  // m_ungetMutex.unlock();
  setState(0);
  m_recipient.clear();
}

int KGPGFile::getch(void)
{
  if(!isOpen())
    return EOF;
  if(!isReadable())
    return EOF;

  int ch;

  if(!m_ungetchBuffer.isEmpty()) {
    // m_ungetMutex.lock();
    ch = m_ungetchBuffer[0];
    m_ungetchBuffer.remove(0, 1);
    // m_ungetMutex.unlock();

  } else {
    char buf[1];
    ch = readBlock(buf,1) == 1 ? buf[0] : EOF;
  }

  return ch;
}

int KGPGFile::ungetch(int ch)
{
  if(!isOpen())
    return EOF;
  if(!isReadable())
    return EOF;

  if(ch != EOF) {
    // m_ungetMutex.lock();
    m_ungetchBuffer.insert(0, ch);
    // m_ungetMutex.unlock();
  }

  return ch;
}

int KGPGFile::putch(int c)
{
  char  buf[1];
  buf[0] = c;
  if(writeBlock(buf, 1) != EOF)
    return c;
  return EOF;
}

Q_LONG KGPGFile::writeBlock(const char *data, Q_ULONG maxlen)
{
  if(!isOpen())
    return EOF;
  if(!isWritable())
    return EOF;

  return _writeBlock(data, maxlen);
}

Q_LONG KGPGFile::_writeBlock(const char *data, Q_ULONG maxlen)
{
  if(!m_process)
    return EOF;
  if(!m_process->isRunning())
    return EOF;

  if(m_process->writeStdin(data, maxlen)) {
    // wait until the data has been written
    m_needExitLoop = true;
    qApp->enter_loop();
    if(!m_process)
      return EOF;
    return maxlen;

  } else
    return EOF;
}

Q_LONG KGPGFile::readBlock(char *data, Q_ULONG maxlen)
{
  if(maxlen == 0)
    return 0;

  if(!isOpen())
    return EOF;
  if(!isReadable())
    return EOF;

  Q_ULONG nread = 0;
  if(!m_ungetchBuffer.isEmpty()) {
    // m_ungetMutex.lock();
    int l = m_ungetchBuffer.length();
    if(maxlen < l)
      l = maxlen;
    memcpy(data, m_ungetchBuffer, l);
    nread += l;
    data = &data[l];
    m_ungetchBuffer.remove(0, l);
    // m_ungetMutex.unlock();

    if(!m_process) {
      // qDebug("read %d bytes from unget buffer", nread);
      return nread;
    }
  }

  // check for EOF
  if(!m_process) {
    // qDebug("EOF (no process)");
    return EOF;
  }

  m_readRemain = maxlen - nread;
  m_ptrRemain = data;
  if(m_readRemain) {
    m_process->resume();
    m_needExitLoop = true;
    qApp->enter_loop();
  }
  // if nothing has been read (maxlen-m_readRemain == 0) then we assume EOF
  if((maxlen - m_readRemain) == 0) {
    // qDebug("EOF");
    return EOF;
  }
  // qDebug("read %d bytes", maxlen - m_readRemain);
  return maxlen - m_readRemain;
}

void KGPGFile::slotGPGExited(KProcess* )
{
  // qDebug("GPG finished");
  if(m_process) {
    if(m_process->normalExit()) {
      m_exitStatus = m_process->exitStatus();
    } else {
      m_exitStatus = -1;
    }
    delete m_process;
    m_process = 0;
  }

  if(m_needExitLoop) {
    m_needExitLoop = false;
    qApp->exit_loop();
  }
}

void KGPGFile::slotDataFromGPG(KProcess* proc, char* buf, int len)
{
  // qDebug("Received %d bytes on stdout", len);

  // copy current buffer to application
  int copylen;
  copylen = m_readRemain < len ? m_readRemain : len;
  if(copylen != 0) {
    memcpy(m_ptrRemain, buf, copylen);
    m_ptrRemain += copylen;
    buf += copylen;
    m_readRemain -= copylen;
    len -= copylen;
  }

  // store rest of buffer in ungetch buffer
  // m_ungetMutex.lock();
  while(len--) {
    m_ungetchBuffer += *buf++;
  }
  // m_ungetMutex.unlock();

  // if we have all the data the app requested, we can safely suspend
  if(m_readRemain == 0) {
    proc->suspend();
    // wake up the recipient
    if(m_needExitLoop) {
      m_needExitLoop = false;
      qApp->exit_loop();
    }
  }
  // qDebug("end slotDataFromGPG");
}

void KGPGFile::slotErrorFromGPG(KProcess *, char *buf, int len)
{
  // qDebug("Received %d bytes on stderr", len);
  QCString msg;
  msg.setRawData(buf, len);
  m_errmsg += msg;
  msg.resetRawData(buf, len);
}

void KGPGFile::slotSendDataToGPG(KProcess *)
{
  // qDebug("wrote stdin");
  if(m_needExitLoop) {
    m_needExitLoop = false;
    qApp->exit_loop();
  }
}

const bool KGPGFile::GPGAvailable(void)
{
  KGPGFile dummy;

  QStringList args;
  args << "--version";
  if(!dummy.startProcess(args)) {
    return false;
  }

  // wait for the process to finish
  while(dummy.m_process && dummy.m_process->isRunning()) {
    kapp->processEvents(100);
  }

  if(dummy.m_exitStatus == 0) {
    return true;
  }

  return false;
}

const bool KGPGFile::keyAvailable(const QString& name)
{
  KGPGFile dummy;

  QStringList args;
  args << "--list-keys" << name;
  if(!dummy.startProcess(args)) {
    return false;
  }

  // wait for the process to finish
  while(dummy.m_process && dummy.m_process->isRunning()) {
    kapp->processEvents(100);
  }

  if(dummy.m_exitStatus == 0) {
    return true;
  }

  return false;
}

/*
// key generation
  char * gpg_input =
    g_strdup_printf("Key-Type: DSA\n"
                    "Key-Length: 1024\n"
                    "Subkey-Type: ELG-E\n"
                    "Subkey-Length: 1024\n"
                    "Name-Real: %s\n"
                    "Name-Comment: %s\n"
                    "Name-Email: %s\n"
                    "Passphrase: %s\n"
                    "%%commit\n",
                    username ? username : "",
                    idstring ? idstring : "",
                    email ? email : "",
                    passphrase ? passphrase : "");
  char * argv [] =
  { "gpg",
    "--batch",
    "-q",
    "--gen-key",
    "--keyring",
    "~/.gnucash/gnucash.pub",
    "--secret-keyring",
    "~/.gnucash/gnucash.sec",
    NULL
  };

  char * retval = gnc_gpg_transform(gpg_input, strlen(gpg_input), NULL, argv);
  g_free(gpg_input);
  return retval;

 */

