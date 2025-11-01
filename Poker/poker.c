#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* =========================
   STRUTTURE BASE
   ========================= */
typedef struct { char nome[8]; } Carta;

typedef struct {
    int fish;       // stack del giocatore
    int puntata;    // puntata corrente nella street
    int attivo;     // 1 in gioco, 0 fold
    Carta mano[2];  // hole cardss
} Giocatore;

typedef struct { int rank; char suit; } CardVal;

typedef struct { int cat; int strength; } HandStrength;

int posSmallBlind = 0;
int posBigBlind = 1;

/* =========================
   MAZZO E DISTRIBUZIONE
   ========================= */
static Carta mazzo[52];
static int idxCarta = 0;

void caricaMazzoDaFile(const char *nomeFile) {
    FILE *f = fopen(nomeFile, "r");
    if (!f) {
        printf("Errore: impossibile aprire %s\n", nomeFile);
        exit(1);
    }
    int i = 0;
    while (i < 52 && fgets(mazzo[i].nome, sizeof(mazzo[i].nome), f)) {
        mazzo[i].nome[strcspn(mazzo[i].nome, "\n")] = '\0';
        i++;
    }
    fclose(f);
    if (i != 52) {
        printf("Errore: file deve avere 52 carte, trovate %d\n", i);
        exit(1);
    }
}

void mescolaMazzo() {
    for (int i = 0; i < 52; i++) {
        int r = rand() % 52;
        Carta t = mazzo[i];
        mazzo[i] = mazzo[r];
        mazzo[r] = t;
    }
    idxCarta = 0;
}

Carta pesca() {
    if (idxCarta >= 52) {
        printf("Errore: mazzo esaurito.\n");
        exit(1);
    }
    return mazzo[idxCarta++];
}

void stampaCarta(Carta c) { printf("%s", c.nome); }

/* =========================
   PARSING CARTE
   ========================= */
int parse_rank(const char *s) {
    if (s[0]=='1' && s[1]=='0') return 10;
    switch (s[0]) {
        case 'A': return 14;
        case 'K': return 13;
        case 'Q': return 12;
        case 'J': return 11;
        case '9': return 9;
        case '8': return 8;
        case '7': return 7;
        case '6': return 6;
        case '5': return 5;
        case '4': return 4;
        case '3': return 3;
        case '2': return 2;
        default:  return 0;
    }
}

char parse_suit(const char *s) {
    int l = (s[0]=='1' && s[1]=='0') ? 3 : 2;
    return s[l-1]; // 'C','F','Q','P'
}

CardVal to_val(Carta c) {
    CardVal v;
    v.rank = parse_rank(c.nome);
    v.suit = parse_suit(c.nome);
    return v;
}

/* =========================
   VALUTAZIONE MANO 10-100
   ========================= */
int is_straight(int ranks[15]) {
    for (int i = 14; i >= 5; i--) {
        int ok = 1;
        for (int k=0;k<5;k++) if (!ranks[i-k]) { ok = 0; break; }
        if (ok) return i;
    }
    if (ranks[5]&&ranks[4]&&ranks[3]&&ranks[2]&&ranks[14]) return 5; // A-5
    return 0;
}

int is_flush(CardVal cards[], int n, char *flushSuit) {
    int cnt[256]={0};
    for (int i=0;i<n;i++) cnt[(unsigned char)cards[i].suit]++;
    for (int s=0;s<256;s++) if (cnt[s]>=5) { *flushSuit=(char)s; return 1; }
    return 0;
}

HandStrength evaluate7(Carta hole[2], Carta board[], int boardN) {
    CardVal cards[7]; int n=0;
    cards[n++] = to_val(hole[0]);
    cards[n++] = to_val(hole[1]);
    for (int i=0;i<boardN;i++) cards[n++] = to_val(board[i]);

    int rankCnt[15]={0}, suitCnt[256]={0};
    for (int i=0;i<n;i++){ rankCnt[cards[i].rank]++; suitCnt[(unsigned char)cards[i].suit]++; }

    char fs=0;
    int hasFlush = is_flush(cards,n,&fs);

    int ranksForStraight[15]={0};
    for (int r=2;r<=14;r++) if (rankCnt[r]) ranksForStraight[r]=1;
    int highStraight = is_straight(ranksForStraight);

    int pairs=0,trips=0,quads=0;
    for (int r=2;r<=14;r++) {
        if (rankCnt[r]==4) quads++;
        else if (rankCnt[r]==3) trips++;
        else if (rankCnt[r]==2) pairs++;
    }

    int straightFlush=0, royal=0;
    if (hasFlush) {
        CardVal fc[7]; int fn=0;
        for (int i=0;i<n;i++) if (cards[i].suit==fs) fc[fn++] = cards[i];
        int fr[15]={0}; for (int i=0;i<fn;i++) fr[fc[i].rank]=1;
        int hs = is_straight(fr);
        if (hs) { straightFlush=1; if (hs==14) royal=1; }
    }

    HandStrength hsOut = (HandStrength){0,10};
    if (royal)           { hsOut.cat=9; hsOut.strength=100; return hsOut; }
    if (straightFlush)   { hsOut.cat=8; hsOut.strength=95;  return hsOut; }
    if (quads)           { hsOut.cat=7; hsOut.strength=90;  return hsOut; }
    if (trips && pairs)  { hsOut.cat=6; hsOut.strength=85;  return hsOut; }
    if (hasFlush)        { hsOut.cat=5; hsOut.strength=75;  return hsOut; }
    if (highStraight)    { hsOut.cat=4; hsOut.strength=70;  return hsOut; }
    if (trips)           { hsOut.cat=3; hsOut.strength=60;  return hsOut; }
    if (pairs>=2)        { hsOut.cat=2; hsOut.strength=50;  return hsOut; }
    if (pairs==1) {
        hsOut.cat=1;
        int pairRank=0; for (int r=14;r>=2;r--) if (rankCnt[r]==2) { pairRank=r; break; }
        hsOut.strength = 35 + (pairRank-2); // 35..47 circa
        if (hsOut.strength>49) hsOut.strength=49;
        return hsOut;
    }

    // Carta alta
    int top=0; for (int r=14;r>=2;r--) if (rankCnt[r]) { top=r; break; }
    hsOut.cat=0;
    hsOut.strength = 10 + (top-10); // 10..25
    if (hsOut.strength<10) hsOut.strength=10;
    if (hsOut.strength>25) hsOut.strength=25;

    // Draw bonus postflop
    if (boardN>=3) {
        int oesd=0,gut=0;
        for (int i=14;i>=5;i--) {
            int cnt=0; for (int k=0;k<5;k++) cnt += ranksForStraight[i-k];
            if (cnt==4) oesd=1; if (cnt==3) gut=1;
        }
        if (oesd) hsOut.strength+=8; else if (gut) hsOut.strength+=4;
        int maxSuit=0; for (int s=0;s<256;s++) if (suitCnt[s]>maxSuit) maxSuit=s;
        if (suitCnt[maxSuit]==4) hsOut.strength += 10; // 4 to flush

        int hb1=to_val(hole[0]).rank, hb2=to_val(hole[1]).rank;
        int topBoard=0, brCnt[15]={0};
        for (int i=0;i<boardN;i++) brCnt[to_val(board[i]).rank]++;
        for (int r=14;r>=2;r--) if (brCnt[r]) { topBoard=r; break; }
        if (hb1>topBoard) hsOut.strength += 3;
        if (hb2>topBoard) hsOut.strength += 3;
    }

    if (hsOut.strength>69 && hsOut.cat<4) hsOut.strength=69; // clamp per draw forti
    if (hsOut.strength>100) hsOut.strength=100;
    return hsOut;
}

int preflop_strength(Carta hole[2]) {
    CardVal a=to_val(hole[0]), b=to_val(hole[1]);
    int high = a.rank>b.rank?a.rank:b.rank;
    int low  = a.rank>b.rank?b.rank:a.rank;
    int sameSuit = (a.suit==b.suit);
    int gap = high-low;
    if (a.rank==b.rank) {
        int base=35+(a.rank-2);
        if (a.rank>=11) base+=6; // JJ+
        if (a.rank>=13) base+=4; // KK+
        if (base>60) base=60;
        return base;
    }
    int s=10;
    if (high>=14 && low>=10) s=28;
    else if (high>=13 && low>=10) s=24;
    else if (high>=12 && low>=10) s=22;
    else s=15+(high-10);
    if (sameSuit) s+=4;
    if (gap==1) s+=4; else if (gap==2) s+=2; else if (gap>=5) s-=2;
    if (s<10) s=10; if (s>40) s=40;
    return s;
}

HandStrength evaluate_stage(Carta hole[2], Carta board[], int boardN) {
    if (boardN==0) { HandStrength hs={0,preflop_strength(hole)}; return hs; }
    return evaluate7(hole,board,boardN);
}

/* =========================
   CPU: DECISIONI PER DIFFICOLTÀ
   ========================= */
int min_int(int a,int b){return a<b?a:b;}

int cpu_action(Giocatore *g, int puntataMinima, int diff,
               Carta hole[2], Carta board[], int boardN, int pot,
               int idx, int primo_giro_preflop) {
    HandStrength hs = evaluate_stage(hole, board, boardN);
    int S = hs.strength, noise = 0, bluffBase = 0;
    if (diff == 1) { noise = (rand() % 21) - 10; bluffBase = 2; }   
    else if (diff == 2) { noise = (rand() % 11) - 5; bluffBase = 5; }
    else { noise = (rand() % 7) - 3; bluffBase = 10; }

    int perceived = S + noise; 
    if (perceived < 5) perceived = 5; 
    if (perceived > 100) perceived = 100;

    int callCost = puntataMinima - g->puntata; 
    if (callCost < 0) callCost = 0;
    double strain = (g->fish > 0) ? (100.0 * callCost) / (g->fish + callCost) : 100.0;

    int boardScary = 0;
    if (boardN >= 3) {
        int suitCnt[256] = {0}, rankMark[15] = {0};
        for (int i = 0; i < boardN; i++) {
            CardVal v = to_val(board[i]);
            suitCnt[(unsigned char)v.suit]++;
            rankMark[v.rank] = 1;
        }
        int maxSuit = 0; 
        for (int s = 0; s < 256; s++) if (suitCnt[s] > maxSuit) maxSuit = s;
        int streak = 0, best = 0;
        for (int r = 2; r <= 14; r++) {
            if (rankMark[r]) { streak++; if (streak > best) best = streak; }
            else streak = 0;
        }
        if (suitCnt[maxSuit] >= 3) boardScary += 10;
        if (best >= 3) boardScary += 10;
    }
    int bluffChance = bluffBase + boardScary / 3;

    int foldThresh = 20 + (int)(strain / 2.0);
    int raiseThresh = 55 + (boardN * 3);
    if (diff == 1) { raiseThresh += 5; foldThresh += 5; }
    if (diff == 3) { raiseThresh -= 5; foldThresh -= 3; }

    int doBluff = 0;
    if (perceived < foldThresh && boardN >= 3) {
        if (rand() % 100 < bluffChance) doBluff = 1;
    }

    // Controllo big blind primo giro pre-flop per evitare fold
    if (idx == posBigBlind && primo_giro_preflop) {
        if (perceived <= foldThresh) {
            int callAmt = (callCost > g->fish) ? g->fish : callCost;
            g->puntata += callAmt;
            g->fish -= callAmt;
            printf("Giocatore %d chiama %d (big blind obbligatorio)\n", idx + 1, callAmt);
            return g->puntata;
        }
    }

    if (doBluff && g->fish > 0) {
        int raise = (g->fish < callCost + 5 + rand() % 15) ? g->fish : callCost + 5 + rand() % 15;
        g->puntata += raise;
        g->fish -= raise;
        printf("Giocatore %d bluffa %d \n", idx + 1, raise);
        return g->puntata;
    }

    if (perceived <= foldThresh) {
        g->attivo = 0;
        printf("Giocatore %d folda \n", idx + 1);
        return g->puntata;
    }

    if (perceived < raiseThresh || callCost == 0) {
        int toCall = (g->fish < callCost) ? g->fish : callCost;
        g->puntata += toCall; g->fish -= toCall;
        if (toCall > 0)
            printf("Giocatore %d chiama %d \n", idx + 1, toCall);
        else
            printf("Giocatore %d check \n", idx + 1);
        return g->puntata;
    }

    int raiseAmt = callCost + 5 + (perceived - raiseThresh) / 3;
    if (diff == 3) raiseAmt += 5;
    if (raiseAmt < callCost + 5) raiseAmt = callCost + 5;
    if (raiseAmt > g->fish) raiseAmt = g->fish;
    g->puntata += raiseAmt; g->fish -= raiseAmt;
    printf("Giocatore %d rilancia a %d \n", idx + 1, g->puntata);
    return g->puntata;
}


/* =========================
   GESTIONE PUNTATE (1 giro)
   ========================= */

    int scegliFishUtente() {
    int fish = 0;
    while (fish < 10 || fish > 100) {
        printf("Con quanti fish vuoi iniziare? (10-100): ");
        scanf("%d", &fish);
        if (fish < 10 || fish > 100) printf("Valore non valido.\n");
    }
    return fish;
}
int gestionePuntate(Giocatore g[], int numG, int diff,
                    Carta hole[][2], Carta board[], int boardN, int *pot,
                    int startPlayer, int puntataMinima) {
    int puntataMax = puntataMinima;

    for (int cont = 0; cont < numG; cont++) {
        int i = (startPlayer + cont) % numG;

        if (!g[i].attivo) continue;

        int callCost = puntataMax - g[i].puntata;
        if (callCost < 0) callCost = 0;

        if (i == 0) { // UTENTE
            printf("\nLe tue carte: ");
            stampaCarta(g[i].mano[0]); printf(" ");
            stampaCarta(g[i].mano[1]); printf("\n");
            printf("Puntata da chiamare: %d, Fish: %d\n", callCost, g[i].fish);
            printf("1=Check/Call  2=Raise  3=Fold: ");
            int scelta = 1;
            scanf("%d", &scelta);
            if (scelta == 3) {
                g[i].attivo = 0;
                printf("Hai foldato.\n");
            } else if (scelta == 2) {
                int raise;
                printf("Quanto vuoi puntare? ");
                scanf("%d", &raise);
                if (raise > g[i].fish) raise = g[i].fish;
                g[i].puntata += raise;
                g[i].fish -= raise;
                *pot += raise;
                if (g[i].puntata > puntataMax) puntataMax = g[i].puntata;
                printf("Hai rilanciato a %d.\n", g[i].puntata);
            } else {
                int toCall = callCost;
                if (toCall > g[i].fish) toCall = g[i].fish;
                g[i].puntata += toCall;
                g[i].fish -= toCall;
                *pot += toCall;
                if (toCall > 0) printf("Hai chiamato %d.\n", toCall);
                else printf("Check.\n");
            }
        } else { // CPU
            int old = g[i].puntata;
            int primo_giro_preflop = (boardN == 0);
            int newBet = cpu_action(&g[i], puntataMax, diff, hole[i], board, boardN, *pot, i, primo_giro_preflop);
            int delta = newBet - old;
            if (delta < 0) delta = 0;
            *pot += delta;
            if (g[i].attivo && g[i].puntata > puntataMax) puntataMax = g[i].puntata;
        }
    }

    return puntataMax;
}

int main(void) {
    srand((unsigned)time(NULL));
    char risp;
    printf("Ciao, stai cercando un'esperienza rilassante che ti faccia divertire? (V/F): ");
    scanf(" %c", &risp);
    if (risp == 'F' || risp == 'f') return 0;

    int numGioc = 0;
    while (numGioc <= 0 || numGioc > 6) {
        printf("Con quanti giocatori ti vuoi sedere al tavolo? (1-6): ");
        scanf("%d", &numGioc);
        if (numGioc <= 0 || numGioc > 6) printf("Numero non valido.\n");
    }

    int diff = 0;
    while (diff <= 0 || diff > 3) {
        printf("Difficolta' (1=facile,2=media,3=difficile): ");
        scanf("%d", &diff);
        if (diff <= 0 || diff > 3) printf("Valore non valido.\n");
    }

    Giocatore g[6];
    for (int i = 0; i < numGioc; i++) {
        g[i].attivo = 1;
        g[i].puntata = 0;
        if (i == 0)
            g[i].fish = scegliFishUtente();
        else
            g[i].fish = 50 + (rand() % 51);
    }

    int posSmallBlind = 0;
    int posBigBlind = 1;

    while (g[0].fish > 0 && numGioc > 1) {
        for (int i = 0; i < numGioc; ++i)
            g[i].attivo = (g[i].fish > 0);

        int pot = 0;

        caricaMazzoDaFile("Carte.txt");
        mescolaMazzo();

        float sbPerc[] = {0.15, 0.125, 0.10, 0.075, 0.05};
        float bbPerc[] = {0.20, 0.175, 0.15, 0.125, 0.10};
        int idx = numGioc - 2;

        int smallBlind = (int)(g[posSmallBlind].fish * sbPerc[idx]);
        int bigBlind = (int)(g[posBigBlind].fish * bbPerc[idx]);
        if (smallBlind < 1) smallBlind = 1;
        if (bigBlind < 1) bigBlind = 1;
        if (smallBlind > bigBlind / 2) smallBlind = bigBlind / 2;
        if (smallBlind < 1) smallBlind = 1;

        for (int i = 0; i < numGioc; i++) g[i].puntata = 0;

        g[posSmallBlind].puntata = smallBlind;
        g[posSmallBlind].fish -= smallBlind;
        pot += smallBlind;

        g[posBigBlind].puntata = bigBlind;
        g[posBigBlind].fish -= bigBlind;
        pot += bigBlind;

        printf("Small Blind (Giocatore %d) paga %d\n", posSmallBlind + 1, smallBlind);
        printf("Big Blind (Giocatore %d) paga %d\n", posBigBlind + 1, bigBlind);

        for (int i = 0; i < numGioc; i++) {
            g[i].mano[0] = pesca();
            g[i].mano[1] = pesca();
        }
        printf("\n=== DISTRIBUZIONE: Le tue carte: ");
        stampaCarta(g[0].mano[0]);
        printf(" ");
        stampaCarta(g[0].mano[1]);
        printf(" ===\n");

        Carta tavolo[5];
        int boardN = 0;
        Carta hole[6][2];
        for (int i = 0; i < numGioc; i++) {
            hole[i][0] = g[i].mano[0];
            hole[i][1] = g[i].mano[1];
        }

        int puntataMinima = bigBlind;
        int startPlayer = (posBigBlind + 1) % numGioc;

        gestionePuntate(g, numGioc, diff, hole, tavolo, boardN, &pot, startPlayer, puntataMinima);

        int countAttivi = 0;
int ultimoAttivo = -1;
for (int i = 0; i < numGioc; i++) {
    if (g[i].attivo) {
        countAttivi++;
        ultimoAttivo = i;
    }
}

if (countAttivi == 1) {
    // Solo un giocatore attivo -> assegna pot e termina mano
    g[ultimoAttivo].fish += pot;
    if (ultimoAttivo == 0) {
        printf("\nHai vinto la mano perché tutti gli altri hanno foldato! Hai preso %d fish!\n", pot);
        system("clear");
    } else {
        printf("\nGiocatore %d ha vinto la mano perché tutti gli altri hanno foldato e prende %d fish.\n", ultimoAttivo+1, pot);
        system("clear");
    }
    // Salta il resto della mano, esci dal ciclo o prepara la mano successiva
    // Ad esempio:
    continue; // se sei dentro un ciclo while
}


        // FLOP
        tavolo[0] = pesca();
        tavolo[1] = pesca();
        tavolo[2] = pesca();
        boardN = 3;
        printf("\n=== FLOP ===\nCarte sul tavolo: ");
        for (int i = 0; i < 3; i++) {
            stampaCarta(tavolo[i]);
            if (i < 2) printf(", ");
        }
        printf("\n");
        for (int i = 0; i < numGioc; i++) g[i].puntata = 0;
        gestionePuntate(g, numGioc, diff, hole, tavolo, boardN, &pot, startPlayer, 0);

         countAttivi = 0;
 ultimoAttivo = -1;
for (int i = 0; i < numGioc; i++) {
    if (g[i].attivo) {
        countAttivi++;
        ultimoAttivo = i;
    }
//system("clear");
}

if (countAttivi == 1) {
    // Solo un giocatore attivo -> assegna pot e termina mano
    g[ultimoAttivo].fish += pot;
    if (ultimoAttivo == 0) {
        printf("\nHai vinto la mano perché tutti gli altri hanno foldato! Hai preso %d fish!\n", pot);
        system("clear");
    } else {
        printf("\nGiocatore %d ha vinto la mano perché tutti gli altri hanno foldato e prende %d fish.\n", ultimoAttivo+1, pot);
        system("clear");
    }
    // Salta il resto della mano, esci dal ciclo o prepara la mano successiva
    // Ad esempio:
    continue; // se sei dentro un ciclo while
}


        // TURN
        tavolo[3] = pesca();
        boardN = 4;
        printf("\n=== TURN ===\nCarte sul tavolo: ");
        for (int i = 0; i < 4; i++) {
            stampaCarta(tavolo[i]);
            if (i < 3) printf(", ");
        }
        printf("\n");
        for (int i = 0; i < numGioc; i++) g[i].puntata = 0;
        gestionePuntate(g, numGioc, diff, hole, tavolo, boardN, &pot, startPlayer, 0);

         countAttivi = 0;
ultimoAttivo = -1;
for (int i = 0; i < numGioc; i++) {
    if (g[i].attivo) {
        countAttivi++;
        ultimoAttivo = i;
    }
//system("clear");
}

if (countAttivi == 1) {
    // Solo un giocatore attivo -> assegna pot e termina mano
    g[ultimoAttivo].fish += pot;
    if (ultimoAttivo == 0) {
        printf("\nHai vinto la mano perché tutti gli altri hanno foldato! Hai preso %d fish!\n", pot);
        system("clear");
    } else {
        printf("\nGiocatore %d ha vinto la mano perché tutti gli altri hanno foldato e prende %d fish.\n", ultimoAttivo+1, pot);
        system("clear");
    }
    // Salta il resto della mano, esci dal ciclo o prepara la mano successiva
    // Ad esempio:
    continue; // se sei dentro un ciclo while
}


        // RIVER
        tavolo[4] = pesca();
        boardN = 5;
        printf("\n=== RIVER ===\nCarte sul tavolo: ");
        for (int i = 0; i < 5; i++) {
            stampaCarta(tavolo[i]);
            if (i < 4) printf(", ");
        }
        printf("\n");
        for (int i = 0; i < numGioc; i++) g[i].puntata = 0;
        gestionePuntate(g, numGioc, diff, hole, tavolo, boardN, &pot, startPlayer, 0);

        printf("\n=== FINE MANO ===\nPot totale: %d\n", pot);
        printf("Giocatori attivi a showdown:\n");
        for (int i = 0; i < numGioc; i++) {
            if (g[i].attivo) {
                printf("Giocatore %d%s: ", i + 1, (i == 0 ? " (TU)" : ""));
                stampaCarta(g[i].mano[0]);
                printf(" ");
                stampaCarta(g[i].mano[1]);
                printf("\n");
            }
        }
        countAttivi = 0;
ultimoAttivo = -1;
for (int i = 0; i < numGioc; i++) {
    if (g[i].attivo) {
        countAttivi++;
        ultimoAttivo = i;
    }
//system("clear");
}

if (countAttivi == 1) {
    // Solo un giocatore attivo -> assegna pot e termina mano
    g[ultimoAttivo].fish += pot;
    if (ultimoAttivo == 0) {
        printf("\nHai vinto la mano perché tutti gli altri hanno foldato! Hai preso %d fish!\n", pot);
        system("clear");
    } else {
        printf("\nGiocatore %d ha vinto la mano perché tutti gli altri hanno foldato e prende %d fish.\n", ultimoAttivo+1, pot);
        system("clear");
    }
    // Salta il resto della mano, esci dal ciclo o prepara la mano successiva
    // Ad esempio:
    continue; // se sei dentro un ciclo while
}


        // Calcolo vincitore mano
        int vincitore = -1, maxStrength = -1;
        HandStrength hs[6];

        for (int i = 0; i < numGioc; i++) {
            if (g[i].attivo) {
                hs[i] = evaluate_stage(hole[i], tavolo, boardN);
                if (hs[i].strength > maxStrength) {
                    maxStrength = hs[i].strength;
                    vincitore = i;
                } else if (hs[i].strength == maxStrength) {
                    vincitore = -2;
                }
            }
        }

        if (vincitore >= 0) {
            g[vincitore].fish += pot;
            printf("\n--- Vince la mano Giocatore %d%s e prende %d fish! ---\n",
                   vincitore + 1, (vincitore == 0 ? " (TU)" : ""), pot);
        } else if (vincitore == -2) {
            int vincitori[6], nv = 0;
            for (int i = 0; i < numGioc; i++) {
                if (g[i].attivo && hs[i].strength == maxStrength) {
                    vincitori[nv++] = i;
                }
            }
            int quota = pot / nv;
            printf("\n--- Split pot tra %d giocatori: ognuno riceve %d fish ---\n", nv, quota);
            for (int j = 0; j < nv; j++) g[vincitori[j]].fish += quota;
        }

        posSmallBlind = posBigBlind;
        posBigBlind = (posBigBlind + 1) % numGioc;
        system("clear");
    }

    printf("\nGrazie per aver giocato!\n");
    return 0;
}
