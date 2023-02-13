/*
	Canvas pour algorithmes de jeux à 2 joueurs
	
	joueur 0 : humain
	joueur 1 : ordinateur
			
*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

// Paramètres du jeu
#define NB_LIGNES 6
#define NB_COLONNES 7

#define LARGEUR_MAX NB_COLONNES 		// nb max de fils pour un noeud (= nb max de coups possibles)

#define TEMPS 1	// temps de calcul pour un coup avec MCTS (en secondes)

#define C sqrt(2)

// macros
#define AUTRE_JOUEUR(i) (1-(i))
#define min(a, b)       ((a) < (b) ? (a) : (b))
#define max(a, b)       ((a) < (b) ? (b) : (a))


// Critères de fin de partie
typedef enum {NON, MATCHNUL, ORDI_GAGNE, HUMAIN_GAGNE } FinDePartie;

// Definition du type Etat (état/position du jeu)
typedef struct EtatSt {

	int joueur; // à qui de jouer ? 

    char plateau[NB_LIGNES][NB_COLONNES];

} Etat;

// Definition du type Coup
typedef struct {

	int colonne;

} Coup;

// Copier un état 
Etat * copieEtat( Etat * src ) {
	Etat * etat = (Etat *)malloc(sizeof(Etat));

	etat->joueur = src->joueur;

	int i,j;
	for (i=0; i<NB_LIGNES; i++)
		for ( j=0; j<NB_COLONNES; j++)
			etat->plateau[i][j] = src->plateau[i][j];

	return etat;
}

// Etat initial 
Etat * etat_initial( void ) {
	Etat * etat = (Etat *)malloc(sizeof(Etat));

	int i,j;
	for (i=0; i<NB_LIGNES; i++)
		for ( j=0; j<NB_COLONNES; j++)
			etat->plateau[i][j] = ' ';
	
	return etat;
}

void drawLine(){
    printf("\n");
    printf("-");
    for (int k = 0; k < NB_COLONNES; ++k) {
        printf("----");
    }
    printf("\n");
}

void afficheJeu(Etat * etat) {

	// TODO: à compléter

	/* par exemple : */
	int i,j;
    printf("|");
	for ( j = 0; j < NB_COLONNES; j++)
		printf(" %d |", j);

    drawLine();

	for(i=0; i < NB_LIGNES; i++) {
        printf("|");
		for ( j = 0; j < NB_COLONNES; j++)
			printf(" %c |", etat->plateau[i][j]);
		drawLine();
	}
}


// Nouveau coup 
Coup * nouveauCoup( int j ) {
	Coup * coup = (Coup *)malloc(sizeof(Coup));

	coup->colonne = j;
	
	return coup;
}

// Demander à l'humain quel coup jouer 
Coup * demanderCoup () {

	int j;
	printf(" quelle colonne ? ") ;
	scanf("%d",&j); 
	
	return nouveauCoup(j);
}

// Modifier l'état en jouant un coup 
// retourne 0 si le coup n'est pas possible
int jouerCoup( Etat * etat, Coup * coup ) {

    if ( etat->plateau[0][coup->colonne] != ' ' ) return 0;

	else {
        int i = NB_LIGNES-1;
        while(etat->plateau[i][coup->colonne] != ' '){
            i--;
        }
		etat->plateau[i][coup->colonne] = etat->joueur ? 'O' : 'X';

		etat->joueur = AUTRE_JOUEUR(etat->joueur);

		return 1;
	}
}

// Retourne une liste de coups possibles à partir d'un etat 
// (tableau de pointeurs de coups se terminant par NULL)
Coup ** coups_possibles( Etat * etat ) {
	
	Coup ** coups = (Coup **) malloc((1+LARGEUR_MAX) * sizeof(Coup *) );
	
	int k = 0;

	int j;
    for (j=0; j < NB_COLONNES; j++) {
        if ( etat->plateau[0][j] == ' ' ) {
            coups[k] = nouveauCoup(j);
            k++;
        }
    }

	coups[k] = NULL;

	return coups;
}


// Definition du type Noeud 
typedef struct NoeudSt {
		
	int joueur; // joueur qui a joué pour arriver ici
	Coup * coup;   // coup joué par ce joueur pour arriver ici
	
	Etat * etat; // etat du jeu
			
	struct NoeudSt * parent; 
	struct NoeudSt * enfants[LARGEUR_MAX]; // liste d'enfants : chaque enfant correspond à un coup possible
	int nb_enfants;	// nb d'enfants présents dans la liste
	
	// POUR MCTS:
	int nb_victoires;
	int nb_simus;
	
} Noeud;


// Créer un nouveau noeud en jouant un coup à partir d'un parent 
// utiliser nouveauNoeud(NULL, NULL) pour créer la racine
Noeud * nouveauNoeud (Noeud * parent, Coup * coup ) {
	Noeud * noeud = (Noeud *)malloc(sizeof(Noeud));
	
	if ( parent != NULL && coup != NULL ) {
		noeud->etat = copieEtat ( parent->etat );
		jouerCoup ( noeud->etat, coup );
		noeud->coup = coup;			
		noeud->joueur = AUTRE_JOUEUR(parent->joueur);		
	}
	else {
		noeud->etat = NULL;
		noeud->coup = NULL;
		noeud->joueur = 0; 
	}
	noeud->parent = parent; 
	noeud->nb_enfants = 0; 
	
	// POUR MCTS:
	noeud->nb_victoires = 0;
	noeud->nb_simus = 0;	
	

	return noeud; 	
}

// Ajouter un enfant à un parent en jouant un coup
// retourne le pointeur sur l'enfant ajouté
Noeud * ajouterEnfant(Noeud * parent, Coup * coup) {
	Noeud * enfant = nouveauNoeud (parent, coup ) ;
	parent->enfants[parent->nb_enfants] = enfant;
	parent->nb_enfants++;
	return enfant;
}

void freeNoeud ( Noeud * noeud) {
	if ( noeud->etat != NULL)
		free (noeud->etat);
		
	while ( noeud->nb_enfants > 0 ) {
		freeNoeud(noeud->enfants[noeud->nb_enfants-1]);
		noeud->nb_enfants --;
	}
	if ( noeud->coup != NULL)
		free(noeud->coup); 

	free(noeud);
}
	

// Test si l'état est un état terminal 
// et retourne NON, MATCHNUL, ORDI_GAGNE ou HUMAIN_GAGNE
FinDePartie testFin( Etat * etat ) {

	// tester si un joueur a gagné
	int i,j,k,n = 0;
	for ( i=0;i < NB_LIGNES; i++) {
		for(j=0; j < NB_COLONNES; j++) {
			if ( etat->plateau[i][j] != ' ') {
				n++;	// nb coups joués
			
				// lignes
				k=0;
				while ( k < 4 && i+k < NB_LIGNES && etat->plateau[i+k][j] == etat->plateau[i][j] )
					k++;
				if ( k == 4 )
					return etat->plateau[i][j] == 'O'? ORDI_GAGNE : HUMAIN_GAGNE;

				// colonnes
				k=0;
				while ( k < 4 && j+k < NB_COLONNES && etat->plateau[i][j+k] == etat->plateau[i][j] )
					k++;
				if ( k == 4 )
					return etat->plateau[i][j] == 'O'? ORDI_GAGNE : HUMAIN_GAGNE;

				// diagonales
				k=0;
				while ( k < 4 && i+k < NB_LIGNES && j+k < NB_COLONNES && etat->plateau[i+k][j+k] == etat->plateau[i][j] )
					k++;
				if ( k == 4 )
					return etat->plateau[i][j] == 'O'? ORDI_GAGNE : HUMAIN_GAGNE;

				k=0;
				while ( k < 4 && i+k < NB_LIGNES && j-k >= 0 && etat->plateau[i+k][j-k] == etat->plateau[i][j] )
					k++;
				if ( k == 4 )
					return etat->plateau[i][j] == 'O'? ORDI_GAGNE : HUMAIN_GAGNE;		
			}
		}
	}

	// et sinon tester le match nul	
	if ( n == NB_LIGNES*NB_COLONNES)
		return MATCHNUL;
		
	return NON;
}

float bValue(Noeud* i, int max){
    float mu = ((float)i->nb_victoires)/((float)i->nb_simus);
    if(!max) mu = -mu;
    return mu + C * sqrt(log(i->parent->nb_simus)/i->nb_simus);
}

Noeud * selectMCTS(Noeud* current, int max ){

    if (testFin(current->etat) != NON)
        return current;

    for (int i = 0; i < current->nb_enfants; ++i) {
        if (current->enfants[i]->nb_simus == 0)
            return current->enfants[i];
    }

    Noeud* selected = current->enfants[0];
    float bmax = bValue(selected, max);
    for (int i = 1; i < current->nb_enfants; ++i) {
        float tmpBValue = bValue(current->enfants[i], max);
        if(bmax < tmpBValue){
            bmax = tmpBValue;
            selected = current->enfants[i];
        }
    }
    selectMCTS(selected, !max);
}

// Calcule et joue un coup de l'ordinateur avec MCTS-UCT
// en tempsmax secondes
void ordijoue_mcts(Etat * etat, int tempsmax) {

	clock_t tic, toc;
	tic = clock();
	int temps;

	Coup ** coups;
	Noeud * meilleur_enfant ;
	
	// Créer l'arbre de recherche
	Noeud * racine = nouveauNoeud(NULL, NULL);	
	racine->etat = copieEtat(etat); 
	
	// créer les premiers noeuds:
	coups = coups_possibles(racine->etat); 
	int k = 0;
	Noeud * enfant;
	while ( coups[k] != NULL) {
		enfant = ajouterEnfant(racine, coups[k]);
		k++;
	}
	
	//meilleur_coup = coups[ rand()%k ]; // choix aléatoire
	
	/*  TODO :
		- supprimer la sélection aléatoire du meilleur coup ci-dessus
		- implémenter l'algorithme MCTS-UCT pour déterminer le meilleur coup ci-dessous
    */

	int iter = 0;
	Noeud * current;

	do {
        //select
        current = selectMCTS(racine, racine->joueur);

        //development
        if (testFin(current->etat) == NON){
            if(current->nb_enfants == 0){
                coups = coups_possibles(current->etat);
                int k = 0;
                while ( coups[k] != NULL) {
                    ajouterEnfant(current, coups[k]);
                    k++;
                }
            }

            int idxInexplo[current->nb_enfants];
            int k = 0;
            for (int i = 0; i < current->nb_enfants; ++i) {
                if (current->enfants[i]->nb_simus == 0) {
                    idxInexplo[k] = i;
                    k++;
                }
            }
            current = current->enfants[idxInexplo[rand()%k]];
        }

        //simulate
        FinDePartie etatPartie = testFin(current->etat);
        while(etatPartie == NON){
            coups = coups_possibles(current->etat);
            int k = 0;
            while ( coups[k] != NULL) {
                ajouterEnfant(current, coups[k]);
                k++;
            }
            int idx = rand()%k;
            current = current->enfants[idx];
            etatPartie = testFin(current->etat);
        }

        int r = etatPartie == ORDI_GAGNE ? 1 : 0; //perdu par default
        current->nb_victoires += r;
        current->nb_simus++;

        //mise à jour
        Noeud* parent = current->parent;
        while (parent != NULL){
            parent->nb_simus++;
            parent->nb_victoires += r;
            parent = parent->parent;
        }
	
		toc = clock(); 
		temps = (int)( ((double) (toc - tic)) / CLOCKS_PER_SEC );
		iter ++;
	} while ( temps < tempsmax );

    float mu = (racine->enfants[0]->nb_victoires != 0 && racine->enfants[0]->nb_simus != 0) ? ((float)racine->enfants[0]->nb_victoires)/((float)racine->enfants[0]->nb_simus) : 0.f;
    meilleur_enfant = racine->enfants[0];
    for (int i = 0; i < racine->nb_enfants; ++i) {
        float tmpMu = (racine->enfants[i]->nb_victoires != 0 && racine->enfants[i]->nb_simus != 0) ? ((float)racine->enfants[i]->nb_victoires)/((float)racine->enfants[i]->nb_simus) : 0.f;
        //printf("%d - %d - %f\n", racine->enfants[i]->nb_victoires, racine->enfants[i]->nb_simus, tmpMu);
        if(mu < tmpMu) {
            meilleur_enfant = racine->enfants[i];
            mu = tmpMu;
        }
    }
	/* fin de l'algorithme  */ 
	// Jouer le meilleur premier coup
    printf("nb simu : %d, proba victoire : %f",meilleur_enfant->nb_simus,mu);
	jouerCoup(etat, meilleur_enfant->coup);
	
	// Penser à libérer la mémoire :
	freeNoeud(racine);
	free (coups);
}

int main(void) {

	Coup * coup;
	FinDePartie fin;
	
	// initialisation
	Etat * etat = etat_initial(); 
	
	// Choisir qui commence : 
	printf("Qui commence (0 : humain, 1 : ordinateur) ? ");
	scanf("%d", &(etat->joueur) );
	
	// boucle de jeu
	do {
		printf("\n");
		afficheJeu(etat);
		
		if ( etat->joueur == 0 ) {
			// tour de l'humain
			
			do {
				coup = demanderCoup();
			} while ( !jouerCoup(etat, coup) );
									
		}
		else {
			// tour de l'Ordinateur
			
			ordijoue_mcts( etat, TEMPS );
			
		}
		
		fin = testFin( etat );
	}	while ( fin == NON ) ;

	printf("\n");
	afficheJeu(etat);
		
	if ( fin == ORDI_GAGNE )
		printf( "** L'ordinateur a gagné **\n");
	else if ( fin == MATCHNUL )
		printf(" Match nul !  \n");
	else
		printf( "** BRAVO, l'ordinateur a perdu  **\n");
	return 0;
}
