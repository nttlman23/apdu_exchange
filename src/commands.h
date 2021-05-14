#ifndef COMMANDS_H
#define COMMANDS_H

#include <stdint.h>

#define INDEX_DEVICE_1          0
#define INDEX_DEVICE_2          1

#define APDU_BUFFER_SIZE                1024

#define BLOCK_SIZE                      0x200

#define CLA_INDEX                       0
#define INS_INDEX                       1
#define P1_INDEX                        2
#define P2_INDEX                        3
#define LEN_INDEX                       4
#define DATA_INDEX                      5

#define APDU_HEAD_LEN                   5

#define APDU_CMD_CLA                    0x80
#define APDU_CMD_INS                    0x72
#define APDU_CMD_INFO_P1                0x13
#define MAGIC_NUMBER                    0x9A79F06E

#define APDU_SELECT_LEN                 13
#define USB_SERIAL_NUMBER_SZ            16
#define HASH_K_E_SZ                     32

// Crypto Type
#define CRYPTO_TYPE_GOST_29147_89       0x00
#define CRYPTO_TYPE_GOST_29147_FAST     0x01

#endif // COMMANDS_H
