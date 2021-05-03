#include <iostream>
#include "pomoc.h"
#include "funkcje.h"
using namespace std;

ifstream plik;
char znak;
const char ile_znakow = 1;
unsigned long rozmiar_znaku = sizeof(znak);
int kod, s;
bool transmisja = false;
bool poprawny_pakiet;
int nr_bloku = 1;
char blok[128];    // Plik jest podzielony na bloki o długości 128 bajtów.

int main()
{
    cout << "NADAJNIK - wysylanie wiadomosci" << endl;
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

    cout << "Rozpoczynanie przesylania wiadomosci z portu "<<nr_portu<<" prosze czekac "<<endl;
    if(ustawienia_portu(nr_portu) == false ) return 0; // Otwieranie portu aby zaczac transmisje

    cout << "Oczekiwanie na zgode transmisji wiadomosci ze strony odbiorcy"<<endl;
    for(int i=0;i<6;i++) {
        ReadFile(id_portu, &znak, ile_znakow, &rozmiar_znaku, NULL);

        if(znak=='C') {
            cout << "Rozpoczecie nadawania z CRC" <<endl;
            kod = 1;
            transmisja = true;
            break;
        }
        else if( znak == NAK ){
            cout  << "Rozpoczecie nadawania z suma kontrolna"<<endl;
            kod = 2;
            transmisja = true;
            break;
        }
    }
    if(!transmisja) exit(1);

    // Ponizej nastepuje transmisja wiadomosci
    plik.open("wysylana.txt", ios::binary);
    while(!plik.eof())
    {
        for(int i = 0; i < 128; i++){
            blok[i] = (char)26;     //Pusta tablica do zapisywania danych z jednego bloku
        }
        s = 0;
        while(s<128 && !plik.eof()) {
            blok[s] = plik.get();
            if(plik.eof()) blok[s] = (char)26;
            s++;
        }
        poprawny_pakiet = false;

        while(!poprawny_pakiet)
        {
            cout << "Trwa wysylanie pakietu  "<< endl;
            cout << "Nr bloku: " << nr_bloku << endl;

            WriteFile(id_portu, &SOH,ile_znakow,&rozmiar_znaku, NULL); // Wyslanie znaku początku naglowka SOH
            znak=(char)nr_bloku;
            WriteFile(id_portu, &znak,ile_znakow,&rozmiar_znaku, NULL); // Wyslanie numeru bloku (1 bajt)
            znak=(char)255-nr_bloku;
            WriteFile(id_portu, &znak,ile_znakow,&rozmiar_znaku, NULL); // Wyslanie dopełnienia (255 - blok)

            for( int i=0; i<128; i++ ){
                WriteFile(id_portu, &blok[i],ile_znakow,&rozmiar_znaku, NULL); // Wysylanie wiadomosci
            }
            if(kod == 2) // suma kontrolna (checksum)
            {
                char suma_kontrolna=(char)26;
                for(int i=0; i<128; i++)
                    suma_kontrolna+=blok[i]%256;
                WriteFile(id_portu, &suma_kontrolna,ile_znakow,&rozmiar_znaku, NULL);
                cout<<"Suma kontrolna = " << (int)suma_kontrolna << endl;
            }
            else if(kod==1) //obliczanie CRC i transfer
            {
                tmpCRC=liczenie_CRC(blok, 128);
                znak=licz_CRC_znaku(tmpCRC, 1);
                WriteFile(id_portu,&znak,ile_znakow,&rozmiar_znaku, NULL);
                cout<<"Pierwszy bajt CRC = " << (int)znak << endl;
                znak=licz_CRC_znaku(tmpCRC, 2);
                WriteFile(id_portu,&znak,ile_znakow,&rozmiar_znaku, NULL);
                cout<<"Drugi bajt CRC = " << (int)znak << endl;
            }

            while(1) {
                znak=' ';
                ReadFile(id_portu,&znak,ile_znakow,&rozmiar_znaku, NULL);

                if(znak == ACK) {
                    poprawny_pakiet=true;
                    cout<< "Pakiet zostal przeslany poprawnie, mozna odczytac wiadomosc" <<endl;
                    break;
                }
                else if(znak == NAK) {
                    cout  << "Odbiorca odrzucil wiadomosc :( " <<endl;
                    break;
                }
                else if(znak == CAN) {
                    cout  << "Polaczenie przerwane!" <<endl;
                    return 1;
                }
            }
        }
        //Jeśli nr bloku przekroczy 255 -  zaczynamy inkrementować od nowa
        if( nr_bloku == 255 ){
             nr_bloku = 1;
        }
        else nr_bloku++;
    }
    plik.close();

    // Zakonczenie trasnmisji wiadomosci
    while(1) {
        znak = EOT; // Znak konca transmisji
        WriteFile(id_portu, &znak, ile_znakow, &rozmiar_znaku, NULL);
        ReadFile(id_portu, &znak, ile_znakow, &rozmiar_znaku, NULL);
        if(znak == ACK) break;
    }
    CloseHandle(id_portu);
    cout  << "Poprawnie przeslano wiadomosc!" <<endl;
    return 0;
}
