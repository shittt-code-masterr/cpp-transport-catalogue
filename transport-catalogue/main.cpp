
#include<iostream>
#include <fstream>
#include <cassert>
#include "json_reader.h"


int main() {    
    
    JSONReader proces;
    proces.ProcessJsonRequests(std::cin, std::cout);
}