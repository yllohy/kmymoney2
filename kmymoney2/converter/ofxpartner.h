#ifndef OFXPARTNER_H
#define OFXPARTNER_H

#include <libofx/libofx.h>
#include <string>
#include <vector>

namespace OfxPartner
{
  void ValidateIndexCache(void);
  OfxFiServiceInfo ServiceInfo(const std::string& fipid);
  std::vector<std::string> BankNames(void);
  std::vector<std::string> FipidForBank(const std::string& bank);
}

#endif // OFXPARTNER_H
