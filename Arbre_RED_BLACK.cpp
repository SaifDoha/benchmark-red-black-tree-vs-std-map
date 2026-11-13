#include <iostream>
#include "Arbre_RED_BLACK.h"

using namespace std;

Noeud* RedBlackTree::createNoeud(int valeur) {
    Noeud *n = new Noeud;
    n->data = valeur;
    n->color = RED;
    n->left = nullptr;
    n->right = nullptr;
    n->parent = nullptr;
    return n;
}

void RedBlackTree::changerCouleur(Noeud* n,Color nouvelleCouleur) {
    if (n != nullptr && n->color != nouvelleCouleur) {
        n->color = nouvelleCouleur;
        nbChangementsCouleur++;
    }
}

void RedBlackTree::rotateLeft(Noeud *x) {
    nbRotationsGauche++;
    Noeud *y = x->right;
    x->right = y->left;
    if (y->left != nullptr)
        y->left->parent = x;
    y->parent = x->parent;
    if (x->parent == nullptr)
        racine = y;
    else if (x == x->parent->left)
        x->parent->left = y;
    else
        x->parent->right = y;

    y->left = x;
    x->parent = y;
}

void RedBlackTree::rotateRight(Noeud *x) {

    nbRotationsDroite++;

    Noeud *y = x->left;

    x->left = y->right;

    if (y->right != nullptr)
        y->right->parent = x;

    y->parent = x->parent;

    if (x->parent == nullptr)
        racine = y;

    else if (x == x->parent->right)
        x->parent->right = y;

    else
        x->parent->left = y;

    y->right = x;
    x->parent = y;
}

// ==================== INSERTION ====================

void RedBlackTree::insert(int value) {

    Noeud *z = createNoeud(value);

    Noeud *y = nullptr;
    Noeud *x = racine;

    while (x != nullptr) {

        y = x;

        nbComparaisons++;

        if (value < x->data)
            x = x->left;

        else {

            nbComparaisons++;

            if (value > x->data)
                x = x->right;

            else {

                delete z;
                return;
            }
        }
    }

    z->parent = y;

    if (y == nullptr)
        racine = z;

    else if (value < y->data)
        y->left = z;

    else
        y->right = z;

    fixInsert(z);
}

// ==================== CORRECTION APRES INSERTION ====================

void RedBlackTree::fixInsert(Noeud *z) {

    while (z != racine && z->parent->color == RED) {

        nbIterationsFix++;

        Noeud *p = z->parent;
        Noeud *g = p->parent;

        if (p == g->left) {

            Noeud *u = g->right;

            // Cas 1 : oncle rouge
            if (u != nullptr && u->color == RED) {

                changerCouleur(p, BLACK);
                changerCouleur(u, BLACK);
                changerCouleur(g, RED);

                z = g;
            }

            else {

                // Cas 2 : triangle gauche-droite
                if (z == p->right) {

                    z = p;

                    rotateLeft(z);

                    p = z->parent;
                    g = p->parent;
                }

                // Cas 3 : ligne gauche-gauche
                changerCouleur(p, BLACK);
                changerCouleur(g, RED);

                rotateRight(g);
            }
        }

        else {

            Noeud *u = g->left;

            // Cas 1 : oncle rouge
            if (u != nullptr && u->color == RED) {

                changerCouleur(p, BLACK);
                changerCouleur(u, BLACK);
                changerCouleur(g, RED);

                z = g;
            }

            else {

                // Cas 2 : triangle droite-gauche
                if (z == p->left) {

                    z = p;

                    rotateRight(z);

                    p = z->parent;
                    g = p->parent;
                }

                // Cas 3 : ligne droite-droite
                changerCouleur(p, BLACK);
                changerCouleur(g, RED);

                rotateLeft(g);
            }
        }
    }

    changerCouleur(racine, BLACK);
}

// ==================== RECHERCHE ====================

Noeud* RedBlackTree::chercherNoeud(int valeur) {

    Noeud* x = racine;

    while (x != nullptr) {

        nbComparaisons++;

        if (valeur == x->data)
            return x;

        nbComparaisons++;

        if (valeur < x->data)
            x = x->left;

        else
            x = x->right;
    }

    return nullptr;
}

bool RedBlackTree::rechercher(int valeur) {

    return chercherNoeud(valeur) != nullptr;
}

// ==================== MINIMUM ====================

Noeud* RedBlackTree::minimum(Noeud* n) {

    while (n->left != nullptr)
        n = n->left;

    return n;
}

// ==================== TRANSPLANTATION ====================

void RedBlackTree::transplant(Noeud* u, Noeud* v) {

    if (u->parent == nullptr)
        racine = v;

    else if (u == u->parent->left)
        u->parent->left = v;

    else
        u->parent->right = v;

    if (v != nullptr)
        v->parent = u->parent;
}

// ==================== CORRECTION APRES SUPPRESSION ====================

void RedBlackTree::fixDelete(
    Noeud* x,
    Noeud* parent,
    bool xEstGauche
) {

    while (x != racine &&
           (x == nullptr || x->color == BLACK)) {

        if (parent == nullptr)
            break;

        if (x != nullptr)
            xEstGauche = (x == parent->left);

        // x est le fils gauche
        if (xEstGauche) {

            Noeud* w = parent->right;

            // Cas 1 : frere rouge
            if (w != nullptr && w->color == RED) {

                changerCouleur(w, BLACK);
                changerCouleur(parent, RED);

                rotateLeft(parent);

                w = parent->right;
            }

            // Cas 2 : frere noir et fils noirs
            if (w == nullptr ||
                ((w->left == nullptr || w->left->color == BLACK) &&
                 (w->right == nullptr || w->right->color == BLACK))) {

                changerCouleur(w, RED);

                x = parent;
                parent = x->parent;
            }

            else {

                // Cas 3 : fils droit noir
                if (w->right == nullptr || w->right->color == BLACK) {

                    changerCouleur(w->left, BLACK);
                    changerCouleur(w, RED);

                    rotateRight(w);

                    w = parent->right;
                }

                // Cas 4 : fils droit rouge
                changerCouleur(w, parent->color);
                changerCouleur(parent, BLACK);
                changerCouleur(w->right, BLACK);

                rotateLeft(parent);

                x = racine;
                parent = nullptr;
            }
        }

        // x est le fils droit
        else {

            Noeud* w = parent->left;

            // Cas 1 : frere rouge
            if (w != nullptr && w->color == RED) {

                changerCouleur(w, BLACK);
                changerCouleur(parent, RED);

                rotateRight(parent);

                w = parent->left;
            }

            // Cas 2 : frere noir et fils noirs
            if (w == nullptr ||
                ((w->right == nullptr || w->right->color == BLACK) &&
                 (w->left == nullptr || w->left->color == BLACK))) {

                changerCouleur(w, RED);

                x = parent;
                parent = x->parent;
            }

            else {

                // Cas 3 : fils gauche noir
                if (w->left == nullptr || w->left->color == BLACK) {

                    changerCouleur(w->right, BLACK);
                    changerCouleur(w, RED);

                    rotateLeft(w);

                    w = parent->left;
                }

                // Cas 4 : fils gauche rouge
                changerCouleur(w, parent->color);
                changerCouleur(parent, BLACK);
                changerCouleur(w->left, BLACK);

                rotateRight(parent);

                x = racine;
                parent = nullptr;
            }
        }
    }

    changerCouleur(x, BLACK);
}

// ==================== SUPPRESSION ====================

void RedBlackTree::supprimer(int valeur) {

    Noeud* z = chercherNoeud(valeur);

    if (z == nullptr)
        return;

    Noeud* y = z;
    Noeud* x = nullptr;
    Noeud* parentX = nullptr;

    bool xEstGauche = false;

    Color couleurOriginale = y->color;

    // Cas 1 : pas de fils gauche
    if (z->left == nullptr) {

        x = z->right;
        parentX = z->parent;

        if (parentX != nullptr)
            xEstGauche = (z == parentX->left);

        transplant(z, z->right);

        if (x != nullptr) {

            parentX = x->parent;

            if (parentX != nullptr)
                xEstGauche = (x == parentX->left);
        }
    }

    // Cas 2 : pas de fils droit
    else if (z->right == nullptr) {

        x = z->left;
        parentX = z->parent;

        if (parentX != nullptr)
            xEstGauche = (z == parentX->left);

        transplant(z, z->left);

        if (x != nullptr) {

            parentX = x->parent;

            if (parentX != nullptr)
                xEstGauche = (x == parentX->left);
        }
    }

    // Cas 3 : deux fils
    else {

        y = minimum(z->right);

        couleurOriginale = y->color;

        x = y->right;

        if (y->parent == z) {

            parentX = y;
            xEstGauche = false;
        }

        else {

            parentX = y->parent;
            xEstGauche = true;

            transplant(y, y->right);

            y->right = z->right;
            y->right->parent = y;
        }

        transplant(z, y);

        y->left = z->left;
        y->left->parent = y;

        changerCouleur(y, z->color);

        if (x != nullptr) {

            parentX = x->parent;

            if (parentX != nullptr)
                xEstGauche = (x == parentX->left);
        }
    }

    delete z;

    if (couleurOriginale == BLACK)
        fixDelete(x, parentX, xEstGauche);
}

// ==================== LIBERATION MEMOIRE ====================

void RedBlackTree::libererRec(Noeud* n) {

    if (n == nullptr)
        return;

    libererRec(n->left);
    libererRec(n->right);

    delete n;
}

RedBlackTree::~RedBlackTree() {

    libererRec(racine);
}

// ==================== AFFICHAGE ====================

void RedBlackTree::afficherRec(Noeud *n, int espace) {

    if (n == nullptr)
        return;

    espace += 6;

    afficherRec(n->right, espace);

    cout << endl;

    for (int i = 6; i < espace; i++)
        cout << " ";

    if (n->color == RED)
        cout << ROUGE;
    else
        cout << NOIR;

    cout << n->data << RESET;

    afficherRec(n->left, espace);
}

void RedBlackTree::afficher() {

    if (racine == nullptr) {

        cout << "Arbre vide." << endl;
        return;
    }

    afficherRec(racine, 0);

    cout << endl;
}
int RedBlackTree::rechercherMinimum() const {
    if (racine == nullptr)
        return 0;

    Noeud* courant = racine;

    while (courant->left != nullptr)
        courant = courant->left;

    return courant->data;
}

int RedBlackTree::rechercherMaximum() const {
    if (racine == nullptr)
        return 0;

    Noeud* courant = racine;

    while (courant->right != nullptr)
        courant = courant->right;

    return courant->data;
}

