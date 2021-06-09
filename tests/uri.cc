#include "gtest/gtest.h"
#include "http/uri.h"

TEST(Uri_Tests, scheme_decoding) {
    using namespace Http;

    {
        Error err(0, "No error");
        EXPECT_TRUE(URI::DecodeScheme("HtTp", &err) == URI::URI_SCHEME_HTTP);
        EXPECT_TRUE(err.IsOk());
    }

    {
        Error err(0, "No error");
        EXPECT_TRUE(URI::DecodeScheme("HTTP", &err) == URI::URI_SCHEME_HTTP);
        EXPECT_TRUE(err.IsOk());
    }

    {
        Error err(0, "No error");
        EXPECT_TRUE(URI::DecodeScheme("http", &err) == URI::URI_SCHEME_HTTP);
        EXPECT_TRUE(err.IsOk());
    }

    {
        Error err(0, "No error");
        EXPECT_TRUE(URI::DecodeScheme("https", &err) == URI::URI_SCHEME_UNKNOWN);
        EXPECT_TRUE(err.IsError());
    }
}

TEST(Uri_Tests, scheme_encoding) {
    using namespace Http;

    EXPECT_TRUE(URI::EncodeScheme(URI::URI_SCHEME_HTTP) == "http");
    EXPECT_TRUE(URI::EncodeScheme(URI::URI_SCHEME_UNKNOWN) == "");
    EXPECT_TRUE(URI::EncodeScheme(URI::URI_SCHEME_URL) == "");

}

TEST(Uri_Tests, percent_encoding) {
    using namespace Http;

    EXPECT_TRUE(URI::PercentEncode(":/?&Hello =") == "%3A%2F%3F%26Hello%20%3D");
    EXPECT_TRUE(URI::PercentEncode("Привет мир") == "%D0%9F%D1%80%D0%B8%D0%B2%D0%B5%D1%82%20%D0%BC%D0%B8%D1%80");
    EXPECT_TRUE(URI::PercentEncode("Hello мир") == "Hello%20%D0%BC%D0%B8%D1%80");
    EXPECT_TRUE(URI::PercentEncode("Helloworld") == "Helloworld");
    EXPECT_TRUE(URI::PercentEncode("नमस्ते दुनिया") == "%E0%A4%A8%E0%A4%AE%E0%A4%B8%E0%A5%8D%E0%A4%A4%E0%A5%87%20%E0%A4%A6%E0%A5%81%E0%A4%A8%E0%A4%BF%E0%A4%AF%E0%A4%BE");
    EXPECT_TRUE(URI::PercentEncode("Привіт світ") == "%D0%9F%D1%80%D0%B8%D0%B2%D1%96%D1%82%20%D1%81%D0%B2%D1%96%D1%82");
    EXPECT_TRUE(URI::PercentEncode("Hello world=Привет мир") == "Hello%20world%3D%D0%9F%D1%80%D0%B8%D0%B2%D0%B5%D1%82%20%D0%BC%D0%B8%D1%80");

}

TEST(Uri_Tests, percent_decoding) {
    using namespace Http;

    EXPECT_TRUE(URI::PercentDecode("%3A%2F%3F%26Hello%20%3D") == ":/?&Hello =");
    EXPECT_TRUE(URI::PercentDecode("%D0%9F%D1%80%D0%B8%D0%B2%D0%B5%D1%82%20%D0%BC%D0%B8%D1%80") == "Привет мир");
    EXPECT_TRUE(URI::PercentDecode("Hello%20%D0%BC%D0%B8%D1%80") == "Hello мир");
    EXPECT_TRUE(URI::PercentDecode("Helloworld") == "Helloworld");
    EXPECT_TRUE(URI::PercentDecode("%E0%A4%A8%E0%A4%AE%E0%A4%B8%E0%A5%8D%E0%A4%A4%E0%A5%87%20%E0%A4%A6%E0%A5%81%E0%A4%A8%E0%A4%BF%E0%A4%AF%E0%A4%BE") == "नमस्ते दुनिया");
    EXPECT_TRUE(URI::PercentDecode("%D0%9F%D1%80%D0%B8%D0%B2%D1%96%D1%82%20%D1%81%D0%B2%D1%96%D1%82") == "Привіт світ");
    EXPECT_TRUE(URI::PercentDecode("Hello%20world%3D%D0%9F%D1%80%D0%B8%D0%B2%D0%B5%D1%82%20%D0%BC%D0%B8%D1%80") == "Hello world=Привет мир");

}

TEST(Uri_Tests, authority_decoding) {
    using namespace Http;

    {
        URI::Authority  auth;
        Error err(0, "No error");
        auth = URI::DecodeAuthority("userinfo:passwrd@www.example.com:1030", &err);

        EXPECT_TRUE(auth.__username == "userinfo");
        EXPECT_TRUE(auth.__password == "passwrd");
        EXPECT_TRUE(auth.__hostname == "www.example.com");
        EXPECT_TRUE(auth.__port == "1030");
        EXPECT_TRUE(err.IsOk());
    }

    {
        URI::Authority auth;
        Error err(0, "No error");
        auth = URI::DecodeAuthority("255.255.255.255:8080", &err);

        EXPECT_TRUE(auth.__username == "");
        EXPECT_TRUE(auth.__password == "");
        EXPECT_TRUE(auth.__hostname == "255.255.255.255");
        EXPECT_TRUE(auth.__port == "8080");
        EXPECT_TRUE(err.IsOk());
    }

    {
        URI::Authority auth;
        Error err(0, "No error");
        auth = URI::DecodeAuthority("www.example.com", &err);

        EXPECT_TRUE(auth.__username == "");
        EXPECT_TRUE(auth.__password == "");
        EXPECT_TRUE(auth.__hostname == "www.example.com");
        EXPECT_TRUE(auth.__port == "");
        EXPECT_TRUE(err.IsOk());
    }

    {
        URI::Authority auth;
        Error err(0, "No error");
        auth = URI::DecodeAuthority("user@www.example.com", &err);

        EXPECT_TRUE(auth.__username == "user");
        EXPECT_TRUE(auth.__password == "");
        EXPECT_TRUE(auth.__hostname == "www.example.com");
        EXPECT_TRUE(auth.__port == "");
        EXPECT_TRUE(err.IsOk());
    }
}

TEST(Uri_Tests, authority_encoding) {
    using namespace Http;

    {
        URI::Authority auth;
        auth.__username = "user";
        auth.__password = "password";
        auth.__hostname = "www.example.com";
        auth.__port = "8080";

        EXPECT_TRUE(URI::EncodeAuthority(auth) == "user:password@www.example.com:8080");
    }

    {
        URI::Authority auth;
        auth.__username = "";
        auth.__password = "";
        auth.__hostname = "www.example.com";
        auth.__port = "8080";

        EXPECT_TRUE(URI::EncodeAuthority(auth) == "www.example.com:8080");
    }

    {
        URI::Authority auth;
        auth.__username = "";
        auth.__password = "";
        auth.__hostname = "255.255.255.255";
        auth.__port = "8080";

        EXPECT_TRUE(URI::EncodeAuthority(auth) == "255.255.255.255:8080");
    }

    {
        URI::Authority auth;
        auth.__username = "DeltaWelta";
        auth.__password = "";
        auth.__hostname = "255.255.255.255";
        auth.__port = "8080";

        EXPECT_TRUE(URI::EncodeAuthority(auth) == "DeltaWelta@255.255.255.255:8080");
    }
}

TEST(Uri_Tests, query_decoding) {
    using namespace Http;

    {
        Error err(0, "No error");
        URI::QueryMap map;
        map = URI::DecodeQuery("f1=v1&f2=v2&f3=v3", &err);

        EXPECT_TRUE(map["f1"] == "v1");
        EXPECT_TRUE(map["f2"] == "v2");
        EXPECT_TRUE(map["f3"] == "v3");
        EXPECT_TRUE(map.size() == 3);
        EXPECT_TRUE(err.IsOk());
    }

    {
        Error err(0, "No error");
        URI::QueryMap map;
        map = URI::DecodeQuery("crcat=test&crsource=test&crkw=buy-a-lot", &err);

        EXPECT_TRUE(map["crcat"] == "test");
        EXPECT_TRUE(map["crsource"] == "test");
        EXPECT_TRUE(map["crkw"] == "buy-a-lot");
        EXPECT_TRUE(map.size() == 3);
        EXPECT_TRUE(err.IsOk());
    }

    {
        Error err(0, "No error");
        URI::QueryMap map;
        map = URI::DecodeQuery("req=" + URI::PercentEncode("Привет мир - UTF8"), &err);

        EXPECT_TRUE(map["req"] == "Привет мир - UTF8");
        EXPECT_TRUE(err.IsOk());
        EXPECT_TRUE(map.size() == 1);
    }

    {
        Error err(0, "No error");
        URI::QueryMap map;
        map = URI::DecodeQuery("", &err);

        EXPECT_TRUE(map.size() == 0);
        EXPECT_TRUE(err.IsOk());
    }
}

TEST(Uri_Tests, query_encoding) {
    using namespace Http;

    {
        URI::QueryMap map;
        map["key1"] = "val1";
        map["key2"] = "val2";
        map["key3"] = "val3";

        EXPECT_TRUE(URI::EncodeQuery(map) == "key1=val1&key2=val2&key3=val3");
        EXPECT_TRUE(map.size() == 3);
    }

        {
        URI::QueryMap map;
        map["a"] = "1";
        map["b"] = "2";
        map["c"] = "3";

        EXPECT_TRUE(URI::EncodeQuery(map) == "a=1&b=2&c=3");
    }

}

// full-uri
// short-uri
// check error
// check error

TEST(Uri_Tests, uri_decoding) {
    using namespace Http;

    {
        Error err(0, "No error");
        URI uri;
        uri = URI::DecodeUri("http://www.example.com/mypage.html?a=test&b=test&c=buy-a-lot#nose", &err);

        EXPECT_TRUE(uri.__scheme == URI::URI_SCHEME_HTTP);
        EXPECT_TRUE(uri.__auth.__username == "");
        EXPECT_TRUE(uri.__auth.__password == "");
        EXPECT_TRUE(uri.__auth.__hostname == "www.example.com");
        EXPECT_TRUE(uri.__auth.__port == "");
        EXPECT_TRUE(uri.__path == "/mypage.html");
        EXPECT_TRUE(uri.__query_params["a"] == "test");
        EXPECT_TRUE(uri.__query_params["b"] == "test");
        EXPECT_TRUE(uri.__query_params["c"] == "buy-a-lot");
        EXPECT_TRUE(uri.__query_params.size() == 3);
        EXPECT_TRUE(uri.__fragment == "nose");
        EXPECT_TRUE(err.IsOk());

    }

    {
        Error err(0, "No error");
        URI uri;
        uri = URI::DecodeUri("http://user@www.example.com:80/mypage.html#nose", &err);
        EXPECT_TRUE(uri.__scheme == URI::URI_SCHEME_HTTP);
        EXPECT_TRUE(uri.__auth.__username == "user");
        EXPECT_TRUE(uri.__auth.__password == "");
        EXPECT_TRUE(uri.__auth.__hostname == "www.example.com");
        EXPECT_TRUE(uri.__auth.__port == "80");
        EXPECT_TRUE(uri.__path == "/mypage.html");
        EXPECT_TRUE(uri.__query_params.size() == 0);
        EXPECT_TRUE(uri.__fragment == "nose");
        EXPECT_TRUE(err.IsOk());

    }

    {
        Error err(0, "No error");
        URI uri;
        uri = URI::DecodeUri("http://user:password@www.example.com:80/mypage.html#nose", &err);

        EXPECT_TRUE(uri.__scheme == URI::URI_SCHEME_HTTP);
        EXPECT_TRUE(uri.__auth.__username == "user");
        EXPECT_TRUE(uri.__auth.__password == "password");
        EXPECT_TRUE(uri.__auth.__hostname == "www.example.com");
        EXPECT_TRUE(uri.__auth.__port == "80");
        EXPECT_TRUE(uri.__path == "/mypage.html");
        EXPECT_TRUE(uri.__query_params.size() == 0);
        EXPECT_TRUE(uri.__fragment == "nose");
        EXPECT_TRUE(err.IsOk());

    }

    {
        Error err(0, "No error");
        URI uri;
        uri = URI::DecodeUri("mypage.html#nose", &err);

        EXPECT_TRUE(uri.__scheme == URI::URI_SCHEME_URL);
        EXPECT_TRUE(uri.__auth.__username == "");
        EXPECT_TRUE(uri.__auth.__password == "");
        EXPECT_TRUE(uri.__auth.__hostname == "");
        EXPECT_TRUE(uri.__auth.__port == "");
        EXPECT_TRUE(uri.__path == "mypage.html");
        EXPECT_TRUE(uri.__query_params.size() == 0);
        EXPECT_TRUE(uri.__fragment == "nose");
        EXPECT_TRUE(err.IsOk());

    }

    {
        Error err(0, "No error");
        URI uri;
        uri = URI::DecodeUri("/mypage.html?hello=56&moor=76", &err);

        EXPECT_TRUE(uri.__scheme == URI::URI_SCHEME_URL);
        EXPECT_TRUE(uri.__auth.__username == "");
        EXPECT_TRUE(uri.__auth.__password == "");
        EXPECT_TRUE(uri.__auth.__hostname == "");
        EXPECT_TRUE(uri.__auth.__port == "");
        EXPECT_TRUE(uri.__path == "/mypage.html");
        EXPECT_TRUE(uri.__query_params["hello"] == "56");
        EXPECT_TRUE(uri.__query_params["moor"] == "76");
        EXPECT_TRUE(uri.__query_params.size() == 2);
        EXPECT_TRUE(uri.__fragment == "");
        EXPECT_TRUE(err.IsOk());

    }

    {
        Error err(0, "No error");
        URI uri = URI::DecodeUri("htTP://User@WWW.ExAmple.cOM/Mypage.html#Nose", &err);

        EXPECT_TRUE(uri.__scheme == URI::URI_SCHEME_HTTP);
        EXPECT_TRUE(uri.__auth.__username == "User");
        EXPECT_TRUE(uri.__auth.__password == "");
        EXPECT_TRUE(uri.__auth.__hostname == "www.example.com");
        EXPECT_TRUE(uri.__auth.__port == "");
        EXPECT_TRUE(uri.__path == "/Mypage.html");
        EXPECT_TRUE(uri.__query_params.size() == 0);
        EXPECT_TRUE(uri.__fragment == "Nose");
        EXPECT_TRUE(err.IsOk());

    }

    {
        Error err(0, "No error");
        URI uri = URI::DecodeUri("", &err);
        EXPECT_TRUE(uri.__scheme == URI::URI_SCHEME_URL);
        EXPECT_TRUE(uri.__auth.__username == "");
        EXPECT_TRUE(uri.__auth.__password == "");
        EXPECT_TRUE(uri.__auth.__hostname == "");
        EXPECT_TRUE(uri.__auth.__port == "");
        EXPECT_TRUE(uri.__query_params.size() == 0);
        EXPECT_TRUE(uri.__path  == "");
        EXPECT_TRUE(uri.__fragment == "");
        EXPECT_TRUE(err.IsOk());
    }

    {
        Error err(0, "No error");
        URI uri = URI::DecodeUri("ftp://unknown.to.us.scheme.com", &err);
        EXPECT_TRUE(err.IsError());
    }

    {
        Error err(0, "No error");
        URI uri = URI::DecodeUri("://unknown.to.us.scheme.com", &err);
        EXPECT_TRUE(err.IsError());
    }

}

TEST(Uri_Tests, uri_encoding) {

    using namespace Http;
    {
        URI uri;
        uri.__scheme = URI::URI_SCHEME_HTTP;
        uri.__auth.__username = "user";
        uri.__auth.__password = "password";
        uri.__auth.__hostname = "www.domain.com";
        uri.__auth.__port = "1010";
        uri.__path = "/path/to/uri.cc";
        uri.__query_params["a"] = "1";
        uri.__query_params["b"] = "2";
        uri.__fragment = "pig";

        EXPECT_TRUE(URI::EncodeUri(uri) == "http://user:password@www.domain.com:1010/path/to/uri.cc?a=1&b=2#pig");
    }

    {
        URI uri;
        uri.__scheme = URI::URI_SCHEME_URL;
        uri.__path = "relative/path";
        uri.__query_params["a"] = "1";
        uri.__query_params["b"] = "2";
        uri.__fragment = "pig";

        EXPECT_TRUE(URI::EncodeUri(uri) == "relative/path?a=1&b=2#pig");
    }

    {
        URI uri;
        uri.__scheme = URI::URI_SCHEME_URL;
        uri.__path = "www.domain.com";

        EXPECT_TRUE(URI::EncodeUri(uri) == "www.domain.com");
    }
}
