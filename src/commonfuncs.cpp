#include <QTime>
#include <QCoreApplication>

#include "commonfuncs.h"
#include "commands.h"

#include <stdint.h>

#if _WIN32 || _WIN64
#include <windows.h>
#else
#include <sys/time.h>
#endif

CommonFuncs::CommonFuncs()
{

}

void CommonFuncs::delayThread(unsigned secs)
{
    QTime dieTime= QTime::currentTime().addSecs(secs);

    while (QTime::currentTime() < dieTime)
    {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    }
}

uint32_t char2uint(unsigned char* src, int size, bool conv)
{
    uint32_t res = 0;
    int offset = 0;
    int i;

    if (!conv)
    {
        i = size;
        while(--i > -1)
        {
            res |= (src[i] << offset);
            offset += 8;
        }
    }
    else
    {
        i = -1;
        while(++i < size)
        {
            res |= (src[i] << offset);
            offset += 8;
        }
    }

    return res;
}

#if __GNUC__
    long getTimeMs()
    {
        struct timeval  tv;
        long time_ms;
        gettimeofday(&tv, NULL);

        time_ms = (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000 ;

        return time_ms;
    }
#endif

#if _WIN32 || _WIN64
char* encode(const wchar_t* wstr, unsigned int codePage)
{
    int sizeNeeded = WideCharToMultiByte(codePage, 0, wstr, -1, NULL, 0, NULL, NULL);
    char* encodedStr = new char[sizeNeeded];
    WideCharToMultiByte(codePage, 0, wstr, -1, encodedStr, sizeNeeded, NULL, NULL);
    return encodedStr;
}

wchar_t* decode(const char* encodedStr, unsigned int codePage)
{
    int sizeNeeded = MultiByteToWideChar(codePage, 0, encodedStr, -1, NULL, 0);
    wchar_t* decodedStr = new wchar_t[sizeNeeded ];
    MultiByteToWideChar(codePage, 0, encodedStr, -1, decodedStr, sizeNeeded );
    return decodedStr;
}

unsigned getTimeMs()
{
    LARGE_INTEGER s_frequency;
    BOOL s_use_qpc = QueryPerformanceFrequency(&s_frequency);

    if (s_use_qpc)
    {
        LARGE_INTEGER now;
        QueryPerformanceCounter(&now);
        return (1000LL * now.QuadPart) / s_frequency.QuadPart;
    }
    else
    {
        return GetTickCount();
    }
}
#endif

char char2bin(char input)
{
    if(input >= '0' && input <= '9')
        return input - '0';
    if(input >= 'A' && input <= 'F')
        return input - 'A' + 10;
    if(input >= 'a' && input <= 'f')
        return input - 'a' + 10;
    return 0;
}

uint32_t crc32_no_table(const unsigned char* data, int size)
{
    int i, j;
    uint32_t res;

    res = 0xffffffff;
    for (i = 0; i < size; ++i)
    {
        res = res ^ data[i];
        for (j = 7; j >= 0; --j)
            res = (res >> 1) ^ (0xedb88320 & (-(res & 1)));
    }
    return ~res;
}

void prepareSendBuffer(uint8_t* buf, uint32_t& size_buf,
                     const uint8_t* cmd, int size_cmd)
{
    memset(buf, 0, APDU_BUFFER_SIZE);
    memcpy(buf, cmd, size_cmd);

    size_buf = size_cmd;
}

void prepareMultipleSendBuffer(uint8_t* buf, uint32_t& size_buf,
                               const uint8_t* cmd1, int size_cmd1,
                               const uint8_t* cmd2, int size_cmd2)
{
    memset(buf, 0, APDU_BUFFER_SIZE);
    memcpy(buf, cmd1, size_cmd1);
    memcpy(buf + size_cmd1, cmd2, size_cmd2);

    size_buf = size_cmd1 + size_cmd2;
    buf[LEN_INDEX] = size_cmd2;
}

