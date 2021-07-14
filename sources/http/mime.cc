#include "common/string_utils.h"
#include "http/mime.h"

namespace Mime {

std::string MapType(const MimeTypesMap& tmap, const std::string& filename,
                                             const std::string& default_type) {

    std::string  mime_type = default_type;
    usize  dot_position = filename.find_last_of('.');

    if (dot_position != std::string::npos) {
        std::string file_extension = filename.substr(dot_position + 1);

        if (!file_extension.empty()) {
            MimeTypesMap::const_iterator mime_it = tmap.find(file_extension);
            if (mime_it != tmap.end()) {
                mime_type = mime_it->second;
            }
        }
    }

    return mime_type;
}

}  // namespace Mime
