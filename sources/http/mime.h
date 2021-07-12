#ifndef HTTP_MIME_H_
#define HTTP_MIME_H_

#include <string>
#include <map>

#include "common/types.h"

namespace Mime {

typedef std::map<std::string, std::string> MimeTypesMap;

std::string MapType(const MimeTypesMap& map, const std::string& filename,
                                             const std::string& default_type = "text/plain");

};

#endif  // HTTP_MIME_H_
