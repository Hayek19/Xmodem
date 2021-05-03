#include <iostream>
#include "pomoc.h"
using namespace std;

int liczenie_CRC(char *wsk, int count) {
    int suma_kontrolna_CRC = 0;
    while (--count >= 0)
    {   //dopełnienie znaku ośmioma zerami
        suma_kontrolna_CRC = suma_kontrolna_CRC ^ (int)*wsk++ << 8;
        for (int i = 0; i < 8; ++i)
// Jeśli lewy bit = 1 to wykonujemy operacje XOR generatorem 1021
// Jeśli lewy bit = 0 to wykonujemy operacje XOR generatorem 0000

            if (suma_kontrolna_CRC & 0x8000) suma_kontrolna_CRC = suma_kontrolna_CRC << 1 ^ 0x1021;
            else suma_kontrolna_CRC = suma_kontrolna_CRC << 1;
    }
    return (suma_kontrolna_CRC & 0xFFFF); //1111 1111 1111 1111
}

int parzystosc(int a, int b) {
    if( b == 0 ) return 1;
    if( b == 1 ) return a;

    int wynik = a;
    for( int i = 2; i <= b; i++ ) wynik = wynik * a;

    return wynik;
}

// Funkcja przeliczająca CRC na postać binarną
char licz_CRC_znaku(int n, int nr_znaku)
{
    int x, tab_bin[16];
    for( int z = 0; z<16; z++ ) tab_bin[z] = 0;

    for( int i = 0; i<16; i++ ) {
        x = n % 2;
        if( x == 1 ) n = (n-1)/2;
        if( x == 0 ) n = n/2;
       tab_bin[15-i] = x;
    }

    //obliczamy poszczegolne znaki suma_kontrolna_CRC (1-szy lub 2-gi)
    x = 0;
    int k;
    if( nr_znaku == 1 ) k = 7;
    if( nr_znaku == 2 ) k = 15;

    for (int i=0; i<8; i++)
        x = x + parzystosc(2,i) * tab_bin[k-i];

    return (char)x;//zwraca 1 lub 2 znaki
}

int main()
{
    cout << "NADAJNIK - wysylanie wiadomosci" << endl;
    cout<<"Autorzy : \n"
        "Bartosz Durys - 229869\n"
        "Filip Hajek - 229891\n";
    ifstream plik;
    char znak;
    int ile_znakow = 1;
    unsigned long rozmiar_znaku = sizeof(znak);
    int kod;
    bool transmisja = false;
    bool poprawny_pakiet;
    int nr_bloku = 1;
    char blok[128];    // Plik jest podzielony na bloki o długości 128 bajtów.

// Otwieranie portu aby zaczac transmisje
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
        default: { cout<<"Wybrano niepoprawny port, sprobuj ponownie "<<endl; break; }
    }
    cout << "Rozpoczynanie przesylania wiadomosci z portu "<<nr_portu<<" prosze czekac "<<endl;
    if(ustawienia_portu(nr_portu) == false )  return 0;

    cout << "Oczekiwanie na zgode transmisji wiadomosci ze strony odbiorcy"<<endl;
    for(int i=0;i<6;i++) {
        ReadFile(id_portu, &znak, ile_znakow, &rozmiar_znaku, NULL);

        if(znak=='C') {
            cout << "Wszystko poprawnie" <<endl;
            kod = 1;
            transmisja = true;
            break;
        }
        else if( znak == NAK ){
            cout  << "Wystapil blad, sprobuj ponownie!"<<endl;
            kod = 2;
            transmisja = true;
            break;
        }
    }
    if(!transmisja) exit(1);

// Ponizej nastepuje transmisja wiadomosci
    plik.open("plik.txt",ios::binary);
    while(!plik.eof())
    {
        //Pusta tablica do zapisywania danych z jednego bloku
        for( int i = 0; i < 128; i++ ){
            blok[i] = (char)26;
        }
        int s = 0;
        while( s<128 && !plik.eof() ) {
            blok[s] = plik.get();
            if(plik.eof()) blok[s] = (char)26;
            s++;
        }
        poprawny_pakiet = false;

        while(!poprawny_pakiet)
        {
            cout << "Trwa wysylanie pakietu  "<<endl;

            cout  << nr_bloku <<endl;
            // Wysłanie znaku początku nagłówka
            WriteFile(id_portu, &SOH,ile_znakow,&rozmiar_znaku, NULL);
            znak=(char)nr_bloku;
            // Wysłanie numeru bloku (1 bajt)
            WriteFile(id_portu, &znak,ile_znakow,&rozmiar_znaku, NULL);
            znak=(char)255-nr_bloku;
            // Wysłanie dopełnienia (255 - blok)
            WriteFile(id_portu, &znak,ile_znakow,&rozmiar_znaku, NULL);


            for( int i=0; i<128; i++ ){
                WriteFile(id_portu, &blok[i],ile_znakow,&rozmiar_znaku, NULL);
            }
            if( kod == 2 ) //suma kontrolna CRC
            {
                char suma_kontrolna=(char)26;
                for(int i=0;i<128;i++)
                suma_kontrolna+=blok[i]%256;
                WriteFile(id_portu, &suma_kontrolna,ile_znakow,&rozmiar_znaku, NULL);
                cout<<" Suma kontrolna  = " << (int)suma_kontrolna << endl;
            }
            else if(kod==1) //obliczanie CRC i transfer
            {
                tmpCRC=liczenie_CRC(blok,128);
                znak=licz_CRC_znaku(tmpCRC,1);
                WriteFile(id_portu,&znak,ile_znakow,&rozmiar_znaku, NULL);
                znak=licz_CRC_znaku(tmpCRC,2);
                WriteFile(id_portu,&znak,ile_znakow,&rozmiar_znaku, NULL);
            }

            while(1) {
                znak=' ';
                ReadFile(id_portu,&znak,ile_znakow,&rozmiar_znaku, NULL);

                if(znak == ACK) {
                    poprawny_pakiet=true;
                    cout<<" Pakiet zostal przeslany poprawnie, mozna odczytac wiadomosc"<<endl;
                    break;
                }

                if(znak == NAK) {
                    cout  << " Odbiorca odrzucil wiadomosc :( " <<endl;
                       return 0;

                }

                if(znak == CAN) {
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
        znak = EOT;
        WriteFile(id_portu,&znak,ile_znakow,&rozmiar_znaku, NULL);
        ReadFile(id_portu,&znak,ile_znakow,&rozmiar_znaku, NULL);
        if(znak == ACK) break;
    }
    CloseHandle(id_portu);
    cout  << "Poprawnie przeslano wiadomosc!"<<endl;

    return 0;
}
