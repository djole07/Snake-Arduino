//We always have to include the library
#include "LedControl.h"
#include <IRremote.h>     // za kontrolu preko daljinskog


/*
  Now we need a LedControl to work with.
 ***** These pin numbers will probably not work with your hardware *****
  pin 12 is connected to the DataIn
  pin 11 is connected to the CLK
  pin 10 is connected to LOAD
  We have only a single MAX72XX.
*/
typedef struct {
  short i;
  short j;
} Telo;

LedControl lc = LedControl(12, 11, 10, 1);
const int RECV_PIN = 7;   // prijemni pin za daljinski

unsigned long vrednost;
IRrecv irrecv(RECV_PIN);
decode_results results;

enum SMER {GORE, LEVO, DOLE, DESNO};
unsigned long delayTime = 700;

unsigned short Tabla[8][8] = {};
unsigned short duzina;

Telo slobodnaMesta[64] = {};
Telo teloZmije[64] = {};

unsigned short trenutniPotez;
unsigned short sledeciPotez;

bool needFood = true;
bool gameOver = false;
bool newGame = true;
bool standBy = false;

char potez;

short glava_i, glava_j;
short OSVETLJENJE = 0;

void setup() {

  irrecv.enableIRIn();    // inicijalizacija daljinskog
  irrecv.blink13(true);

  // za unos komandi sa tastature
  Serial.begin(9600);
  randomSeed(analogRead(5));

}
void obrisiTablu() {
  for (int  i = 0 ; i < 8 ; i++)
    for (int j = 0 ; j < 8 ; j++)
      Tabla[i][j] = 0;
}
void inicijalizuj() {
  /*
    The MAX72XX is in power-saving mode on startup,
    we have to do a wakeup call
  */
  if (!standBy)
    lc.shutdown(0, false);

  /* Set the brightness to a medium values */
  lc.setIntensity(0, OSVETLJENJE);
  /* and clear the display */
  lc.clearDisplay(0);
  delay(500);

  obrisiTablu();

  // kozmetika
  // 3

  // gornji deo
  Tabla[1][2] = 1;
  Tabla[1][3] = 1;
  Tabla[1][4] = 1;
  Tabla[1][5] = 1;

  Tabla[2][5] = 1;      //
  Tabla[3][3] = 1;
  Tabla[3][4] = 1;
  Tabla[3][5] = 1;      //
  Tabla[4][5] = 1;

  // donji deo
  Tabla[5][2] = 1;
  Tabla[5][3] = 1;
  Tabla[5][4] = 1;
  Tabla[5][5] = 1;



  stampaj();
  delay(500);
  lc.clearDisplay(0);
  delay(500);

  //2
  Tabla[4][5] = 0;
  Tabla[2][5] = 1;
  Tabla[3][2] = 1;
  Tabla[4][2] = 1;

  stampaj();
  delay(500);
  lc.clearDisplay(0);
  delay(500);

  // 1
  Tabla[1][2] = 0;
  Tabla[1][3] = 0;
  Tabla[1][4] = 0;

  Tabla[3][2] = 0;
  Tabla[4][2] = 0;

  Tabla[5][2] = 0;
  Tabla[5][3] = 0;
  Tabla[5][4] = 0;

  Tabla[3][3] = 0;
  Tabla[3][4] = 0;

  Tabla[1][5] = 1;
  Tabla[2][5] = 1;
  Tabla[3][5] = 1;
  Tabla[4][5] = 1;
  Tabla[5][5] = 1;

  stampaj();
  delay(500);
  lc.clearDisplay(0);
  delay(500);


  obrisiTablu();

  // zmijica na pocetku ima duzinu 2
  teloZmije[0].i = 2;
  teloZmije[0].j = 2;

  teloZmije[1].i = 2;
  teloZmije[1].j = 3;


  Tabla[2][2] = 1;
  Tabla[2][3] = 1;

  glava_i = 2;
  glava_j = 3;

  duzina = 2;

  trenutniPotez = DESNO;
  sledeciPotez = DESNO;
  needFood = true;

  //standBy = false;
  // delayTime = 700;
}
void brightnessDown() {
  if (OSVETLJENJE >= 0) {
    OSVETLJENJE--;
    lc.setIntensity(0, OSVETLJENJE);
  }
}
void brightnessUp() {
  if (OSVETLJENJE <= 15) {
    OSVETLJENJE++;
    lc.setIntensity(0, OSVETLJENJE);
  }
}
void speedDown() {
  delayTime += 50;
}
void speedUp() {
  delayTime -= 50;
}
void testAll() {
  for (int  i = 0 ; i < 8 ; i++)
    for (int j = 0 ; j < 8 ; j++) {
      Tabla[i][j] = 1;
    }
  stampaj();
  for (int  i = 0 ; i < 8 ; i++)
    for (int j = 0 ; j < 8 ; j++) {
      Tabla[i][j] = 1;
    }
  stampaj();

}

void moveSnake(int hor, int ver) {
  int i;

  glava_i = glava_i + ver;
  glava_j = glava_j + hor;
  /*
    Serial.print("glava_i= ");
    Serial.print(glava_i);
    Serial.print(",   glava_j= ");
    Serial.println(glava_j);
  */

  if (glava_i < 0 || glava_j < 0 || glava_i > 7 || glava_j > 7 || Tabla[glava_i][glava_j] == 1) {
    gameOver = true;
    return;
  }

  if (Tabla[glava_i][glava_j] == 2) {   // da proverimo da li smo pojeli hranu
    needFood = true;
    duzina++;
    teloZmije[duzina - 1].i = glava_i;
    teloZmije[duzina - 1].j = glava_j;
    if (delayTime > 100) delayTime -= 50;
  }
  else {
    Tabla[teloZmije[0].i][teloZmije[0].j] = 0;    // gasimo rep


    for (i = 1 ; i < duzina ; i++) {              // pomeramo telo zmije za jednu poziciju
      teloZmije[i - 1].i = teloZmije[i].i;
      teloZmije[i - 1].j = teloZmije[i].j;
    }
    teloZmije[i - 1].i = glava_i;
    teloZmije[i - 1].j = glava_j;
  }
  Tabla[glava_i][glava_j] = 1;                // ukljucujemo glavu

}
void pomeranje_zmije() {
  if (sledeciPotez == trenutniPotez || sledeciPotez % 2 != trenutniPotez % 2) {   // 0 i 2 reprezentuju GORE i DOLE, dok 1 i 3 LEVO i DESNO. Ako uzmemo moduo 2 od ovih brojeva dobijamo
    //  da ako je moduo == 0 onda se krecemo vertikalno, odnosno kao je moduo == 1 vertikalno. Stoga, mozemo da promenimo smer kretanja zmijice samo ako su
    // moduli razliciti. Ovo je pravljeno ako korisnik zeli da se vrati u suprotnom smeru u kom se zaputio.
    //Naravno, ukoliko se korisnik kretao u jednom
    // pravcu, a zatim taj pravac i zadrzao onda nece uci u if jer bi to bilo bespotrebno koriscenje procesora

    // Serial.println("Pomeranje zmije");

    trenutniPotez = sledeciPotez;
    switch (sledeciPotez) {
      case GORE: moveSnake(0, -1); break;
      case LEVO: moveSnake(-1, 0); break;
      case DOLE: moveSnake(0, 1); break;
      case DESNO: moveSnake(1, 0); break;
    }
  }
}
void generateFood() {
  int z = 0;
  randomSeed(analogRead(0));

  for (int  i = 0 ; i < 8 ; i++) {
    for (int j = 0 ; j < 8 ; j++) {
      if (Tabla[i][j] == 0) {
        slobodnaMesta[z].i = i;
        slobodnaMesta[z].j = j;
        z = z + 1;
      }
    }
  }
  int hrana;
  hrana = random(1, z - 1);
  Serial.print("Moj broj je: ");
  Serial.print(hrana);
  Serial.print("a z = : ");
  Serial.println(z);
  hrana = random(1, z - 1);
  Serial.print("Moj broj je: ");
  Serial.println(hrana);
  Tabla[slobodnaMesta[hrana].i][slobodnaMesta[hrana].j] = 2;    // postavljamo hranu
}
void stampaj() {
  for (int  i = 0 ; i < 8 ; i++)
    for (int j = 0 ; j < 8 ; j++) {
      if (Tabla[i][j])
        lc.setLed(0, i, j, true);
      else
        lc.setLed(0, i, j, false);
    }
}
void zavrsioSi() {
  obrisiTablu();

  Tabla[2][2] = 1;
  Tabla[2][5] = 1;
  Tabla[5][2] = 1;
  Tabla[5][3] = 1;
  Tabla[5][4] = 1;
  Tabla[5][5] = 1;
}

void loop() {
  if (!gameOver && !newGame) {
    if (needFood) {
      Serial.println("\nGeneriseHranu\n");
      generateFood();
      needFood = false;
    }

    /*
       Ako igramo preko tastaure

      if (Serial.available() > 0) {
      potez = Serial.read();
      Serial.print("Vas potez je: ");
      Serial.println(potez);
      switch (potez) {
        case '2': sledeciPotez = DOLE; break;
        case '4': sledeciPotez = LEVO; break;
        case '6': sledeciPotez = DESNO; break;
        case '8': sledeciPotez = GORE; break;
        default : sledeciPotez = trenutniPotez; break;
      }
      if (potez == '9') {
        Serial.print("BRISI");
        lc.clearDisplay(0);
        delay(1000);
      }
      }
    */
    if (irrecv.decode(&results)) {
      vrednost = results.value;
      Serial.print("Vrednost je :");
      Serial.println(vrednost, HEX);
      /*
         VIVAX DALJINSKI

          switch (vrednost) {
            case 0xE3994B88: sledeciPotez = GORE; break;
            case 0x23D8CC84: sledeciPotez = DESNO; break;
            case 0x9E66CA64: sledeciPotez = LEVO; break;
            case 0x4E6D7BA0: sledeciPotez = DOLE; break;
            default: Serial.println("NESTO TRECE"); break;
          }
      */

      //  COLOSSUS DALJINSKI

      switch (vrednost) {
        case 0xFE30CF: sledeciPotez = GORE; break;
        case 0xFE708F: sledeciPotez = DESNO; break;
        case 0xFEF00F: sledeciPotez = LEVO; break;
        case 0xFEB04F: sledeciPotez = DOLE; break;
        case 0xFEA857: newGame = true; break;            // restart         crveno dugme
        case 0xFE6897: lc.shutdown(0, true); standBy = true; break;            // standBy   zeleno dugme
        case 0xFED827: brightnessUp(); break;           // VOL+
        case 0xFE58A7: brightnessDown(); break;         // VOL-
        case 0xFE9867: speedUp(); break;                // CH+
        case 0xFE18E7: speedDown(); break;              // CH-
        default: Serial.println("NESTO TRECE"); break;
      }

      irrecv.resume();
    }


    Serial.print("Sledeci smer je: ");
    Serial.println(sledeciPotez);
    pomeranje_zmije();
    stampaj();
    delay(delayTime);

  }
  else {
    if (gameOver) {
      zavrsioSi();
      stampaj();
      delay(3000);
      lc.shutdown(0, true);
      newGame = true;
      gameOver = false;
    }
    else {
      inicijalizuj();
      newGame = false;
    }

  }

}
