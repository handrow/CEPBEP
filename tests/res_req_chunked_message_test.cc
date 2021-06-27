#include "common/tests.h"
#include "gtest/gtest.h"
#include "http/reader.h"

TEST(Chunked_Request_Test, req_common_test) {
    using namespace Http;
    {
        Headers hdrs;
        hdrs.__map["user-agent"] = "Mozilla/5.0";
        hdrs.__map["Content-Length"] = "10";
        hdrs.__map["Transfer-Encoding"] = "deflate, chunked, gzip";

        Request req = {
            .method = METHOD_POST,
            .uri = {
                .hostname = "www.example.com",
                .path = "/path",
            },
            .version = HTTP_1_1,
            .headers = hdrs,
            .body = "Helloworld"
        };

        EXPECT_EQ(req.ToString(), "POST http://www.example.com/path HTTP/1.1\r\n"
                                  "Content-Length: 10\r\n"
                                  "Transfer-Encoding: deflate, gzip\r\n"
                                  "user-agent: Mozilla/5.0\r\n"
                                  "\r\n"
                                  "Helloworld");
    }

    {
        Headers hdrs;
        hdrs.__map["user-agent"] = "Mozilla/5.0";
        hdrs.__map["Transfer-Encoding"] = "gzip, deflate, chunked";

        Request req = {
            .method = METHOD_POST,
            .uri = {
                .hostname = "www.example.com",
                .path = "/path",
            },
            .version = HTTP_1_1,
            .headers = hdrs,
            .body = "The Transfer-Encoding"
        };

        EXPECT_EQ(req.ToString(), "POST http://www.example.com/path HTTP/1.1\r\n"
                                  "Content-Length: 21\r\n"
                                  "Transfer-Encoding: gzip, deflate\r\n"
                                  "user-agent: Mozilla/5.0\r\n"
                                  "\r\n"
                                  "The Transfer-Encoding");
    }

}

