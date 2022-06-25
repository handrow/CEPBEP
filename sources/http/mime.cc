#include "common/string_utils.h"
#include "http/mime.h"

namespace Mime {

std::string MapType(const MimeTypesMap& tmap, const std::string& filename,
                                             const std::string& defaultType) {

    std::string  mimeType = defaultType;
    USize  dotPos = filename.find_last_of('.');

    if (dotPos != std::string::npos) {
        std::string fileExt = filename.substr(dotPos + 1);

        if (!fileExt.empty()) {
            MimeTypesMap::const_iterator mime_it = tmap.find(fileExt);
            if (mime_it != tmap.end()) {
                mimeType = mime_it->second;
            }
        }
    }

    return mimeType;
}

}  // namespace Mime
