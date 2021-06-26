#include "http/reader.h"
#include <iostream>


int main() {
    Http::ResponseReader res;

    res.Read("HTTP/1.1 200 OK\nUser: agent\nTransfer-Encoding: chunked\n\n");
    res.Read("23\r\nThis is the data in the first chunk\r\n");
    res.Read("1A\r\nand this is the second one\r\n");
    res.Read("0\r\n\r\n");
    res.Process();

}
