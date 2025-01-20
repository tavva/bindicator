#include "utils.h"

String Utils::urlEncode(const String& str) {
    String encoded = "";
    char c;
    char code0;
    char code1;
    for (int i = 0; i < str.length(); i++) {
        c = str.charAt(i);
        if (c == ' ') {
            encoded += '+';
        } else if (isalnum(c)) {
            encoded += c;
        } else {
            code1 = (c & 0xf) + '0';
            if ((c & 0xf) > 9) {
                code1 = (c & 0xf) - 10 + 'A';
            }
            c = (c >> 4) & 0xf;
            code0 = c + '0';
            if (c > 9) {
                code0 = c - 10 + 'A';
            }
            encoded += '%';
            encoded += code0;
            encoded += code1;
        }
    }
    return encoded;
}
