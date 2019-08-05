#include <stdio.h>
#include <stdint.h>
#include "SLABCP2112.h"

int main() {
    HID_SMBUS_STATUS status;
    DWORD numDevices = 0;
    WORD vid = 0x10C4;
    WORD pid = 0xEA90;
    status = HidSmbus_GetNumDevices(&numDevices, vid, pid);
    if (status != HID_SMBUS_SUCCESS) {
        printf("HidSmbus_GetNumDevices error: %d\n", status);
        return status;
    }
    printf("Number of connected devices: %lu\n", numDevices);

    HID_SMBUS_DEVICE device;
    status = HidSmbus_Open(&device, 0, vid, pid);
    if (status != HID_SMBUS_SUCCESS) {
        printf("HidSmbus_Open error: %d\n", status);
        return status;
    }

    DWORD bitRate;
    BYTE selfAddress;
    BOOL autoReadRespond;
    WORD writeTimeout;
    WORD readTimeout;
    BOOL sclLowTimeout;
    WORD transferRetries;

    status = HidSmbus_GetSmbusConfig(device, &bitRate, &selfAddress, &autoReadRespond, &writeTimeout, &readTimeout,
                                     &sclLowTimeout, &transferRetries);
    if (status != HID_SMBUS_SUCCESS) {
        printf("HidSmbus_GetSmbusConfig error: %d\n", status);
        return status;
    }

    printf("bitRate: %ld\n", bitRate);
    printf("selfAddress: %d\n", selfAddress);
    printf("autoReadRespond: %d\n", autoReadRespond);
    printf("writeTimeout: %d\n", writeTimeout);
    printf("readTimeout: %d\n", readTimeout);
    printf("sclLowTimeout: %d\n", sclLowTimeout);
    printf("transferRetries: %d\n", transferRetries);
    printf("\n");

    autoReadRespond = TRUE;
    bitRate = 400000;

    status = HidSmbus_SetSmbusConfig(device, bitRate, selfAddress, autoReadRespond, writeTimeout, readTimeout,
                                     sclLowTimeout, transferRetries);
    if (status != HID_SMBUS_SUCCESS) {
        printf("HidSmbus_SetSmbusConfig error: %d\n", status);
        return status;
    }


    BYTE slaveAddress = 0xD0;
    uint32_t number = 0;
    uint32_t slaveTime = 0;
    for (int i = 0; i < 200; i++) {
        number++;
        printf("Writing data to I2C Slave: %u, size: %d\n", number, (BYTE) sizeof(number));
        status = HidSmbus_WriteRequest(device, slaveAddress, (BYTE *) &number, (BYTE) sizeof(number));
        if (status != HID_SMBUS_SUCCESS) {
            printf("HidSmbus_WriteRequest error: %d\n", status);
            break;
        }
        status = HidSmbus_TransferStatusRequest(device);
        if (status != HID_SMBUS_SUCCESS) {
            printf("HidSmbus_TransferStatusRequest error: %d\n", status);
            break;
        }
        HID_SMBUS_S0 s0;
        HID_SMBUS_S1 detailedStatus;
        WORD numRetries = 0;
        WORD bytesRead = 0;
        status = HidSmbus_GetTransferStatusResponse(device, &s0, &detailedStatus, &numRetries, &bytesRead);
        if (status != HID_SMBUS_SUCCESS) {
            printf("HidSmbus_GetTransferStatusResponse error: %d\n", status);
            break;
        }
        printf("Transfer Status: %d\n", s0);
        printf("Transfer Detailed Status: %d\n", detailedStatus);
        printf("numRetries: %d\n", numRetries);
        printf("bytesRead: %d\n", bytesRead);

        status = HidSmbus_ReadRequest(device, slaveAddress, (BYTE) sizeof(slaveTime));
        if (status != HID_SMBUS_SUCCESS) {
            printf("HidSmbus_ReadRequest error: %d\n", status);
            break;
        }
        printf("\n");

        BYTE readBuffer[64];
        BYTE totalRead = 0;
        BYTE nRead = 0;
        while (totalRead < (BYTE) sizeof(slaveTime)) {
            status = HidSmbus_GetReadResponse(device, &s0, readBuffer + totalRead, sizeof(readBuffer) - totalRead,
                                              &nRead);
            if (status != HID_SMBUS_SUCCESS) {
                printf("HidSmbus_GetReadResponse error: %d\n", status);
                break;
            }
            totalRead += nRead;
        }
        slaveTime = *(uint32_t *) readBuffer;
        printf("Slave time: %u\n", slaveTime);

        Sleep(500);
    }

    HidSmbus_Close(device);

    return 0;
}