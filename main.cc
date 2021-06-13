#include "common/types.h"
#include "common/error.h"
#include <iostream>
#include "sources/http/uri.h"

// #include <cstdio>

// enum ParseErrors {
//     OK,
//     BAD_HEADER,
//     BAD_GUY
// };

// const char* ParseErrMessages[] = {
//     "No error",
//     "Bad header",
//     "Bad ass"
// };

// struct HttpError: public Error {
//     explicit HttpError(ParseErrors ec) : Error(ec, ParseErrMessages[ec]) {}
// };

int main(int, char**) {
    Http::URI::Authority au;
    Http::URI::QueryMap m;
    // errno = 9;
    // SystemError write_err(errno);
    // printf("%d: %s\n", write_err.errcode, write_err.message.c_str());

    // HttpError http_err(BAD_HEADER);
    // printf("%d: %s\n", http_err.errcode, http_err.message.c_str());
    Http::URI uu;
    Error err(0, "no error");
    // au = uu.DecodeAuthority("userinfo:passwrd@www.example.com:8080", &err);
    // foo://example.com:8042/over/there?name=ferret#nose
    // http://www.example.com/mypage.html?crcat=test&crsource=test&crkw=buy-a-lot
    // std::cout << uu.EncodeAuthority(au) << std::endl;
    //m = uu.DecodeQuery("f1=v1&f2=v2&f3=v3");
    // std::cout << uu.EncodeQuery(m) << std::endl;
    // uu.DecodeUri("http://www.example.com/mypage.html?crcat=test&crsource=test&crkw=buy-a-lot#nose");

    // {
    //     Error err(0, "No error");
    //     std::cout << uu.EncodeUri(uu.DecodeUri("http://www.example.com/mypage.html?crcat=test1&crsource=test_source&crkw=buy-a-lot&crcat=test2#nose", &err)) << std::endl;
    //     std::cout << "Error: (Code: " << err.errcode <<  ", Msg: \"" << err.message << "\")\n";
    // }

    // {
    //     Error err(0, "No error");
    //     Http::URI uri = uu.DecodeUri("http://www.example.com?req=%D0%9F%D1%80%D0%B8%D0%B2%D0%B5%D1%82%20%D0%BC%D0%B8%D1%80%20-%20UTF8&req_eng=Hello_World", &err);
    //     std::cout << uri.__query_params["req"] << std::endl;
    //     std::cout << uu.EncodeUri(uri) << std::endl;
    //     std::cout << "Error: (Code: " << err.errcode <<  ", Msg: \"" << err.message << "\")\n";
    // }
    uu = uu.DecodeUri("http://hello.world.com", &err);

}
