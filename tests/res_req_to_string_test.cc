#include "common/tests.h"
#include "gtest/gtest.h"
#include "http/reader.h"

TEST(Request_to_string_test, req_multiple_headers_test) {
    using namespace Http;
    {
        Headers hdrs;
        hdrs.__map["Alpha"] = "Dima";
        hdrs.__map["Bravo"] = "Sonya";

        Request req = {
            .version = HTTP_1_1,
            .uri = {
                .hostname = "www.example.com",
                .path = "/wow",
            },
            .method = METHOD_GET,
            .headers = hdrs,
            .body = "ABSADBSADB"
        };

        EXPECT_EQ(req.ToString(), "GET http://www.example.com/wow HTTP/1.1\r\n"
                                  "Alpha: Dima\r\n"
                                  "Bravo: Sonya\r\n"
                                  "\r\n");
    }

    {
        Headers hdrs;
        hdrs.__map["content-length"] = "8";
        hdrs.__map["user"] = "do";
        hdrs.__map["user"] = "did";

        Request req = {
            .version = HTTP_1_0,
            .uri = {
                .path = "/path",
            },
            .method = METHOD_POST,
            .headers = hdrs,
            .body = "hellodog"
        };

        EXPECT_EQ(req.ToString(), "POST /path HTTP/1.0\r\n"
                                  "content-length: 8\r\n"
                                  "user: did\r\n"
                                  "\r\n"
                                  "hellodog");
    }
}


TEST(Response_to_string_test, res_multiple_headers_test) {
    using namespace Http;
    {
        Headers hdrs;
        hdrs.__map["server"] = "do";
        hdrs.__map["server"] = "did";

        Response res = {
            .version = HTTP_1_1,
            .code = 301,
            .code_message = "Moved Permanently",
            .headers = hdrs,
            .body = "hellodog"
        };

        EXPECT_EQ(res.ToString(), "HTTP/1.1 301 Moved Permanently\r\n"
                                  "Content-Length: 8\r\n"
                                  "server: did\r\n"
                                  "\r\n"
                                  "hellodog");
    }
}

TEST(Response_to_string_test, res_smaller_cl_test) {
    using namespace Http;
    {
        Headers hdrs;
        hdrs.__map["content-length"] = "8";
        hdrs.__map["server"] = "do";

        Response res = {
            .version = HTTP_1_1,
            .code = 301,
            .code_message = "Moved Permanently",
            .headers = hdrs,
            .body = "hellodog"
        };

        EXPECT_EQ(res.ToString(), "HTTP/1.1 301 Moved Permanently\r\n"
                                  "content-length: 8\r\n"
                                  "server: do\r\n"
                                  "\r\n"
                                  "hellodog");
    }
}


