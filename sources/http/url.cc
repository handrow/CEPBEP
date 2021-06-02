#include <iostream>
#include "http.h"
#include "utils.h"

namespace Url {

    inline std::string  ByteToHexStr(unsigned char byte) {
        static const char* hex_alphabet = "0123456789ABCDEF";
        return std::string(1, hex_alphabet[byte / 16])
               + std::string(1, hex_alphabet[byte % 16]);
    }

    typedef std::map<char, int> SymMap;

    inline int  HexSymToNum(char c) {
        static SymMap* hex_symbols;
        if (hex_symbols == NULL) {
            hex_symbols = new SymMap;
            (*hex_symbols)['0'] = 0;
            (*hex_symbols)['1'] = 1;
            (*hex_symbols)['2'] = 2;
            (*hex_symbols)['3'] = 3;
            (*hex_symbols)['4'] = 4;
            (*hex_symbols)['5'] = 5;
            (*hex_symbols)['6'] = 6;
            (*hex_symbols)['7'] = 7;
            (*hex_symbols)['8'] = 8;
            (*hex_symbols)['9'] = 9;
            (*hex_symbols)['A'] = 10;
            (*hex_symbols)['B'] = 11;
            (*hex_symbols)['C'] = 12;
            (*hex_symbols)['D'] = 13;
            (*hex_symbols)['E'] = 14;
            (*hex_symbols)['F'] = 15;
        }
        SymMap::iterator it = hex_symbols->find(c);
        if (it == hex_symbols->end())
            return -1;
        return it->second;
    }

    std::string         PercentEncode(const std::string& decoded_str) {
        std::string encoded_str;
        for (size_t sym_idx = 0; sym_idx < decoded_str.length(); ++sym_idx) {
            if (!Http::IsUnrsvdSym(decoded_str[sym_idx]))
                encoded_str += ("%" + ByteToHexStr(decoded_str[sym_idx]));
            else
                encoded_str += decoded_str[sym_idx];
        }
        return encoded_str;
    }

    std::string         PercentDecode(const std::string& encoded_str) {
        std::string decoded_str;
        for (size_t i = 0; i < encoded_str.length(); ++i) {
            if (encoded_str[i] == '%') {
                int high_part = HexSymToNum(encoded_str[i + 1]);
                int low_part = HexSymToNum(encoded_str[i + 2]);
                if (high_part < 0 || low_part < 0)
                    return "";
                char decoded_sym = char(uint(high_part) * 16 + uint(low_part));
                i += 2;
                decoded_str += decoded_sym;
            }
            else
                decoded_str += encoded_str[i];
        }
        return decoded_str;
    }

} // namespace Url