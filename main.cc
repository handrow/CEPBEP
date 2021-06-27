#include "common/types.h"
#include "common/error.h"
#include "common/string_utils.h"
#include "config/config.h"

#include <cstdio>
#include <iostream>
#include <string>



int main(int, char**) {
    printf("%d\n", Match("a**********b", "ab"));
    printf("%d\n", Match("a**********b", "a1234124b"));
    printf("%d\n", Match("a*b", "a1234124b"));
    printf("%d\n", Match("a*b", "*ab"));
    printf("%d\n", Match("a*b", "a*b"));
    printf("%d\n", Match("[aaaaa|bng]", "bng"));
    printf("%d\n", Match("[aaaaa|bng]aa", "bngaa"));
    printf("%d\n", Match("[aaaaa|b*g]aa", "b*gaa"));
    printf("%d\n", Match("[aa|b*g]aa", "aaaa"));
    printf("%d\n", Match("[aa|b*g]aa", "aaaaa"));
}  
