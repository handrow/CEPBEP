#include "http/reader.h"
#include <iostream>


int main() {
    Http::RequestReader res;

    res.Read("POST /  HTTP/1.1   \nUser: agent\nTransfer-Encoding: abc, chunked, gzip\n\n");
    res.Process();
    res.Read("23\nThis is the data in the first chunk\n");
    res.Process();
    res.Read("1A\r\nand this is the second one\n");
    res.Process();
    res.Read("0\r\n\r\n");
    res.Process();

}
