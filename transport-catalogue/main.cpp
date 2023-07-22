
#include<iostream>
#include <fstream>
#include <cassert>
#include"input_reader.h"
#include "stat_reader.h"


int main() {
    //TestTransportCatalogue();
    std::ifstream in("hello.txt");
    transport_catalogue::stat_reader::ProcessRequests(in, std::cout);
    return 0;
}
