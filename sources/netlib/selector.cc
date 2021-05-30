#include "../common/time.h"
#include "selector.h"

namespace Ft {
namespace Netlib {

Selector::Selector() {
    FD_ZERO(&__rx_fdset);
    FD_ZERO(&__tx_fdset);
    __timeout.tv_sec = 0;
    __timeout.tv_usec = 0;
}

void Selector::AddRx(fd_t fd) {
    FD_SET(fd, &__rx_fdset);
}

void Selector::AddTx(fd_t fd) {
    FD_SET(fd, &__tx_fdset);
}

void Selector::SetTimeOut(u64 usec) {
    __timeout = ::usec_to_tv(usec);
}

void Selector::RmRx(fd_t fd) {
    FD_CLR(fd, &__rx_fdset);
}

void Selector::RmTx(fd_t fd) {
    FD_CLR(fd, &__tx_fdset);
}

Selector::SelectResult
Selector::Select() const {
    fd_set_t hot_tx_set = __tx_fdset;
    fd_set_t hot_rx_set = __rx_fdset;
    timeval_t tv = __timeout;  // select could mutate it, so we give him a copy

    timeval_t* tvptr = (tv.tv_sec == 0 && tv.tv_usec == 0) ? NULL
                                                           : &tv;

    int slc_ret_code = select(FD_SETSIZE, &hot_rx_set, &hot_tx_set, NULL, tvptr);

    SelectResult res(-1, SELECT_ERROR, NO_SET_READY);
    if (slc_ret_code == 0) {
        res.retcode = SELECT_TIMEOUT;
    }
    else if (slc_ret_code > 0) {
        res.retcode = SELECT_OK;
        for (int i = 0; i < FD_SETSIZE; ++i) {
            if (FD_ISSET(i, &__tx_fdset)) {
                res.fd = i;
                res.type = TX_SET_READY;
                break;
            }
            else if (FD_ISSET(i, &__rx_fdset)) {
                res.fd = i;
                res.type = RX_SET_READY;
                break;
            }
        }
    }

    return res;
}

#include <stdio.h>

void Selector::SelectResult::DebugPrint() const {
    const char* rc_str;
    switch (retcode) {
        case (Selector::SELECT_OK):      rc_str = "SELECT_OK"; break;
        case (Selector::SELECT_TIMEOUT): rc_str = "SELECT_TIMEOUT"; break;
        case (Selector::SELECT_ERROR):
        default:                                        rc_str = "SELECT_ERROR";
    }

    const char* tp_str;
    switch (type) {
        case (Selector::RX_SET_READY):      tp_str = "RX_EVENT"; break;
        case (Selector::TX_SET_READY):      tp_str = "TX_EVENT"; break;
        case (Selector::NO_SET_READY):
        default:                                        tp_str = "NO_EVENT";
    }

    printf("SelectResult::DEBUG_INFO:\n  fd: %d\n  rc: %s\n  tp: %s\n"
                                         , fd, rc_str, tp_str);
}

}  // namespace Netlib
}  // namespace Ft
