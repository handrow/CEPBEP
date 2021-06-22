#include "common/tests.h"
#include "gtest/gtest.h"
#include "http/reader.h"

namespace {

TEST(Http_Response_Reader, Default_Constructor) {
    using namespace Http;

    ResponseReader res_rdr;

    EXPECT_EQ(res_rdr.__i, 0ull);
    EXPECT_EQ(res_rdr.__state, ResponseReader::STT_SKIP_EMPTY_LINES);
    EXPECT_TRUE(res_rdr.__err.IsOk());
    EXPECT_TRUE(res_rdr.__buffer.empty());

}

TEST(Http_Request_Reader, Read_Reset) {
    using namespace Http;

    ResponseReader res_rdr;

    res_rdr.Read("HTTP/1.1 200 OK\n");
    res_rdr.Read("\n");
    res_rdr.Read("http/1.1 301 Moved Permanently\n");
    res_rdr.Read("\n");

    EXPECT_STREQ(res_rdr.__buffer.c_str(), "HTTP/1.1 200 OK\n\nhttp/1.1 301 Moved Permanently\n\n" );
    EXPECT_EQ(res_rdr.__i, 0ull);
    EXPECT_EQ(res_rdr.__state, ResponseReader::STT_SKIP_EMPTY_LINES);

    res_rdr.Process();
    res_rdr.Reset();

    EXPECT_STREQ(res_rdr.__buffer.c_str(), "" );
    EXPECT_EQ(res_rdr.__i, 0ull);
    EXPECT_EQ(res_rdr.__state, ResponseReader::STT_SKIP_EMPTY_LINES);
}

TEST(Http_Response_Reader, One_Byte_Input_Processing) {
    using namespace Http;

    Response        res;
    ResponseReader  res_rdr;
    std::string     http_res_str =
        "HTTP/1.1   200  OK\r\n"
        "h1: V1\n"
        "h2: v2\r\n"
        "h3: v3\n"
        "content-length: 5\n"
        "\r\n"
        "ABCDEFGH";

    for (usize i = 0; i < http_res_str.size(); ++i) {
        res_rdr.Read( http_res_str.substr(i, 1) );
        res_rdr.Process();
        EXPECT_FALSE(res_rdr.HasError());
        if (res_rdr.HasMessage())
            res = res_rdr.GetMessage();
    }

    EXPECT_EQ( res.version, HTTP_1_1 );
    EXPECT_EQ(res.code, 200);
    EXPECT_STREQ( res.code_message.c_str(), "OK" );
    EXPECT_STREQ( res.headers.__map["H1"].c_str(), "V1" );
    EXPECT_STREQ( res.headers.__map["h2"].c_str(), "v2" );
    EXPECT_STREQ( res.headers.__map["H3"].c_str(), "v3" );
    EXPECT_STREQ( res.headers.__map["Content-Length"].c_str(), "5" );
    EXPECT_EQ( Headers::GetContentLength(res.headers), 5ull );
    EXPECT_STREQ( res.body.c_str(), "ABCDE" );

    EXPECT_STREQ( res_rdr.__buffer.c_str(), "FGH" );
}

TEST(Http_Request_Reader, Full_Input_Processing) {
    using namespace Http;

    Response       res;
    ResponseReader res_rdr;
    std::string   http_res_str =
        "HTTP/1.1   301  Moved Permanently\r\n"
        "h1: V1\n"
        "h2: v2\r\n"
        "h3: v3\n"
        "content-length: 7\n"
        "\r\n"
        "ABCDEFGH";

    res_rdr.Read(http_res_str);
    res_rdr.Process();

    EXPECT_TRUE( res_rdr.HasMessage() );
    EXPECT_FALSE( res_rdr.HasError() );

    res = res_rdr.GetMessage();

    res_rdr.Process();
    EXPECT_FALSE( res_rdr.HasMessage() );
    EXPECT_FALSE( res_rdr.HasError() );

    EXPECT_EQ( res.version, HTTP_1_1 );
    EXPECT_EQ(res.code, 301);
    EXPECT_STREQ( res.code_message.c_str(), "Moved Permanently" );
    EXPECT_STREQ( res.headers.__map["H1"].c_str(), "V1" );
    EXPECT_STREQ( res.headers.__map["h2"].c_str(), "v2" );
    EXPECT_STREQ( res.headers.__map["H3"].c_str(), "v3" );
    EXPECT_STREQ( res.headers.__map["Content-Length"].c_str(), "7" );
    EXPECT_EQ( Headers::GetContentLength(res.headers), 7ull );
    EXPECT_STREQ( res.body.c_str(), "ABCDEFG" );

    EXPECT_STREQ( res_rdr.__buffer.c_str(), "H" );
}

TEST(Http_Request_Reader, Few_Messages_Input_Processing) {
    using namespace Http;

    ResponseReader res_rdr;
    std::string   http_res_str =
        "\n"
        "\r\n"
        "http/1.1  200  OK\r\n"     // RESPONSE #1
        "content-length: 10\n"      //
        "\r\n"                      //
        "1234567890"                //
        "HTTP/1.1 301 Moved Permanently  \n"       // RESPONSE #2
        "\r\n\n"                    //
        "FAILED_MESSAGE\n"          // RESPONSE #3 (err)
        "\n\n\n\n"                  //
        "REMAINDER";                // Remainder must stay at buffer

    res_rdr.Read(http_res_str);

    /// RES 1
    {
        Response res;

        res_rdr.Process();
        EXPECT_TRUE( res_rdr.HasMessage() );
        EXPECT_FALSE( res_rdr.HasError() );
        res = res_rdr.GetMessage();

        EXPECT_EQ( res.version, HTTP_1_1 );
        EXPECT_EQ(res.code, 200);
        EXPECT_STREQ( res.code_message.c_str(), "OK" );

        EXPECT_STREQ( res.headers.__map["Content-Length"].c_str(), "10" );
        EXPECT_EQ( Headers::GetContentLength(res.headers), 10ull );
        EXPECT_STREQ( res.body.c_str(), "1234567890" );
    }

    /// RES 2
    {
        Response res;

        res_rdr.Process();
        EXPECT_TRUE( res_rdr.HasMessage() );
        EXPECT_FALSE( res_rdr.HasError() );
        res = res_rdr.GetMessage();

        EXPECT_EQ( res.version, HTTP_1_1 );
        EXPECT_EQ(res.code, 301);
        EXPECT_STREQ( res.code_message.c_str(), "Moved Permanently" );

        EXPECT_EQ( res.headers.__map.size(), 0ull );
        EXPECT_EQ( Headers::GetContentLength(res.headers), 0ull );
        EXPECT_STREQ( res.body.c_str(), "" );
    }

    /// RES 3 (error)
    {
        Error err;

        res_rdr.Process();
        EXPECT_FALSE( res_rdr.HasMessage() );
        EXPECT_TRUE( res_rdr.HasError() );
        err = res_rdr.GetError();

        EXPECT_FALSE( err.IsOk() );
        EXPECT_EQ( err.errcode, HTTP_READER_BAD_START_LINE );
    }


    /// REMAINDER CHECK
    {
        res_rdr.Process();
        EXPECT_FALSE( res_rdr.HasMessage() );
        EXPECT_FALSE( res_rdr.HasError() );
        EXPECT_STREQ( res_rdr.__buffer.c_str(), "REMAINDER" );
        EXPECT_EQ( res_rdr.__i, 9ull );
    }
}

TEST(Http_Request_Reader, Header_Override) {
    using namespace Http;

    ResponseReader res_rdr;

    res_rdr.Read("Http/1.1 200 OK\r\n");
    res_rdr.Read("HeaDer: I'am cool\r\n");
    res_rdr.Read("hEaDer: Bee Movie  \r\n");
    res_rdr.Read("header: What is Happening  \r\n");
    res_rdr.Read("HEADER: No case matters  \r\n");
    res_rdr.Read("\n");
    res_rdr.Process();

    EXPECT_FALSE(res_rdr.HasError());
    EXPECT_TRUE(res_rdr.HasMessage());

    Headers hdrs(res_rdr.GetMessage().headers);

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
