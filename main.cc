#include "common/types.h"
#include "common/error.h"
#include "sources/config/config.h"
#include "sources/config/utils.h"

#include <cstdio>
#include <iostream>
#include <string>

int main(int, char**) {
    Error err(0, "No error");
    Config::Category conf = Config::Category::ParseFromINI("../default.ini", &err);
    
    Config::Category::DumpToINI(conf, "../New.ini", &err);
}  
