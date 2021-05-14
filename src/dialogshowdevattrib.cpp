
#include <QDebug>

#ifdef __GNUC__
#include <PCSC/winscard.h>
#include <PCSC/pcsclite.h>
#endif

#include "dialogshowdevattrib.h"
#include "ui_dialogshowdevattrib.h"

DialogShowDevAttrib::DialogShowDevAttrib(QWidget *parent,
                                         smcard::SmartCard *SCard) :
    QDialog(parent),
    ui(new Ui::DialogShowDevAttrib)
{
    ui->setupUi(this);

    updateDeviceAttrib(SCard);
}

bool DialogShowDevAttrib::updateDeviceAttrib(smcard::SmartCard *SCard)
{
    ui->textEditDevAttrib->append("Attributes");

    if (!this->getSCardAttrib(SCard))
    {
        ui->textEditDevAttrib->append("Reader is not avalaible");
    }

    QTextCursor cursor = ui->textEditDevAttrib->textCursor();
    cursor.movePosition(QTextCursor::Start);
    ui->textEditDevAttrib->setTextCursor(cursor);

    return true;
}

DialogShowDevAttrib::~DialogShowDevAttrib()
{
    delete ui;
}

bool DialogShowDevAttrib::getSCardAttrib(smcard::SmartCard *SCard)
{
#if _WIN32 || _WIN64
    if(SCard && SCard->isActive())
    {
        QString attribTex;
        LONG            lReturn;
        LPBYTE   pbAttr = NULL;
        DWORD    cByte = SCARD_AUTOALLOCATE;
        DWORD    i;
        DWORD dwAttrId = SCARD_ATTR_DEVICE_SYSTEM_NAME;

        pbAttr = nullptr;
        cByte = SCARD_AUTOALLOCATE;
        attribTex = "DEVICE_SYSTEM_NAME:\t\t";

        lReturn = SCardGetAttrib(SCard->getSCardHandle(),
                                 dwAttrId,
                                 (LPBYTE)&pbAttr,
                                 &cByte);

        if ( SCARD_S_SUCCESS != lReturn )
        {
            if ( ERROR_NOT_SUPPORTED == lReturn )
                attribTex += "Value not supported";
            else
            {
                // Some other error occurred.
                attribTex += "Failed SCardGetAttrib";
                return false;
            }
        }
        else
        {

            for (i = 0; i < cByte; i++)
            {
                attribTex += QString(*(pbAttr + i));
            }

            // Free the memory when done.
            // hContext was set earlier by SCardEstablishContext
            SCardFreeMemory( SCard->getSCardContext(), pbAttr );
        }

        attribTex += "\n";

//---------------------------------

        pbAttr = nullptr;
        dwAttrId = SCARD_ATTR_ATR_STRING;
        cByte = SCARD_AUTOALLOCATE;

        attribTex += "ATR:\t\t\t";

        lReturn = SCardGetAttrib(SCard->getSCardHandle(),
                                 dwAttrId,
                                 (LPBYTE)&pbAttr,
                                 &cByte);
        if ( SCARD_S_SUCCESS != lReturn )
        {
            if ( ERROR_NOT_SUPPORTED == lReturn )
                attribTex += "Value not supported";
            else
            {
                // Some other error occurred.
                attribTex += "Failed SCardGetAttrib";
                return false;
            }
        }
        else
        {

            // Output the bytes.
            for (i = 0; i < cByte; i++)
            {
                attribTex += QString("%1").arg(*(pbAttr+i), 2, 16, QChar('0')).toUpper();
                attribTex += " ";
            }

            // Free the memory when done.
            // hContext was set earlier by SCardEstablishContext
            SCardFreeMemory( SCard->getSCardContext(), pbAttr );
            pbAttr = nullptr;
        }

        attribTex += "\n";


//---------------------------------

        pbAttr = nullptr;
        dwAttrId = SCARD_ATTR_CHANNEL_ID;
        cByte = SCARD_AUTOALLOCATE;
        attribTex += "CHANNEL_ID:\t\t";

        lReturn = SCardGetAttrib(SCard->getSCardHandle(),
                                 dwAttrId,
                                 (LPBYTE)&pbAttr,
                                 &cByte);
        if ( SCARD_S_SUCCESS != lReturn )
        {
            if ( ERROR_NOT_SUPPORTED == lReturn )
                attribTex += "Value not supported";
            else
            {
                // Some other error occurred.
                attribTex += "Failed SCardGetAttrib";
                return false;
            }
        }
        else
        {
            // Output the bytes.
            for (i = 0; i < cByte; i++)
            {
                attribTex += QString("%1").arg(*(pbAttr+i), 2, 16, QChar('0')).toUpper();
                attribTex += " ";
            }


            // Free the memory when done.
            // hContext was set earlier by SCardEstablishContext
            SCardFreeMemory( SCard->getSCardContext(), pbAttr );
        }

        attribTex += "\n";

//---------------------------------

        pbAttr = nullptr;
        dwAttrId = SCARD_ATTR_CHARACTERISTICS;
        cByte = SCARD_AUTOALLOCATE;
        attribTex += "ATTR CHARACTERISTICS:\t\t";

        lReturn = SCardGetAttrib(SCard->getSCardHandle(),
                                 dwAttrId,
                                 (LPBYTE)&pbAttr,
                                 &cByte);
        if ( SCARD_S_SUCCESS != lReturn )
        {
            if ( ERROR_NOT_SUPPORTED == lReturn )
                attribTex += "Value not supported";
            else
            {
                // Some other error occurred.
                attribTex += "Failed SCardGetAttrib";
                return false;
            }
        }
        else
        {
            // Output the bytes.
            for (i = 0; i < cByte; i++)
            {
                attribTex += QString("%1").arg(*(pbAttr+i), 2, 16, QChar('0')).toUpper();
                attribTex += " ";
            }

            // Free the memory when done.
            // hContext was set earlier by SCardEstablishContext
            SCardFreeMemory( SCard->getSCardContext(), pbAttr );
        }

        attribTex += "\n";

//---------------------------------

        pbAttr = nullptr;
        dwAttrId = SCARD_ATTR_CURRENT_BWT;
        cByte = SCARD_AUTOALLOCATE;
        attribTex += "CURRENT_BWT:\t\t";

        lReturn = SCardGetAttrib(SCard->getSCardHandle(),
                                 dwAttrId,
                                 (LPBYTE)&pbAttr,
                                 &cByte);
        if ( SCARD_S_SUCCESS != lReturn )
        {
            if ( ERROR_NOT_SUPPORTED == lReturn )
                attribTex += "Value not supported";
            else
            {
                // Some other error occurred.
                attribTex += "Failed SCardGetAttrib";
                return false;
            }
        }
        else
        {
            // Output the bytes.
            for (i = 0; i < cByte; i++)
            {
                attribTex += QString("%1").arg(*(pbAttr+i), 2, 16, QChar('0')).toUpper();
                attribTex += " ";
            }

            // Free the memory when done.
            // hContext was set earlier by SCardEstablishContext
            SCardFreeMemory( SCard->getSCardContext(), pbAttr );
        }

        attribTex += "\n";

//---------------------------------

        pbAttr = nullptr;
        dwAttrId = SCARD_ATTR_CURRENT_CLK;
        cByte = SCARD_AUTOALLOCATE;
        attribTex += "CURRENT_CLK:\t\t";

        lReturn = SCardGetAttrib(SCard->getSCardHandle(),
                                 dwAttrId,
                                 (LPBYTE)&pbAttr,
                                 &cByte);
        if ( SCARD_S_SUCCESS != lReturn )
        {
            if ( ERROR_NOT_SUPPORTED == lReturn )
                attribTex += "Value not supported";
            else
            {
                // Some other error occurred.
                attribTex += "Failed SCardGetAttrib";
                return false;
            }
        }
        else
        {
            // Output the bytes.
            for (i = 0; i < cByte; i++)
            {
                attribTex += QString("%1").arg(*(pbAttr+i), 2, 16, QChar('0')).toUpper();
                attribTex += " ";
            }

            // Free the memory when done.
            // hContext was set earlier by SCardEstablishContext
            SCardFreeMemory( SCard->getSCardContext(), pbAttr );
        }

        attribTex += "\n";

//---------------------------------

        pbAttr = nullptr;
        dwAttrId = SCARD_ATTR_CURRENT_CWT;
        cByte = SCARD_AUTOALLOCATE;
        attribTex += "CURRENT_CWT:\t\t";

        lReturn = SCardGetAttrib(SCard->getSCardHandle(),
                                 dwAttrId,
                                 (LPBYTE)&pbAttr,
                                 &cByte);
        if ( SCARD_S_SUCCESS != lReturn )
        {
            if ( ERROR_NOT_SUPPORTED == lReturn )
                attribTex += "Value not supported";
            else
            {
                // Some other error occurred.
                attribTex += "Failed SCardGetAttrib";
                return false;
            }
        }
        else
        {
            // Output the bytes.
            for (i = 0; i < cByte; i++)
            {
                attribTex += QString("%1").arg(*(pbAttr+i), 2, 16, QChar('0')).toUpper();
                attribTex += " ";
            }

            // Free the memory when done.
            // hContext was set earlier by SCardEstablishContext
            SCardFreeMemory( SCard->getSCardContext(), pbAttr );
        }

        attribTex += "\n";

//---------------------------------

        pbAttr = nullptr;
        dwAttrId = SCARD_ATTR_CURRENT_D;
        cByte = SCARD_AUTOALLOCATE;
        attribTex += "CURRENT_D:\t\t\t";

        lReturn = SCardGetAttrib(SCard->getSCardHandle(),
                                 dwAttrId,
                                 (LPBYTE)&pbAttr,
                                 &cByte);
        if ( SCARD_S_SUCCESS != lReturn )
        {
            if ( ERROR_NOT_SUPPORTED == lReturn )
                attribTex += "Value not supported";
            else
            {
                // Some other error occurred.
                attribTex += "Failed SCardGetAttrib";
                return false;
            }
        }
        else
        {
            // Output the bytes.
            for (i = 0; i < cByte; i++)
            {
                attribTex += QString("%1").arg(*(pbAttr+i), 2, 16, QChar('0')).toUpper();
                attribTex += " ";
            }

            // Free the memory when done.
            // hContext was set earlier by SCardEstablishContext
            SCardFreeMemory( SCard->getSCardContext(), pbAttr );
        }

        attribTex += "\n";

//---------------------------------

        pbAttr = nullptr;
        dwAttrId = SCARD_ATTR_CURRENT_EBC_ENCODING;
        cByte = SCARD_AUTOALLOCATE;
        attribTex += "CURRENT_EBC_ENCODING:\t";

        lReturn = SCardGetAttrib(SCard->getSCardHandle(),
                                 dwAttrId,
                                 (LPBYTE)&pbAttr,
                                 &cByte);
        if ( SCARD_S_SUCCESS != lReturn )
        {
            if ( ERROR_NOT_SUPPORTED == lReturn )
                attribTex += "Value not supported";
            else
            {
                // Some other error occurred.
                attribTex += "Failed SCardGetAttrib";
                return false;
            }
        }
        else
        {
            // Output the bytes.
            for (i = 0; i < cByte; i++)
            {
                attribTex += QString("%1").arg(*(pbAttr+i), 2, 16, QChar('0')).toUpper();
                attribTex += " ";
            }

            // Free the memory when done.
            // hContext was set earlier by SCardEstablishContext
            SCardFreeMemory( SCard->getSCardContext(), pbAttr );
        }

        attribTex += "\n";

//---------------------------------

        pbAttr = nullptr;
        dwAttrId = SCARD_ATTR_CURRENT_F;
        cByte = SCARD_AUTOALLOCATE;
        attribTex += "CURRENT_F:\t\t\t";

        lReturn = SCardGetAttrib(SCard->getSCardHandle(),
                                 dwAttrId,
                                 (LPBYTE)&pbAttr,
                                 &cByte);
        if ( SCARD_S_SUCCESS != lReturn )
        {
            if ( ERROR_NOT_SUPPORTED == lReturn )
                attribTex += "Value not supported";
            else
            {
                // Some other error occurred.
                attribTex += "Failed SCardGetAttrib";
                return false;
            }
        }
        else
        {
            // Output the bytes.
            for (i = 0; i < cByte; i++)
            {
                attribTex += QString("%1").arg(*(pbAttr+i), 2, 16, QChar('0')).toUpper();
                attribTex += " ";
            }




            // Free the memory when done.
            // hContext was set earlier by SCardEstablishContext
            SCardFreeMemory( SCard->getSCardContext(), pbAttr );
        }

        attribTex += "\n";

//---------------------------------

        pbAttr = nullptr;
        dwAttrId = SCARD_ATTR_CURRENT_IFSC;
        cByte = SCARD_AUTOALLOCATE;
        attribTex += "CURRENT_IFSC:\t\t";

        lReturn = SCardGetAttrib(SCard->getSCardHandle(),
                                 dwAttrId,
                                 (LPBYTE)&pbAttr,
                                 &cByte);
        if ( SCARD_S_SUCCESS != lReturn )
        {
            if ( ERROR_NOT_SUPPORTED == lReturn )
                attribTex += "Value not supported";
            else
            {
                // Some other error occurred.
                attribTex += "Failed SCardGetAttrib";
                return false;
            }
        }
        else
        {
            // Output the bytes.
            for (i = 0; i < cByte; i++)
            {
                attribTex += QString("%1").arg(*(pbAttr+i), 2, 16, QChar('0')).toUpper();
                attribTex += " ";
            }




            // Free the memory when done.
            // hContext was set earlier by SCardEstablishContext
            SCardFreeMemory( SCard->getSCardContext(), pbAttr );
        }

        attribTex += "\n";

//---------------------------------

        pbAttr = nullptr;
        dwAttrId = SCARD_ATTR_CURRENT_IFSD;
        cByte = SCARD_AUTOALLOCATE;
        attribTex += "CURRENT_IFSD:\t\t";

        lReturn = SCardGetAttrib(SCard->getSCardHandle(),
                                 dwAttrId,
                                 (LPBYTE)&pbAttr,
                                 &cByte);
        if ( SCARD_S_SUCCESS != lReturn )
        {
            if ( ERROR_NOT_SUPPORTED == lReturn )
                attribTex += "Value not supported";
            else
            {
                // Some other error occurred.
                attribTex += "Failed SCardGetAttrib";
                return false;
            }
        }
        else
        {
            // Output the bytes.
            for (i = 0; i < cByte; i++)
            {
                attribTex += QString("%1").arg(*(pbAttr+i), 2, 16, QChar('0')).toUpper();
                attribTex += " ";
            }




            // Free the memory when done.
            // hContext was set earlier by SCardEstablishContext
            SCardFreeMemory( SCard->getSCardContext(), pbAttr );
        }

        attribTex += "\n";

//---------------------------------

        pbAttr = nullptr;
        dwAttrId = SCARD_ATTR_CURRENT_N;
        cByte = SCARD_AUTOALLOCATE;
        attribTex += "CURRENT_N:\t\t\t";

        lReturn = SCardGetAttrib(SCard->getSCardHandle(),
                                 dwAttrId,
                                 (LPBYTE)&pbAttr,
                                 &cByte);
        if ( SCARD_S_SUCCESS != lReturn )
        {
            if ( ERROR_NOT_SUPPORTED == lReturn )
                attribTex += "Value not supported";
            else
            {
                // Some other error occurred.
                attribTex += "Failed SCardGetAttrib";
                return false;
            }
        }
        else
        {
            // Output the bytes.
            for (i = 0; i < cByte; i++)
            {
                attribTex += QString("%1").arg(*(pbAttr+i), 2, 16, QChar('0')).toUpper();
                attribTex += " ";
            }




            // Free the memory when done.
            // hContext was set earlier by SCardEstablishContext
            SCardFreeMemory( SCard->getSCardContext(), pbAttr );
        }

        attribTex += "\n";

//---------------------------------

        pbAttr = nullptr;
        dwAttrId = SCARD_ATTR_CURRENT_PROTOCOL_TYPE;
        cByte = SCARD_AUTOALLOCATE;
        attribTex += "CURRENT_PROTOCOL_TYPE:\t";

        lReturn = SCardGetAttrib(SCard->getSCardHandle(),
                                 dwAttrId,
                                 (LPBYTE)&pbAttr,
                                 &cByte);
        if ( SCARD_S_SUCCESS != lReturn )
        {
            if ( ERROR_NOT_SUPPORTED == lReturn )
                attribTex += "Value not supported";
            else
            {
                // Some other error occurred.
                attribTex += "Failed SCardGetAttrib";
                return false;
            }
        }
        else
        {
            // Output the bytes.
            for (i = 0; i < cByte; i++)
            {
                attribTex += QString("%1").arg(*(pbAttr+i), 2, 16, QChar('0')).toUpper();
                attribTex += " ";
            }




            // Free the memory when done.
            // hContext was set earlier by SCardEstablishContext
            SCardFreeMemory( SCard->getSCardContext(), pbAttr );
        }

        attribTex += "\n";

//---------------------------------

        pbAttr = nullptr;
        dwAttrId = SCARD_ATTR_CURRENT_W;
        cByte = SCARD_AUTOALLOCATE;
        attribTex += "CURRENT_W:\t\t\t";

        lReturn = SCardGetAttrib(SCard->getSCardHandle(),
                                 dwAttrId,
                                 (LPBYTE)&pbAttr,
                                 &cByte);
        if ( SCARD_S_SUCCESS != lReturn )
        {
            if ( ERROR_NOT_SUPPORTED == lReturn )
                attribTex += "Value not supported";
            else
            {
                // Some other error occurred.
                attribTex += "Failed SCardGetAttrib";
                return false;
            }
        }
        else
        {
            // Output the bytes.
            for (i = 0; i < cByte; i++)
            {
                attribTex += QString("%1").arg(*(pbAttr+i), 2, 16, QChar('0')).toUpper();
                attribTex += " ";
            }




            // Free the memory when done.
            // hContext was set earlier by SCardEstablishContext
            SCardFreeMemory( SCard->getSCardContext(), pbAttr );
        }

        attribTex += "\n";

//---------------------------------

        pbAttr = nullptr;
        dwAttrId = SCARD_ATTR_DEFAULT_CLK;
        cByte = SCARD_AUTOALLOCATE;
        attribTex += "DEFAULT_CLK:\t\t";

        lReturn = SCardGetAttrib(SCard->getSCardHandle(),
                                 dwAttrId,
                                 (LPBYTE)&pbAttr,
                                 &cByte);
        if ( SCARD_S_SUCCESS != lReturn )
        {
            if ( ERROR_NOT_SUPPORTED == lReturn )
                attribTex += "Value not supported";
            else
            {
                // Some other error occurred.
                attribTex += "Failed SCardGetAttrib";
                return false;
            }
        }
        else
        {
            // Output the bytes.
            for (i = 0; i < cByte; i++)
            {
                attribTex += QString("%1").arg(*(pbAttr+i), 2, 16, QChar('0')).toUpper();
                attribTex += " ";
            }




            // Free the memory when done.
            // hContext was set earlier by SCardEstablishContext
            SCardFreeMemory( SCard->getSCardContext(), pbAttr );
        }

        attribTex += "\n";

//---------------------------------


        pbAttr = nullptr;
        dwAttrId = SCARD_ATTR_DEFAULT_DATA_RATE;
        cByte = SCARD_AUTOALLOCATE;
        attribTex += "DEFAULT_DATA_RATE:\t\t";

        lReturn = SCardGetAttrib(SCard->getSCardHandle(),
                                 dwAttrId,
                                 (LPBYTE)&pbAttr,
                                 &cByte);
        if ( SCARD_S_SUCCESS != lReturn )
        {
            if ( ERROR_NOT_SUPPORTED == lReturn )
                attribTex += "Value not supported";
            else
            {
                // Some other error occurred.
                attribTex += "Failed SCardGetAttrib";
                return false;
            }
        }
        else
        {
            // Output the bytes.
            for (i = 0; i < cByte; i++)
            {
                attribTex += QString("%1").arg(*(pbAttr+i), 2, 16, QChar('0')).toUpper();
                attribTex += " ";
            }




            // Free the memory when done.
            // hContext was set earlier by SCardEstablishContext
            SCardFreeMemory( SCard->getSCardContext(), pbAttr );
        }

        attribTex += "\n";

//---------------------------------

        pbAttr = nullptr;
        dwAttrId = SCARD_ATTR_DEVICE_FRIENDLY_NAME;
        cByte = SCARD_AUTOALLOCATE;
        attribTex += "DEVICE_FRIENDLY_NAME:\t\t";

        lReturn = SCardGetAttrib(SCard->getSCardHandle(),
                                 dwAttrId,
                                 (LPBYTE)&pbAttr,
                                 &cByte);
        if ( SCARD_S_SUCCESS != lReturn )
        {
            if ( ERROR_NOT_SUPPORTED == lReturn )
                attribTex += "Value not supported";
            else
            {
                // Some other error occurred.
                attribTex += "Failed SCardGetAttrib";
                return false;
            }
        }
        else
        {
            for (i = 0; i < cByte; i++)
            {
                attribTex += QString(*(pbAttr + i));
            }

            // Free the memory when done.
            // hContext was set earlier by SCardEstablishContext
            SCardFreeMemory( SCard->getSCardContext(), pbAttr );
        }

        attribTex += "\n";

//---------------------------------

        pbAttr = nullptr;
        dwAttrId = SCARD_ATTR_DEVICE_IN_USE;
        cByte = SCARD_AUTOALLOCATE;
        attribTex += "DEVICE_IN_USE:\t\t";

        lReturn = SCardGetAttrib(SCard->getSCardHandle(),
                                 dwAttrId,
                                 (LPBYTE)&pbAttr,
                                 &cByte);
        if ( SCARD_S_SUCCESS != lReturn )
        {
            if ( ERROR_NOT_SUPPORTED == lReturn )
                attribTex += "Value not supported";
            else
            {
                // Some other error occurred.
                attribTex += "Failed SCardGetAttrib";
                return false;
            }
        }
        else
        {
            // Output the bytes.
            for (i = 0; i < cByte; i++)
            {
                attribTex += QString("%1").arg(*(pbAttr+i), 2, 16, QChar('0')).toUpper();
                attribTex += " ";
            }




            // Free the memory when done.
            // hContext was set earlier by SCardEstablishContext
            SCardFreeMemory( SCard->getSCardContext(), pbAttr );
        }

        attribTex += "\n";

//---------------------------------

        pbAttr = nullptr;
        dwAttrId = SCARD_ATTR_DEVICE_UNIT;
        cByte = SCARD_AUTOALLOCATE;
        attribTex += "DEVICE_UNIT:\t\t";

        lReturn = SCardGetAttrib(SCard->getSCardHandle(),
                                 dwAttrId,
                                 (LPBYTE)&pbAttr,
                                 &cByte);
        if ( SCARD_S_SUCCESS != lReturn )
        {
            if ( ERROR_NOT_SUPPORTED == lReturn )
                attribTex += "Value not supported";
            else
            {
                // Some other error occurred.
                attribTex += "Failed SCardGetAttrib";
                return false;
            }
        }
        else
        {
            // Output the bytes.
            for (i = 0; i < cByte; i++)
            {
                attribTex += QString("%1").arg(*(pbAttr+i), 2, 16, QChar('0')).toUpper();
                attribTex += " ";
            }




            // Free the memory when done.
            // hContext was set earlier by SCardEstablishContext
            SCardFreeMemory( SCard->getSCardContext(), pbAttr );
        }

        attribTex += "\n";

//---------------------------------

        pbAttr = nullptr;
        dwAttrId = SCARD_ATTR_ICC_INTERFACE_STATUS;
        cByte = SCARD_AUTOALLOCATE;
        attribTex += "ICC_INTERFACE_STATUS:\t\t";

        lReturn = SCardGetAttrib(SCard->getSCardHandle(),
                                 dwAttrId,
                                 (LPBYTE)&pbAttr,
                                 &cByte);
        if ( SCARD_S_SUCCESS != lReturn )
        {
            if ( ERROR_NOT_SUPPORTED == lReturn )
                attribTex += "Value not supported";
            else
            {
                // Some other error occurred.
                attribTex += "Failed SCardGetAttrib";
                return false;
            }
        }
        else
        {
            // Output the bytes.
            for (i = 0; i < cByte; i++)
            {
                attribTex += QString("%1").arg(*(pbAttr+i), 2, 16, QChar('0')).toUpper();
                attribTex += " ";
            }




            // Free the memory when done.
            // hContext was set earlier by SCardEstablishContext
            SCardFreeMemory( SCard->getSCardContext(), pbAttr );
        }

        attribTex += "\n";

//---------------------------------

        pbAttr = nullptr;
        dwAttrId = SCARD_ATTR_ICC_PRESENCE;
        cByte = SCARD_AUTOALLOCATE;
        attribTex += "ICC_PRESENCE:\t\t";

        lReturn = SCardGetAttrib(SCard->getSCardHandle(),
                                 dwAttrId,
                                 (LPBYTE)&pbAttr,
                                 &cByte);
        if ( SCARD_S_SUCCESS != lReturn )
        {
            if ( ERROR_NOT_SUPPORTED == lReturn )
                attribTex += "Value not supported";
            else
            {
                // Some other error occurred.
                attribTex += "Failed SCardGetAttrib";
                return false;
            }
        }
        else
        {
            // Output the bytes.
            for (i = 0; i < cByte; i++)
            {
                attribTex += QString("%1").arg(*(pbAttr+i), 2, 16, QChar('0')).toUpper();
                attribTex += " ";
            }




            // Free the memory when done.
            // hContext was set earlier by SCardEstablishContext
            SCardFreeMemory( SCard->getSCardContext(), pbAttr );
        }

        attribTex += "\n";

//---------------------------------

        pbAttr = nullptr;
        dwAttrId = SCARD_ATTR_ICC_TYPE_PER_ATR;
        cByte = SCARD_AUTOALLOCATE;
        attribTex += "ICC_TYPE_PER_ATR:\t\t";

        lReturn = SCardGetAttrib(SCard->getSCardHandle(),
                                 dwAttrId,
                                 (LPBYTE)&pbAttr,
                                 &cByte);
        if ( SCARD_S_SUCCESS != lReturn )
        {
            if ( ERROR_NOT_SUPPORTED == lReturn )
                attribTex += "Value not supported";
            else
            {
                // Some other error occurred.
                attribTex += "Failed SCardGetAttrib";
                return false;
            }
        }
        else
        {
            // Output the bytes.
            for (i = 0; i < cByte; i++)
            {
                attribTex += QString("%1").arg(*(pbAttr+i), 2, 16, QChar('0')).toUpper();
                attribTex += " ";
            }




            // Free the memory when done.
            // hContext was set earlier by SCardEstablishContext
            SCardFreeMemory( SCard->getSCardContext(), pbAttr );
        }

        attribTex += "\n";

//---------------------------------

        pbAttr = nullptr;
        dwAttrId = SCARD_ATTR_MAX_DATA_RATE;
        cByte = SCARD_AUTOALLOCATE;
        attribTex += "MAX_DATA_RATE:\t\t";

        lReturn = SCardGetAttrib(SCard->getSCardHandle(),
                                 dwAttrId,
                                 (LPBYTE)&pbAttr,
                                 &cByte);
        if ( SCARD_S_SUCCESS != lReturn )
        {
            if ( ERROR_NOT_SUPPORTED == lReturn )
                attribTex += "Value not supported";
            else
            {
                // Some other error occurred.
                attribTex += "Failed SCardGetAttrib";
                return false;
            }
        }
        else
        {
            // Output the bytes.
            for (i = 0; i < cByte; i++)
            {
                attribTex += QString("%1").arg(*(pbAttr+i), 2, 16, QChar('0')).toUpper();
                attribTex += " ";
            }




            // Free the memory when done.
            // hContext was set earlier by SCardEstablishContext
            SCardFreeMemory( SCard->getSCardContext(), pbAttr );
        }

        attribTex += "\n";

//---------------------------------

        pbAttr = nullptr;
        dwAttrId = SCARD_ATTR_MAX_IFSD;
        cByte = SCARD_AUTOALLOCATE;
        attribTex += "MAX_IFSD:\t\t\t";

        lReturn = SCardGetAttrib(SCard->getSCardHandle(),
                                 dwAttrId,
                                 (LPBYTE)&pbAttr,
                                 &cByte);
        if ( SCARD_S_SUCCESS != lReturn )
        {
            if ( ERROR_NOT_SUPPORTED == lReturn )
                attribTex += "Value not supported";
            else
            {
                // Some other error occurred.
                attribTex += "Failed SCardGetAttrib";
                return false;
            }
        }
        else
        {
            // Output the bytes.
            for (i = 0; i < cByte; i++)
            {
                attribTex += QString("%1").arg(*(pbAttr+i), 2, 16, QChar('0')).toUpper();
                attribTex += " ";
            }




            // Free the memory when done.
            // hContext was set earlier by SCardEstablishContext
            SCardFreeMemory( SCard->getSCardContext(), pbAttr );
        }

        attribTex += "\n";

//---------------------------------

        pbAttr = nullptr;
        dwAttrId = SCARD_ATTR_POWER_MGMT_SUPPORT;
        cByte = SCARD_AUTOALLOCATE;
        attribTex += "POWER_MGMT_SUPPORT:\t\t";

        lReturn = SCardGetAttrib(SCard->getSCardHandle(),
                                 dwAttrId,
                                 (LPBYTE)&pbAttr,
                                 &cByte);
        if ( SCARD_S_SUCCESS != lReturn )
        {
            if ( ERROR_NOT_SUPPORTED == lReturn )
                attribTex += "Value not supported";
            else
            {
                // Some other error occurred.
                attribTex += "Failed SCardGetAttrib";
                return false;
            }
        }
        else
        {
            // Output the bytes.
            for (i = 0; i < cByte; i++)
            {
                attribTex += QString("%1").arg(*(pbAttr+i), 2, 16, QChar('0')).toUpper();
                attribTex += " ";
            }




            // Free the memory when done.
            // hContext was set earlier by SCardEstablishContext
            SCardFreeMemory( SCard->getSCardContext(), pbAttr );
        }

        attribTex += "\n";

//---------------------------------

        pbAttr = nullptr;
        dwAttrId = SCARD_ATTR_PROTOCOL_TYPES;
        cByte = SCARD_AUTOALLOCATE;
        attribTex += "PROTOCOL_TYPES:\t\t";

        lReturn = SCardGetAttrib(SCard->getSCardHandle(),
                                 dwAttrId,
                                 (LPBYTE)&pbAttr,
                                 &cByte);
        if ( SCARD_S_SUCCESS != lReturn )
        {
            if ( ERROR_NOT_SUPPORTED == lReturn )
                attribTex += "Value not supported";
            else
            {
                // Some other error occurred.
                attribTex += "Failed SCardGetAttrib";
                return false;
            }
        }
        else
        {
            // Output the bytes.
            for (i = 0; i < cByte; i++)
            {
                attribTex += QString("%1").arg(*(pbAttr+i), 2, 16, QChar('0')).toUpper();
                attribTex += " ";
            }




            // Free the memory when done.
            // hContext was set earlier by SCardEstablishContext
            SCardFreeMemory( SCard->getSCardContext(), pbAttr );
        }

        attribTex += "\n";

//---------------------------------

        pbAttr = nullptr;
        dwAttrId = SCARD_ATTR_VENDOR_IFD_SERIAL_NO;
        cByte = SCARD_AUTOALLOCATE;
        attribTex += "VENDOR_IFD_SERIAL_NO:\t\t";

        lReturn = SCardGetAttrib(SCard->getSCardHandle(),
                                 dwAttrId,
                                 (LPBYTE)&pbAttr,
                                 &cByte);
        if ( SCARD_S_SUCCESS != lReturn )
        {
            if ( ERROR_NOT_SUPPORTED == lReturn )
                attribTex += "Value not supported";
            else
            {
                // Some other error occurred.
                attribTex += "Failed SCardGetAttrib";
                return false;
            }
        }
        else
        {

            for (i = 0; i < cByte; i++)
            {
                attribTex += QString(*(pbAttr + i));
            }

            // Free the memory when done.
            // hContext was set earlier by SCardEstablishContext
            SCardFreeMemory( SCard->getSCardContext(), pbAttr );
        }

        attribTex += "\n";

//---------------------------------

        pbAttr = nullptr;
        dwAttrId = SCARD_ATTR_VENDOR_IFD_TYPE;
        cByte = SCARD_AUTOALLOCATE;
        attribTex += "VENDOR_IFD_TYPE:\t\t";

        lReturn = SCardGetAttrib(SCard->getSCardHandle(),
                                 dwAttrId,
                                 (LPBYTE)&pbAttr,
                                 &cByte);
        if ( SCARD_S_SUCCESS != lReturn )
        {
            if ( ERROR_NOT_SUPPORTED == lReturn )
                attribTex += "Value not supported";
            else
            {
                // Some other error occurred.
                attribTex += "Failed SCardGetAttrib";
                return false;
            }
        }
        else
        {
            for (i = 0; i < cByte; i++)
            {
                attribTex += QString(*(pbAttr + i));
            }

            // Free the memory when done.
            // hContext was set earlier by SCardEstablishContext
            SCardFreeMemory( SCard->getSCardContext(), pbAttr );
        }

        attribTex += "\n";

//---------------------------------

        pbAttr = nullptr;
        dwAttrId = SCARD_ATTR_VENDOR_IFD_VERSION;
        cByte = SCARD_AUTOALLOCATE;
        attribTex += "VENDOR_IFD_VERSION:\t\t";

        lReturn = SCardGetAttrib(SCard->getSCardHandle(),
                                 dwAttrId,
                                 (LPBYTE)&pbAttr,
                                 &cByte);
        if ( SCARD_S_SUCCESS != lReturn )
        {
            if ( ERROR_NOT_SUPPORTED == lReturn )
                attribTex += "Value not supported";
            else
            {
                // Some other error occurred.
                attribTex += "Failed SCardGetAttrib";
                return false;
            }
        }
        else
        {

            // Output the bytes.
            for (i = 0; i < cByte; i++)
            {
                attribTex += QString("%1").arg(*(pbAttr+i), 2, 16, QChar('0')).toUpper();
                attribTex += " ";
            }

            // Free the memory when done.
            // hContext was set earlier by SCardEstablishContext
            SCardFreeMemory( SCard->getSCardContext(), pbAttr );
        }

        attribTex += "\n";

//---------------------------------

        pbAttr = nullptr;
        dwAttrId = SCARD_ATTR_VENDOR_NAME;
        cByte = SCARD_AUTOALLOCATE;
        attribTex += "VENDOR_NAME:\t\t";

        lReturn = SCardGetAttrib(SCard->getSCardHandle(),
                                 dwAttrId,
                                 (LPBYTE)&pbAttr,
                                 &cByte);
        if ( SCARD_S_SUCCESS != lReturn )
        {
            if ( ERROR_NOT_SUPPORTED == lReturn )
                attribTex += "Value not supported";
            else
            {
                // Some other error occurred.
                attribTex += "Failed SCardGetAttrib";
                return false;
            }
        }
        else
        {
            for (i = 0; i < cByte; i++)
            {
                attribTex += QString(*(pbAttr + i));
            }

            // Free the memory when done.
            // hContext was set earlier by SCardEstablishContext
            SCardFreeMemory( SCard->getSCardContext(), pbAttr );
        }

        attribTex += "\n";

//---------------------------------

        ui->textEditDevAttrib->append(attribTex);

    }
    else
    {
        return false;
    }

    ui->textEditDevAttrib->append("\n\n\n");
#endif
    return true;
}
