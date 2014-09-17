// ConsoleApplication1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

typedef unsigned char uint8_t;


/* -------------------------------------------------- Device Type -------------------------------------------------- */
#define DEVICE_TYPE_REMOTE_DIS         0x00  // Remote Display Device
#define DEVICE_TYPE_RF                 0x01  // RF Device
#define DEVICE_TYPE_CARD_READER        0x02  // Card Reader Device


/* ----------------------------------------------- Acknowledge Cmd ------------------------------------------------- */
#define CMD_ACK_SUCCESS                0x00  // receive cmd and execute success
#define CMD_ACK_FAILURE                0x01  // receive cmd but execute failure
#define CMD_ACK_RECEIVE                0x02  // receive cmd and checked ok

/* ----------------------------------------------- Card Cmd ------------------------------------------------------- */
#define CMD_CARD_CHECK_IN              0x10  // Add One Card Info to Remote-Display Info-Database
#define CMD_CARD_CHECK_OUT             0x11  // Remove One Card Info from Remote-Display Info-Database
#define CMD_CARD_QUERY                 0x12  // Query Specified Card Info from Remote-Display Info-Database

/* --------------------------------------------- Diagnose Cmd ----------------------------------------------------- */
#define CMD_HEART_BEAT                 0x20  // Heart-Beat Command


/* ----------------------------------------------- Cmd Frame Struct ----------------------------------------------- */

#pragma pack(1)
typedef struct
{
    uint8_t Hdr[3];                          // Frame Start Flag 0xA6 0xA6 0xA6
    uint8_t DstDev : 4;                      // Destination Device
    uint8_t SrcDev : 4;                      // Source Device
    uint8_t Seq;                             // Sequence Number
    uint8_t Cmd;                             // Command Type
    uint8_t DeskID;                          // The ID of desk
    uint8_t CardID;                          // The ID of Card(Menu)
    uint8_t Resv[3];                         // Reserved
    uint8_t Ecc;                             // XOR Checksum
}CMD_FRAME_t;

#define FSM_STATE_HDR                  0
#define FSM_STATE_DATA                 1
#define FSM_STATE_ECC                  2

#define CMD_HDR_BYTE                   (uint8_t)0xA6
#define CMD_HDR_LEN                    (uint8_t)3

#define FSM_RESET_ALL()                \
do                                     \
{                                      \
    state = FSM_STATE_HDR;             \
    i = 0;                             \
    xor = 0;                           \
    findit = 0;                        \
}while (0)

uint8_t CMD_Unpack(uint8_t data, CMD_FRAME_t* pFrame)
{
    static uint8_t buf[sizeof(CMD_FRAME_t)];
    static uint8_t state = FSM_STATE_HDR;
    static uint8_t i = 0;
    static uint8_t xor = 0;
    static uint8_t findit = 0;

    if (NULL == pFrame)
    {
        return 0;
    }

    switch (state)
    {
        /* ---------- Header Phase -------------- */
        case FSM_STATE_HDR:
        {
            if (data == CMD_HDR_BYTE)
            {
                buf[i++] = data;
                xor ^= data;
                if (i >= CMD_HDR_LEN)
                {
                    state = FSM_STATE_DATA;
                }
            }
            else
            {
                FSM_RESET_ALL();
            }

            break;
        }

        /* ---------- Data   Phase -------------- */
        case FSM_STATE_DATA:
        {
            buf[i++] = data;
            xor ^= data;
            if (i >= sizeof(CMD_FRAME_t) - 1)
            {
                state = FSM_STATE_ECC;
            }

            break;
        }

        /* ---------- XOR   Phase -------------- */
        case FSM_STATE_ECC:
        {
            buf[i++] = data;
            if (xor == data)
            {
                // Check OK!
                // copy data then reset all status value
                for (i = 0; i < sizeof(CMD_FRAME_t); i++)
                {
                  ((uint8_t*)pFrame)[i] = buf[i];
                }

                findit = 1;
            }
            else
            {
                // reset all statue value
                FSM_RESET_ALL();
            }

            break;
        }

        default:
        {
            // reset fsm to known status
            FSM_RESET_ALL();

            break;
        }
    }

    return (findit);
}

uint8_t CMD_Pack(CMD_FRAME_t* pFrame)
{
    uint8_t  i = 0;
    uint8_t  xor = 0;

    if (NULL == pFrame)
    {
        return 0;
    }

    // Fill Header Info
    for (i = 0; i < CMD_HDR_LEN; i++)
    {
        pFrame->Hdr[i] = CMD_HDR_BYTE;
        xor ^= CMD_HDR_BYTE;
    }

    // Calculate XOR value
    while (i < sizeof(CMD_FRAME_t)-1)
    {
        xor ^= pFrame->Hdr[i++];
    }

    pFrame->Ecc = xor;

    return (1);
}


static CMD_FRAME_t T1;
static CMD_FRAME_t T2;

int _tmain(int argc, _TCHAR* argv[])
{

    uint8_t buf[sizeof(CMD_FRAME_t)];

    T1.DstDev = 4;
    T1.SrcDev = 3;
    T1.Cmd    = CMD_CARD_CHECK_IN;

    if (!CMD_Pack(&T1))
    {
        while (1);
    }

    memcpy((void*)buf, (void*)&T1, sizeof(CMD_FRAME_t));

    for (size_t i = 0; i < sizeof(CMD_FRAME_t); i++)
    {
        if (CMD_Unpack(buf[i], &T1))
        {
            while (1);
        }
    }

    T2.Hdr[0] = CMD_HDR_BYTE;
    T2.Hdr[1] = CMD_HDR_BYTE;
    T2.Hdr[2] = CMD_HDR_BYTE+1;

    while (true)
    {

    }

    return 0;
}
