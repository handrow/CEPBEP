#include "gtest/gtest.h"
#include "http/uri.h"

TEST(Uri_Tests, percent_encoding) {
    using namespace Http;

    EXPECT_TRUE(PercentEncode(":/?&Hello =", IsUnrsvdSym) == "%3A%2F%3F%26Hello%20%3D");
    EXPECT_TRUE(PercentEncode("Привет мир", IsUnrsvdSym) == "%D0%9F%D1%80%D0%B8%D0%B2%D0%B5%D1%82%20%D0%BC%D0%B8%D1%80");
    EXPECT_TRUE(PercentEncode("Hello мир", IsUnrsvdSym) == "Hello%20%D0%BC%D0%B8%D1%80");
    EXPECT_TRUE(PercentEncode("Helloworld", IsUnrsvdSym) == "Helloworld");
    EXPECT_TRUE(PercentEncode("नमस्ते दुनिया", IsUnrsvdSym) == "%E0%A4%A8%E0%A4%AE%E0%A4%B8%E0%A5%8D%E0%A4%A4%E0%A5%87%20%E0%A4%A6%E0%A5%81%E0%A4%A8%E0%A4%BF%E0%A4%AF%E0%A4%BE");
    EXPECT_TRUE(PercentEncode("Привіт світ", IsUnrsvdSym) == "%D0%9F%D1%80%D0%B8%D0%B2%D1%96%D1%82%20%D1%81%D0%B2%D1%96%D1%82");
    EXPECT_TRUE(PercentEncode("Hello world=Привет мир", IsUnrsvdSym) == "Hello%20world%3D%D0%9F%D1%80%D0%B8%D0%B2%D0%B5%D1%82%20%D0%BC%D0%B8%D1%80");

}

TEST(Uri_Tests, percent_decoding) {
    using namespace Http;

    EXPECT_TRUE(PercentDecode("%3A%2F%3F%26Hello%20%3D") == ":/?&Hello =");
    EXPECT_TRUE(PercentDecode("%D0%9F%D1%80%D0%B8%D0%B2%D0%B5%D1%82%20%D0%BC%D0%B8%D1%80") == "Привет мир");
    EXPECT_TRUE(PercentDecode("Hello%20%D0%BC%D0%B8%D1%80") == "Hello мир");
    EXPECT_TRUE(PercentDecode("Helloworld") == "Helloworld");
    EXPECT_TRUE(PercentDecode("%E0%A4%A8%E0%A4%AE%E0%A4%B8%E0%A5%8D%E0%A4%A4%E0%A5%87%20%E0%A4%A6%E0%A5%81%E0%A4%A8%E0%A4%BF%E0%A4%AF%E0%A4%BE") == "नमस्ते दुनिया");
    EXPECT_TRUE(PercentDecode("%D0%9F%D1%80%D0%B8%D0%B2%D1%96%D1%82%20%D1%81%D0%B2%D1%96%D1%82") == "Привіт світ");
    EXPECT_TRUE(PercentDecode("Hello%20world%3D%D0%9F%D1%80%D0%B8%D0%B2%D0%B5%D1%82%20%D0%BC%D0%B8%D1%80") == "Hello world=Привет мир");

}

TEST(Uri_Tests, uri_to_string) {
    using namespace Http;
     {
        URI uri;
        uri.userinfo = "user:password";
        uri.hostname = "www.domain.com:1010";
        uri.path = "/path/to/uri.cc";
        uri.query_str = "a=1&b=2";
        uri.fragment = "pig";

        EXPECT_TRUE(uri.ToString() == "http://user:password@www.domain.com:1010/path/to/uri.cc?a=1&b=2#pig");
    }

    {
        URI uri;
        uri.path = "/relative/path";
        uri.query_str = "a=1&b=2";
        uri.fragment = "pig";

        EXPECT_TRUE(uri.ToString() == "/relative/path?a=1&b=2#pig");
    }

    {
        URI uri;
        uri.hostname = "www.domain.com";

        EXPECT_TRUE(uri.ToString() == "http://www.domain.com");
    }
}

TEST(Uri_Tests, uri_parse) {
    using namespace Http;
    {
        Error err(0, "No error");
        URI uri;
        uri = URI::Parse("http://www.example.com/mypage.html?a=test&b=test&c=buy-a-lot#nose", &err);

        EXPECT_TRUE(uri.userinfo == "");
        EXPECT_TRUE(uri.hostname == "www.example.com");
        EXPECT_TRUE(uri.path == "/mypage.html");
        EXPECT_TRUE(uri.query_str == "a=test&b=test&c=buy-a-lot");
        EXPECT_TRUE(uri.fragment == "nose");
        EXPECT_TRUE(err.IsOk());

    }

    {
        Error err(0, "No error");
        URI uri;
        uri = URI::Parse("http://user@www.example.com:80/mypage.html#nose", &err);
        EXPECT_TRUE(uri.userinfo == "user");
        EXPECT_TRUE(uri.hostname == "www.example.com:80");
        EXPECT_TRUE(uri.path == "/mypage.html");
        EXPECT_TRUE(uri.query_str.size() == 0);
        EXPECT_TRUE(uri.fragment == "nose");
        EXPECT_TRUE(err.IsOk());

    }

    {
        Error err(0, "No error");
        URI uri = URI::Parse("http://user:password@www.example.com:80/mypage.html#nose", &err);

        EXPECT_TRUE(uri.userinfo == "user:password");
        EXPECT_TRUE(uri.hostname == "www.example.com:80");
        EXPECT_TRUE(uri.path == "/mypage.html");
        EXPECT_TRUE(uri.query_str.size() == 0);
        EXPECT_TRUE(uri.fragment == "nose");
        EXPECT_TRUE(err.IsOk());

    }

    {
        Error err(0, "No error");
        URI uri = URI::Parse("/mypage.html#nose", &err);

        EXPECT_TRUE(uri.userinfo == "");
        EXPECT_TRUE(uri.hostname == "");
        EXPECT_TRUE(uri.path == "/mypage.html");
        EXPECT_TRUE(uri.query_str.size() == 0);
        EXPECT_TRUE(uri.fragment == "nose");
        EXPECT_TRUE(err.IsOk());
    }

    {
        Error err(0, "No error");
        URI uri = URI::Parse("/mypage.html?hello=56&moor=76", &err);

        EXPECT_TRUE(uri.userinfo == "");
        EXPECT_TRUE(uri.hostname == "");
        EXPECT_TRUE(uri.path == "/mypage.html");
        EXPECT_TRUE(uri.query_str == "hello=56&moor=76");
        EXPECT_TRUE(uri.fragment == "");
        EXPECT_TRUE(err.IsOk());
    }

    {
        Error err(0, "No error");
        URI uri = URI::Parse("htTP://User@WWW.ExAmple.cOM/Mypage.html#Nose", &err);

        EXPECT_TRUE(uri.userinfo == "User");
        EXPECT_TRUE(uri.hostname == "www.example.com");
        EXPECT_TRUE(uri.path == "/Mypage.html");
        EXPECT_TRUE(uri.query_str.size() == 0);
        EXPECT_TRUE(uri.fragment == "Nose");
        EXPECT_TRUE(err.IsOk());

    }

    {
        Error err(0, "No error");
        URI uri = URI::Parse("", &err);
        EXPECT_TRUE(uri.userinfo == "");
        EXPECT_TRUE(uri.hostname == "");
        EXPECT_TRUE(uri.query_str.size() == 0);
        EXPECT_TRUE(uri.path  == "/");
        EXPECT_TRUE(uri.fragment == "");
        EXPECT_TRUE(err.IsOk());
    }

    {
        Error err(0, "No error");
        URI uri = URI::Parse("ftp://unknown.to.us.scheme.com", &err);
        EXPECT_TRUE(err.IsError());
    }

    {
        Error err(0, "No error");
        URI uri = URI::Parse("://unknown.to.us.scheme.com", &err);
        EXPECT_TRUE(err.IsError());
    }

    {
        Error err(0, "No error");
        URI uri = URI::Parse("relative/path", &err);
        EXPECT_TRUE(err.IsError());
    }

    {
        Error err(0, "No error");
        URI uri = URI::Parse("http:/www.domain.com:80/path/to/jjj?a=b#hruh", &err);
        EXPECT_TRUE(err.IsError());
    }

    {
        Error err(0, "No error");
        URI uri = URI::Parse("//path/", &err);
        EXPECT_TRUE(err.IsError());
    }

    {
        Error err(0, "No error");
        URI uri = URI::Parse("/path/", &err);
        EXPECT_TRUE(err.IsOk());
    }

    {
        Error err(0, "No error");
        URI uri = URI::Parse("/", &err);
        EXPECT_TRUE(uri.path == "/");
        EXPECT_TRUE(err.IsOk());
    }

    {
        Error err(0, "No error");
        URI uri = URI::Parse("http://www.example.com?a=b", &err);
        EXPECT_TRUE(uri.path == "/");
        EXPECT_TRUE(err.IsOk());
    }
}

TEST(Query_Tests, query_to_string) {
    using namespace Http;

    {
        Query q;

        q.param_map["key1"] = "val1";
        q.param_map["key2"] = "val2";
        q.param_map["key3"] = "val3";

        EXPECT_TRUE(q.ToString() == "key1=val1&key2=val2&key3=val3");
    }

    {
        Query q;
        q.param_map["a"] = "1";
        q.param_map["b"] = "2";
        q.param_map["c"] = "3";

        EXPECT_TRUE(q.ToString() == "a=1&b=2&c=3");
    }
}

TEST(Query_Tests, query_parse) {
    using namespace Http;

    {
        Error err(0, "No error");
        Query q;
        q = Query::Parse("f1=v1&f2=v2&f3=v3", &err);

        EXPECT_TRUE(q.param_map["f1"] == "v1");
        EXPECT_TRUE(q.param_map["f2"] == "v2");
        EXPECT_TRUE(q.param_map["f3"] == "v3");
        EXPECT_TRUE(q.param_map.size() == 3);
        EXPECT_TRUE(err.IsOk());
    }

    {
        Error err(0, "No error");
        Query q;
        q = Query::Parse("crcat=test&crsource=test&crkw=buy-a-lot", &err);

        EXPECT_TRUE(q.param_map["crcat"] == "test");
        EXPECT_TRUE(q.param_map["crsource"] == "test");
        EXPECT_TRUE(q.param_map["crkw"] == "buy-a-lot");
        EXPECT_TRUE(q.param_map.size() == 3);
        EXPECT_TRUE(err.IsOk());
    }

    {
        Error err(0, "No error");
        Query q;
        q = Query::Parse("req=" + PercentEncode("Привет мир - UTF8", IsUnrsvdSym), &err);

        EXPECT_TRUE(q.param_map["req"] == "Привет мир - UTF8");
        EXPECT_TRUE(err.IsOk());
        EXPECT_TRUE(q.param_map.size() == 1);
    }

    {
        Error err(0, "No error");
        Query q;
        q = Query::Parse("", &err);

        EXPECT_TRUE(q.param_map.size() == 0);
        EXPECT_TRUE(err.IsOk());
    }
}

