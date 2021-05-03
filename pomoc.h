#ifndef POMOC_H_INCLUDED
#define POMOC_H_INCLUDED
#include <fstream>
#include <windows.h>
#include <string.h>
#include <iostream>

using namespace std;

// Lista znaków sterujacych
const char SOH = 0x1;   // Naglowek wiadomosci
const char EOT = 0x4;   // Koniec transmisji
const char ACK = 0x6;   // Naglowek zgody przesylania danych
const char NAK = 0xF;   // Naglowek  braku zgody przesylania danych
const char CAN = 0x12;  // Przerwanie dzialania

// Lista infomacji o portach
HANDLE          id_portu;      // ID danego portu
LPCTSTR         nr_portu;      // Nazwa portu
DCB             sterownik;     // struktura kontroli portu szeregowego
COMSTAT         zasoby_portu;
DWORD           error;         // informacje o bledzie
COMMTIMEOUTS    czas;
USHORT tmpCRC;

// Uruchomienie portu i dodanie parametrów dla portu
bool ustawienia_portu( LPCTSTR nr_portu ) {
    id_portu = CreateFile(nr_portu, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (id_portu != INVALID_HANDLE_VALUE) {
        sterownik.DCBlength = sizeof(sterownik);
        GetCommState(id_portu, &sterownik);
        sterownik.BaudRate=CBR_9600;     // predkosc transmisji
        sterownik.Parity = NOPARITY;     // bez bitu parzystosci
        sterownik.StopBits = ONESTOPBIT; // ustawienie bitu stopu (jeden bit)
        sterownik.ByteSize = 8;          // liczba wysylanych bitow
        sterownik.fParity = TRUE;
        sterownik.fDtrControl = DTR_CONTROL_DISABLE; // Kontrola linii DTR: sygnal nieaktywny
        sterownik.fRtsControl = RTS_CONTROL_DISABLE; // Kontrola linii RTS: sygnal nieaktywny
        sterownik.fOutxCtsFlow = FALSE;
        sterownik.fOutxDsrFlow = FALSE;
        sterownik.fDsrSensitivity = FALSE;
        sterownik.fAbortOnError = FALSE;
        sterownik.fOutX = FALSE;
        sterownik.fInX = FALSE;
        sterownik.fErrorChar = FALSE;
        sterownik.fNull = FALSE;

        czas.ReadIntervalTimeout = 10000;
        czas.ReadTotalTimeoutMultiplier = 10000;
        czas.ReadTotalTimeoutConstant = 10000;
        czas.WriteTotalTimeoutMultiplier = 100;
        czas.WriteTotalTimeoutConstant = 100;

        SetCommState(id_portu, &sterownik);
        SetCommTimeouts(id_portu, &czas);
        ClearCommError(id_portu, &error ,&zasoby_portu);
        return true;
    }
    else {
        cout <<"Port " << nr_portu << " nie jest aktywny!"<<endl;
        return false;
    }
}
#endif // POMOC_H_INCLUDED
