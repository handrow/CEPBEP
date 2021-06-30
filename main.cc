#include "common/types.h"
#include "common/error.h"
#include "http/writer.h"

#include <cstdio>
#include <iostream>
#include <string>

int main(void) {
    Http::ResponseWriter wr;

    wr.Header().Add("Connection", "keep-alive");
    wr.Header().Add("Server", "nginx/2.15");

    wr.Write("Hello Amsterdam\n");
    wr.Write("My name is Philichipano\n");
    wr.Write("Bidirective shooting\n");
    std::cout << wr.SendToString(200) << std::endl;

    wr.Header().Add("Connection", "keep-alive");
    wr.Header().Add("Server", "nginx/2.15");

    wr.Write("Hello Amsterdam\n");
    wr.Write("My name is Philichipano\n");
    wr.Write("Bidirective shooting\n");
    std::cout << wr.SendToString(200) << std::endl;
}
