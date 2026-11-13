
#include "Arbre_RED_BLACK.h"
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <map>
#include <random>
#include <string>
#include <vector>
#include <unordered_map>

using namespace std;
using namespace chrono;

const int NB_TESTS = 10;
const int POURCENTAGE_OPS = 80;
const int POURCENTAGE_DESORDRE = 10;
const int LARGEUR_TABLEAU = 114;

volatile long long resultatRecherche = 0;

enum Ordre {
    ALEATOIRE,
    TRIE,
    SEMI_TRIE
};

struct Mesure {
    double temps = 0;
    double rotationsGauche = 0;
    double rotationsDroite = 0;
    double changementsCouleur = 0;

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
    long long gauche = 0;
    long long droite = 0;
    long long couleurs = 0;
};

struct MesureCollision {
    double temps = 0;
    long long nbCollisions = 0;
};

// ===== NOUVELLES STRUCTURES POUR MINIMUM ET MAXIMUM =====

struct MesureMinMax {
    int minimum = 0;
    int maximum = 0;
    double tempsMin = 0;
    double tempsMax = 0;

    MesureMinMax& operator+=(const MesureMinMax& m) {
        tempsMin += m.tempsMin;
        tempsMax += m.tempsMax;
        minimum = m.minimum;
        maximum = m.maximum;
        return *this;
    }

    MesureMinMax& operator/=(double d) {
        tempsMin /= d;
        tempsMax /= d;
        return *this;
    }
};

struct ResultatMinMax {
    MesureMinMax rb, mp, hash;

    ResultatMinMax& operator+=(const ResultatMinMax& r) {
        rb += r.rb;
        mp += r.mp;
        hash += r.hash;
        return *this;
    }

    ResultatMinMax& operator/=(double d) {
        rb /= d;
        mp /= d;
        hash /= d;
        return *this;
    }
};

// ===== FONCTION DE HACHAGE AVEC COLLISIONS =====

struct HashAvecCollisions {
    size_t operator()(int valeur) const noexcept {
        return static_cast<size_t>(valeur % 4096);
    }
};

// ===== GENERATION DES DONNEES =====

vector<int> genererValeurs(
    int n,
    mt19937& generateur,
    Ordre ordre
) {
    vector<int> valeurs(n);

    for (int i = 0; i < n; i++)
        valeurs[i] = i + 1;

    if (ordre == ALEATOIRE) {
        shuffle(valeurs.begin(), valeurs.end(), generateur);
    }
    else if (ordre == SEMI_TRIE) {
        uniform_int_distribution<int> position(0, n - 1);

        int nbEchanges = n * POURCENTAGE_DESORDRE / 100;

        for (int i = 0; i < nbEchanges; i++) {
            swap(
                valeurs[position(generateur)],
                valeurs[position(generateur)]
            );
        }
    }

    return valeurs;
}

vector<int> selectionnerValeurs(
    const vector<int>& valeurs,
    mt19937& generateur
) {
    vector<int> indices(valeurs.size());

    for (int i = 0; i < (int)indices.size(); i++)
        indices[i] = i;

    shuffle(indices.begin(), indices.end(), generateur);

    int nbOperations =
        (int)valeurs.size() * POURCENTAGE_OPS / 100;

    vector<int> selection;
    selection.reserve(nbOperations);

    for (int i = 0; i < nbOperations; i++)
        selection.push_back(valeurs[indices[i]]);

    return selection;
}

// ===== CHRONOMETRAGE =====

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
Mesure mesurerRB(
    RedBlackTree& arbre,
    Operation&& operation
) {
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

// ===== TESTS CLASSIQUES =====

Resultat testerRedBlackTree(
    const vector<int>& valeurs,
    const vector<int>& selection
) {
    Resultat r;
    RedBlackTree arbre;

    r.insertion = mesurerRB(arbre, [&] {
        for (int v : valeurs)
            arbre.insert(v);
    });

    long long trouvees = 0;

    r.recherche = mesurerRB(arbre, [&] {
        for (int v : selection)
            trouvees += arbre.rechercher(v);
    });

    resultatRecherche += trouvees;

    r.suppression = mesurerRB(arbre, [&] {
        for (int v : selection)
            arbre.supprimer(v);
    });

    r.reinsertion = mesurerRB(arbre, [&] {
        for (int v : selection)
            arbre.insert(v);
    });

    return r;
}

Resultat testerMap(
    const vector<int>& valeurs,
    const vector<int>& selection
) {
    Resultat r;
    map<int, int> monMap;

    r.insertion = mesurerMap([&] {
        for (int v : valeurs)
            monMap.emplace(v, 0);
    });

    long long trouvees = 0;

    r.recherche = mesurerMap([&] {
        for (int v : selection)
            trouvees += (monMap.find(v) != monMap.end());
    });

    resultatRecherche += trouvees;

    r.suppression = mesurerMap([&] {
        for (int v : selection)
            monMap.erase(v);
    });

    r.reinsertion = mesurerMap([&] {
        for (int v : selection)
            monMap.emplace(v, 0);
    });

    return r;
}

Resultat testerUnorderedMap(
    const vector<int>& valeurs,
    const vector<int>& selection
) {
    Resultat r;
    unordered_map<int, int> monUnorderedMap;

    r.insertion = mesurerMap([&] {
        for (int v : valeurs)
            monUnorderedMap.emplace(v, 0);
    });

    long long trouvees = 0;

    r.recherche = mesurerMap([&] {
        for (int v : selection)
            trouvees += (
                monUnorderedMap.find(v) != monUnorderedMap.end()
            );
    });

    resultatRecherche += trouvees;

    r.suppression = mesurerMap([&] {
        for (int v : selection)
            monUnorderedMap.erase(v);
    });

    r.reinsertion = mesurerMap([&] {
        for (int v : selection)
            monUnorderedMap.emplace(v, 0);
    });

    return r;
}

// ===== TEST DE RECHERCHE AVEC COLLISIONS =====

MesureCollision testerRechercheCollision(
    const vector<int>& valeurs,
    const vector<int>& selection
) {
    unordered_map<int, int, HashAvecCollisions> monMapCollision;

    for (int v : valeurs)
        monMapCollision.emplace(v, 0);

    unordered_map<size_t, long long> frequencesHachage;

    for (int v : valeurs) {
        size_t h = HashAvecCollisions{}(v);
        frequencesHachage[h]++;
    }

    MesureCollision m;

    for (int v : selection) {
        size_t h = HashAvecCollisions{}(v);
        m.nbCollisions += frequencesHachage[h] - 1;
    }

    long long trouvees = 0;

    m.temps = chronometrer([&] {
        for (int i = 0; i < NB_TESTS; i++) {
            for (int v : selection) {
                trouvees += (
                    monMapCollision.find(v) != monMapCollision.end()
                );
            }
        }
    });

    m.temps /= NB_TESTS;

    resultatRecherche += trouvees;

    return m;
}

// ===== RECHERCHE DU MINIMUM ET DU MAXIMUM =====

// std::map : les clés sont ordonnées.

int rechercherMinimumMap(map<int, int>& m) {
    return m.begin()->first;
}

int rechercherMaximumMap(map<int, int>& m) {
    return m.rbegin()->first;
}

// unordered_map : les clés ne sont pas ordonnées.
// Il faut parcourir toute la table.

int rechercherMinimumHash(unordered_map<int, int>& m) {
    int minimum = m.begin()->first;

    for (const auto& element : m) {
        if (element.first < minimum)
            minimum = element.first;
    }

    return minimum;
}

int rechercherMaximumHash(unordered_map<int, int>& m) {
    int maximum = m.begin()->first;

    for (const auto& element : m) {
        if (element.first > maximum)
            maximum = element.first;
    }

    return maximum;
}

// ===== TEST MINIMUM ET MAXIMUM =====

ResultatMinMax testerMinMax(const vector<int>& valeurs) {
    ResultatMinMax resultat;

    RedBlackTree arbre;
    map<int, int> monMap;
    unordered_map<int, int> monUnorderedMap;

    // Construction des trois structures hors chronometrage.

    for (int v : valeurs) {
        arbre.insert(v);
        monMap.emplace(v, 0);
        monUnorderedMap.emplace(v, 0);
    }

    // RedBlackTree

    long long sommeRB = 0;

    resultat.rb.tempsMin = chronometrer([&] {
        for (int i = 0; i < NB_TESTS; i++)
            sommeRB += arbre.rechercherMinimum();
    }) / NB_TESTS;

    resultat.rb.tempsMax = chronometrer([&] {
        for (int i = 0; i < NB_TESTS; i++)
            sommeRB += arbre.rechercherMaximum();
    }) / NB_TESTS;

    resultat.rb.minimum = arbre.rechercherMinimum();
    resultat.rb.maximum = arbre.rechercherMaximum();

    // std::map

    long long sommeMap = 0;

    resultat.mp.tempsMin = chronometrer([&] {
        for (int i = 0; i < NB_TESTS; i++)
            sommeMap += rechercherMinimumMap(monMap);
    }) / NB_TESTS;

    resultat.mp.tempsMax = chronometrer([&] {
        for (int i = 0; i < NB_TESTS; i++)
            sommeMap += rechercherMaximumMap(monMap);
    }) / NB_TESTS;

    resultat.mp.minimum = rechercherMinimumMap(monMap);
    resultat.mp.maximum = rechercherMaximumMap(monMap);

    // unordered_map

    long long sommeHash = 0;

    resultat.hash.tempsMin = chronometrer([&] {
        for (int i = 0; i < NB_TESTS; i++)
            sommeHash += rechercherMinimumHash(monUnorderedMap);
    }) / NB_TESTS;

    resultat.hash.tempsMax = chronometrer([&] {
        for (int i = 0; i < NB_TESTS; i++)
            sommeHash += rechercherMaximumHash(monUnorderedMap);
    }) / NB_TESTS;

    resultat.hash.minimum = rechercherMinimumHash(monUnorderedMap);
    resultat.hash.maximum = rechercherMaximumHash(monUnorderedMap);

    // Eviter que les recherches soient considerees inutiles.

    resultatRecherche += sommeRB + sommeMap + sommeHash;

    return resultat;
}

// ===== AFFICHAGE DES RESULTATS CLASSIQUES =====

void afficherTableau(
    int n,
    int nbOperations,
    const Resultat& rb,
    const Resultat& mp,
    const Resultat& um,
    const MesureCollision& collision
) {
    cout << "\nN = " << n << endl;
    cout << "Moyenne sur " << NB_TESTS << " repetitions\n\n";

    cout << left << setw(18) << "Operation"
         << right << setw(12) << "Nombre"
         << setw(16) << "RedBlackTree"
         << setw(14) << "std::map"
         << setw(18) << "unordered_map"
         << setw(24) << "Avec collisions(ms)"
         << setw(20) << "Nb collisions"
         << endl;

    cout << string(122, '-') << endl;

    cout << left << setw(18) << "Insertion"
         << right << setw(12) << n
         << setw(16) << rb.insertion.temps
         << setw(14) << mp.insertion.temps
         << setw(18) << um.insertion.temps
         << setw(24) << "N/A"
         << setw(20) << "N/A"
         << endl;

    cout << left << setw(18) << "Recherche"
         << right << setw(12) << nbOperations
         << setw(16) << rb.recherche.temps
         << setw(14) << mp.recherche.temps
         << setw(18) << um.recherche.temps
         << setw(24) << collision.temps
         << setw(20) << collision.nbCollisions
         << endl;

    cout << string(122, '=') << endl;
}

// ===== AFFICHAGE MINIMUM ET MAXIMUM =====

void afficherMinMax(const ResultatMinMax& r) {
    cout << "\n";
    cout << "============================================================\n";
    cout << "              RECHERCHE DU MINIMUM ET MAXIMUM\n";
    cout << "============================================================\n";

    cout << left
         << setw(20) << "Structure"
         << right
         << setw(12) << "Minimum"
         << setw(15) << "Temps min(ms)"
         << setw(12) << "Maximum"
         << setw(15) << "Temps max(ms)"
         << endl;

    cout << string(74, '-') << endl;

    cout << left << setw(20) << "RedBlackTree"
         << right << setw(12) << r.rb.minimum
         << setw(15) << r.rb.tempsMin
         << setw(12) << r.rb.maximum
         << setw(15) << r.rb.tempsMax
         << endl;

    cout << left << setw(20) << "std::map"
         << right << setw(12) << r.mp.minimum
         << setw(15) << r.mp.tempsMin
         << setw(12) << r.mp.maximum
         << setw(15) << r.mp.tempsMax
         << endl;

    cout << left << setw(20) << "unordered_map"
         << right << setw(12) << r.hash.minimum
         << setw(15) << r.hash.tempsMin
         << setw(12) << r.hash.maximum
         << setw(15) << r.hash.tempsMax
         << endl;

    cout << string(74, '=') << endl;
}

// ===== TITRE DES TABLEAUX =====

void afficherTitre(Ordre ordre) {
    const string ligne(60, '=');

    cout << "\n" << ligne << "\n";

    if (ordre == ALEATOIRE)
        cout << "       PREMIER TABLEAU : VALEURS ALEATOIRES\n";

    else if (ordre == TRIE)
        cout << "       DEUXIEME TABLEAU : VALEURS TRIEES\n";

    else
        cout << "       TROISIEME TABLEAU : VALEURS SEMI-TRIEES ("
             << POURCENTAGE_DESORDRE << " % de desordre)\n";

    cout << ligne << "\n";
}

// ===== BENCHMARK =====

void benchmark(
    const vector<int>& tailles,
    mt19937& generateur,
    Ordre ordre
) {
    afficherTitre(ordre);

    for (int n : tailles) {

        vector<int> valeurs =
            genererValeurs(n, generateur, ordre);

        vector<int> selection =
            selectionnerValeurs(valeurs, generateur);

        Resultat totalRB, totalMap, totalUnordered;
        ResultatMinMax totalMinMax;

        // Tests classiques : 10 repetitions.

        for (int i = 0; i < NB_TESTS; i++) {

            if (i % 2 == 0) {
                totalRB += testerRedBlackTree(valeurs, selection);
                totalMap += testerMap(valeurs, selection);
                totalUnordered += testerUnorderedMap(valeurs, selection);
            }
            else {
                totalUnordered += testerUnorderedMap(valeurs, selection);
                totalRB += testerRedBlackTree(valeurs, selection);
                totalMap += testerMap(valeurs, selection);
            }

            // Test minimum et maximum.

            totalMinMax += testerMinMax(valeurs);
        }

        // Calcul des moyennes.

        totalRB /= NB_TESTS;
        totalMap /= NB_TESTS;
        totalUnordered /= NB_TESTS;
        totalMinMax /= NB_TESTS;

        // Test supplémentaire : unordered_map avec collisions.

        MesureCollision collision =
            testerRechercheCollision(valeurs, selection);

        // Affichage des resultats.

        afficherTableau(
            n,
            (int)selection.size(),
            totalRB,
            totalMap,
            totalUnordered,
            collision
        );

        afficherMinMax(totalMinMax);
    }
}

// ===== MAIN =====

int main() {
    const vector<int> tailles = {50000, 500000};

    mt19937 generateur(42);

    cout << fixed << setprecision(3);

    benchmark(tailles, generateur, ALEATOIRE);
    benchmark(tailles, generateur, TRIE);
    benchmark(tailles, generateur, SEMI_TRIE);

    cout << "\n\nResultat des recherches : "
         << resultatRecherche << endl;

    cout << "\nProcessus termine.\n";

    return 0;
}
