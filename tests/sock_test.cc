#include "netlib/socket.h"
#include "gtest/gtest.h"

TEST(IpAddrV4_TEST, HEX_IN_TEST) {
	NetLib::IpAddrV4 ip(INADDR_LOOPBACK);
    EXPECT_TRUE("127.0.0.1" == std::string(ip));
	EXPECT_TRUE(INADDR_LOOPBACK == ip);
}

TEST(IpAddrV4_TEST, STR_IN_TEST) {
	NetLib::IpAddrV4 ip("127.0.0.1");
    EXPECT_TRUE("127.0.0.1" == std::string(ip));
	EXPECT_TRUE(INADDR_LOOPBACK == ip);
}

TEST(Port_TEST, SHORT_IN_TEST) {
	NetLib::Port ip(5555);
    EXPECT_TRUE(5555 == ip);
	EXPECT_TRUE("5555"== std::string(ip));
}

TEST(Port_TEST, STR_IN_TEST) {
	NetLib::Port ip("5555");
    EXPECT_TRUE(5555 == ip);
	EXPECT_TRUE("5555"== std::string(ip));
}

TEST(SockInfo_TEST, MULTI_TEST) {
	NetLib::IpAddrV4 ip(INADDR_LOOPBACK);
	NetLib::Port p(5555);
	NetLib::SockInfo si(ip, p);
	struct sockaddr_in a = si;
	EXPECT_TRUE(5555 == ntohs(si.port_BE.__val.raw));
	EXPECT_TRUE(5555 == ntohs(a.sin_port));
	EXPECT_TRUE(INADDR_LOOPBACK == ntohl(a.sin_addr.s_addr));
}
