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
            .method = METHOD_GET,
            .uri = {
                .hostname = "www.example.com",
                .path = "/wow",
            },
            .version = HTTP_1_1,
            .headers = hdrs,
            .body = "ABSADBSADB"
        };

        EXPECT_EQ(req.ToString(), "GET http://www.example.com/wow HTTP/1.1\n"
                                  "Alpha: Dima\n"
                                  "Bravo: Sonya\n"
                                  "\n");
    }

    {
        Headers hdrs;
        hdrs.__map["content-length"] = "8";
        hdrs.__map["user"] = "do";
        hdrs.__map["user"] = "did";

        Request req = {
            .method = METHOD_POST,
            .uri = {
                .path = "/path",
            },
            .version = HTTP_1_0,
            .headers = hdrs,
            .body = "hellodog"
        };

        EXPECT_EQ(req.ToString(), "POST /path HTTP/1.0\n"
                                  "content-length: 8\n"
                                  "user: did\n"
                                  "\n"
                                "hellodog");
    }
}


TEST(Response_to_string_test, res_multiple_headers_test) {
    using namespace Http;
    {
        Headers hdrs;
        hdrs.__map["content-length"] = "8";
        hdrs.__map["server"] = "do";
        hdrs.__map["server"] = "did";

        Response res = {
            .version = HTTP_1_1,
            .code = 301,
            .code_message = "Moved Permanently",
            .headers = hdrs,
            .body = "hellodog"
        };

        EXPECT_EQ(res.ToString(), "HTTP/1.1 301 Moved Permanently\n"
                                  "content-length: 8\n"
                                  "server: did\n"
                                  "\n"
                                "hellodog");
    }
}

TEST(Response_to_string_test, res_smaller_cl_test) {
    using namespace Http;
    {
        Headers hdrs;
        hdrs.__map["content-length"] = "7";
        hdrs.__map["server"] = "do";

        Response res = {
            .version = HTTP_1_1,
            .code = 301,
            .code_message = "Moved Permanently",
            .headers = hdrs,
            .body = "hellodog"
        };

        EXPECT_EQ(res.ToString(), "HTTP/1.1 301 Moved Permanently\n"
                                  "content-length: 7\n"
                                  "server: do\n"
                                  "\n"
                                "hellodo");
    }
}


