#include "http.h"

namespace Http {

std::string         Headers::ToString() const {
    std::string headers_str;
    for (Map::const_iterator it = __map.begin(); it != __map.end(); ++it) {
        headers_str += HeaderPairToString(*it) + std::string("\n");
    }
    return headers_str;
}

std::string         Headers::HeaderPairToString(const Pair& head) {
    return head.first + std::string(": ") + head.second;
}

void                Headers::SetHeader(const std::string& h_name, const std::string& h_val) {
    __map[h_name] = h_val;
}

std::string         Headers::GetHeader(const std::string& h_name) const {
    Map::const_iterator it = __map.find(h_name);
    if (it != __map.end())
        return it->first;
    return "";
}

Headers::DelStatus   Headers::DeleteHeader(const std::string& h_name) {
    return __map.erase(h_name) == 0 ? HEADER_DEL_FAIL : HEADER_DEL_OK;
}

}  // namespace Http
