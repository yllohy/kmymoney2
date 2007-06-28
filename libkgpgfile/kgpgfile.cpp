/***************************************************************************
                             kgpgfile.cpp
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

#ifdef HAVE_CONFIG
#include <config.h>
#endif

#include "kdecompat.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <qfile.h>
#include <qdir.h>
#include <qstring.h>

#if QT_IS_VERSION(3,3,0)
#include <qeventloop.h>
#endif


// ----------------------------------------------------------------------------
// KDE Includes

#include <kapplication.h>
#include <klocale.h>
#include <kprocess.h>
#include <kpassdlg.h>
#include <klibloader.h>

// ----------------------------------------------------------------------------
// Project Includes

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

void KGPGFile::addRecipient(const QCString& recipient)
{
  m_recipient << recipient;
}

bool KGPGFile::open(int mode)
{
  return open(mode, QString(), false);
}

bool KGPGFile::open(int mode, const QString& cmdArgs, bool skipPasswd)
{
  bool useOwnPassphrase = (getenv("GPG_AGENT_INFO") == 0);

  // qDebug("KGPGFile::open(%d)", mode);
  m_errmsg.resize(1);
  if(isOpen()) {
    // qDebug("File already open");
    return false;
  }

  // qDebug("check filename empty");
  if(m_fn.isEmpty())
    return false;

  // qDebug("setup file structures");
  init();
  setMode(mode);

  // qDebug("check valid access mode");
  if(!(isReadable() || isWritable()))
    return false;

  if(isWritable()) {
    // qDebug("check recipient count");
    if(m_recipient.count() == 0)
      return false;
    // qDebug("check access rights");
    if(!checkAccess(m_fn, W_OK))
      return false;
  }

  QStringList args;
  if(cmdArgs.isEmpty()) {
    args << "--homedir" << QString("\"") + m_homedir + QString("\"")
        << "-q"
        << "--batch";

    if(isWritable()) {
      args << "-ea"
          << "-z" << "6"
          << "--comment" << QString("\"") + m_comment + QString("\"")
          << "-o" << QString("\"") + m_fn + QString("\"");
      QValueList<QCString>::Iterator it;
      for(it = m_recipient.begin(); it != m_recipient.end(); ++it)
        args << "-r" << QCString("\"") + *it + QCString("\"");

      // some versions of GPG had trouble to replace a file
      // so we delete it first
      QFile::remove(m_fn);
    } else {
      args << "-da";
      if(useOwnPassphrase)
        args << "--passphrase-fd" << "0";
      args << "--no-default-recipient" << QString("\"") + m_fn + QString("\"");
    }
  } else {
    args = QStringList::split(" ", cmdArgs);
  }

  QCString pwd;
  if(isReadable() && useOwnPassphrase && !skipPasswd) {
    if(KPasswordDialog::getPassword(pwd, i18n("Enter passphrase"), 0) == QDialog::Rejected)
      return false;
  }

  // qDebug("starting GPG process");
  if(!startProcess(args))
    return false;

  // qDebug("check GPG process running");
  if(!m_process) {
    // if the process is not present anymore, we have to check
    // if it was a read operation and we might already have data
    // and the process finished normally. In that case, we
    // just continue.
    if(isReadable()) {
      if(m_ungetchBuffer.isEmpty())
        return false;
    } else
      return false;
  }

  if(isReadable() && useOwnPassphrase && !skipPasswd) {
    // qDebug("Passphrase is '%s'", pwd.data());
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

  // QString arglist = args.join(":");
  // qDebug("gpg '%s'", arglist.data());

  connect(m_process, SIGNAL(processExited(KProcess *)),
          this, SLOT(slotGPGExited(KProcess *)));

  connect(m_process, SIGNAL(receivedStdout(KProcess*, char*, int)),
          this, SLOT(slotDataFromGPG(KProcess*, char*, int)));

  connect(m_process, SIGNAL(receivedStderr(KProcess*, char*, int)),
          this, SLOT(slotErrorFromGPG(KProcess*, char*, int)));

  connect(m_process, SIGNAL(wroteStdin(KProcess *)),
          this, SLOT(slotSendDataToGPG(KProcess *)));

  if(!m_process->start(KProcess::NotifyOnExit, (KProcess::Communication)(KProcess::Stdin|KProcess::Stdout|KProcess::Stderr))) {
    // qDebug("m_process->start failed");
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
  // qDebug("KGPGFile::close()");
  if(!isOpen()) {
    // qDebug("File not open");
    return;
  }

  // finish the KProcess and clean up things
  if(m_process) {
    if(isWritable()) {
      // qDebug("Finish writing");
      if(m_process->isRunning()) {
        m_process->closeStdin();
        // now wait for GPG to finish
        m_needExitLoop = true;
        qApp->enter_loop();
      } else
        m_process->kill();

    } else if(isReadable()) {
      // qDebug("Finish reading");
      if(m_process->isRunning()) {
        m_process->closeStdout();
        // now wait for GPG to finish
        m_needExitLoop = true;
        qApp->enter_loop();
      } else
        m_process->kill();
    }
  }
  m_ungetchBuffer = QCString();
  setState(0);
  m_recipient.clear();
  // qDebug("File closed");
}

int KGPGFile::getch(void)
{
  if(!isOpen())
    return EOF;
  if(!isReadable())
    return EOF;

  int ch;

  if(!m_ungetchBuffer.isEmpty()) {
    ch = (m_ungetchBuffer)[0] & 0xff;
    m_ungetchBuffer.remove(0, 1);

  } else {
    char buf[1];
    ch = (readBlock(buf,1) == 1) ? (buf[0] & 0xff) : EOF;
  }

  // qDebug("getch returns 0x%02X", ch);
  return ch;
}

int KGPGFile::ungetch(int ch)
{
  if(!isOpen())
    return EOF;
  if(!isReadable())
    return EOF;

  if(ch != EOF) {
    // qDebug("store 0x%02X in ungetchbuffer", ch & 0xff);
    m_ungetchBuffer.insert(0, ch & 0xff);
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
  // char *oridata = data;
  if(maxlen == 0)
    return 0;

  if(!isOpen())
    return EOF;
  if(!isReadable())
    return EOF;

  Q_ULONG nread = 0;
  if(!m_ungetchBuffer.isEmpty()) {
    unsigned l = m_ungetchBuffer.length();
    if(maxlen < l)
      l = maxlen;
    memcpy(data, m_ungetchBuffer, l);
    nread += l;
    data = &data[l];
    m_ungetchBuffer.remove(0, l);

    if(!m_process) {
      // qDebug("read %d bytes from unget buffer", nread);
      // dumpBuffer(oridata, nread);
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
    // qDebug("EOF (nothing read)");
    return EOF;
  }
  // qDebug("return %d bytes", maxlen - m_readRemain);
  // dumpBuffer(oridata, maxlen - m_readRemain);
  return maxlen - m_readRemain;
}

QByteArray KGPGFile::readAll(void)
{
  // use a larger blocksize than in the QIODevice version
  const int blocksize = 8192;
  int nread = 0;
  QByteArray ba;
  while ( !atEnd() ) {
    ba.resize( nread + blocksize );
    int r = readBlock( ba.data()+nread, blocksize );
    if ( r < 0 )
      return QByteArray();
    nread += r;
  }
  ba.resize( nread );
  return ba;
}

void KGPGFile::slotGPGExited(KProcess* )
{
  // qDebug("GPG finished");
  if(m_process) {
    if(m_process->normalExit()) {
      m_exitStatus = m_process->exitStatus();
      if(m_exitStatus != 0)
        setStatus(IO_UnspecifiedError);
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
  while(len--) {
    m_ungetchBuffer += *buf++;
  }

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
  QString output;
  char  buffer[1024];
  Q_LONG len;

  KGPGFile file;
  file.open(IO_ReadOnly, "--version", true);
  while((len = file.readBlock(buffer, sizeof(buffer)-1)) != EOF) {
    buffer[len] = 0;
    output += QString(buffer);
  }
  file.close();
  return !output.isEmpty();
}

const bool KGPGFile::keyAvailable(const QString& name)
{
  QString output;
  char  buffer[1024];
  Q_LONG len;

  KGPGFile file;
  QString args = QString("--list-keys %1").arg(name);
  file.open(IO_ReadOnly, args, true);
  while((len = file.readBlock(buffer, sizeof(buffer)-1)) != EOF) {
    buffer[len] = 0;
    output += QString(buffer);
  }
  file.close();
  return !output.isEmpty();
}

void KGPGFile::secretKeyList(QStringList& list)
{
  QString output;
  char  buffer[1024];
  Q_LONG len;

  list.clear();
  KGPGFile file;
  file.open(IO_ReadOnly, "--list-secret-keys --with-colons", true);
  while((len = file.readBlock(buffer, sizeof(buffer)-1)) != EOF) {
    buffer[len] = 0;
    output += QString(buffer);
  }
  file.close();

  // now parse the data. it looks like:
  /*
    sec::1024:17:9C59DB40B75DD3BA:2001-06-23::::Thomas Baumgart <ipwizard@users.sourceforge.net>:::
    uid:::::::::Thomas Baumgart <thb@net-bembel.de>:
    ssb::1024:16:85968A70D1F83C2B:2001-06-23:::::::
    sec::1024:17:59B0F826D2B08440:2005-01-03:2010-01-02:::KMyMoney emergency data recovery <kmymoney-recover@users.sourceforge.net>:::
    ssb::2048:16:B3DABDC48C0FE2F3:2005-01-03:::::::
  */
  QStringList lines = QStringList::split("\n", output);
  QStringList::iterator it;
  QString currentKey;
  for(it = lines.begin(); it != lines.end(); ++it) {
    // qDebug("Parsing: '%s'", (*it).data());
    QStringList fields = QStringList::split(":", (*it), true);
    if(fields[0] == "sec") {
      currentKey = fields[4];
      list << QString("%1:%2").arg(currentKey).arg(fields[9]);
    } else if(fields[0] == "uid") {
      list << QString("%1:%2").arg(currentKey).arg(fields[9]);
    }
  }
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

#ifdef KMM_DEBUG
void KGPGFile::dumpBuffer(char *s, int len) const
{
  QString data, tmp, chars;
  unsigned long addr = 0x0;

  while(1) {
    if(addr && !(addr & 0x0f)) {
      qDebug("%s %s", data.data(), chars.data());
      if(!len)
        break;
    }
    if(!(addr & 0x0f)) {
      data = tmp.sprintf("%08lX", addr);
      chars = QString();
    }
    if(!(addr & 0x03)) {
      data += " ";
    }
    ++addr;

    if(!len) {
      data += "  ";
      chars += " ";
      continue;
    }

    data += tmp.sprintf("%02X", *s & 0xff);
    if(*s >= ' ' && *s <= '~')
      chars += *s & 0xff;
    else
      chars += '.';
    ++s;
    --len;
  }
}
#endif

#include "kgpgfile.moc"
