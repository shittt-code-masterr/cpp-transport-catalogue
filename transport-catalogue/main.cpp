
#include<iostream>
#include <fstream>
#include <cassert>
#include "json_reader.h"


int main() {    
    std::ofstream in("stdout.json");
    JSONReader proces;
    proces.ProcessJsonRequests(std::cin, std::cout);
}