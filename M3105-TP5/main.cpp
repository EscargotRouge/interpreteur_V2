#include <iostream>
using namespace std;
#include "Interpreteur.h"
#include "Exceptions.h"

int main(int argc, char* argv[]) {
    string nomFich;

    if (argc != 2) {
        cout << "Usage : " << argv[0] << " nom_fichier_source" << endl << endl;
        cout << "Entrez le nom du fichier que voulez-vous interpréter : ";
        getline(cin, nomFich);
    } else {
        nomFich = argv[1];
    }

    ifstream fichier(nomFich.c_str());

    try {
        Interpreteur interpreteur(fichier);
        cout << "ETAT BOOLEAN : " << interpreteur.getErreurExistante() << endl;
        interpreteur.analyse();
        cout << "ETAT BOOLEAN : " << interpreteur.getErreurExistante() << endl;
        
        // Analyse syntaxique : "ça passe ou pas ?"
        if (interpreteur.getErreurExistante()) { // Si exception declenchée, l'analyse syntaxique échoue
            cout << endl << "================ Syntaxe Incorrecte" << endl;
        } else { // Si pas d'exception déclenchée, l'analyse syntaxique a réussi
            cout << endl << "================ Syntaxe Correcte" << endl;


            // On affiche le contenu de la table des symboles avant d'exécuter le programme
            cout << endl << "================ Table des symboles avant exécution : " << interpreteur.getTable();
            cout << endl << "================ Execution de l'arbre" << endl;

            // On exécute le programme si l'arbre n'est pas vide
            if (interpreteur.getArbre() != nullptr) interpreteur.getArbre()->executer();

            // Et on vérifie qu'il a fonctionné en regardant comment il a modifié la table des symboles
            cout << endl << "================ Table des symboles apres exécution : " << interpreteur.getTable();


            //Lecture du code C++ Générée
            cout << endl << "================ Génération du code C++ : " << endl;
            if (interpreteur.getArbre() != nullptr) {
                interpreteur.traduitEnCPP(cout);
            }
            
            
            
//            cout << nomFich + ".cpp" << endl;
//            ofstream fichiercpp(nomFich + ".cpp", ios::out | ios::trunc); //à la fin du fichier
//            if (interpreteur.getArbre() != nullptr) {
//                interpreteur.traduitEnCPP(fichiercpp);
//            }
        }
    } catch (InterpreteurException & e) {
        cout << e.what() << endl;
    }
    return 0;
}
