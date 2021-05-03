#include <iostream>
#include "pomoc.h"
#include "funkcje.h"

using namespace std;

ofstream plik;
char blok_danych[128];
char znak;
unsigned long rozmiar_znaku= sizeof(znak);
const char ile_znakow = 1;
bool transmisja=false;
bool poprawny_pakiet;
int nr_bloku;
bool CRC;
char dopelnienieDo255;
char walidacja[2];

int main(){
    cout << "ODBIORNIK - odbieranie wiadomosci" << endl;
    cout<<"Autorzy : \n"
        "Bartosz Durys - 229869\n"
        "Filip Hajek - 229891\n";
    cout << "Wybierz numer portu:" << endl
    << "1- COM1\n"
    "2- COM2\n"
    "3- COM3\n"
    "4- COM4\n" << endl;
     switch(getchar()) {
        case '1':
        {
            nr_portu = "COM1"; break;
        }
        case '2':
        {
            nr_portu = "COM2"; break;
        }
        case '3':
        {
             nr_portu = "COM3"; break;
         }
        case '4':
        {
            nr_portu = "COM4"; break;
        }
        default: { cout<<"Wybrano niepoprawny port, sprobuj ponownie "<<endl; return 0; }
    }

    if(ustawienia_portu(nr_portu) == false) return 0;

    cout << "Wybierz metode sprawdzania poprawnosci?" << endl
        << "1: CRC" << endl
        << "2: Suma kontrolna (checksum)" << endl;

    getchar();

    switch(getchar()) {
        case '1': {
            znak = 'C';
            CRC = true;
            break;
        }
        case '2': {
            znak = NAK;
            CRC = false;
            break;
        }
        default: {
           cout<<"Wybrano niepoprawna opcje"<<endl;
           return 0;
        }
    }

    for(int i=0;i<6;i++) {
        //WriteFile(HANDLE, LPVOID, DWORD, LPDWORD, LPOVERLAPPED)
        WriteFile(id_portu, &znak, ile_znakow, &rozmiar_znaku, NULL);
        cout<<"Oczekiwanie na komunikat SOH"<<endl;
        ReadFile(id_portu, &znak, ile_znakow, &rozmiar_znaku, NULL);
        cout << "Znak: " << (int)znak << endl;
        if(znak==SOH) {
            cout << "Ustanowienie polaczenia powiodlo sie!"<< endl;
            transmisja = true;
            break;
        }
    }
    if(!transmisja) {
        cout << "Polaczenie nieudane."<< endl;
        return 1;
    }

    plik.open("odebrana.txt", ios::binary);
    cout<<"Trwa odbieranie pliku"<<endl;

    ReadFile(id_portu, &znak,ile_znakow,&rozmiar_znaku, NULL);
    nr_bloku=(int)znak;

    ReadFile(id_portu, &znak,ile_znakow,&rozmiar_znaku, NULL);
    dopelnienieDo255=znak;

    for(int i=0;i<128;i++) {
        ReadFile(id_portu, &znak,ile_znakow,&rozmiar_znaku, NULL);
        blok_danych[i] = znak;
    }

    if(CRC) {
        ReadFile(id_portu, &znak,ile_znakow,&rozmiar_znaku, NULL);
        walidacja[0]=znak;
        ReadFile(id_portu, &znak,ile_znakow,&rozmiar_znaku, NULL);
        walidacja[1]=znak;
    }
    else {
        ReadFile(id_portu, &znak,ile_znakow,&rozmiar_znaku, NULL);
        walidacja[0]=znak;
    }
    poprawny_pakiet=true;


    if ((char)(255-nr_bloku)!=dopelnienieDo255) {
        cout << "Niepoprawny numer pakietu"<<endl;
        WriteFile(id_portu, &NAK,ile_znakow,&rozmiar_znaku, NULL);
        poprawny_pakiet = false;
    }
    else if(CRC) {
        tmpCRC=liczenie_CRC(blok_danych,128); // sprawdzanie czy sumy kontrole sa poprawne

        int CRC_1 = licz_CRC_znaku(tmpCRC,1);
        int CRC_2 = licz_CRC_znaku(tmpCRC,2);

        if(CRC_1 != walidacja[0] || CRC_2 != walidacja[1]) {
            cout << "Nieprawidlowa wartosc CRC!" << endl;
            cout << "Bajt CRC nr 1: " << CRC_1 << endl;
            cout << "Bajt CRC nr 2: " << CRC_2 << endl;
            WriteFile(id_portu, &NAK,ile_znakow,&rozmiar_znaku, NULL); //NAK
            poprawny_pakiet=false;
        }
    }
    else {
        char suma_kontrolna=(char)26;
        for(int i=0; i<128; i++)
            suma_kontrolna+=blok_danych[i]%256;
        if(suma_kontrolna!=walidacja[0]) {
            cout << "Nieprawidlowa wartosc sumy kontrolnej:"<< suma_kontrolna << endl;
            WriteFile(id_portu, &NAK,ile_znakow,&rozmiar_znaku, NULL); //NAK
            poprawny_pakiet=false;
        }
    }

    if(poprawny_pakiet) {
        for(int i=0;i<128;i++) {
            if(blok_danych[i]!=26) plik<<blok_danych[i];
        }
        cout << "Odbieranie pakietu: " <<nr_bloku << endl;
        cout << "Pakiet odebrany poprawnie."<< endl;
        WriteFile(id_portu, &ACK, ile_znakow, &rozmiar_znaku, NULL);
    }

    while(1) {
        ReadFile(id_portu, &znak,ile_znakow,&rozmiar_znaku, NULL);
        if(znak==EOT || znak==CAN) break;
        cout << "Odbieranie pakietu" << endl;
        cout << nr_bloku+1 << endl;

        ReadFile(id_portu, &znak,ile_znakow,&rozmiar_znaku, NULL);
        nr_bloku=(int)znak;

        ReadFile(id_portu, &znak,ile_znakow,&rozmiar_znaku, NULL);
        dopelnienieDo255=znak;

        for(int i=0;i<128;i++) {
            ReadFile(id_portu, &znak,ile_znakow,&rozmiar_znaku, NULL);
            blok_danych[i] = znak;
        }


        if(CRC) {
            ReadFile(id_portu, &znak,ile_znakow,&rozmiar_znaku, NULL);
            walidacja[0]=znak;
            ReadFile(id_portu, &znak,ile_znakow,&rozmiar_znaku, NULL);
            walidacja[1]=znak;
        }
        else {
            ReadFile(id_portu, &znak,ile_znakow,&rozmiar_znaku, NULL);
            walidacja[0]=znak;
        }
        poprawny_pakiet=true;

        if((char)(255-nr_bloku)!=dopelnienieDo255) {
            cout  << "Niepoprawny numer pakietu!"<< endl;
            WriteFile(id_portu, &NAK,ile_znakow,&rozmiar_znaku, NULL);
            poprawny_pakiet=false;
        }
        else if(CRC) {
            tmpCRC=liczenie_CRC(blok_danych,128); // sprawdzanie czy sumy kontrole sa poprawne

            int CRC_1 = licz_CRC_znaku(tmpCRC,1);
            int CRC_2 = licz_CRC_znaku(tmpCRC,2);

            if(CRC_1 != walidacja[0] || CRC_2 != walidacja[1]) {
                cout << "Nieprawidlowa wartosc CRC!" << endl;
                cout << "Bajt CRC nr 1: " << CRC_1 << endl;
                cout << "Bajt CRC nr 2: " << CRC_2 << endl;
                WriteFile(id_portu, &NAK,ile_znakow,&rozmiar_znaku, NULL); //NAK
                poprawny_pakiet=false;
            }
        }
        else {
            char suma_kontrolna=(char)26;
            for(int i=0; i<128; i++)
                suma_kontrolna+=blok_danych[i]%256;
            if(suma_kontrolna!=walidacja[0]) {
                cout << "Nieprawidlowa wartosc sumy kontrolnej:"<< suma_kontrolna << endl;
                WriteFile(id_portu, &NAK,ile_znakow,&rozmiar_znaku, NULL); //NAK
                poprawny_pakiet=false;
            }
        }
        if(poprawny_pakiet) {
            for(int i=0;i<128;i++) {
                if(blok_danych[i]!=26)
                plik<<blok_danych[i];
            }

            cout << " Pakiet odebrany poprawnie"<< endl;
            WriteFile(id_portu, &ACK,ile_znakow,&rozmiar_znaku, NULL);
        }
    }
    WriteFile(id_portu, &ACK,ile_znakow,&rozmiar_znaku, NULL);

    plik.close();
    CloseHandle(id_portu);

    if(znak == CAN) cout << "Blad transmisji - polaczenie zostalo przerwane."<< endl;
    else cout << "Transfer pliku zakonczony pomyslnie :)"<< endl;

 return 0;
}
