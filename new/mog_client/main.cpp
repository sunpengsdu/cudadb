#include <string>
#include <iostream>
#include "mog.h"

int main() {
    st::mog test;
    
    std::string value;
    int32_t len;
    len = test.get("/home/sunp/18/race_data/asian_female/sy/__machine5_2015-11-21_0-0-0-1-94-24-42_2015-11-21_10-16-47_jpg_498_943_173_173.jpg", value);
    std::cout << len << "->" << value.length() << "\n";
    return 0;
}
