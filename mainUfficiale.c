#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdbool.h>

struct box{
    char *testo; //riga di testo da max 1024 caratteri
};

struct pila{
    int ind1, ind2;
    char *letteraComando;
    char **configurazione; //mi salvo la configurazione dei puntatori alle righe
    struct pila *next;
    int righe; //serve solo per debuggare -> da togliere alla fine
    int indMaxPreOp;
    int indMaxPostOp;
    bool interaConfigurazioneSaved; //true quando salvo l'intera configurazione (ogni tot change e quando faccio delete)
};

int counter=1;
int indirizzoMax = 0;
int dimensioneBox = 0;
int qntChange=0;

char **Puntatori = NULL;

struct box *Change(struct box *arrayFrasi, int ind1, int ind2, int righeRichieste){
    //creo la struttura che contiene tutti i testi
    //e anche la struttura con i puntatori a questi testi

    if(indirizzoMax<ind2) indirizzoMax = ind2;


    if(50000*counter<indirizzoMax){
        counter++;
        Puntatori = realloc(Puntatori, sizeof(char*)*50000*counter);
    }

    int i, s=0;
    char punto[4];
    char str[1027];
    arrayFrasi = (struct box*)realloc(arrayFrasi, sizeof(struct box)*dimensioneBox);

    for(i=dimensioneBox-righeRichieste; i<dimensioneBox; i++){
        fgets(str, 1024, stdin);
        (arrayFrasi+i)->testo = malloc(strlen(str)+2);
        strcpy((arrayFrasi+i)->testo, str); //vedi memcpy
        Puntatori[ind1-1+s] = (arrayFrasi+i)->testo;
        s++;
    }
    fgets(punto, 4, stdin);
    return arrayFrasi;
}

void Delete(int ind1, int ind2){

    int righedaEliminare;
    int i=0;

    if(ind2>indirizzoMax) ind2=indirizzoMax;

    righedaEliminare=ind2-ind1+1;

    if(ind2 == indirizzoMax){ //se 1,5c e facciamo x,5d -> pongo uguale a NULL gli ultimi puntatori
        for (i=ind1-1; i<ind2; i++) {
            Puntatori[i] = NULL;
        }
    }

    else{
        for (i=ind1-1; i<indirizzoMax; i++) {
            if(i+righedaEliminare < indirizzoMax){
                Puntatori[i] = Puntatori[i+righedaEliminare];
            }
            else Puntatori[i] = NULL;

        }
    }

    indirizzoMax = indirizzoMax-righedaEliminare;
}


void DeleteUndo(int ind1, int ind2){ // cancello le righe che poi andranno sostituite

    int i;
    for(i=ind1-1; i<ind2; i++){
        Puntatori[i] = NULL;
    }
}

struct pila *CreatePilaUndo(struct pila *StackUndo, int ind1, int ind2, char *lettera){

    int i;
    struct pila *newNode = NULL;
    newNode = malloc(sizeof(struct pila));

    newNode->ind1 = ind1;
    newNode->ind2 = ind2;
    newNode->indMaxPreOp = indirizzoMax;
    newNode->indMaxPostOp = indirizzoMax;
    newNode->letteraComando = malloc(sizeof(char)*2);
    strcpy(newNode->letteraComando, lettera);

    if(lettera[0] == 99){ //se la lettera è una c -> mi salvo solo i puntatori che variano

        if(qntChange==110){ //SALVO TUTTI I PUNTATORI POST CHANGE

            newNode->configurazione = malloc(sizeof(char*)*(indirizzoMax)+1); //dimensione della configurazione uguale al numero di righe da salvare

            for(i=0; i<indirizzoMax; i++){
                newNode->configurazione[i] = Puntatori[i];
            }
            newNode->righe = indirizzoMax;
            newNode->interaConfigurazioneSaved = true;
            qntChange=0;
        }
        else {

            newNode->righe = ind2-ind1+1;
            newNode->interaConfigurazioneSaved = false;
            newNode->configurazione = malloc(sizeof(char*)*(newNode->righe)); //dimensione della configurazione uguale al numero di righe da salvare

            for(i=0; i<newNode->righe; i++){
                newNode->configurazione[i] = Puntatori[ind1-1+i];
            }
        }

    }

    if(lettera[0] == 100){ //quando faccio la delete, mi devo prima salvare tutta la configurazione dei puntatori

        newNode->configurazione = malloc(sizeof(char*)*(indirizzoMax)+1); //dimensione della configurazione uguale al numero di righe da salvare
        newNode->interaConfigurazioneSaved = true;
        for(i=0; i<indirizzoMax; i++){
            newNode->configurazione[i] = Puntatori[i];
        }
        newNode->righe = indirizzoMax;
    }

    newNode->next = StackUndo;

    return newNode;

}

struct pila *ModificaIndMax(struct pila *StackUndo){

    StackUndo->indMaxPostOp = indirizzoMax;
    return StackUndo;
}

void ModificaPile(struct pila **sorgente, struct pila **dest){

    struct pila *newNode = *sorgente;

    *sorgente = (*sorgente)->next;
    newNode->next = *dest;
    *dest = newNode;

}


void Print(int ind1, int ind2){

    int i;
    for (i=ind1-1; i<ind2; i++){
        if(i<indirizzoMax){
            fputs(Puntatori[i], stdout);
        }
        else fputs(".\n", stdout);

    }
}


int main() {

    struct box *arrayFrasi = NULL; //array che contiene tutte le frasi
    arrayFrasi = malloc(sizeof(struct box));

    Puntatori = malloc(sizeof(char*)*50000); //array di puntatori alle frasi

    struct pila *StackUndo = NULL;
    struct pila *StackRedo = NULL;

    char comandiInput[200]; // stringa che raccoglie i comandi dall'input

    int numIstruzioniStackUndo=0; //quantità di istruzioni presente all'interno di StackUndo
    int ind1, ind2;
    int istruzionidaEseguire; //per undo e redo
    int righeRichieste;
    int i=0, s, e=0;
    int ind;
    int print=0;

    int numIstruzioniStackRedo=0;
    int copianumIstrStackUndo;
    int copianumIstrStackRedo;
    int differenzaUndoRedo = 0;
    int differenzaRedoUndo = 0;
    int diffPosizLastConfig=0;

    bool interaConfTrovata = false;
    int posizioneConf=0;
    bool continua = true;

    // PARSING DELL'INPUT
    do {
        fgets(comandiInput, 200, stdin);
        s = 0;

        switch (comandiInput[strlen(comandiInput) - 2]) {

            case 'c':
                sscanf(comandiInput, "%d,%d", &ind1, &ind2);
                numIstruzioniStackUndo++;
                qntChange++;


                righeRichieste = ind2 - ind1 + 1;
                dimensioneBox = dimensioneBox + righeRichieste;

                arrayFrasi = Change(arrayFrasi, ind1, ind2, righeRichieste);
                StackUndo = CreatePilaUndo(StackUndo, ind1, ind2,"c"); //mi salvo i puntatori

                StackRedo = NULL;
                numIstruzioniStackRedo = 0;
                differenzaRedoUndo = 0;

                break;


            case 'd':
                sscanf(comandiInput, "%d,%d", &ind1, &ind2);

                numIstruzioniStackUndo++;

                StackUndo = CreatePilaUndo(StackUndo, ind1, ind2,"d"); //mi salvo l'intera configurazione prima di fare delete

                if (ind1 <= indirizzoMax && ind2 != 0) {

                    if (ind1 == 0) { //CASO 0,xd
                        Delete(1, ind2); //pongo ind1=1
                    } else { // TUTTI GLI ALTRI CASI
                        Delete(ind1, ind2);
                    }

                }
                StackUndo = ModificaIndMax(StackUndo);
                StackRedo = NULL;
                numIstruzioniStackRedo = 0;
                break;


            case 'p':
                sscanf(comandiInput, "%d,%d", &ind1, &ind2);

                if (ind1 == 0 && ind2 == 0) {
                    fputs(".\n", stdout);
                    break;
                } else if (ind1 == 0 && ind2 != 0) {
                    fputs(".\n", stdout);
                    ind1 = 1;
                }
                Print(ind1, ind2);
                break;

            default:
                continua=false;

            case 'u':
            case 'r':
                sscanf(comandiInput, "%d", &istruzionidaEseguire);
                differenzaUndoRedo=0;
                differenzaRedoUndo=0;

                if (StackUndo != NULL || StackRedo != NULL) {

                    copianumIstrStackUndo = numIstruzioniStackUndo;
                    copianumIstrStackRedo = numIstruzioniStackRedo;

                    if (comandiInput[strlen(comandiInput) - 2] == 'u') {
                        if (istruzionidaEseguire > copianumIstrStackUndo) istruzionidaEseguire = copianumIstrStackUndo;
                        copianumIstrStackUndo = copianumIstrStackUndo - istruzionidaEseguire;
                        copianumIstrStackRedo = numIstruzioniStackRedo + istruzionidaEseguire;
                    }

                    if (comandiInput[strlen(comandiInput) - 2] == 'r') {
                        if (istruzionidaEseguire > copianumIstrStackRedo) istruzionidaEseguire = copianumIstrStackRedo;
                        copianumIstrStackRedo = copianumIstrStackRedo - istruzionidaEseguire;
                        copianumIstrStackUndo = copianumIstrStackUndo + istruzionidaEseguire;

                    }

                    do { //LETTURA IN AVANTI DELL'INPUT
                        fgets(comandiInput, 200, stdin);
                        sscanf(comandiInput, "%d", &istruzionidaEseguire);

                        if (comandiInput[strlen(comandiInput) - 2] == 117 || comandiInput[strlen(comandiInput) - 2] == 114) { //se sono u o r
                            switch (comandiInput[strlen(comandiInput) - 2]) {
                                case 'u':

                                    if (istruzionidaEseguire > copianumIstrStackUndo) istruzionidaEseguire = copianumIstrStackUndo;
                                    copianumIstrStackUndo = copianumIstrStackUndo - istruzionidaEseguire;
                                    copianumIstrStackRedo = copianumIstrStackRedo + istruzionidaEseguire;

                                    break;

                                case 'r':

                                    if (istruzionidaEseguire > copianumIstrStackRedo) istruzionidaEseguire = copianumIstrStackRedo;
                                    copianumIstrStackUndo = copianumIstrStackUndo + istruzionidaEseguire;
                                    copianumIstrStackRedo = copianumIstrStackRedo - istruzionidaEseguire;

                                    break;

                            }
                        }

                    } while (comandiInput[strlen(comandiInput) - 2] == 117 || comandiInput[strlen(comandiInput) - 2] == 114); //vado avanti fino a che non ci sono u o r

                    differenzaUndoRedo = numIstruzioniStackUndo - copianumIstrStackUndo; //eseguo tot undo

                    differenzaRedoUndo = numIstruzioniStackRedo - copianumIstrStackRedo;


                    if (differenzaUndoRedo > 0) { //eseguo le undo
                        istruzionidaEseguire = differenzaUndoRedo;

                        interaConfTrovata = false;
                        posizioneConf = 0;
                        struct pila *temp, *scorrimento, *lastfound;
                        scorrimento = StackUndo;
                        lastfound = StackUndo;

                        i = 0;
                        while (i < istruzionidaEseguire) { //andiamo nella prima lettera con configurazione intera
                            if (scorrimento->interaConfigurazioneSaved == true) {
                                lastfound = scorrimento;
                                posizioneConf = i; //indica la posizione di una delete partendo da undo escluso
                                interaConfTrovata = true;
                            }
                            if (scorrimento->next != NULL || scorrimento != NULL) {
                                scorrimento = scorrimento->next;
                            }
                            i++;
                        }

                        diffPosizLastConfig = istruzionidaEseguire - posizioneConf; //istruzioni rimanenti da eseguire

                        if (interaConfTrovata == true) { //sposto in redo tutto ciò che non mi serve
                            for (i = 0; i < posizioneConf; i++) {
                                ModificaPile(&StackUndo, &StackRedo);
                            }
                            StackUndo = lastfound;

                            istruzionidaEseguire = diffPosizLastConfig;

                            if(*StackUndo->letteraComando == 'c'){

                                for (int j = 0; j < StackUndo->indMaxPostOp; j++) {
                                    Puntatori[j] = StackUndo->configurazione[j];
                                }
                                indirizzoMax=StackUndo->indMaxPostOp;
                            }

                            else{
                                for (int j = 0; j < StackUndo->indMaxPreOp; j++) {
                                    Puntatori[j] = StackUndo->configurazione[j];
                                }
                                istruzionidaEseguire--;
                                indirizzoMax=StackUndo->indMaxPreOp;
                                ModificaPile(&StackUndo, &StackRedo);
                            }
                        }


                        while (istruzionidaEseguire > 0) {

                            switch (*StackUndo->letteraComando) {

                                case 'c':

                                    if (StackUndo->next != NULL) {

                                        if (StackUndo->ind1 > StackUndo->next->indMaxPostOp) { //caso di WriteOnly
                                            DeleteUndo(StackUndo->ind1, StackUndo->ind2); //faccio l'operazione inversa, eliminando le ultime righe inserite
                                            indirizzoMax = StackUndo->next->indMaxPostOp;
                                        }
                                        else { //se sono stringhe che si sovrappongono, devo andare a prendere a ritroso le stringhe mancanti

                                            DeleteUndo(StackUndo->ind1, StackUndo->ind2); //creo spazio per inserire poi le stringhe

                                            ind = StackUndo->ind1;

                                            while (ind <= StackUndo->next->indMaxPostOp) { //cambiato iin postop
                                                e=0;
                                                temp = StackUndo;
                                                while (ind < temp->next->ind1 || ind > temp->next->ind2 && *temp->next->letteraComando == 99) {
                                                    temp = temp->next;
                                                }

                                                switch (*temp->next->letteraComando) {

                                                    case 'c':
                                                        Puntatori[ind-1] = temp->next->configurazione[ind-temp->next->ind1];
                                                        ind++;
                                                        break;

                                                    case 'd': //anche nel caso di una change con configurazione salvata

                                                        if(ind<temp->next->ind1) Puntatori[ind-1] = temp->next->configurazione[ind-1];
                                                        else Puntatori[ind-1] = temp->next->configurazione[ind+temp->next->ind2-temp->next->ind1];
                                                        ind++;
                                                        break;
                                                }
                                            }

                                            indirizzoMax = temp->next->indMaxPostOp;
                                        }
                                    } //fine controllo stackundo null

                                    else {
                                        DeleteUndo(StackUndo->ind1, StackUndo->ind2);
                                        indirizzoMax = 0;
                                    }

                                    break;


                                case 'd':
                                    for (i = 0; i < StackUndo->indMaxPreOp; i++) {
                                        Puntatori[i] = StackUndo->configurazione[i];
                                    }
                                    indirizzoMax = StackUndo->indMaxPreOp;
                                    break;
                            }

                            ModificaPile(&StackUndo, &StackRedo);
                            istruzionidaEseguire--;
                        }

                    }//fine undo

                    if (differenzaRedoUndo > 0) { //eseguo le redo

                        istruzionidaEseguire = differenzaRedoUndo;

                        interaConfTrovata = false;
                        posizioneConf = 0;
                        struct pila *temp, *scorrimento, *lastfound;
                        scorrimento = StackRedo;
                        lastfound = StackRedo;

                        i = 0;
                        while (i < istruzionidaEseguire) { //andiamo nella prima 'd' trovata
                            if (scorrimento->interaConfigurazioneSaved == true) {
                                lastfound = scorrimento;
                                posizioneConf = i; //indica la posizione di una delete partendo da undo escluso
                                interaConfTrovata = true;
                            }
                            if (scorrimento->next != NULL || scorrimento != NULL) {
                                scorrimento = scorrimento->next;
                            }
                            i++;
                        }

                        diffPosizLastConfig = istruzionidaEseguire - posizioneConf; //istruzioni rimanenti da eseguire

                        if (interaConfTrovata == true) { //sposto in redo tutto ciò che non mi serve
                            for (i = 0; i < posizioneConf; i++) {
                                ModificaPile(&StackRedo, &StackUndo);
                            }
                            StackRedo = lastfound;

                            istruzionidaEseguire = diffPosizLastConfig;

                            if(*StackRedo->letteraComando == 'c'){ //se last found è una c, mi salvo la configurazione e finisco la redo

                                for (int j = 0; j < StackRedo->indMaxPostOp; j++) {
                                    Puntatori[j] = StackRedo->configurazione[j];
                                }
                                indirizzoMax=StackRedo->indMaxPostOp;
                                istruzionidaEseguire--;
                                ModificaPile(&StackRedo, &StackUndo);
                            }

                            else{
                                for (int j = 0; j < StackRedo->indMaxPreOp; j++) {
                                    Puntatori[j] = StackRedo->configurazione[j];
                                }
                                indirizzoMax=StackRedo->indMaxPreOp;
                            }

                        }

                        if (StackRedo != NULL) {

                        while (istruzionidaEseguire > 0) {

                            switch (*StackRedo->letteraComando) {

                                case 'c':

                                    for (i = 0; i < StackRedo->righe; i++) {
                                        Puntatori[StackRedo->ind1 - 1 +
                                                  i] = StackRedo->configurazione[i];
                                    }
                                    indirizzoMax = StackRedo->indMaxPostOp;

                                    break;

                                case 'd':
                                    Delete(StackRedo->ind1, StackRedo->ind2);
                                    indirizzoMax = StackRedo->indMaxPostOp;
                                    break;
                            }

                            ModificaPile(&StackRedo, &StackUndo);
                            istruzionidaEseguire--;
                            }
                        }

                    }//fine redo

                    numIstruzioniStackUndo = copianumIstrStackUndo;
                    numIstruzioniStackRedo = copianumIstrStackRedo;

                    switch (comandiInput[strlen(comandiInput) - 2]) {

                        case 'c':
                            sscanf(comandiInput, "%d,%d", &ind1, &ind2);
                            numIstruzioniStackUndo++;
                            qntChange++;

                            righeRichieste = ind2 - ind1 + 1;
                            dimensioneBox = dimensioneBox + righeRichieste;

                            arrayFrasi = Change(arrayFrasi, ind1, ind2, righeRichieste);

                            StackUndo = CreatePilaUndo(StackUndo, ind1, ind2,"c"); //mi salvo i puntatori

                            //dopo change o delete la pila dei redo si cancella
                            StackRedo = NULL;
                            numIstruzioniStackRedo = 0;
                            differenzaRedoUndo = 0;

                            break;

                        case 'd':
                            sscanf(comandiInput, "%d,%d", &ind1, &ind2);

                            numIstruzioniStackUndo++;
                            
                            StackUndo = CreatePilaUndo(StackUndo, ind1, ind2,"d"); //mi salvo l'intera configurazione prima di fare delete

                            if (ind1 <= indirizzoMax && ind2 != 0) {

                                if (ind1 == 0) { //CASO 0,xd
                                    Delete(1, ind2); //pongo ind1=1
                                } else { // TUTTI GLI ALTRI CASI
                                    Delete(ind1, ind2);
                                }

                            }
                            StackUndo = ModificaIndMax(StackUndo);
                            //dopo change o delete la pila dei redo si cancella
                            StackRedo = NULL;
                            numIstruzioniStackRedo = 0;
                            break;

                        case 'p':
                            sscanf(comandiInput, "%d,%d", &ind1, &ind2);

                            if (ind1 == 0 && ind2 == 0) {
                                fputs(".\n", stdout);
                                break;
                            } else if (ind1 == 0 && ind2 != 0) {
                                fputs(".\n", stdout);
                                ind1 = 1;
                            }
                            Print(ind1, ind2);
                            break;

                        default:
                            continua = false;
                            return 0;
                    }
                }
                break;

        }

    }while (continua);
    return 0;

}
