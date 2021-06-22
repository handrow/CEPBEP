#include "http/reader.h"
#include <iostream>

int main() {
    Http::RequestReader req_rdr;

    req_rdr.Read("GET / HTTP/1.1       \n");
    req_rdr.Read("Ouw");
    req_rdr.Process();

    req_rdr.Read(":   BeeMovie    \r\n");
    req_rdr.Read("ouW:   Dog Fighters Movie\n");
    req_rdr.Read("oUw:Pig Movie\n");
    req_rdr.Read("\n");

    req_rdr.Process();
}
