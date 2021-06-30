#include "common/types.h"
#include "common/error.h"
#include "common/string_utils.h"
#include "config/config.h"

#include <cstdio>
#include <iostream>
#include <string>

void print_subcategories(const Config::Category& c) {
    Config::Category::SubcategoryConstRange srange = c.GetSubcatoryIterRange();
    while (srange.first != srange.second) {
        std::cout << "[" << srange.first->first << "]" << std::endl;
        ++srange.first;
    }
}

void print_fields(const Config::Category& c) {
    Config::Category::FieldsConstRange frange = c.GetFieldsIterRange();
    while (frange.first != frange.second) {
        std::cout << frange.first->first << ": " << frange.first->second << std::endl;
        ++frange.first;
    }
}

int main(void) {
    Config::Category c;

    Error err;
    c = Config::Category::ParseFromINI("../full_config.ini", &err);

    print_fields(c);
    print_subcategories(c);
    print_fields(c.GetSubcategoryRef("default"));
}
