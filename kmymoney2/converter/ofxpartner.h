#ifndef OFXPARTNER_H
#define OFXPARTNER_H

#include <libofx/libofx.h>
#include <string>
#include <vector>

namespace OfxPartner
{
  void ValidateIndexCache(const std::string& directory);
  OfxFiServiceInfo ServiceInfo(const std::string& fipid);
  std::vector<std::string> BankNames(const std::string& directory);
  std::vector<std::string> FipidForBank(const std::string& bank);
}

#endif // OFXPARTNER_H
