#include <Arduino.h>

struct ErrorCode {
    int key;
    String message;
};

ErrorCode ErrorCodes[] = {
    {411, "Unbekannter ESP Datenkey erhalten"},
    {412, "Unbekannter Slave 2 Datenkey erhalten"},
    {413, "Unbekannter Slave 3 Datenkey erhalten"},
    {414, "Unbekannter Slave 4 Datenkey erhalten"},
    {415, "Unbekannter Slave 5 Datenkey erhalten"},
};

String getErrorMessage(int errorCode) {
    for (int i = 0; i < sizeof(ErrorCodes); i++) {
        if (ErrorCodes[i].key == errorCode) {
            return ErrorCodes[i].message;
        }
    }
    return "Errorcode not found: " + errorCode;
}