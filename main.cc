#include "common/types.h"
#include "common/error.h"
#include "config/config.h"

#include <cstdio>
#include <iostream>
#include <string>


int main(int, char**) {
    Error err(0, "No error");
    Config::Category conf = Config::Category::ParseFromINI("../config_examples/default.ini", &err);

    conf.GetSubcategoryRef("server1")
        .GetSubcategoryRef("location")
        .SetField("Google", "Boogle");
    
    Config::Category::DumpToINI(conf, "/dev/stdout", &err);

}  
