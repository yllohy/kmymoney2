#ifndef OFXPARTNER_H
#define OFXPARTNER_H

#include <libofx/libofx.h>
#include <string>
#include <vector>

namespace OfxPartner
{
  /**
    * setup the directory where the files will be stored.
    * @a dir must end with a '/' and must exist. Call this
    * before any other of the functions of OfxPartner. The
    * default will be to store the files in the current
    * directory.
    */
  void setDirectory(const std::string& dir);

  void ValidateIndexCache(void);
  OfxFiServiceInfo ServiceInfo(const std::string& fipid);
  std::vector<std::string> BankNames(void);
  std::vector<std::string> FipidForBank(const std::string& bank);
}

#endif // OFXPARTNER_H
