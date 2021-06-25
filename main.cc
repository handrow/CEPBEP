#include "http/reader.h"
#include <iostream>

int main() {
    // Http::RequestReader req_rdr;

    // req_rdr.Read("GET / HTTP/1.1       \n");
    // req_rdr.Read("Ouw");
    // req_rdr.Process();

    // req_rdr.Read(":   BeeMovie    \r\n");
    // req_rdr.Read("ouW:   Dog Fighters Movie\n");
    // req_rdr.Read("oUw:Pig Movie\n");
    // req_rdr.Read("\n");

    // req_rdr.Read("GET    /path/to/gggg http/1.1  \n");

    // req_rdr.Read("user-Agent");
    // req_rdr.Process();
    // req_rdr.Read(": FireFox \r\n");
    // req_rdr.Read("Host: mail.ru       \r\n");
    // req_rdr.Read("\n");

    // req_rdr.Process();

    Http::ResponseReader res_rdr;

    res_rdr.Read("http/1.1 200 OK   \n");
    res_rdr.Read("server");
    res_rdr.Process();
    res_rdr.Read(": pravoslavniy \r");
    res_rdr.Read("\nOn pokasival");
    res_rdr.Read(": pisun \r\n");
    res_rdr.Read("Content-Length: 6 \r\n");

    res_rdr.Read("\n");
    res_rdr.Read("zoche");
    res_rdr.Process();
    res_rdr.Read("m");
    res_rdr.Process();


    std::cout << Http::ResponseToString(res_rdr.GetMessage());
}
