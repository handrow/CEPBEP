#include "http/uri.h"
#include <iostream>

int main(void) {
    Http::URI uri;

    Error err(0, "No error");
    uri = Http::URI::Parse("HtTp:/Host/mirea/%D0%98%D0%92%D0%91%D0%9E-06-17?A=B&C=D&J=E#sosi_jopy", &err);

    std::cout << "USER:  " << uri.userinfo << "\n"
              << "HOST:  " << uri.hostname << "\n"
              << "PATH:  " << uri.path << "\n"
              << "QUERY: " << uri.query_str << "\n"
              << "FRAG:  " << uri.fragment << "\n";

    std::cout << "\n" << uri.ToString() << "\n";

    {
        Http::Query query = Http::Query::Parse(uri.query_str, &err);
        std::cout << "\nQueries:\n";
        for (Http::Query::ParamMap::iterator it = query.param_map.begin();
            it != query.param_map.end();
            ++it
        ) {
            std::cout << "    " << it->first << "=" << it->second << "\n";
        }
    }

    {
        Http::Query query = Http::Query::Parse("", &err);
        std::cout << "\nQueries:\n";
        for (Http::Query::ParamMap::iterator it = query.param_map.begin();
            it != query.param_map.end();
            ++it
        ) {
            std::cout << "    " << it->first << "=" << it->second << "\n";
        }
    }

    {
        Http::Query query = Http::Query::Parse("abc=egh&", &err);
        std::cout << "\nQueries:\n";
        for (Http::Query::ParamMap::iterator it = query.param_map.begin();
            it != query.param_map.end();
            ++it
        ) {
            std::cout << "    " << it->first << "=" << it->second << "\n";
        }
    }
}