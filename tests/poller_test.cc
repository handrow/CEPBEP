#include "gtest/gtest.h"

#include "common/tests.h"
#include "netlib/io/poller.h"

namespace {

static const fd_t    stdout_fd = 1;

TEST(Netlib_IO_Poller, Stdout_Sample) {
    using namespace IO;

    Poller poller;
    poller.AddFd(1, Poller::POLL_WRITE);

    EXPECT_EQ(poller.GetEvMask(stdout_fd), Poller::POLL_WRITE);

    Error err(0);
    Poller::Result pres;

    pres = poller.Poll(&err);
    EXPECT_TRUE(err.IsOk());
    EXPECT_EQ(pres.fd, stdout_fd);
    EXPECT_EQ(pres.ev, Poller::POLL_WRITE);

    poller.RmEvMask(stdout_fd, Poller::POLL_WRITE);
    pres = poller.Poll(&err);
    EXPECT_TRUE(err.IsOk());
    EXPECT_NE(pres.fd, stdout_fd);
    EXPECT_EQ(pres.ev, Poller::POLL_NONE);

    poller.RmFd(stdout_fd);
    EXPECT_EQ(poller.__pfds.size(), 0x0ull);
}

}  // namespace
