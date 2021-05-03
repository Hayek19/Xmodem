#ifndef FUNKCJE_H_INCLUDED
#define FUNKCJE_H_INCLUDED

int liczenie_CRC(char *wsk, int count) {
    int suma_kontrolna_CRC = 0;
    while (--count >= 0) {
        //dope³nienie znaku oœmioma zerami
        suma_kontrolna_CRC = suma_kontrolna_CRC ^ (int)*wsk++ << 8;

        // Jeœli lewy bit = 1 to wykonujemy operacje XOR generatorem 1021
        // Jeœli lewy bit = 0 to wykonujemy operacje XOR generatorem 0000
        for (int i = 0; i < 8; ++i){
            if (suma_kontrolna_CRC & 0x8000) suma_kontrolna_CRC = suma_kontrolna_CRC << 1 ^ 0x1021;
            else suma_kontrolna_CRC = suma_kontrolna_CRC << 1;
        }
    }
    return (suma_kontrolna_CRC & 0xFFFF); //1111 1111 1111 1111
}

int parzystosc(int a, int b) {
    if( b == 0 ) return 1;
    if( b == 1 ) return a;

    int wynik = a;
    for( int i = 2; i <= b; i++ ) {
        wynik = wynik * a;
    }
    return wynik;
}

// Funkcja przeliczaj¹ca CRC na postaæ binarn¹
char licz_CRC_znaku(int n, int nr_znaku)
{
    int x, tab_bin[16];
    for(int z = 0; z<16; z++) tab_bin[z] = 0;

    for(int i = 0; i<16; i++) {
        x = n % 2;
        if(x == 1) n = (n-1)/2;
        else if(x == 0) n = n/2;
        tab_bin[15-i] = x;
    }

    //obliczamy poszczegolne znaki suma_kontrolna_CRC (1-szy lub 2-gi)
    x = 0;
    int k;
    if(nr_znaku == 1) k = 7;
    else if(nr_znaku == 2) k = 15;

    for (int i=0; i<8; i++)
        x += parzystosc(2,i) * tab_bin[k-i];

    return (char)x; //zwraca pierwszy lub drugi znak
}

#endif // FUNKCJE_H_INCLUDED
