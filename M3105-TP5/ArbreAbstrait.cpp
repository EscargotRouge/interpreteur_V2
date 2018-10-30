#include <stdlib.h>
#include <algorithm>
#include "ArbreAbstrait.h"
#include "Symbole.h"
#include "SymboleValue.h"
#include "Exceptions.h"
#include <typeinfo>

////////////////////////////////////////////////////////////////////////////////
// NoeudSeqInst
////////////////////////////////////////////////////////////////////////////////

NoeudSeqInst::NoeudSeqInst() : m_instructions() {
}

int NoeudSeqInst::executer() {
    for (unsigned int i = 0; i < m_instructions.size(); i++)
        m_instructions[i]->executer(); // on exécute chaque instruction de la séquence
    return 0; // La valeur renvoyée ne représente rien !
}

void NoeudSeqInst::ajoute(Noeud* instruction) {
    if (instruction != nullptr) m_instructions.push_back(instruction);
}

void NoeudSeqInst::traduitEnCPP(ostream & cout, unsigned int indentation) const {
	for (unsigned int i = 0; i < m_instructions.size(); i++) {
		cout << setw(indentation) << "\t";
		m_instructions[i]->traduitEnCPP(cout, indentation);
		cout << endl;
	}
}

////////////////////////////////////////////////////////////////////////////////
// NoeudAffectation
////////////////////////////////////////////////////////////////////////////////

NoeudAffectation::NoeudAffectation(Noeud* variable, Noeud* expression)
: m_variable(variable), m_expression(expression) {
}

int NoeudAffectation::executer() {
    int valeur = m_expression->executer(); // On exécute (évalue) l'expression
    ((SymboleValue*) m_variable)->setValeur(valeur); // On affecte la variable
    return 0; // La valeur renvoyée ne représente rien !
}

////////////////////////////////////////////////////////////////////////////////
// NoeudOperateurBinaire
////////////////////////////////////////////////////////////////////////////////

NoeudOperateurBinaire::NoeudOperateurBinaire(Symbole operateur, Noeud* operandeGauche, Noeud* operandeDroit)
: m_operateur(operateur), m_operandeGauche(operandeGauche), m_operandeDroit(operandeDroit) {
}

int NoeudOperateurBinaire::executer() {
    int og, od, valeur;
    if (m_operandeGauche != nullptr) og = m_operandeGauche->executer(); // On évalue l'opérande gauche
    if (m_operandeDroit != nullptr) od = m_operandeDroit->executer(); // On évalue l'opérande droit
    // Et on combine les deux opérandes en fonctions de l'opérateur
    if (this->m_operateur == "+") valeur = (og + od);
    else if (this->m_operateur == "-") valeur = (og - od);
    else if (this->m_operateur == "*") valeur = (og * od);
    else if (this->m_operateur == "==") valeur = (og == od);
    else if (this->m_operateur == "!=") valeur = (og != od);
    else if (this->m_operateur == "<") valeur = (og < od);
    else if (this->m_operateur == ">") valeur = (og > od);
    else if (this->m_operateur == "<=") valeur = (og <= od);
    else if (this->m_operateur == ">=") valeur = (og >= od);
    else if (this->m_operateur == "et") valeur = (og && od);
    else if (this->m_operateur == "ou") valeur = (og || od);
    else if (this->m_operateur == "non") valeur = (!og);
    else if (this->m_operateur == "/") {
        if (od == 0) throw DivParZeroException();
        valeur = og / od;
    }
    return valeur; // On retourne la valeur calculée
}

////////////////////////////////////////////////////////////////////////////////
// NoeudInstSiRiche
////////////////////////////////////////////////////////////////////////////////

NoeudInstSiRiche::NoeudInstSiRiche(vector<Noeud*> vCondition, vector<Noeud*> vSequence)
: m_vCondition(vCondition), m_vSequence(vSequence) {
}

int NoeudInstSiRiche::executer() {

    int i = 0;
    for (auto uneCond : m_vCondition) { //dès qu'on trouve la prmière true on execute et on laisse tomber le reste
        if (uneCond->executer()) {
            m_vSequence[i]->executer();
            return 0;
        }
        i++;
    }

    if (m_vSequence.size() > m_vCondition.size()) {
        m_vSequence[m_vSequence.size() - 1]->executer();
    }

    return 0; // La valeur renvoyée ne représente rien !
}


////////////////////////////////////////////////////////////////////////////////
// NoeudInstTantQue
////////////////////////////////////////////////////////////////////////////////

NoeudInstTantQue::NoeudInstTantQue(Noeud* condition, Noeud* sequence)
: m_condition(condition), m_sequence(sequence) {
}

int NoeudInstTantQue::executer() {

    while (m_condition->executer()) {
        m_sequence->executer();
    }
    return 0; // La valeur renvoyée ne représente rien !
}

void NoeudSeqInst::traduitEnCPP(ostream & cout, unsigned int indentation) const {
	
    
    
    
    
    for (unsigned int i = 0; i < m_instructions.size(); i++) {
		cout << setw(indentation) << "\t";
		m_instructions[i]->traduitEnCPP(cout, indentation);
		cout << endl;
	}
}

////////////////////////////////////////////////////////////////////////////////
// NoeudInstRepeter
////////////////////////////////////////////////////////////////////////////////

NoeudInstRepeter::NoeudInstRepeter(Noeud* condition, Noeud* sequence)
: m_condition(condition), m_sequence(sequence) {
}

int NoeudInstRepeter::executer() {

    do {
        m_sequence->executer();
    } while (!m_condition->executer());
    return 0; // La valeur renvoyée ne représente rien !
}

void NoeudInstRepeter::traduitEnCPP(ostream & cout, unsigned int indentation) const {
	cout << setw(indentation) << "do {" << endl;  //Ecrit "do {" avec un décalage de 4*indentation espaces
	m_sequence->traduitEnCPP(cout, indentation + 9);  //traduit en C++ la séquence avec indentation augmentée
	cout << setw(indentation) << "\t" << "}" << endl;
	cout << setw(indentation) << "\t" << "while( ";
	m_condition->traduitEnCPP(cout, 0); //Traduit la condition en C++ sans décalage
	cout << " );";
}

////////////////////////////////////////////////////////////////////////////////
// NoeudInstPour
////////////////////////////////////////////////////////////////////////////////

NoeudInstPour::NoeudInstPour(Noeud* initialisation, Noeud* condition, Noeud* incrementation, Noeud* sequence)
: m_initialisation(initialisation), m_condition(condition), m_incrementation(incrementation), m_sequence(sequence) {
}

int NoeudInstPour::executer() {

    for (m_initialisation->executer(); m_condition->executer(); m_incrementation->executer()) {
        m_sequence->executer();
    }

    return 0; // La valeur renvoyée ne représente rien !
}

////////////////////////////////////////////////////////////////////////////////
// NoeudInstEcrire
////////////////////////////////////////////////////////////////////////////////

NoeudInstEcrire::NoeudInstEcrire() {
}

int NoeudInstEcrire::executer() {
    for (auto p : m_vInstruction) { //auto équivaut à Noeud* , on boucle pour chaque instruction

        // on regarde si l'objet pointé par p est de type SymboleValue et si c'est une chaïne
        if ((typeid (*p) == typeid (SymboleValue) && *((SymboleValue*) p) == "<CHAINE>")) {
            //cas d'une chaine
            string str(((SymboleValue*) p)->getChaine()); // transforme la <CHAINE> en chaine c++(string)
            str = str.erase(0, 1); //supprime la première guillemet de la chaine
            str.pop_back(); //On supprime la dernière guillemet de la chaine

            cout << str; //transormera le x en x            
        } else { // cas d'une expression

            cout << p->executer(); // transformera la variable x en sa valeur
        }
    }
    cout << endl; //Lisibilité sur la console -> Du coup on a réalisé une variante d'EcrireLigne en soit.
    return 0; // La valeur renvoyée ne représente rien !
}

void NoeudInstEcrire::ajoute(Noeud* instruction) {
    m_vInstruction.push_back(instruction);
}

NoeudInstLire::NoeudInstLire() {
}

int NoeudInstLire::executer() {
    for (auto uneVariable : m_vVariable) { //auto équivaut à Noeud* , on boucle pour chaque instruction
        
        int intSaisi; //y'a un e ou pas à Saisi ? check à la fin
        
        cin >> intSaisi;
        cin.ignore(256, '\n'); // Warning Philippe Martin Cour Magistral, permet de prendre des chaines de caractères avec des espaces style : "Tour Eiffel"
        ((SymboleValue*) uneVariable)->setValeur(intSaisi); //faut insérer la valeur saisie dans uneVariable
    }
    return 0; // La valeur renvoyée ne représente rien !
}

void NoeudInstLire::ajoute(Noeud* variable) {
    m_vVariable.push_back(variable);
}