#include <iostream>
#include <fstream>
#include <array>
#include <sstream>
#include <string>
#include <algorithm>

int main(int argc, char* argv[])
{
    if (argc != 4) {
        std::cerr << "usage " << argv [0] << " <key> <originalFile> <newFile>\n";
        return -1;
    }

    std::string keyString { argv[1] };
    std::string originalFileName { argv[2] };
    std::string newFileName { argv[3] };
    uint32_t key_uint32 { 0 };
    uint8_t key;

    if (keyString.size() > 2 && (keyString.substr(0,2) == "0x") || keyString.substr(0,2) == "0X")
        keyString = keyString.substr(2);

    std::stringstream tmp;
    tmp << keyString;
    tmp >> std::hex >> key_uint32; /* stringstream interprets uint8 as char and does not convert into a number */
    key = static_cast<uint8_t>(key_uint32);

    std::ifstream ifs ( originalFileName.c_str(), std::ios_base::binary);
    std::ofstream ofs ( newFileName.c_str(), std::ios_base::binary);

    if (!ifs.good() || !ofs.good()) {
        return -1;
    }

    std::array<char,256> buffer;
    while (true) {
        std::cerr << ".";
        long len = ifs.readsome(buffer.data(), buffer.size());
        if (len == 0) break;
        std::for_each(std::begin(buffer), std::begin(buffer)+len, [key](char& _s) {
            auto s {static_cast<uint8_t>(_s)};
            s = s ^ key;
            _s = static_cast<char>(s);
        });
        ofs.write(buffer.data(), len);
    }

    std::cerr << "\n";
    return 0;

}
