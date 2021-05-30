#ifndef SELECTOR_H_
# define SELECTOR_H_

# include <fcntl.h>
# include <unistd.h>

# include "../common/types.h"

namespace Ft {
namespace Netlib {

class Selector {
 public:
    enum ReadyEvent : u8 {
        NO_SET_READY   = 0,
        TX_SET_READY   = 1,  // when fd from TX set is active
        RX_SET_READY   = 2,  // when fd from RX set is active
        // EX_SET_READY   = 3,
    };

    enum SelectRetCode : i8 {
        SELECT_ERROR    = -1,
        SELECT_OK       = 0,
        SELECT_TIMEOUT  = 1,
    };

    struct SelectResult {
        fd_t            fd;
        SelectRetCode   retcode;
        ReadyEvent      type;

        SelectResult(fd_t f, SelectRetCode rc, ReadyEvent tp) : fd(f), retcode(rc), type(tp) {}
        void DebugPrint() const;
    };

 private:
    fd_set_t        __rx_fdset;
    fd_set_t        __tx_fdset;
    // fd_set_t        __exc_fdset;  // Don't know how to use it in our case ...
    timeval_t       __timeout;

 public:
    Selector();

    void AddRx(fd_t fd);
    void AddTx(fd_t fd);
    void SetTimeOut(u64 usec);

    void RmRx(fd_t fd);
    void RmTx(fd_t fd);

    SelectResult Select() const;
};

}  // namespace Netlib
}  // namespace Ft

#endif  // SELECTOR_H_
