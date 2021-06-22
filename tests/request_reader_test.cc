#include "common/tests.h"
#include "gtest/gtest.h"
#include "http/reader.h"

namespace {

TEST(Http_Request_Reader, Default_Constructor) {
    using namespace Http;

    RequestReader req_rdr;

    EXPECT_EQ(req_rdr.__i, 0ull);
    EXPECT_EQ(req_rdr.__state, RequestReader::STT_SKIP_EMPTY_LINES);
    EXPECT_TRUE(req_rdr.__err.IsOk());
    EXPECT_TRUE(req_rdr.__buffer.empty());

}

TEST(Http_Request_Reader, Read_Reset) {
    using namespace Http;

    RequestReader req_rdr;

    req_rdr.Read("GET / HTTP/1.1\n");
    req_rdr.Read("\n");
    req_rdr.Read("GET / HTTP/1.1\n");
    req_rdr.Read("\n");

    EXPECT_STREQ(req_rdr.__buffer.c_str(), "GET / HTTP/1.1\n\nGET / HTTP/1.1\n\n" );
    EXPECT_EQ(req_rdr.__i, 0ull);
    EXPECT_EQ(req_rdr.__state, RequestReader::STT_SKIP_EMPTY_LINES);

    req_rdr.Process();
    req_rdr.Reset();

    EXPECT_STREQ(req_rdr.__buffer.c_str(), "" );
    EXPECT_EQ(req_rdr.__i, 0ull);
    EXPECT_EQ(req_rdr.__state, RequestReader::STT_SKIP_EMPTY_LINES);
}

TEST(Http_Request_Reader, One_Byte_Input_Processing) {
    using namespace Http;

    Request       req;
    RequestReader req_rdr;
    std::string   http_req_str =
        "POST   /example  HTTP/1.0\r\n"
        "h1: V1\n"
        "h2: v2\r\n"
        "h3: v3\n"
        "content-length: 5\n"
        "\r\n"
        "ABCDEFGH";

    for (usize i = 0; i < http_req_str.size(); ++i) {
        req_rdr.Read( http_req_str.substr(i, 1) );
        req_rdr.Process();
        EXPECT_FALSE(req_rdr.HasError());
        if (req_rdr.HasMessage())
            req = req_rdr.GetMessage();
    }

    EXPECT_EQ( req.method, METHOD_POST );
    EXPECT_EQ( req.version, HTTP_1_0 );
    EXPECT_STREQ( req.uri.path.c_str(), "/example" );
    EXPECT_STREQ( req.headers.__map["H1"].c_str(), "V1" );
    EXPECT_STREQ( req.headers.__map["h2"].c_str(), "v2" );
    EXPECT_STREQ( req.headers.__map["H3"].c_str(), "v3" );
    EXPECT_STREQ( req.headers.__map["Content-Length"].c_str(), "5" );
    EXPECT_EQ( Headers::GetContentLength(req.headers), 5ull );
    EXPECT_STREQ( req.body.c_str(), "ABCDE" );

    EXPECT_STREQ( req_rdr.__buffer.c_str(), "FGH" );
}

TEST(Http_Request_Reader, Full_Input_Processing) {
    using namespace Http;

    Request       req;
    RequestReader req_rdr;
    std::string   http_req_str =
        "POST   /example  HTTP/1.0\r\n"
        "h1: V1\n"
        "h2: v2\r\n"
        "h3: v3\n"
        "content-length: 7\n"
        "\r\n"
        "ABCDEFGH";

    req_rdr.Read(http_req_str);
    req_rdr.Process();

    EXPECT_TRUE( req_rdr.HasMessage() );
    EXPECT_FALSE( req_rdr.HasError() );

    req = req_rdr.GetMessage();

    req_rdr.Process();
    EXPECT_FALSE( req_rdr.HasMessage() );
    EXPECT_FALSE( req_rdr.HasError() );

    EXPECT_EQ( req.method, METHOD_POST );
    EXPECT_EQ( req.version, HTTP_1_0 );
    EXPECT_STREQ( req.uri.path.c_str(), "/example" );
    EXPECT_STREQ( req.headers.__map["H1"].c_str(), "V1" );
    EXPECT_STREQ( req.headers.__map["h2"].c_str(), "v2" );
    EXPECT_STREQ( req.headers.__map["H3"].c_str(), "v3" );
    EXPECT_STREQ( req.headers.__map["Content-Length"].c_str(), "7" );
    EXPECT_EQ( Headers::GetContentLength(req.headers), 7ull );
    EXPECT_STREQ( req.body.c_str(), "ABCDEFG" );

    EXPECT_STREQ( req_rdr.__buffer.c_str(), "H" );
}

TEST(Http_Request_Reader, Few_Messages_Input_Processing) {
    using namespace Http;

    RequestReader req_rdr;
    std::string   http_req_str =
        "\n"
        "\r\n"
        "POST   /r1  HTTP/1.0\r\n"  // REQUEST #1
        "content-length: 10\n"      //
        "\r\n"                      //
        "1234567890"                //
        "GET /r2  HTTP/1.1\n"       // REQUEST #2
        "\r\n\n"                    //
        "FAILED_MESSAGE\n"          // REQUEST #3 (err)
        "\n"                        //
        "DeleTe /r4 HTTP/1.0\n"     // REQUEST #4
        "Host: buba.net\r\n"        //
        "\n\n\n\n"                  //
        "REMAINDER";                // Remainder must stay at buffer

    req_rdr.Read(http_req_str);

    /// REQ 1
    {
        Request req;

        req_rdr.Process();
        EXPECT_TRUE( req_rdr.HasMessage() );
        EXPECT_FALSE( req_rdr.HasError() );
        req = req_rdr.GetMessage();

        EXPECT_EQ( req.method, METHOD_POST );
        EXPECT_EQ( req.version, HTTP_1_0 );
        EXPECT_STREQ( req.uri.path.c_str(), "/r1" );

        EXPECT_STREQ( req.headers.__map["Content-Length"].c_str(), "10" );
        EXPECT_EQ( Headers::GetContentLength(req.headers), 10ull );
        EXPECT_STREQ( req.body.c_str(), "1234567890" );
    }

    /// REQ 2
    {
        Request req;

        req_rdr.Process();
        EXPECT_TRUE( req_rdr.HasMessage() );
        EXPECT_FALSE( req_rdr.HasError() );
        req = req_rdr.GetMessage();

        EXPECT_EQ( req.method, METHOD_GET );
        EXPECT_EQ( req.version, HTTP_1_1 );
        EXPECT_STREQ( req.uri.path.c_str(), "/r2" );

        EXPECT_EQ( req.headers.__map.size(), 0ull );
        EXPECT_EQ( Headers::GetContentLength(req.headers), 0ull );
        EXPECT_STREQ( req.body.c_str(), "" );
    }

    /// REQ 3 (error)
    {
        Error err;

        req_rdr.Process();
        EXPECT_FALSE( req_rdr.HasMessage() );
        EXPECT_TRUE( req_rdr.HasError() );
        err = req_rdr.GetError();

        EXPECT_FALSE( err.IsOk() );
        EXPECT_EQ( err.errcode, HTTP_READER_BAD_START_LINE );
    }

    /// REQ 4
    {
        Error   err;
        Request req;

        req_rdr.Process();
        EXPECT_TRUE( req_rdr.HasMessage() );
        EXPECT_FALSE( req_rdr.HasError() );
        req = req_rdr.GetMessage();

        EXPECT_EQ( req.method, METHOD_DELETE );
        EXPECT_EQ( req.version, HTTP_1_0 );
        EXPECT_STREQ( req.uri.path.c_str(), "/r4" );

        EXPECT_EQ( req.headers.__map.size(), 1ull );
        EXPECT_STREQ( req.headers.__map["host"].c_str(), "buba.net" );
        EXPECT_EQ( Headers::GetContentLength(req.headers), 0ull );
        EXPECT_STREQ( req.body.c_str(), "" );

        err = req_rdr.GetError();

        EXPECT_TRUE( err.IsOk() );
        EXPECT_EQ( err.errcode, Error::ERR_OK );
    }

    /// REMAINDER CHECK
    {
        req_rdr.Process();
        EXPECT_FALSE( req_rdr.HasMessage() );
        EXPECT_FALSE( req_rdr.HasError() );
        EXPECT_STREQ( req_rdr.__buffer.c_str(), "REMAINDER" );
        EXPECT_EQ( req_rdr.__i, 9ull );
    }
}

TEST(Http_Request_Reader, Header_Override) {
    using namespace Http;

    RequestReader req_rdr;

    req_rdr.Read("get / http/1.1\r\n");
    req_rdr.Read("HeaDer: I'am cool\r\n");
    req_rdr.Read("hEaDer: Bee Movie  \r\n");
    req_rdr.Read("header: What is Happening  \r\n");
    req_rdr.Read("HEADER: No case matters  \r\n");
    req_rdr.Read("\n");
    req_rdr.Process();

    EXPECT_FALSE(req_rdr.HasError());
    EXPECT_TRUE(req_rdr.HasMessage());

    Headers hdrs(req_rdr.GetMessage().headers);

    EXPECT_STREQ(hdrs.__map["header"].c_str(), "No case matters");
    EXPECT_STREQ(hdrs.__map["Header"].c_str(), "No case matters");
    EXPECT_STREQ(hdrs.__map["hEader"].c_str(), "No case matters");
    EXPECT_STREQ(hdrs.__map["heAder"].c_str(), "No case matters");
    EXPECT_STREQ(hdrs.__map["heaDer"].c_str(), "No case matters");
    EXPECT_STREQ(hdrs.__map["headEr"].c_str(), "No case matters");
    EXPECT_STREQ(hdrs.__map["headeR"].c_str(), "No case matters");
    EXPECT_STREQ(hdrs.__map["HEADER"].c_str(), "No case matters");

    EXPECT_EQ(hdrs.__map.size(), 1ull);

}

}  // namespace
