#ifndef COMMONFUNCS_H
#define COMMONFUNCS_H

#include <string>
#include <stdint.h>


class CommonFuncs
{
public:
    CommonFuncs();

    static void delayThread(unsigned secs);
};

uint32_t char2uint(unsigned char* src, int size, bool conv);

#if _WIN32 || _WIN64
char* encode(const wchar_t* wstr, unsigned int codePage);
wchar_t* decode(const char* encodedStr, unsigned int codePage);
unsigned getTimeMs();
#endif
#if __GNUC__
long getTimeMs();
#endif
char char2bin(char input);
uint32_t crc32_no_table(const unsigned char* data, int size);

void prepareSendBuffer(uint8_t* buf, uint32_t& size_buf,
                     const uint8_t* cmd, int size_cmd);

void prepareMultipleSendBuffer(uint8_t* buf, uint32_t& size_buf,
                               const uint8_t* cmd1, int size_cmd1,
                               const uint8_t* cmd2, int size_cmd2);

#endif // COMMONFUNCS_H
