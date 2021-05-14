
#if __GNUC__
#include <unistd.h>
#endif

#include <QDebug>
#include <QCoreApplication>

#include "smartcard.h"
#include "commands.h"
#include "logger.h"
#include "commonfuncs.h"

using namespace smcard;

#if _WIN32 || _WIN64
long __cdecl scard_start(long hCard)
{
    Q_UNUSED(hCard)
    return 0;
}
long __cdecl scard_stop(long hCard)
{
    Q_UNUSED(hCard)
    return 0;
}
#endif
#if _WIN32 || _WIN64
long __cdecl scard_transmit(long hCard, const char *cmd, int cLength,
                            char *rpl, int *rLength)
{
    DWORD res;
    LPCSCARD_IO_REQUEST req;
    //BYTE getResponse[] = { 0x00, 0xC0, 0x00, 0x00, 0x00 };
    DWORD rplLength = *rLength;

    DWORD __protocol = SCARD_PROTOCOL_T1;
    SCARDHANDLE __card_handle = hCard;

    if (__protocol == SCARD_PROTOCOL_T1)
        req = SCARD_PCI_T1;
    else
        req = SCARD_PCI_T0;

    res = SCardTransmit(__card_handle, req, (LPCBYTE)cmd, cLength,
                        NULL, (LPBYTE)rpl, &rplLength);

    *rLength = (int)rplLength;

    if (SCARD_S_SUCCESS != res)
    {
        return 1;
    }

    return 0;
}
#endif

SmartCard::SmartCard()
{
    this->devices_name = "";
    this->hCard = 0;
    this->readerActive = false;
    this->Protocol = 100;
    this->_lastSCardError = SCARD_S_SUCCESS;
}

SmartCard::~SmartCard()
{
    this->disconnect();
}

bool SmartCard::connect(bool exclusive)
{
    LONG            lReturn;

    this->_lastSCardError = SCARD_S_SUCCESS;

    DWORD dwShareMode = SCARD_SHARE_SHARED;

    if (exclusive)
    {
        dwShareMode = SCARD_SHARE_EXCLUSIVE;
    }

    lReturn = SCardEstablishContext(SCARD_SCOPE_USER,
                                                     NULL,
                                                     NULL,
                                                     &this->CardContext);

    if (SCARD_S_SUCCESS != lReturn)
    {
#ifdef DEBUG_SMART_CARD_CONNECTION
        DEBUG(DBG_ERROR, "Failed SCardEstablishContext " <<
              (QString("%1").arg((0xffffffff & lReturn), 8, 16, QChar('0')).toUpper()).toStdString() <<
              ". " <<
              getSCardErrorDescription(lReturn));

#endif // DEBUG_SMART_CARD_CONNECTION
        this->_lastSCardError = lReturn;
        return false;
    }

#if _WIN32 || _WIN64
    std::wstring stemp = std::wstring(this->devices_name.begin(),
                                      this->devices_name.end());
    LPCWSTR swlpcDeviceName = stemp.c_str();

    lReturn = SCardConnect(this->CardContext,
                                            swlpcDeviceName,
                                            dwShareMode,
                                            SCARD_PROTOCOL_T1 | SCARD_PROTOCOL_T0,
                                            &this->CardHandle,
                                            &this->Protocol);
#elif __GNUC__
    lReturn = SCardConnect(this->CardContext,
                           (LPTSTR)this->devices_name.c_str(),
                           dwShareMode,
                           SCARD_PROTOCOL_T1 | SCARD_PROTOCOL_T0,
                           &this->CardHandle,
                           &this->Protocol);
#endif

    if (SCARD_S_SUCCESS != lReturn)
    {
#ifdef DEBUG_SMART_CARD_CONNECTION
        DEBUG(DBG_ERROR, "Failed SCardConnect " <<
              (QString("%1").arg((0xffffffff & lReturn), 8, 16, QChar('0')).toUpper()).toStdString() <<
              ". " <<
              getSCardErrorDescription(lReturn));

#endif // DEBUG_SMART_CARD_CONNECTION

        if (this->CardContext)
        {
            SCardReleaseContext(this->CardContext);
        }

        this->_lastSCardError = lReturn;
        return false;
    }

    lReturn = SCardReconnect(this->CardHandle,
                                              SCARD_SHARE_SHARED,
                                              SCARD_PROTOCOL_T1 | SCARD_PROTOCOL_T0,
                                              SCARD_UNPOWER_CARD,
                                              &this->Protocol);

    if (SCARD_S_SUCCESS != lReturn)
    {
#ifdef DEBUG_SMART_CARD_CONNECTION
        DEBUG(DBG_ERROR, "Failed SCardReconnect" <<
              (QString("%1").arg((0xffffffff & lReturn), 8, 16, QChar('0')).toUpper()).toStdString() <<
              ". " <<
              getSCardErrorDescription(lReturn));

#endif // DEBUG_SMART_CARD_CONNECTION

        if (this->CardContext)
        {
            SCardReleaseContext(this->CardContext);
        }

        this->_lastSCardError = lReturn;
        return false;
    }

    switch(this->Protocol)
    {
        case SCARD_PROTOCOL_T0:
            break;
        case SCARD_PROTOCOL_T1:
            break;
        case SCARD_PROTOCOL_UNDEFINED:
        default:
            break;
    }

    this->readerActive = true;
    return true;
}

bool SmartCard::disconnect()
{
    if (!this->readerActive)
    {
        return true;
    }

    if (this->CardHandle)
    {
        if(SCARD_S_SUCCESS != SCardDisconnect(this->CardHandle, SCARD_LEAVE_CARD))
        {
//            return false;
        }
    }
    if(this->CardContext)
    {
        if(SCARD_S_SUCCESS == SCardReleaseContext(this->CardContext))
        {
            this->readerActive = false;
        }
        else
        {
            this->readerActive = false;
            return false;
        }
    }

    return true;
}

bool SmartCard::reconnect(unsigned timeout_ms, bool exclusive)
{
    this->disconnect();

    CommonFuncs::delayThread(timeout_ms/1000);

    for (int i = 0;i < 10;i++)
    {
        if (!this->devices_name.empty())
        {
            if (!check_reader(this->devices_name))
            {
                CommonFuncs::delayThread(2);
                continue;
            }
            else
            {
                CommonFuncs::delayThread(4);
                if (this->connect(exclusive))
                {
                    break;
                }
            }
        }
    }

    return true;
}

bool SmartCard::create_scard_ctx()
{
    return true;
}

bool SmartCard::destroy_scard_ctx()
{
    return true;
}

void SmartCard::setDeviceName(std::string dev_name)
{
    this->devices_name = dev_name;
}

std::string SmartCard::getDeviceName()
{
    return this->devices_name;
}

bool SmartCard::isActive()
{
    return this->readerActive;
}

unsigned long SmartCard::getProto()
{
    switch(this->Protocol)
    {
        case SCARD_PROTOCOL_T0:
            return 0;
        case SCARD_PROTOCOL_T1:
            return 1;
        case SCARD_PROTOCOL_UNDEFINED:
        default:
            return 16;
    }
}

long SmartCard::transmit(const char *cmd, int cLength, char *rpl, int *rLength)
{
    DWORD res;
    LPCSCARD_IO_REQUEST req;

    *rLength = 1024;

    DWORD rplLength = *rLength;

    memset(rpl,0,rplLength);

    this->_lastSCardError = SCARD_S_SUCCESS;

    if (this->Protocol == SCARD_PROTOCOL_T1)
    {
        req = SCARD_PCI_T1;
    }
    else
    {
        req = SCARD_PCI_T0;
    }

    res = SCardTransmit(this->CardHandle,
                        req,
                        (LPCBYTE)cmd,
                        cLength,
                        NULL,
                        (LPBYTE)rpl,
                        &rplLength);

    *rLength = (int)rplLength;

#ifdef DEBUG_SMART_CARD_TRANSMISSION
    printf("C-APDU: ");

    for (int i = 0;i < cLength;i++ )
    {
        printf("%02x ", (0xff & cmd[i]));
    }
    printf("\n");

    printf("R-APDU: ");

    for (int i = 0;i < *rLength;i++ )
    {
        printf("%02x ", (0xff & rpl[i]));
    }
    printf("\n");
    fflush(stderr);
    fflush(stdout);
#endif // DEBUG_SMART_CARD_CONNECTION

    if (SCARD_S_SUCCESS != res)
    {
        DEBUG(DBG_ERROR, "Failed SCardTransmit " <<
              (QString("%1").arg((0xffffffff & res), 8, 16, QChar('0')).toUpper()).toStdString() <<
              ". " <<
              getSCardErrorDescription(res));

        this->_lastSCardError = res;
        return res;
    }
    // PROTOCOL_T0
    if ((rpl[(*rLength) - 2] == 0x61) || (rpl[(*rLength) - 2] == 0x6C))
    {
        BYTE getResponse[] = { 0x00, 0xC0, 0x00, 0x00, 0x00 };

        getResponse[LEN_INDEX] = rpl[(*rLength) - 1];

        if (SCARD_S_SUCCESS != SCardTransmit(this->CardHandle,
                                             req,
                                             getResponse,
                                             sizeof(getResponse),
                                             NULL,
                                             (LPBYTE)rpl,
                                             (LPDWORD)rLength))
        {
            return 1;
        }
    }

    return 0;
}

void SmartCard::beginTransaction()
{
    SCardBeginTransaction(this->CardHandle);
}

void SmartCard::endTransaction()
{
    SCardEndTransaction(this->CardHandle, SCARD_LEAVE_CARD);
}

char * SmartCard::getSCardErrorDescription(long resVal)
{
    static char strError[75];

#if 1
    switch (resVal)
    {
        case SCARD_S_SUCCESS:
            (void)STRNCPY(strError, "Command successful.", sizeof(strError));
            break;
        case SCARD_E_CANCELLED:
            (void)STRNCPY(strError, "Command cancelled.", sizeof(strError));
            break;
        case SCARD_E_CANT_DISPOSE:
            (void)STRNCPY(strError, "Cannot dispose handle.", sizeof(strError));
            break;
        case SCARD_E_INSUFFICIENT_BUFFER:
            (void)STRNCPY(strError, "Insufficient buffer.", sizeof(strError));
            break;
        case SCARD_E_INVALID_ATR:
            (void)STRNCPY(strError, "Invalid ATR.", sizeof(strError));
            break;
        case SCARD_E_INVALID_HANDLE:
            (void)STRNCPY(strError, "Invalid handle.", sizeof(strError));
            break;
        case SCARD_E_INVALID_PARAMETER:
            (void)STRNCPY(strError, "Invalid parameter given.", sizeof(strError));
            break;
        case SCARD_E_INVALID_TARGET:
            (void)STRNCPY(strError, "Invalid target given.", sizeof(strError));
            break;
        case SCARD_E_INVALID_VALUE:
            (void)STRNCPY(strError, "Invalid value given.", sizeof(strError));
            break;
        case SCARD_E_NO_MEMORY:
            (void)STRNCPY(strError, "Not enough memory.", sizeof(strError));
            break;
        case SCARD_F_COMM_ERROR:
            (void)STRNCPY(strError, "RPC transport error.", sizeof(strError));
            break;
        case SCARD_F_INTERNAL_ERROR:
            (void)STRNCPY(strError, "Internal error.", sizeof(strError));
            break;
        case SCARD_F_UNKNOWN_ERROR:
            (void)STRNCPY(strError, "Unknown error.", sizeof(strError));
            break;
        case SCARD_F_WAITED_TOO_LONG:
            (void)STRNCPY(strError, "Waited too long.", sizeof(strError));
            break;
        case SCARD_E_UNKNOWN_READER:
            (void)STRNCPY(strError, "Unknown reader specified.", sizeof(strError));
            break;
        case SCARD_E_TIMEOUT:
            (void)STRNCPY(strError, "Command timeout.", sizeof(strError));
            break;
        case SCARD_E_SHARING_VIOLATION:
            (void)STRNCPY(strError, "Sharing violation.", sizeof(strError));
            break;
        case SCARD_E_NO_SMARTCARD:
            (void)STRNCPY(strError, "No smart card inserted.", sizeof(strError));
            break;
        case SCARD_E_UNKNOWN_CARD:
            (void)STRNCPY(strError, "Unknown card.", sizeof(strError));
            break;
        case SCARD_E_PROTO_MISMATCH:
            (void)STRNCPY(strError, "Card protocol mismatch.", sizeof(strError));
            break;
        case SCARD_E_NOT_READY:
            (void)STRNCPY(strError, "Subsystem not ready.", sizeof(strError));
            break;
        case SCARD_E_SYSTEM_CANCELLED:
            (void)STRNCPY(strError, "System cancelled.", sizeof(strError));
            break;
        case SCARD_E_NOT_TRANSACTED:
            (void)STRNCPY(strError, "Transaction failed.", sizeof(strError));
            break;
        case SCARD_E_READER_UNAVAILABLE:
            (void)STRNCPY(strError, "Reader is unavailable.", sizeof(strError));
            break;
        case SCARD_W_UNSUPPORTED_CARD:
            (void)STRNCPY(strError, "Card is not supported.", sizeof(strError));
            break;
        case SCARD_W_UNRESPONSIVE_CARD:
            (void)STRNCPY(strError, "Card is unresponsive.", sizeof(strError));
            break;
        case SCARD_W_UNPOWERED_CARD:
            (void)STRNCPY(strError, "Card is unpowered.", sizeof(strError));
            break;
        case SCARD_W_RESET_CARD:
            (void)STRNCPY(strError, "Card was reset.", sizeof(strError));
            break;
        case SCARD_W_REMOVED_CARD:
            (void)STRNCPY(strError, "Card was removed.", sizeof(strError));
            break;
        case SCARD_E_UNSUPPORTED_FEATURE:
            (void)STRNCPY(strError, "Feature not supported.", sizeof(strError));
            break;
        case SCARD_E_PCI_TOO_SMALL:
            (void)STRNCPY(strError, "PCI struct too small.", sizeof(strError));
            break;
        case SCARD_E_READER_UNSUPPORTED:
            (void)STRNCPY(strError, "Reader is unsupported.", sizeof(strError));
            break;
        case SCARD_E_DUPLICATE_READER:
            (void)STRNCPY(strError, "Reader already exists.", sizeof(strError));
            break;
        case SCARD_E_CARD_UNSUPPORTED:
            (void)STRNCPY(strError, "Card is unsupported.", sizeof(strError));
            break;
        case SCARD_E_NO_SERVICE:
            (void)STRNCPY(strError, "Service not available.", sizeof(strError));
            break;
        case SCARD_E_SERVICE_STOPPED:
            (void)STRNCPY(strError, "Service was stopped.", sizeof(strError));
            break;
        case SCARD_E_NO_READERS_AVAILABLE:
            (void)STRNCPY(strError, "Cannot find a smart card reader.", sizeof(strError));
            break;
        default:
            break;
            //(void)STRNCPY(strError, sizeof(strError) - 1, "Unkown error: 0x%08X", resVal);
    };
#endif
    /* add a null byte */
    strError[sizeof(strError) - 1] = '\0';

    return strError;
}

bool SmartCard::check_reader(std::string reader_name)
{
    bool ret = false;

    uint8_t			ReadersCnt = 0;

#if __GNUC__

    LPTSTR			Readers[MAX_READERS_AVAILABLE];

    SCARDCONTEXT	tmpCardCtx;
    char 			ReadersList[READER_LIST_SIZE];

    DWORD			size = READER_LIST_SIZE;

    DWORD res = SCardEstablishContext(SCARD_SCOPE_USER, NULL, NULL, &tmpCardCtx);

    if (SCARD_S_SUCCESS != res)
    {
        qDebug() << __FUNCTION__ <<
                    "Failed to establish context: ";
        /*<<
                    pcsc_stringify_error(res);*/
        return ret;
    }

    res = SCardListReaders(tmpCardCtx, NULL, (LPTSTR)ReadersList, &size);

    if (tmpCardCtx)
    {
        SCardReleaseContext(tmpCardCtx);
    }

    if (SCARD_S_SUCCESS != res)
    {
        qDebug() << "WARNING: Reader not available";
    }
    else
    {
        char *pReader = ReadersList;

        while ('\0' != *pReader)
        {
            Readers[ReadersCnt++] = pReader;
            pReader = pReader + strlen((const char *)pReader) + 1;

            if (ReadersCnt >= MAX_READERS_AVAILABLE)
            {
                qDebug() << __FUNCTION__ <<
                            "ERROR: Too much Reader, MAX = " <<
                            MAX_READERS_AVAILABLE <<
                            ", available: " <<
                            ReadersCnt;
                return ret;
            }
        }
    }

#else

    SCARDCONTEXT    hSC;
    LONG            lReturn;
    char *			Readers[MAX_READERS_AVAILABLE];

    lReturn = SCardEstablishContext(SCARD_SCOPE_USER,
                                    NULL,
                                    NULL,
                                    &hSC);

    LPTSTR          pmszReaders = NULL;
    LPTSTR          pReader;
    LONG            lReturn2;
    DWORD           cch = SCARD_AUTOALLOCATE;

    // Retrieve the list the readers.
    // hSC was set by a previous call to SCardEstablishContext.
    lReturn = SCardListReaders(hSC,
                               NULL,
                               (LPTSTR)&pmszReaders,
                               &cch );

    switch( lReturn )
    {
        case SCARD_E_NO_READERS_AVAILABLE:
            // "Reader is not in groups";
            return false;
        case SCARD_S_SUCCESS:

            pReader = pmszReaders;

            while ( '\0' != *pReader )
            {
                Readers[ReadersCnt++] = encode(pReader, CP_UTF8);

                pReader = pReader + wcslen((wchar_t *)pReader) + 1;
            }

            lReturn2 = SCardFreeMemory( hSC, pmszReaders );

            break;

    default:
            return ret;
    }

    if (hSC)
        SCardReleaseContext(hSC);
#endif // __GNUC__

    for (uint8_t i = 0; i < ReadersCnt; i++)
    {
        if (strstr(this->devices_name.c_str(), (char *)Readers[i]))
        {
            ret = true;
            break;
        }
    }

    return ret;
}

