#ifndef CODER_H
#define CODER_H

enum {
    CodesCount = 256
};

class Coder {
public:

    static int GetCode(uint64_t mask, int position) {
//        return 0;
        uint64_t m = mask;
        int p = position;
//        return (m >> p) & 0xff; // 256
//        return ((m >> p) & 0xf) | ((m >> (p + 12)) & 0xf0); // 256
//        return ((m >> p) & 1) | ((m >> (p + 4 - 1)) & 2) | ((m >> (p + 16 - 2)) & 4); // 8
//        return ((m >> p) & 3) | ((m >> (p + 4 - 2)) & (4 + 8)) | ((m >> (p + 16 - 4)) & (16 + 32)) | ((m >> (p + 20 - 6)) & (64 + 128)); // 256
        return ((m >> p) & 3) | ((m >> (p + 3 - 2)) & (4 + 8 + 16)) | ((m >> (p + 16 - 5)) & (32 + 64 + 128)); // 256
//        return ((m >> p) & 3) | ((m >> (p + 3 - 2)) & (4 + 8 + 16)) | ((m >> (p + 15 - 5)) & (32 + 64 + 128)) | ((m >> (p + 19 - 8)) & (256 + 512 + 1024)); // 2048
//        return ((m >> p) & (3 + 2048 + 4096 + 8192)) | ((m >> (p + 3 - 2)) & (4 + 8 + 16)) | ((m >> (p + 15 - 5)) & (32 + 64 + 128)) | ((m >> (p + 19 - 8)) & (256 + 512 + 1024)); // 16384 | ((m >> (p + 11 - 11)) & (2048 + 4096 + 8192));
    }

    static bool IsCompatible(uint64_t piece, int position, int code) {
        return (GetCode(piece, position) & code) == 0;
    }

};

#endif // CODER_H
