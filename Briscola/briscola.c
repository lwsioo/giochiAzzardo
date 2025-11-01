#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define MAX_CARDS 40
#define HAND_SIZE 3

typedef struct {
    char code[4]; // Esempio: "AB", "10S"
} Card;

int cardValue(const Card *c) {
    if (c->code[0] == 'A') return 11;
    if (c->code[0] == '3') return 10;
    if (strcmp(c->code, "10B") == 0 || strcmp(c->code, "10C") == 0 || strcmp(c->code, "10D") == 0 || strcmp(c->code, "10S") == 0) return 4;
    if (strcmp(c->code, "9B") == 0 || strcmp(c->code, "9C") == 0 || strcmp(c->code, "9D") == 0 || strcmp(c->code, "9S") == 0) return 3;
    if (strcmp(c->code, "8B") == 0 || strcmp(c->code, "8C") == 0 || strcmp(c->code, "8D") == 0 || strcmp(c->code, "8S") == 0) return 2;
    return 0;
}

bool sameSuit(const Card *a, const Card *b) {
    return a->code[strlen(a->code)-1] == b->code[strlen(b->code)-1];
}

void clearConsole() {
    system("clear");
}

int menu() {
    int ans = 0;
    do {
        clearConsole();
        printf("Benvenuto al tavolo da briscola!\n");
        printf("1: Gioca\n");
        printf("2: Regole\n");
        printf("3: Esci\n");
        scanf("%d", &ans);
        while(getchar() != '\n');
        clearConsole();
    } while (ans != 1 && ans != 2 && ans != 3);
    return ans;
}

int chooseDifficulty() {
    int diff = 0;
    printf("Scegli la difficoltà del bot:\n");
    printf("1: Facile\n");
    printf("2: Media\n");
    printf("3: Difficile\n");
    scanf("%d", &diff);
    while(getchar() != '\n');
    if (diff < 1 || diff > 3) diff = 1;
    return diff;
}

int loadDeck(Card deck[], const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Errore nell'apertura di %s\n", filename);
        exit(1);
    }
    int count = 0;
    while (fscanf(file, "%3s", deck[count].code) == 1 && count < MAX_CARDS)
        count++;
    fclose(file);
    if (count != MAX_CARDS) {
        printf("Il mazzo non è completo (%d/40 carte)\n", count);
        exit(1);
    }
    return count;
}

void shuffleDeck(Card deck[], int n) {
    srand(time(NULL));
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        Card temp = deck[i];
        deck[i] = deck[j];
        deck[j] = temp;
    }
}

void dealCard(Card deck[], int *curr, Card *out) {
    *out = deck[(*curr)++];
}

void printHand(Card hand[], int count) {
    printf("Le tue carte:\n");
    for (int i = 0; i < count; i++)
        printf("%d: %s\n", i+1, hand[i].code);
}

int chooseCard(Card hand[], int count) {
    int choice;
    printf("Scegli una carta da giocare (1-%d): ", count);
    scanf("%d", &choice);
    while(getchar() != '\n');
    while (choice < 1 || choice > count) {
        printf("Scelta non valida. Riprova: ");
        scanf("%d", &choice);
        while(getchar() != '\n');
    }
    return choice - 1;
}

int botEasy(Card hand[], int count) {
    int choice = rand() % count;
    printf("Il bot sta pensando...\n");
    sleep(2);
    return choice;
}

int botMedium(Card hand[], int count, const Card *lastPlayer, const Card *briscola) {
    printf("Il bot sta pensando...\n");
    sleep(2);
    for (int i = 0; i < count; i++) {
        if (sameSuit(&hand[i], lastPlayer) && cardValue(&hand[i]) > cardValue(lastPlayer))
            return i;
        if (!sameSuit(&hand[i], lastPlayer) && hand[i].code[strlen(hand[i].code)-1] == briscola->code[strlen(briscola->code)-1])
            return i;
    }
    int idxLow = 0;
    for (int i = 1; i < count; i++)
        if (cardValue(&hand[i]) < cardValue(&hand[idxLow])) idxLow = i;
    return idxLow;
}

int botHard(Card hand[], int count, const Card *lastPlayer, const Card *briscola) {
    printf("Il bot sta ragionando intensamente...\n");
    sleep(3);
    int idxBest = 0;
    int maxVal = -1;
    for (int i = 0; i < count; i++) {
        if (sameSuit(&hand[i], lastPlayer) && cardValue(&hand[i]) > cardValue(lastPlayer)) {
            if (cardValue(&hand[i]) > maxVal) {
                maxVal = cardValue(&hand[i]);
                idxBest = i;
            }
        }
        if (!sameSuit(&hand[i], lastPlayer) && hand[i].code[strlen(hand[i].code)-1] == briscola->code[strlen(briscola->code)-1]) {
            if (cardValue(&hand[i]) > maxVal) {
                maxVal = cardValue(&hand[i]);
                idxBest = i;
            }
        }
    }
    if (maxVal != -1) return idxBest;
    int idxLow = 0;
    for (int i = 1; i < count; i++)
        if (cardValue(&hand[i]) < cardValue(&hand[idxLow])) idxLow = i;
    return idxLow;
}

bool isFirstPlayerWinner(const Card *a, const Card *b, const Card *briscola) {
    char br = briscola->code[strlen(briscola->code)-1];
    char sa = a->code[strlen(a->code)-1], sb = b->code[strlen(b->code)-1];
    if (sa == sb)
        return cardValue(a) > cardValue(b);
    if (sa == br && sb != br) return true;
    if (sb == br && sa != br) return false;
    return true;
}

void removeCard(Card hand[], int *count, int idx) {
    for (int i = idx; i < (*count)-1; i++)
        hand[i] = hand[i+1];
    (*count)--;
}

void rules() {
    clearConsole();
    printf("REGOLE DELLA BRISCOLA\n");
    printf("- Si gioca con 40 carte (A,2,3,4,5,6,7,Fante,Cavallo,Re) suddivise in 4 semi.\n");
    printf("- All'inizio si scopre una carta, il suo seme è la briscola.\n");
    printf("- Si distribuiscono 3 carte ciascuno. Dopo ogni mano, si pesca fino a finire il mazzo.\n");
    printf("- Chi vince la mano pesca la penultima carta del mazzo; l'altro prende la briscola scoperta.\n");
    printf("- I valori: Asso=11, 3=10, Re=4, Cavallo=3, Fante=2, tutte le altre=0.\n");
    printf("- Vince la mano chi mette la carta più alta del seme che ha iniziato, oppure chi mette la briscola.\n");
    printf("- Alla fine, chi ha più punti vince la partita.\n");
    sleep(8);
}

bool game(int botDiff) {
    Card deck[MAX_CARDS];
    Card player[HAND_SIZE], bot[HAND_SIZE];
    int curr = 0, playerCards = HAND_SIZE, botCards = HAND_SIZE;
    int playerPoints = 0, botPoints = 0;

    loadDeck(deck, "carte.txt");
    shuffleDeck(deck, MAX_CARDS);

    for (int i = 0; i < HAND_SIZE; ++i) dealCard(deck, &curr, &player[i]);
    for (int i = 0; i < HAND_SIZE; ++i) dealCard(deck, &curr, &bot[i]);

    Card briscola = deck[MAX_CARDS-1]; // ultima carta scoperta
    printf("La briscola è: %s\n", briscola.code);
    sleep(3);

    int turnPlayer = 0; // 0: tu inizi, 1: bot inizia
    while (playerCards > 0 && botCards > 0) {
        clearConsole();
        printf("Briscola: %s\n", briscola.code);
        printf("Punti - Tu: %d | Bot: %d\n", playerPoints, botPoints);
        printf("Carte rimaste nel mazzo: %d\n", MAX_CARDS - curr - 1); // -1 per la briscola

        Card played, botPlayed;
        int idx, botIdx;

        if (turnPlayer % 2 == 0) { // Tu inizi
            printHand(player, playerCards);
            idx = chooseCard(player, playerCards);
            played = player[idx];

            if (botDiff == 1) botIdx = botEasy(bot, botCards);
            else if (botDiff == 2) botIdx = botMedium(bot, botCards, &played, &briscola);
            else botIdx = botHard(bot, botCards, &played, &briscola);
            botPlayed = bot[botIdx];

            printf("Hai giocato: %s\n", played.code);
            printf("Il bot ha giocato: %s\n", botPlayed.code);

            bool haiPreso = isFirstPlayerWinner(&played, &botPlayed, &briscola);
            if (haiPreso) {
                printf("Hai preso la mano!\n");
                playerPoints += cardValue(&played) + cardValue(&botPlayed);
                turnPlayer = 0;
            } else {
                printf("Il bot prende la mano!\n");
                botPoints += cardValue(&played) + cardValue(&botPlayed);
                turnPlayer = 1;
            }
            removeCard(player, &playerCards, idx);
            removeCard(bot, &botCards, botIdx);

            sleep(2);

            // Gestione pescate
            if (curr < MAX_CARDS - 2) {
                if (turnPlayer == 0) {
                    dealCard(deck, &curr, &player[playerCards++]);
                    dealCard(deck, &curr, &bot[botCards++]);
                } else {
                    dealCard(deck, &curr, &bot[botCards++]);
                    dealCard(deck, &curr, &player[playerCards++]);
                }
            } else if (curr == MAX_CARDS - 2) {
                if (haiPreso) {
                    dealCard(deck, &curr, &player[playerCards++]);
                    bot[botCards++] = briscola;
                } else {
                    dealCard(deck, &curr, &bot[botCards++]);
                    player[playerCards++] = briscola;
                }
                curr = MAX_CARDS;
            }
        } else { // Bot inizia
            if (botDiff == 1) botIdx = botEasy(bot, botCards);
            else if (botDiff == 2) botIdx = botMedium(bot, botCards, &bot[botIdx], &briscola);
            else botIdx = botHard(bot, botCards, &bot[botIdx], &briscola);
            botPlayed = bot[botIdx];

            printf("Il bot ha giocato: %s\n", botPlayed.code);
            printHand(player, playerCards);
            idx = chooseCard(player, playerCards);
            played = player[idx];
            printf("Hai giocato: %s\n", played.code);

            bool botPreso = isFirstPlayerWinner(&botPlayed, &played, &briscola);
            if (botPreso) {
                printf("Il bot prende la mano!\n");
                botPoints += cardValue(&played) + cardValue(&botPlayed);
                turnPlayer = 1;
            } else {
                printf("Hai preso la mano!\n");
                playerPoints += cardValue(&played) + cardValue(&botPlayed);
                turnPlayer = 0;
            }
            removeCard(player, &playerCards, idx);
            removeCard(bot, &botCards, botIdx);

            sleep(2);

            if (curr < MAX_CARDS - 2) {
                if (turnPlayer == 0) {
                    dealCard(deck, &curr, &player[playerCards++]);
                    dealCard(deck, &curr, &bot[botCards++]);
                } else {
                    dealCard(deck, &curr, &bot[botCards++]);
                    dealCard(deck, &curr, &player[playerCards++]);
                }
            } else if (curr == MAX_CARDS - 2) {
                if (botPreso) {
                    dealCard(deck, &curr, &bot[botCards++]);
                    player[playerCards++] = briscola;
                } else {
                    dealCard(deck, &curr, &player[playerCards++]);
                    bot[botCards++] = briscola;
                }
                curr = MAX_CARDS;
            }
        }
    }

    clearConsole();
    printf("Partita terminata!\n");
    printf("Punteggio finale: Tu %d, Bot %d\n", playerPoints, botPoints);

    return playerPoints > botPoints;
}

int main() {
    while (1) {
        int choice = menu();
        switch (choice) {
            case 1: {
                int diff = chooseDifficulty();
                bool victory = game(diff);
                if (victory) {
                    printf("Hai vinto!\n");
                    return 1;
                } else {
                    printf("Hai perso :(\n");
                    return 0;
                }
                break;
            }
            case 2:
                rules();
                break;
            case 3:
                printf("Uscita dal programma\n");
                sleep(2);
                return 0;
            default:
                break;
        }
    }
}
