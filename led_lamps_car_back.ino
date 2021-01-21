/*
   Author: Grad
   Date: 16.09.2020r
   ZADANIA:

*/

#include <Adafruit_NeoPixel.h>

#define ledCount 30 //ilosc diod w pasku
#define ledStripPin1t 10 //podlaczenie lewego paska tyl
#define ledStripPin2t 9 //podlaczenie prawego paska tyl
#define Brightness 255 // max 255 - jasnosc
#define dlugoscMigacza 7  //dlugosc weza migacza
#define opoznienieZmianyMigacza 20 //czas w [ms] po jakim ma sie zmieniac stan diod przy kierunkowskazach

//Sygnaly sterujace
#define sstop 1  // swiatlo stopu
#define swiatlaDzienTyl 5
#define swiatlaCofania 7
#define awaryjne 2 //swiatla awaryjne
#define kierunekPrawy 9
#define kierunekLewy 6
#define wlaczenieSystemu 4
#define wylaczanieSystemu 14
#define wylaczKierunki 12
#define wylaczStop 11
#define wylaczSwiatlaCofania 17
#define wylaczDzienTyl 15


Adafruit_NeoPixel strip1t(ledCount, ledStripPin1t, NEO_GRB + NEO_KHZ800); // tyl lewy
Adafruit_NeoPixel strip2t(ledCount, ledStripPin2t, NEO_GRB + NEO_KHZ800); //tyl prawy


String odebraneDane = ""; //Pusty ciąg odebranych danych
int aktualneZadanieKierunki = 0;
int aktualneDaneLampaTyl = 0; //dot STOPU, dzien, cofania
int odebraneDaneInt = 0;
int ktoryKierunek = 0;
int flagaTylDzien = 0;  // 1 oznacza ze sa dzien tyl
int flagaTylDzienAwaryjne = 0; // 1 oznacza, ze sa tyl dzien, 1 tu i 0 flagaTylDzien oznacza, ze zostaly wylaczone na czas kierunkow
int flagaTylStop = 0;  // 1 oznacza, ze tyl stop dziala
int flagaTylCofanie = 0; // 1 oznacza, ze tyl cofanie jest wlaczone
int flagaKierunek = 0; // 1 oznacza, ze Kierunki sa wlaczone
int flagaAwaryjne = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200); //inicjalizacji transmisji szeregowej - ustawic predkosc na !!!115200!!!
  strip1t.begin();
  strip2t.begin();
  strip1t.setBrightness(Brightness); //poziom intenyswnosci swiecenia
  strip2t.setBrightness(Brightness); //poziom intenyswnosci swiecenia
  clearLed12t();
}

void loop() {
  //#################### OBSLUGA POLECEN ODEBRANYCH OD PORTU SZEREGOWEGO ##################
  if (Serial.available() > 0) { //Jesli Arduino odebrało dane  sterujace
    odebraneDane = Serial.readStringUntil('\n');
    odebraneDaneInt = StoI_f(odebraneDane); // konwersja string na int

    switch (odebraneDaneInt) {
      case sstop:  // SWIATLO STOP - LAMPA TYL
        flagaTylStop = 1;
        swiatlo_stop();
        aktualneDaneLampaTyl = 1;
        break;

      case awaryjne: // SWIATLA AWARYJNE - OBA KIERUNKI SWIECA
        flagaAwaryjne = 1;
        flagaKierunek = 1;
        flagaTylDzien = 0; //wylaczamy tyl dzien, zeby sie nie pokrywaly, wlaczymy je wylaczajac kierunki
        if (flagaTylStop or flagaTylCofanie) { // JESLI JEST ZAPALONE SWIATLO COFANIA / SWIATLO STOP
          swiatla_awaryjne_tyl_stop_cofanie(); // wersja z kierunkami od polowy swiatla
          if (flagaTylCofanie == 1) aktualneDaneLampaTyl = 7;
          aktualneZadanieKierunki = 3;
        }
        else {// JESLI NIE JEST WLACZONE SWIATLO COFANIA / SWIATLO STOP
          swiatla_awaryjne(); // wersja z zwyklymi kierunkami
          aktualneZadanieKierunki = 2;
          aktualneDaneLampaTyl = 0;
        }
        break;

      case kierunekLewy:  // KIERUNEK LEWY
        ktoryKierunek = 6;
        flagaKierunek = 1;
        if (flagaTylStop or flagaTylCofanie) { // JESLI JEST ZAPALONE SWIATLO COFANIA / SWIATLO STOP
          kierunkowskazy_tyl_stop_cofanie(6);
          aktualneDaneLampaTyl = 61;
        }
        else { // JESLI NIE JEST WLACZONE SWIATLO COFANIA / SWIATLO STOP
          kierunkowskazy_tyl(6);
          aktualneDaneLampaTyl = 6;
          aktualneZadanieKierunki = 6;
        }
        break;

      case kierunekPrawy: // KIERUNEK PRAWY
        ktoryKierunek = 9;
        flagaKierunek = 1;
        if (flagaTylStop or flagaTylCofanie) { // JESLI JEST ZAPALONE SWIATLO COFANIA / SWIATLO STOP
          kierunkowskazy_tyl_stop_cofanie(9);
          aktualneDaneLampaTyl = 91;
        }
        else { // JESLI NIE JEST WLACZONE SWIATLO COFANIA / SWIATLO STOP
          kierunkowskazy_tyl(9);
          aktualneDaneLampaTyl = 9;
          aktualneZadanieKierunki = 9;
        }

        break;

      case wlaczenieSystemu: //  ANIMACJA URUCHAMIANIA SYSTEMU
        inicjalizacja_systemu();
        swiatlo_dzien_tyl();
        flagaTylDzien = 1;
        flagaTylDzienAwaryjne = 1;
        aktualneDaneLampaTyl = 5;
        break;

      case swiatlaDzienTyl: // SWIATLA DZIENNE - LAMPA TYL
        swiatlo_dzien_tyl();
        flagaTylDzien = 1;
        flagaTylDzienAwaryjne = 1;
        aktualneDaneLampaTyl = 5;
        break;

      case swiatlaCofania:  // SWIATLA COFANIA -  LAMPA TYL
        swiatlo_cofania(flagaTylDzien);
        flagaTylCofanie = 1;
        aktualneDaneLampaTyl = 7;
        break;


      case wylaczanieSystemu: // ANIMACJA WYLACZANIA SYSTEMU
        wylaczanie_systemu();
        flagaTylDzien = 0;
        flagaTylDzienAwaryjne = 0;
        aktualneDaneLampaTyl = 0;
        break;

      case wylaczKierunki: //WYLACZ AWARYJNE / WYLACZ KIERUNKI
        clearLed12t();
        flagaKierunek = 0;
        flagaAwaryjne = 0;
        aktualneZadanieKierunki = 12;
        if (flagaTylDzienAwaryjne == 1) flagaTylDzien = 1; // jesli przed wlaczeniem kierunkow, przy zapallonym stopie dzialala pozycja to ja spowrotem wlaczamy
        if (flagaTylStop == 1) aktualneDaneLampaTyl = 1;
        else if (flagaTylDzien == 1 && flagaTylStop == 0) aktualneDaneLampaTyl = 5; // jest pozycja dzien, ale nie ma stop
        else if (flagaTylStop == 1) aktualneDaneLampaTyl = 1; //
        if ( flagaTylCofanie == 1) aktualneDaneLampaTyl = 7;
        ktoryKierunek = 0;
        break;

      case wylaczStop: // WYLACZANIE STOPU - LAMPA TYL
        clearLed12t();
        flagaTylStop = 0;
        if (flagaTylDzien == 0) 
        {
          if(flagaTylCofanie == 1) aktualneDaneLampaTyl = 7;
        }
          else aktualneDaneLampaTyl = 0;   //WERSJA Z WYLACZANIEM SWIATEL DZIENNYCH5
        
        if (flagaTylDzien == 1) 
        {
          if(flagaTylCofanie == 1) aktualneDaneLampaTyl = 7;
        }
          else aktualneDaneLampaTyl = 5;
        
        if (flagaKierunek == 1)  { // po wylaczeniu stopu, jesli byl zapalony kierunek, to trzeba go spowrotem zapalic
          if(flagaTylCofanie == 0)
            {
            aktualneDaneLampaTyl = ktoryKierunek;
            aktualneZadanieKierunki = ktoryKierunek; 
            }
            else
            {
              aktualneDaneLampaTyl = (ktoryKierunek*10+1); // 6->61, 9->91
              aktualneZadanieKierunki = ktoryKierunek; 
            }
        }
        if (flagaAwaryjne == 1) {
          swiatla_awaryjne(); // wersja z zwyklymi kierunkami
          aktualneZadanieKierunki = 2;
          aktualneDaneLampaTyl = 0;
        }
        break;

      case wylaczSwiatlaCofania: //  WYLACZ SWIATLA COFANIA - LAMPA TYL - ZOSTAJE POZYCJA
        clearLed12t();
        flagaTylCofanie = 0;
        if (flagaTylDzien == 0) aktualneDaneLampaTyl = 0;
        else if (flagaTylStop == 1) aktualneDaneLampaTyl = 1;
        else aktualneDaneLampaTyl = 5;
        break;

      case wylaczDzienTyl: // WYLACZ SWIATLA DZIENNE - LAMPA TYL
        clearLed12t();
        flagaTylDzien = 0;
        flagaTylDzienAwaryjne = 0;
        flagaTylDzienAwaryjne = 0;
        aktualneDaneLampaTyl = 0;
        flagaTylDzienAwaryjne = 0;
        break;
      default:
        break;
    }

  }
  // ################ AKTUALNY STAN LAMP #########################
  if (aktualneZadanieKierunki == 2) swiatla_awaryjne();
  if (aktualneZadanieKierunki == 3)  swiatla_awaryjne_tyl_stop_cofanie();
  if (aktualneZadanieKierunki == 6) kierunkowskazy_tyl(6);
  if (aktualneZadanieKierunki == 9) kierunkowskazy_tyl(9);
  if (aktualneDaneLampaTyl == 1) {
    swiatlo_stop();
    if (flagaTylCofanie == 1) swiatlo_cofania(flagaTylDzien);
  }
  else if (aktualneDaneLampaTyl == 5) swiatlo_dzien_tyl();
  if (aktualneDaneLampaTyl == 15) ;
  if (aktualneDaneLampaTyl == 7) swiatlo_cofania(flagaTylDzien);
  if ( aktualneDaneLampaTyl == 91)kierunkowskazy_tyl_stop_cofanie(9);
  if ( aktualneDaneLampaTyl == 61)kierunkowskazy_tyl_stop_cofanie(6);
}

// ################ FUNKCJE REALIZUJACE STEROWANIE SWIATLAMI ###################

/// ############### KIERUNKOWSKAZY ################################
void swiatla_awaryjne() // OBYDWA KIERUNKOWSKAZY
{
  for ( int i = ledCount - 1; i >= 0; i--) {
    strip1t.setPixelColor(i, strip1t.Color(255, 155, 0));
    strip2t.setPixelColor(i, strip2t.Color(255, 155, 0));
    delay(opoznienieZmianyMigacza);

    if ( i < ledCount - dlugoscMigacza) { //gaszenie paska od poczatku
      strip1t.setPixelColor(i + dlugoscMigacza, strip1t.Color(0, 0, 0));
      strip2t.setPixelColor(i + dlugoscMigacza, strip2t.Color(0, 0, 0));
    }
    strip1t.show();
    strip2t.show();
  }
  for (int k = dlugoscMigacza; k >= 0 ; k--) { //gaszenie koncowki paska (znikanie)
    strip1t.setPixelColor(k, strip1t.Color(0, 0, 0));
    strip2t.setPixelColor(k, strip2t.Color(0, 0, 0));
    strip1t.show();
    strip2t.show();
    delay(opoznienieZmianyMigacza);
  }
}
void swiatla_awaryjne_tyl_stop_cofanie() {

  for ( int i = ledCount / 2; i >= 0; i--) {
    strip1t.setPixelColor(i, strip1t.Color(0, 0, 0));
    strip2t.setPixelColor(i, strip2t.Color(0, 0, 0));
    if ( i < ledCount / 2 - dlugoscMigacza + 1); //gaszenie paska od poczatku
  }
  for ( int i = ledCount / 2; i >= 0; i--) {
    strip1t.setPixelColor(i, strip1t.Color(255, 155, 0));
    strip2t.setPixelColor(i, strip2t.Color(255, 155, 0));
    delay(opoznienieZmianyMigacza + 10);
    if ( i < ledCount / 2 - dlugoscMigacza + 1) { //gaszenie paska od poczatku
      strip1t.setPixelColor(i + dlugoscMigacza, strip1t.Color(0, 0, 0));
      strip2t.setPixelColor(i + dlugoscMigacza, strip2t.Color(0, 0, 0));
    }
    strip2t.show();
    strip1t.show();
  }
  for (int k = dlugoscMigacza - 1; k >= 0 ; k--) { //gaszenie koncowki paska (znikanie)
    strip1t.setPixelColor(k, strip1t.Color(0, 0, 0));
    strip2t.setPixelColor(k, strip1t.Color(0, 0, 0));
    strip1t.show();
    strip2t.show();
    delay(opoznienieZmianyMigacza + 10);
  }
}

void kierunkowskazy_tyl(int ktory) { // JEDEN KIERUNKOWSKAZ, PRZYJMUJE ZMIENNA WSKAZUJACA KTORY - 6 -LEWY, 9 -PRAWY
  if (ktory == 6) { //lewy
    //    clearLed1t();
    for ( int i = ledCount - 1; i >= 0; i--) {
      strip1t.setPixelColor(i, strip1t.Color(255, 155, 0));
      delay(opoznienieZmianyMigacza);
      if ( i < ledCount - dlugoscMigacza) { //gaszenie paska od poczatku
        strip1t.setPixelColor(i + dlugoscMigacza, strip1t.Color(0, 0, 0));
      }
      strip1t.show();
    }
    for (int k = dlugoscMigacza; k >= 0 ; k--) { //gaszenie koncowki paska (znikanie)
      strip1t.setPixelColor(k, strip1t.Color(0, 0, 0));
      strip1t.show();
      delay(opoznienieZmianyMigacza);
    }
  }
  else if (ktory == 9) { //prawy
    for ( int i = ledCount - 1; i >= 0; i--) {
      strip2t.setPixelColor(i, strip2t.Color(255, 155, 0));
      delay(opoznienieZmianyMigacza);
      if ( i < ledCount - dlugoscMigacza) { //gaszenie paska od poczatku
        strip2t.setPixelColor(i + dlugoscMigacza, strip1t.Color(0, 0, 0));
      }
      strip2t.show();
    }
    for (int k = dlugoscMigacza; k >= 0 ; k--) { //gaszenie koncowki paska (znikanie)
      strip2t.setPixelColor(k, strip2t.Color(0, 0, 0));
      strip2t.show();
      delay(opoznienieZmianyMigacza);
    }
  }
}
void kierunkowskazy_tyl_stop_cofanie(int ktory) {
  if (ktory == 6) { //lewy
    for ( int i = ledCount / 2; i >= 0; i--) {
      strip1t.setPixelColor(i, strip1t.Color(255, 155, 0));
      delay(opoznienieZmianyMigacza + 10);
      if ( i < ledCount / 2 - dlugoscMigacza + 1) { //gaszenie paska od poczatku
        strip1t.setPixelColor(i + dlugoscMigacza, strip1t.Color(0, 0, 0));
      }
      strip1t.show();
    }
    for (int k = dlugoscMigacza - 1; k >= 0 ; k--) { //gaszenie koncowki paska (znikanie)
      strip1t.setPixelColor(k, strip1t.Color(0, 0, 0));
      strip1t.show();
      delay(opoznienieZmianyMigacza + 10);
    }
  }
  else if (ktory == 9) { //prawy
    for ( int i = ledCount / 2; i >= 0; i--) {
      strip2t.setPixelColor(i, strip2t.Color(255, 155, 0));
      delay(opoznienieZmianyMigacza + 10);
      if ( i < ledCount / 2 - dlugoscMigacza + 1) { //gaszenie paska od poczatku
        strip2t.setPixelColor(i + dlugoscMigacza, strip2t.Color(0, 0, 0));
      }
      strip2t.show();
    }
    for (int k = dlugoscMigacza - 1; k >= 0 ; k--) { //gaszenie koncowki paska (znikanie)
      strip2t.setPixelColor(k, strip2t.Color(0, 0, 0));
      strip2t.show();
      delay(opoznienieZmianyMigacza + 10);
    }
  }
}

//################## LAMPA PRZOD FUNKCJE ################################

//################## LAMPA TYL FUNKCJE ################################
//################## INICJALIZACJA / WYLACZANIE SYSTEMU ###############
void inicjalizacja_systemu() {
  //// PASEK RUSZA W PRAWO
  for ( int i = 0; i < ledCount; i++) {
    strip1t.setPixelColor(i, strip1t.Color(255, 0, 0));
    strip2t.setPixelColor(i, strip2t.Color(255, 0, 0));
    delay(opoznienieZmianyMigacza);

    if ( i >= dlugoscMigacza) { //gaszenie paska od poczatku
      strip1t.setPixelColor(i - dlugoscMigacza, strip1t.Color(0, 0, 0));
      strip2t.setPixelColor(i - dlugoscMigacza, strip2t.Color(0, 0, 0));
    }
    strip1t.show();
    strip2t.show();
  } /// MA NIE ZNIKAC CALKIEM DO SRODKA, JAKBY MIAL ZNIKNAC TO ODKOMENTOWAC TO PODSPODEM
  //        for(int k = (ledCount-dlugoscMigacza); k <= ledCount ; k++){  //gaszenie koncowki paska (znikanie)
  //        strip1.setPixelColor(k, strip1.Color(0, 0, 0));
  //        strip2.setPixelColor(k, strip2.Color(0, 0, 0));
  //        strip1.show();
  //        strip2.show();
  //        delay(opoznienieZmianyMigacza);
  //        }

  for ( int i = ledCount - (dlugoscMigacza - 1); i >= 0; i--) { // pasek rusza w lewo
    strip1t.setPixelColor(i, strip1t.Color(255, 0, 0));
    strip2t.setPixelColor(i, strip2t.Color(255, 0, 0));
    delay(opoznienieZmianyMigacza);

    if ( i < ledCount - dlugoscMigacza) {
      strip1t.setPixelColor(i + dlugoscMigacza + 2, strip1t.Color(0, 0, 0));
      strip2t.setPixelColor(i + dlugoscMigacza + 2, strip2t.Color(0, 0, 0));
    }
    strip1t.show();
    strip2t.show();
  } // WERSJA BEZ AUTOMATYCZNYCH SWIATEL DZIENNYCH USUNAC FOR PONIZEJ ZEBY ZOSTALY DZIENNE
  for (int k = dlugoscMigacza + 1; k >= 0 ; k--) { //gaszenie koncowki paska (znikanie)
    strip1t.setPixelColor(k, strip1t.Color(0, 0, 0));
    strip2t.setPixelColor(k, strip2t.Color(0, 0, 0));
    strip1t.show();
    strip2t.show();
    delay(opoznienieZmianyMigacza);
  }

}

void wylaczanie_systemu() {
  for ( int i = (0 + dlugoscMigacza); i < ledCount; i++) { // pasek rusza w prawo
    strip1t.setPixelColor(i, strip1t.Color(255, 0, 0));
    strip2t.setPixelColor(i, strip2t.Color(255, 0, 0));
    delay(opoznienieZmianyMigacza);

    if ( i >= dlugoscMigacza) { //gaszenie paska od poczatku
      strip1t.setPixelColor(i - dlugoscMigacza, strip1t.Color(0, 0, 0));
      strip2t.setPixelColor(i - dlugoscMigacza, strip2t.Color(0, 0, 0));
    }
    strip1t.show();
    strip2t.show();
  }

  for ( int i = ledCount - dlugoscMigacza; i >= 0; i--) { // pasek rusza w lewo
    strip1t.setPixelColor(i, strip1t.Color(255, 0, 0));
    strip2t.setPixelColor(i, strip2t.Color(255, 0, 0));
    delay(opoznienieZmianyMigacza);

    if ( i < ledCount - dlugoscMigacza) { //gaszenie paska od poczatku
      strip1t.setPixelColor(i + dlugoscMigacza, strip1t.Color(0, 0, 0));
      strip2t.setPixelColor(i + dlugoscMigacza, strip2t.Color(0, 0, 0));
    }
    strip1t.show();
    strip2t.show();
  }
  for (int k = dlugoscMigacza; k >= 0 ; k--) { //gaszenie koncowki paska (znikanie)
    strip1t.setPixelColor(k, strip1t.Color(0, 0, 0));
    strip2t.setPixelColor(k, strip2t.Color(0, 0, 0));
    strip1t.show();
    strip2t.show();
    delay(opoznienieZmianyMigacza);
  }
}
//###########################################################################################

void swiatlo_stop() {
  for (int i = 0; i < ledCount; i++) {
    strip1t.setPixelColor(i, strip1t.Color(255, 0, 0));
    strip2t.setPixelColor(i, strip2t.Color(255, 0, 0));
  }
  strip1t.show();
  strip2t.show();
}

void swiatlo_dzien_tyl() {
  for (int i = 0; i < (ledCount / 3); i++) {
    strip1t.setPixelColor(i, strip1t.Color(255, 0, 0));
    strip2t.setPixelColor(i, strip2t.Color(255, 0, 0));
  }
  strip1t.show();
  strip2t.show();
}


void swiatlo_cofania(int flagaDzienTyl) {
  for (int i = int((2 * ledCount / 3 - 2)); i < (ledCount - 2); i++) {
    strip1t.setPixelColor(i, strip1t.Color(255, 255, 255));
    strip2t.setPixelColor(i, strip2t.Color(255, 255, 255));
  }
  if (flagaDzienTyl == 1) swiatlo_dzien_tyl();
  strip1t.show();
  strip2t.show();
}


// Funkcja Macka


//######################## FUNKCJE CZYSZCZENIA PASKOW ############################

void clearLed1t() {
  for (int i = 0; i < ledCount; i++) {
    strip1t.setPixelColor(i, 0, 0, 0);
  }
  strip1t.show();
}

void clearLed2t() {
  for (int i = 0; i < ledCount; i++) {
    strip2t.setPixelColor(i, 0, 0, 0);
  }
  strip2t.show();
}

void clearLed12t() {
  for (int i = 0; i < ledCount; i++) {
    strip1t.setPixelColor(i, 0, 0, 0);
    strip2t.setPixelColor(i, 0, 0, 0);

  }
  strip1t.show();
  strip2t.show();
}
//############### FUNKCJA DO KONWERSJI ODEBRANEGO Z PORTU SZEREGOWEGO STRINGA NA WARTOSC INT DO POROWNANIA W SWITCH-CASE#####################
int StoI_f(String daneString) {
  int odebraneDaneInt = 0;
  if ( daneString == "1") odebraneDaneInt = 1;
  else if ( daneString == "2") odebraneDaneInt = 2;
  else if ( daneString == "3") odebraneDaneInt = 3;
  else if ( daneString == "4") odebraneDaneInt = 4;
  else if ( daneString == "5") odebraneDaneInt = 5;
  else if ( daneString == "6") odebraneDaneInt = 6;
  else if ( daneString == "7") odebraneDaneInt = 7;
  else if ( daneString == "8") odebraneDaneInt = 8;
  else if ( daneString == "9") odebraneDaneInt = 9;
  else if ( daneString == "10") odebraneDaneInt = 10;
  else if ( daneString == "11") odebraneDaneInt = 11;
  else if ( daneString == "12") odebraneDaneInt = 12;
  else if ( daneString == "13") odebraneDaneInt = 13;
  else if ( daneString == "14") odebraneDaneInt = 14;
  else if ( daneString == "15") odebraneDaneInt = 15;
  else if ( daneString == "16") odebraneDaneInt = 16;
  else if ( daneString == "17") odebraneDaneInt = 17;
  else if ( daneString == "18") odebraneDaneInt = 18;
  else if ( daneString == "19") odebraneDaneInt = 19;
  //else odebraneDaneInt = 0;
  return odebraneDaneInt;
}
