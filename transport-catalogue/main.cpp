
#include<iostream>
#include <fstream>
#include <string_view>
#include <cassert>
#include "json_reader.h"

int main(int argc, const char** argv) {
    if (argc != 2) {

        return 1;
    }

    const std::string_view mode(argv[1]);
    JSONReader proces;
    if (mode == "make_base") {

        proces.ProcessJsonRequests(std::cin, std::cout, true);

    }
    else if (mode == "process_requests") {

        proces.ProcessJsonRequests(std::cin, std::cout, false);
    }


}
