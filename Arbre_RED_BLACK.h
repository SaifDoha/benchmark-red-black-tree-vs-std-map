#ifndef ARBRE_RED_BLACK_H_INCLUDED
#define ARBRE_RED_BLACK_H_INCLUDED
#define RESET  "\033[0m"
#define ROUGE  "\033[31m"
#define NOIR   "\033[90m"

enum Color {
    RED,
    BLACK
};

struct Noeud {
    int data;
    Color color;
    Noeud *left;
    Noeud *right;
    Noeud *parent;
};

class RedBlackTree {

private:

    Noeud* racine;

    long long nbRotationsGauche = 0;
    long long nbRotationsDroite = 0;
    long long nbComparaisons = 0;
    long long nbIterationsFix = 0;
    long long nbChangementsCouleur = 0;

public:

    RedBlackTree() {
        racine = nullptr;
    }

    ~RedBlackTree();

    void insert(int value);
    bool rechercher(int valeur);
    void supprimer(int valeur);
    void afficher();

    long long getRotationsGauche() const {
        return nbRotationsGauche;
    }

    long long getRotationsDroite() const {
        return nbRotationsDroite;
    }

    long long getNbComparaisons() const {
        return nbComparaisons;
    }

    long long getNbIterationsFix() const {
        return nbIterationsFix;
    }

    long long getNbChangementsCouleur() const {
        return nbChangementsCouleur;
    }

private:

    Noeud* createNoeud(int valeur);

    void changerCouleur(Noeud* n, Color nouvelleCouleur);

    void rotateLeft(Noeud* x);
    void rotateRight(Noeud* x);

    void fixInsert(Noeud* z);
    void fixDelete(Noeud* x, Noeud* parent, bool xEstGauche);

    Noeud* chercherNoeud(int valeur);
    Noeud* minimum(Noeud* n);

    void transplant(Noeud* u, Noeud* v);

    void libererRec(Noeud* n);
    void afficherRec(Noeud* n, int espace);
};

#endif
