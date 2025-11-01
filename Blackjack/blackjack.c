#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define DECK_SIZE 52

typedef struct {
  int value;
  char suit;
} Card;

typedef struct {
  Card cards[12];
  int num_cards;
} Hand;

void playGame();
void showRules();
void initDeck(Card deck[]);
void shuffleDeck(Card deck[]);
void printHand(Hand *hand, int first);
int calculateScore(Hand *hand);
Card drowCard(Card deck[], int *cardIndex);
void printCardGraphic(int value, char suit);

int main() {

  int choise = 0;

  printf("******* BLACKJACK *******\n\n");

  printf("Benvenuto nel gioco del Blackjack\n\n");

  while (choise != 3) {

    printf("Scegli un opzione:\n");
    printf("1. Gioca\n2. Visualizza le regole\n3. Esci dal gioco\n");
    scanf(" %d", &choise);

    switch (choise) {
      case 1:
        playGame();
        break;
      case 2:
        showRules();
        break;
      case 3:
        exit(0);
      default:
        printf("Errore nell'insermento\n");
        break;
    }
}

  return 0;
}

void playGame(){
  srand(time(NULL));
  Card deck[DECK_SIZE];
  initDeck(deck);
  shuffleDeck(deck);

  Hand player = {{0}, 0};
  Hand dealer = {{0}, 0};

  int cardIndex = 0;
  int bj = 0;

  player.cards[0] = drowCard(deck, &cardIndex);
  dealer.cards[0] = drowCard(deck, &cardIndex);
  player.cards[1] = drowCard(deck, &cardIndex);
  dealer.cards[1] = drowCard(deck, &cardIndex);
  player.num_cards = 2;
  dealer.num_cards = 2;


  while (bj == 0) {
    printf("\nGiocatore:\n\n");
    printHand(&player, 0);
    int score = calculateScore(&player);
    if(score == 99){
      printf("BLACKJACK! Hai Vinto!\n");
      bj = 1;
    } else {
      printf("Totale: %d\n", score);
    }

    printf("\n\nBanco:\n\n");
    printHand(&dealer, 1);

    char choise = 0;
    printf("Desideri ricevere un'altra carta? (y/n)\n");
    scanf(" %c", &choise);

    if (choise == 'y') {
      player.cards[player.num_cards++] = drowCard(deck, &cardIndex);
      int score = calculateScore(&player);
      printHand(&player, 0);
      if(score != 99){
        printf("Totale: %d\n", score);
      }
      if(score > 21){
        printf("\nHai sballato!\n");
        break;
      }
    } else if (choise == 'n'){
        break;
    } else {
      printf("\nErrore\n");
    }
  }

  if(calculateScore(&player) <= 21){
    printf("Turno del banco\n");
    printf("Il banco scopre la carta coperta\n");
    printHand(&dealer, 0);
    int score = calculateScore(&dealer);
    if(score == 99){
      printf("BLACKJACK! Il banco ha vinto!\n");
    } else {
      printf("Totale: %d\n", score);
    }
    while (score < 17) {
      printf("Il banco pesca una carta\n");
      dealer.cards[dealer.num_cards++] = drowCard(deck, &cardIndex);
      score = calculateScore(&dealer);
      printHand(&dealer, 0);
      if(score != 99){
        printf("Totale: %d\n", score);
      }
    }

    int player_score = calculateScore(&player);
    int dealer_score = calculateScore(&dealer);

    if ((dealer_score > 21 && dealer_score != 99) || player_score > dealer_score) {
      printf("\nHai vinto!\n\n");
    } else if ((player_score > 21 && player_score != 99) || dealer_score > player_score){
      printf("\nIl banco ha vinto\n\n");
    } else {
      printf("Pareggio\n");
    }
  }
}

void showRules(){
    printf("Regole del Blackjack\n\n");

    printf("Il Blackjack e' un gioco di carte in cui l'obiettivo e' ottenere un punteggio piu' alto del banco senza superare 21.\n\n");

    printf("Valore delle carte\n");
    printf("- Le carte numeriche valgono il loro numero.\n");
    printf("- Le figure (J, Q, K) valgono 10.\n");
    printf("- L'asso vale 1 o 11, a scelta del giocatore.\n\n");

    printf("Svolgimento del gioco\n");
    printf("1. Ogni giocatore riceve due carte scoperte, mentre il banco ne mostra una e ne tiene una coperta.\n");
    printf("2. Il giocatore puo':\n");
    printf("   - Chiedere carta (Hit) per aggiungere una nuova carta.\n");
    printf("   - Stare (Stand) per mantenere il punteggio attuale.\n");
    printf("   - Raddoppiare (Double Down) puntando il doppio in cambio di una sola carta aggiuntiva.\n");
    printf("   - Dividere (Split) se ha due carte uguali, creando due mani separate.\n");
    printf("3. Il banco deve pescare fino a raggiungere almeno 17 punti.\n");
    printf("4. Vince chi si avvicina di piu' a 21 senza sforarlo.\n\n");

    printf("Situazioni speciali\n");
    printf("- Blackjack: Asso + carta da 10 punti, la miglior mano possibile.\n");
    printf("- Se entrambi fanno 21 con due carte, la partita e' un pareggio.\n\n");

    printf("Buon divertimento!\n\n");
}


void initDeck (Card deck[]) {
  char suits[] = {'P','F','C','Q'};
  int count = 0;
  for (int s = 0; s < 4; s++) {
    for (int v = 1; v < 14; v++) {
      deck[count].value = v;
      deck[count].suit = suits[s];
      count++;
    }
  }
}

void shuffleDeck(Card deck[]) {
  for (int i = DECK_SIZE - 1; i > 0; i--) {
    int j = rand() % (i + 1);
    Card temp = deck[i];
    deck[i] = deck[j];
    deck[j] = temp;
  }
}

void printHand(Hand *hand, int first){
  if (first) {
    printf("Carta 1:\n");
    printCardGraphic(hand->cards[0].value, hand->cards[0].suit);
    printf("Carta 2: (Carta coperta)\n");
  } else {
    for (int i = 0; i < hand->num_cards; i++) {
      printf("Carta %d:\n", i + 1);
      printCardGraphic(hand->cards[i].value, hand->cards[i].suit);
    }
  }
}

int calculateScore(Hand *hand){
  int score = 0;
  int assi = 0;
  int fig = 0;
  int ten = 0;
  int bj = 0;
  for (int i = 0; i < hand->num_cards; i++) {
    int real_value = hand->cards[i].value;
    if (real_value == 10){
      ten++;
    }
    if (real_value > 10){
      real_value = 10;
      fig++;
    }
    if (real_value == 1) {
      real_value = 11;
      assi++;
    }
    score += real_value;
  }

  if(bj == 0 && assi > 0 && (fig > 0 || ten >0)){
    bj = 1;
    score = 99;
  } else {
    while (score > 21 && assi > 0){
      score = score - 10;
      assi--;
    }
  }
  return score;
}

Card drowCard(Card deck[], int *cardIndex){
  return deck[(*cardIndex)++];
}

void printCardGraphic(int value, char suit) {
    const char *suit_sym;
    switch (suit) {
        case 'P': suit_sym = "♠"; break;
        case 'F': suit_sym = "♥"; break;
        case 'C': suit_sym = "♦"; break;
        case 'Q': suit_sym = "♣"; break;
        default: suit_sym = "?"; break;
    }

    char *value_str;
    static char valbuf[3];
    if (value == 1) value_str = "A";
    else if (value == 11) value_str = "J";
    else if (value == 12) value_str = "Q";
    else if (value == 13) value_str = "K";
    else {
        sprintf(valbuf, "%d", value);
        value_str = valbuf;
    }
    if(value != 10){
      printf("┌───────────┐\n");
      printf("│ %-2s      %-2s│\n", value_str, value_str);
    }

    switch(value) {
        case 1: // Asso
            printf("│           │\n");
            printf("│           │\n");
            printf("│     %s     │\n", suit_sym);
            printf("│           │\n");
            printf("│           │\n");
            break;
        case 2:
            printf("│     %s     │\n", suit_sym);
            printf("│           │\n");
            printf("│           │\n");
            printf("│           │\n");
            printf("│     %s     │\n", suit_sym);
            break;
        case 3:
            printf("│     %s     │\n", suit_sym);
            printf("│           │\n");
            printf("│     %s     │\n", suit_sym);
            printf("│           │\n");
            printf("│     %s     │\n", suit_sym);
            break;
        case 4:
            printf("│ %s       %s │\n", suit_sym, suit_sym);
            printf("│           │\n");
            printf("│           │\n");
            printf("│           │\n");
            printf("│ %s       %s │\n", suit_sym, suit_sym);
            break;
        case 5:
            printf("│ %s       %s │\n", suit_sym, suit_sym);
            printf("│           │\n");
            printf("│     %s     │\n", suit_sym);
            printf("│           │\n");
            printf("│ %s       %s │\n", suit_sym, suit_sym);
            break;
        case 6:
            printf("│ %s       %s │\n", suit_sym, suit_sym);
            printf("│ %s       %s │\n", suit_sym, suit_sym);
            printf("│           │\n");
            printf("│           │\n");
            printf("│ %s       %s │\n", suit_sym, suit_sym);
            break;
        case 7:
            printf("│ %s       %s │\n", suit_sym, suit_sym);
            printf("│     %s     │\n", suit_sym);
            printf("│ %s       %s │\n", suit_sym, suit_sym);
            printf("│           │\n");
            printf("│ %s       %s │\n", suit_sym, suit_sym);
            break;
        case 8:
            printf("│ %s       %s │\n", suit_sym, suit_sym);
            printf("│     %s     │\n", suit_sym);
            printf("│ %s       %s │\n", suit_sym, suit_sym);
            printf("│     %s     │\n", suit_sym);
            printf("│ %s       %s │\n", suit_sym, suit_sym);
            break;
        case 9:
            printf("│ %s       %s │\n", suit_sym, suit_sym);
            printf("│ %s       %s │\n", suit_sym, suit_sym);
            printf("│     %s     │\n", suit_sym);
            printf("│ %s       %s │\n", suit_sym, suit_sym);
            printf("│ %s       %s │\n", suit_sym, suit_sym);
            break;
        case 10:
            printf("┌────────────┐\n");
            printf("│ %-2s      %-2s │\n", value_str, value_str);
            printf("│ %s       %s  │\n", suit_sym, suit_sym);
            printf("│ %s   %s   %s  │\n", suit_sym, suit_sym, suit_sym);
            printf("│            │\n");
            printf("│ %s   %s   %s  │\n", suit_sym, suit_sym, suit_sym);
            printf("│ %s       %s  │\n", suit_sym, suit_sym);
            printf("│ %-2s      %-2s │\n", value_str, value_str);
            printf("└────────────┘\n");
            break;
        case 11: // Jack
            printf("│           │\n");
            printf("│   %s %s %s   │\n", suit_sym, suit_sym, suit_sym);
            printf("│     %s     │\n", suit_sym);
            printf("│    %s %s    │\n", suit_sym, suit_sym);
            printf("│           │\n");
            break;
        case 12: // Queen
            printf("│           │\n");
            printf("│   %s %s %s   │\n", suit_sym, suit_sym, suit_sym);
            printf("│     %s     │\n", suit_sym);
            printf("│   %s %s %s   │\n", suit_sym, suit_sym, suit_sym);
            printf("│           │\n");
            break;
        case 13: // King
            printf("│           │\n");
            printf("│   %s %s %s   │\n", suit_sym, suit_sym, suit_sym);
            printf("│     %s     │\n", suit_sym);
            printf("│    %s %s    │\n", suit_sym, suit_sym);
            printf("│           │\n");
            break;
        default:
            printf("│           │\n");
            printf("│   %s   %s  │\n", suit_sym, suit_sym);
            printf("│           │\n");
            printf("│   %s   %s  │\n", suit_sym, suit_sym);
            printf("│           │\n");
            break;
    }
    if (value != 10) {
      printf("│ %-2s      %-2s│\n", value_str, value_str);
      printf("└───────────┘\n");
    }

}
