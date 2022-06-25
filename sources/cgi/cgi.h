#ifndef CGI_CGI_H_
#define CGI_CGI_H_

#include <cstring>
#include <cstdlib>
#include <vector>

#include "http/http.h"

namespace C {

class string {
 private:
    char* Data_;

 public:
    ~string() { free(Data_); }

    string& operator=(const string& source) {
        if (&source != this) {
            if (source.Data_ == NULL)
                Data_ = NULL;
            else
                Data_ = ::strdup(source.Data_);
        }
        return *this;
    }

    string(const string& source)
    : Data_( NULL ) {
        if (source.Data_ == NULL)
            Data_ = NULL;
        else
            Data_ = ::strdup(source.Data_);
    }

    string()
    : Data_( NULL ) {
    }

    string(const std::string& str) // NOLINT(*)
    : Data_( ::strdup(str.c_str()) ) {
    }

    string(const char* str) { // NOLINT(*)
        if (str == NULL) {
            Data_ = NULL;
        } else {
            Data_ = ::strdup(str);
        }
    }

    operator std::string() const {
        return std::string(Data_ == NULL ? "" : Data_);
    }

    operator const char*() const {
        return Data_;
    }
};

}  // namespace C

namespace Cgi {

typedef std::vector<C::string> CStringVec;
typedef char* const* Envs;

Envs CastToEnvs(const CStringVec& vec);

class Metavars {
 private:
    std::map<std::string, std::string>  Map_;

 public:
    void AddHttpHeaders(const Http::Headers& hdrs);
    void AddEnvs(const Envs& envs);
    void AddMetavars(const Metavars& mtvrs);

    std::map<std::string, std::string>& GetMapRef();
    const std::map<std::string, std::string>& GetMapRef() const;

    CStringVec ToEnvVector() const;
};

}  // namespace Cgi

#endif  // CGI_CGI_H_
