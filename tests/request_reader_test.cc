#include "gtest/gtest.h"
#include "http/reader.h"

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
