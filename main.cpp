#include "Arbre_RED_BLACK.h"
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <map>
#include <random>
#include <string>
#include <vector>
using namespace std;
using namespace chrono;


const int NB_TESTS = 10;
const int POURCENTAGE_OPS = 20;
const int POURCENTAGE_DESORDRE = 10;   // semi-trie : % d'echanges aleatoires sur une suite triee
const int LARGEUR_TABLEAU = 114;
volatile long long resultatRecherche = 0;

enum Ordre {
    ALEATOIRE,
    TRIE,
    SEMI_TRIE
};
struct Mesure {
    double temps = 0,
    rotationsGauche = 0,
    rotationsDroite = 0,
    changementsCouleur = 0;
    Mesure& operator+=(const Mesure& m) {
        temps += m.temps;
        rotationsGauche += m.rotationsGauche;
        rotationsDroite += m.rotationsDroite;
        changementsCouleur += m.changementsCouleur;
        return *this;
    }
    Mesure& operator/=(double d) {
        temps /= d;
        rotationsGauche /= d;
        rotationsDroite /= d;
        changementsCouleur /= d;
        return *this;
    }
};
struct Resultat {
    Mesure insertion, recherche, suppression, reinsertion;
    Resultat& operator+=(const Resultat& r) {
        insertion += r.insertion;
        recherche += r.recherche;
        suppression += r.suppression;
        reinsertion += r.reinsertion;
        return *this;
    }
    Resultat& operator/=(double d) {
        insertion /= d;
        recherche /= d;
        suppression /= d;
        reinsertion /= d;
        return *this;
    }
};
struct Compteurs {
    long long gauche = 0,
    droite = 0,
    couleurs = 0;
};
// ===== GENERATION DES DONNEES =====
vector<int> genererValeurs(int n, mt19937& generateur, Ordre ordre) {
    vector<int> valeurs(n);
    for (int i = 0; i < n; i++)
        valeurs[i] = i + 1;
    if (ordre == ALEATOIRE)
        shuffle(valeurs.begin(),
        valeurs.end(),
        generateur);
    else if (ordre == SEMI_TRIE) {
        // suite triee dont POURCENTAGE_DESORDRE % d'echanges aleatoires cassent l'ordre
        uniform_int_distribution<int> position(0, n - 1);
        int nbEchanges = n * POURCENTAGE_DESORDRE / 100;
        for (int i = 0; i < nbEchanges; i++)
            swap(valeurs[position(generateur)], valeurs[position(generateur)]);
    }
    return valeurs;
}
vector<int> selectionnerValeurs(const vector<int>& valeurs, mt19937& generateur) {
    vector<int> indices(valeurs.size());
    for (int i = 0; i < (int)indices.size(); i++) indices[i] = i;
    shuffle(indices.begin(), indices.end(), generateur);
    int nbOperations = (int)valeurs.size() * POURCENTAGE_OPS / 100;
    vector<int> selection;
    selection.reserve(nbOperations);
    for (int i = 0; i < nbOperations; i++) selection.push_back(valeurs[indices[i]]);
    return selection;
}
// ===== OUTILS DE MESURE =====
template <typename Operation>
double chronometrer(Operation&& operation) {
    auto debut = steady_clock::now();
    operation();
    auto fin = steady_clock::now();
    return duration<double, milli>(fin - debut).count();
}
Compteurs lireCompteurs(RedBlackTree& arbre) {
    Compteurs c;
    c.gauche = arbre.getRotationsGauche();
    c.droite = arbre.getRotationsDroite();
    c.couleurs = arbre.getNbChangementsCouleur();
    return c;
}
template <typename Operation>
Mesure mesurerRB(RedBlackTree& arbre, Operation&& operation) {
    Mesure m;
    Compteurs avant = lireCompteurs(arbre);
    m.temps = chronometrer(operation);
    Compteurs apres = lireCompteurs(arbre);
    m.rotationsGauche = apres.gauche - avant.gauche;
    m.rotationsDroite = apres.droite - avant.droite;
    m.changementsCouleur = apres.couleurs - avant.couleurs;
    return m;
}
template <typename Operation>
Mesure mesurerMap(Operation&& operation) {
    Mesure m;
    m.temps = chronometrer(operation);
    return m;
}
// ===== TESTS =====
Resultat testerRedBlackTree(const vector<int>& valeurs, const vector<int>& selection) {
    Resultat r;
    RedBlackTree arbre;
    r.insertion   = mesurerRB(arbre, [&] { for (int v : valeurs)   arbre.insert(v); });
    r.recherche   = mesurerRB(arbre, [&] { for (int v : selection) resultatRecherche += arbre.rechercher(v); });
    r.suppression = mesurerRB(arbre, [&] { for (int v : selection) arbre.supprimer(v); });
    r.reinsertion = mesurerRB(arbre, [&] { for (int v : selection) arbre.insert(v); });
    return r;
}
Resultat testerMap(const vector<int>& valeurs, const vector<int>& selection) {
    Resultat r;
    map<int, int> monMap;
    r.insertion   = mesurerMap([&] { for (int v : valeurs)   monMap.emplace(v, 0); });
    r.recherche   = mesurerMap([&] { for (int v : selection) resultatRecherche += (monMap.find(v) != monMap.end()); });
    r.suppression = mesurerMap([&] { for (int v : selection) monMap.erase(v); });
    r.reinsertion = mesurerMap([&] { for (int v : selection) monMap.emplace(v, 0); });
    return r;
}
// ===== AFFICHAGE =====
void afficherLigne(const string& structure, const string& operation, int nombre, const Mesure& m, bool estRB) {
    cout << left << setw(17) << structure << setw(16) << operation
         << right << setw(12) << nombre << setw(14) << m.temps;
    if (estRB)
        cout << setw(13) << m.rotationsGauche << setw(13) << m.rotationsDroite << setw(16) << m.changementsCouleur;
    else
        cout << setw(13) << "N/A" << setw(13) << "N/A" << setw(16) << "N/A";
    cout << endl;
}
void afficherBloc(const string& nom, int n, int nbOperations, const Resultat& r, bool estRB) {
    afficherLigne(nom, "Insertion",   n,            r.insertion,   estRB);
    afficherLigne(nom, "Recherche",   nbOperations, r.recherche,   estRB);
    afficherLigne(nom, "Suppression", nbOperations, r.suppression, estRB);
    afficherLigne(nom, "Reinsertion", nbOperations, r.reinsertion, estRB);
}
void afficherTableau(int n, int nbOperations, const Resultat& rb, const Resultat& mp) {
    cout << "\nN = " << n << endl;
    cout << "Moyenne sur " << NB_TESTS << " repetitions\n\n";
    cout << left << setw(17) << "Structure" << setw(16) << "Operation"
         << right << setw(12) << "Nombre" << setw(14) << "Temps(ms)"
         << setw(13) << "Rot. gauche" << setw(13) << "Rot. droite" << setw(16) << "Chg. couleur" << endl;
    cout << string(LARGEUR_TABLEAU, '-') << endl;
    afficherBloc("RedBlackTree", n, nbOperations, rb, true);
    cout << string(LARGEUR_TABLEAU, '-') << endl;
    afficherBloc("std::map", n, nbOperations, mp, false);
    cout << string(LARGEUR_TABLEAU, '=') << endl;
}
void afficherTitre(Ordre ordre) {
    const string ligne(60, '=');
    cout << "\n\n" << ligne << "\n";
    if (ordre == ALEATOIRE)      cout << "       PREMIER TABLEAU : VALEURS ALEATOIRES\n";
    else if (ordre == TRIE)      cout << "       DEUXIEME TABLEAU : VALEURS TRIEES\n";
    else                         cout << "       TROISIEME TABLEAU : VALEURS SEMI-TRIEES (" << POURCENTAGE_DESORDRE << " % de desordre)\n";
    cout << ligne << "\n";
}
// ===== BENCHMARK =====
void benchmark(const vector<int>& tailles, mt19937& generateur, Ordre ordre) {
    afficherTitre(ordre);
    for (int n : tailles) {
        vector<int> valeurs = genererValeurs(n, generateur, ordre);
        vector<int> selection = selectionnerValeurs(valeurs, generateur);
        Resultat totalRB, totalMap;
        for (int i = 0; i < NB_TESTS; i++) {
            if (i % 2 == 0) {   // alterner l'ordre des tests
                totalRB  += testerRedBlackTree(valeurs, selection);
                totalMap += testerMap(valeurs, selection);
            } else {
                totalMap += testerMap(valeurs, selection);
                totalRB  += testerRedBlackTree(valeurs, selection);
            }
        }
        totalRB  /= NB_TESTS;
        totalMap /= NB_TESTS;
        afficherTableau(n, (int)selection.size(), totalRB, totalMap);
    }
}
// ===== MAIN =====
int main() {
    const vector<int> tailles = {10, 100, 1000, 5000, 10000, 50000, 100000, 500000};
    mt19937 generateur(42);
    cout << fixed << setprecision(3);
    benchmark(tailles, generateur, ALEATOIRE);   // ordre aleatoire
    benchmark(tailles, generateur, TRIE);        // ordre croissant
    benchmark(tailles, generateur, SEMI_TRIE);   // presque trie
    return 0;
}
