#include "http/reader.h"
#include <iostream>
#include <fnmatch.h>

int main(void) {

    Http::RequestReader rreader;

    rreader.Read("GET /path/to/someone HTTP/1.1\r\n");
    std::cout << rreader.HasParsedMessage() << std::endl;

    rreader.Read("Host:   Hello man  \r\n");
    std::cout << rreader.HasParsedMessage() << std::endl;

    rreader.Read("\n");
    std::cout << rreader.HasParsedMessage() << std::endl;

    Http::Request req = rreader.GetParsedMessage();

    req.body = "";
}
