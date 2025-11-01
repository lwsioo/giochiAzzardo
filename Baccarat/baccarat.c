#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>

#define MAX_CARDS 52
#define CARD_STR_LEN 4   // Per supportare "10H" con 4 caratteri

void clearConsole(){
  system("clear");
}

int valoreCarta(int cardNumber) {
  if (cardNumber >= 10)
    return 0;
  else
    return cardNumber;
}

int sommaPunti(int c1, int c2, int c3) {
  int sum = c1 + c2 + c3;
  return sum % 10;
}

int caricaMazzo(const char* filename, char cards[MAX_CARDS][CARD_STR_LEN]) {
  FILE *fp = fopen(filename, "r");
  if (fp == NULL) {
    printf("Errore apertura file %s\n", filename);
    return -1;
  }
  int i = 0;
  while (i < MAX_CARDS && fscanf(fp, " %3s", cards[i]) == 1) {
    i++;
  }
  fclose(fp);
  if (i != MAX_CARDS) {
    printf("Attenzione: lette %d carte invece di %d\n", i, MAX_CARDS);
  }
  return i;
}

int cardValueFromString(char *card) {
  if (card[0] == 'A') return 1;
  else if (card[0] >= '2' && card[0] <= '9') return card[0] - '0';
  else if (card[0] == '1' && card[1] == '0') return 10; // Per "10H", "10C" ecc.
  else if (card[0] == 'J') return 11;
  else if (card[0] == 'Q') return 12;
  else if (card[0] == 'K') return 13;
  else return 0;
}

int chooseWho() {
  int who = 0;
  do {
    printf("Seleziona su cosa puntare:\n");
    printf("1: Banco\n");
    printf("2: Giocatore\n");
    printf("3: Pareggio\n");
    printf("4: Esci\n");
    scanf("%d", &who);
    while(getchar() != '\n');
  } while (who < 1 || who > 4);
  return who;
}

int chooseAmount(int balance) {
  int bet = 0;
  do {
    printf("Hai %d euro. Quanto vuoi puntare? ", balance);
    scanf("%d", &bet);
    while(getchar() != '\n');
    if (bet <= 0) {
      printf("La puntata deve essere positiva.\n");
      sleep(1);
    } else if (bet > balance) {
      printf("Non puoi puntare piu' di quello che hai.\n");
      sleep(1);
    }
  } while (bet <= 0 || bet > balance);
  return bet;
}

int menu() {
  int ans = 0;
  do {
    clearConsole();
    printf("Benvenuto al tavolo da Baccarat!\n\n");
    printf("1: Entra\n");
    printf("2: Regole\n");
    printf("3: Esci\n");
    scanf("%d", &ans);
    while(getchar() != '\n');
    clearConsole();
  } while (ans != 1 && ans != 2 && ans != 3);
  return ans;
}

void rules() {
  clearConsole();
  printf("=== REGOLE DEL BACCARAT ===\n\n");
  printf("Obiettivo: Avvicinarsi il piu' possibile a 9 punti con 2 o 3 carte.\n\n");
  printf("Valore delle carte:\n");
  printf(" - Asso: 1 punto\n");
  printf(" - 2-9: valore nominale\n");
  printf(" - 10, J, Q, K: 0 punti\n\n");
  printf("Distribuzione:\n");
  printf(" - Si distribuiscono 2 carte al Giocatore e 2 al Banco.\n");
  printf(" - Se il totale supera 9, conta solo la cifra delle unita' (es: 7+8=15 -> 5).\n\n");
  printf("Regole per la terza carta:\n");
  printf(" - Se Giocatore ha 0-5: pesca una terza carta.\n");
  printf(" - Se Giocatore ha 6-7: sta.\n");
  printf(" - Se Giocatore sta, il Banco pesca con 0-5 e sta con 6-7.\n");
  printf(" - Se Giocatore pesca, il Banco segue regole specifiche in base ai punti e alla terza carta del Giocatore.\n\n");
  printf("Vince la mano piu' vicina a 9 punti.\n");
  printf("In caso di parita', la puntata su 'Pareggio' vince.\n\n");
  printf("Vincita:\n");
  printf(" - Puntata su Giocatore: 1 a 1\n");
  printf(" - Puntata su Banco: 1 a 1\n");
  printf(" - Puntata su Pareggio: 8 a 1\n\n");
  printf("Premi Invio per tornare al menu...\n");
  getchar();
  getchar();
  clearConsole();
}

bool game() {
  char cards[MAX_CARDS][CARD_STR_LEN];
  int totCards = caricaMazzo("carte.txt", cards);
  if (totCards != MAX_CARDS) {
    printf("Errore nel caricamento del mazzo, esco.\n");
    sleep(2);
    return true;
  }

  int balance = 0;
  do {
    printf("Inserisci il tuo cash iniziale per giocare: ");
    scanf("%d", &balance);
    while(getchar() != '\n');
    if (balance <= 0) {
      printf("Devi inserire un valore positivo.\n");
      sleep(1);
    }
    clearConsole();
  } while (balance <= 0);

  srand(time(NULL));

  while (true) {
    if (balance <= 0) {
      printf("Hai esaurito i soldi. Game over.\n");
      sleep(2);
      break;
    }

    int who = chooseWho();
    if (who == 4) {
      printf("Uscita dal gioco.\n");
      sleep(1);
      break;
    }

    int bet = chooseAmount(balance);

    // Estrazione carte senza duplicati
    int p1 = rand() % MAX_CARDS;
    int p2 = rand() % MAX_CARDS;
    while (p2 == p1) p2 = rand() % MAX_CARDS;
    int b1 = rand() % MAX_CARDS;
    while (b1 == p1 || b1 == p2) b1 = rand() % MAX_CARDS;
    int b2 = rand() % MAX_CARDS;
    while (b2 == p1 || b2 == p2 || b2 == b1) b2 = rand() % MAX_CARDS;

    int vp1 = valoreCarta(cardValueFromString(cards[p1]));
    int vp2 = valoreCarta(cardValueFromString(cards[p2]));
    int vb1 = valoreCarta(cardValueFromString(cards[b1]));
    int vb2 = valoreCarta(cardValueFromString(cards[b2]));

    int playerPoints = sommaPunti(vp1, vp2, 0);
    int bankPoints = sommaPunti(vb1, vb2, 0);

    printf("Player ha carte: %s, %s - punti: %d\n", cards[p1], cards[p2], playerPoints);
    sleep(2);
    printf("Banco ha carte: %s, %s - punti: %d\n", cards[b1], cards[b2], bankPoints);
    sleep(2);

    // Controllo naturale e terza carta
    if (playerPoints >= 8 || bankPoints >= 8) {
      printf("Risultato naturale!\n");
      sleep(2);
    } else {
      int playerThirdCardIndex = -1;
      if (playerPoints <= 5) {
        do {
          playerThirdCardIndex = rand() % MAX_CARDS;
        } while(playerThirdCardIndex == p1 || playerThirdCardIndex == p2 || playerThirdCardIndex == b1 || playerThirdCardIndex == b2);
        int vp3 = valoreCarta(cardValueFromString(cards[playerThirdCardIndex]));
        playerPoints = sommaPunti(vp1, vp2, vp3);
        printf("Player pesca terza carta: %s\n", cards[playerThirdCardIndex]);
        sleep(2);
        printf("Player ha ora: %d punti\n", playerPoints);
        sleep(2);
      } else {
        printf("Player sta con %d punti\n", playerPoints);
        sleep(2);
      }
    }

    // Determinazione vincitore CORRETTO
    int winner = 0; // 1=Banco, 2=Giocatore, 3=Pareggio
    if (playerPoints > bankPoints) winner = 2;      // Vince il Giocatore
    else if (bankPoints > playerPoints) winner = 1; // Vince il Banco
    else winner = 3;                                 // Pareggio

    // Gestione vincite CORRETTA
    if (winner == 3 && who == 3) {
      printf("Hai vinto la puntata sul pareggio!\n");
      balance += bet * 9; // Vincita 8:1 + restituzione puntata = 9 volte
    } else if (winner == who) {
      printf("Hai vinto la puntata!\n");
      balance += bet; // Vincita 1:1, mantieni la puntata originale
    } else {
      printf("Hai perso la puntata.\n");
      balance -= bet; // Perdi la puntata
    }

    printf("Saldo attuale: %d euro\n", balance);
    sleep(2);
    clearConsole();
  }

  return false;
}

int main() {
  int ans;

  do {
    ans = menu();

    switch(ans) {
      case 1:
        if (game()) {
          ans = 3;
        }
        break;
      case 2:
        rules();
        break;
      case 3:
        printf("Uscita dal programma. Arrivederci!\n");
        sleep(2);
        break;
      default:
        break;
    }
  } while(ans != 3);

  return 0;
}
