#ifndef SMARTCARD_H
#define SMARTCARD_H

#include <string>

#if _WIN32 || _WIN64
#include <windows.h>
#include <Winscard.h>
#include <stdint.h>
#else
//typedef unsigned long DWORD;
//typedef DWORD *LPDWORD;
typedef char *LPSTR;
typedef LPSTR LPTSTR;
#endif

#if __MACH__ || __GNUC__
#include <PCSC/winscard.h>
#include <PCSC/pcsclite.h>
#include <PCSC/wintypes.h>
#include <stdexcept>
#endif

#define Rslt_t                      uint8_t

#define OK                          0
#define FAILURE                     1
#define BUSY                        2
#define PROCEED                     3
#define UNEXPECTED_CODE             4
#define DISCARD                     5
#define WRONG_APDU                  6
#define SHUTDOWN                    7
#define EMPTY                       8

#define APDU_RECIEVE_BUFFER_SIZE	(uint32_t)(261+4+2)
#define READER_LIST_SIZE			1024
#define MAX_READERS_AVAILABLE		10
#define READER_NAME_MAX_LENGTH		64
#define MAX_READER_CTX_CNT			(uint8_t)2
#define MAX_COMMON_ARGC				(uint8_t)3
#define MAX_COMMAND_LINE_ARGC		(uint8_t)(MAX_COMMON_ARGC + MAX_READER_CTX_CNT)

#define MAX_DEVICE_CONN             2

#define DEFAULT_RECON_TIMEOUT_MS    2000
#define UPDATE_RECON_TIMEOUT_MS     1000

#define DEBUG_SMART_CARD_TRANSMISSION
//#undef DEBUG_SMART_CARD_TRANSMISSION

#define DEBUG_SMART_CARD_CONNECTION
//#undef DEBUG_SMART_CARD_CONNECTION

#if _WIN32 || _WIN64
#define STRNCPY     strncpy_s
#else
#define STRNCPY     strncpy
#endif

namespace smcard {

class SmartCard
{
public:
    SmartCard();
    virtual ~SmartCard();

    bool connect(bool exclusive = false);
    bool disconnect();
    bool reconnect(unsigned timeout_ms, bool exclusive = false);

    bool create_scard_ctx();
    bool destroy_scard_ctx();

    // set
    void setDeviceName(std::string dev_name);

    // get
    std::string getDeviceName();
    bool isActive();
    unsigned long getProto();

    long transmit(const char *cmd, int cLength, char *rpl, int *rLength);

    void beginTransaction();
    void endTransaction();

    SCARDHANDLE getSCardHandle()
    {
        return this->CardHandle;
    }

    SCARDCONTEXT getSCardContext()
    {
        return this->CardContext;
    }

    DWORD getLastSCardErrror()
    {
        DWORD ret = this->_lastSCardError;
        this->_lastSCardError = SCARD_S_SUCCESS;
        return ret;
    }

    char *getSCardErrorDescription(long resVal);

    //long scard_start(long hCard) { return 0; }
    //long scard_stop(long hCard)  { return 0; }
private:
    bool check_reader(std::string reader_name);

private:
    std::string devices_name;
    long hCard;
    SCARDCONTEXT CardContext;
    SCARDHANDLE CardHandle;
    DWORD Protocol;
    bool readerActive;
    DWORD _lastSCardError;
};
}

#endif // SMARTCARD_H
