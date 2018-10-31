#include "Interpreteur.h"
#include <stdlib.h>
#include <iostream>
#include <vector>
using namespace std;

Interpreteur::Interpreteur(ifstream & fichier) :
m_lecteur(fichier), m_table(), m_arbre(nullptr) {
}

void Interpreteur::analyse() {
    m_erreurExistante = false; //pas d'erreur au début du programme;
    m_arbre = programme(); // on lance l'analyse de la première règle
}

void Interpreteur::traduitEnCPP(ostream & cout, unsigned int indentation) const {

    //cout << "#include <iostream>" << endl << endl; // à faire pour plus tard : ecriture dans un fichier traduction.cpp
    cout << setw(4 * indentation) << "int main() {" << endl; //Début du programme C++

    // Ecrire en C++ la déclaration des variables présentes dans le programme...
    // ... variables dont on retrouvera le nom en parcourant la table des symboles !
    // Par exemple, si le programme contient i,j,k, il faudra écrire : int i; int j; int k; ...

    for (int i = 0; i < m_table.getTaille(); i++)
        if (m_table[i] == "<VARIABLE>") {
            cout << setw(4 * indentation) << "int " << m_table[i].getChaine() << ";" << endl;
        }
    cout << endl;
    
    getArbre()->traduitEnCPP(cout, indentation + 1); //lance l'opération traduitEnCPP sur la racine

    cout << setw(4 * (indentation + 1)) << "\t" << "return 0;" << endl;
    cout << setw(4 * indentation) << "}" << endl; //Fin du programme C++
}

void Interpreteur::tester(const string & symboleAttendu) const throw (SyntaxeException) {
    // Teste si le symbole courant est égal au symboleAttendu... Si non, lève une exception
    static char messageWhat[256];
    if (m_lecteur.getSymbole() != symboleAttendu) {
        sprintf(messageWhat,
                "Ligne %d, Colonne %d - Erreur de syntaxe - Symbole attendu : %s - Symbole trouvé : %s",
                m_lecteur.getLigne(), m_lecteur.getColonne(),
                symboleAttendu.c_str(), m_lecteur.getSymbole().getChaine().c_str());
        throw SyntaxeException(messageWhat);
    }
}

void Interpreteur::testerEtAvancer(const string & symboleAttendu) throw (SyntaxeException) {
    // Teste si le symbole courant est égal au symboleAttendu... Si oui, avance, Sinon, lève une exception
    tester(symboleAttendu);
    m_lecteur.avancer();
}

void Interpreteur::erreur(const string & message) const throw (SyntaxeException) {
    // Lève une exception contenant le message et le symbole courant trouvé
    // Utilisé lorsqu'il y a plusieurs symboles attendus possibles...
    static char messageWhat[256];
    sprintf(messageWhat,
            "Ligne %d, Colonne %d - Erreur de syntaxe - %s - Symbole trouvé : %s",
            m_lecteur.getLigne(), m_lecteur.getColonne(), message.c_str(), m_lecteur.getSymbole().getChaine().c_str());
    throw SyntaxeException(messageWhat);
}

Noeud* Interpreteur::programme() {
    // <programme> ::= procedure principale() <seqInst> finproc FIN_FICHIER
    testerEtAvancer("procedure");
    testerEtAvancer("principale");
    testerEtAvancer("(");
    testerEtAvancer(")");
    Noeud* sequence = seqInst();
    try {
        testerEtAvancer("finproc");
    } catch (SyntaxeException s) {
        cout << "Exception déclenchée : " << s.what() << endl;
        m_erreurExistante = true;
    }
    tester("<FINDEFICHIER>");

    if (m_erreurExistante) {
        m_arbre = nullptr; //on vide l'arbre
    }
    return sequence;
}

Noeud* Interpreteur::seqInst() {
    // <seqInst> ::= <inst> { <inst> }
    NoeudSeqInst* sequence = new NoeudSeqInst();
    do {
        sequence->ajoute(inst());
    } while (m_lecteur.getSymbole() == "<VARIABLE>"
            || m_lecteur.getSymbole() == "si"
            || m_lecteur.getSymbole() == "tantque"
            || m_lecteur.getSymbole() == "repeter"
            || m_lecteur.getSymbole() == "pour"
            || m_lecteur.getSymbole() == "ecrire"
            || m_lecteur.getSymbole() == "lire")
        ;
    // Tant que le symbole courant est un début possible d'instruction...
    // Il faut compléter cette condition chaque fois qu'on rajoute une nouvelle instruction
    return sequence;
}

Noeud* Interpreteur::inst() {
    // <inst> ::= <affectation> ; | <instSiRiche> | <instTantQue> | <instRepeter> ; | <instPour> | <instEcrire> ; | <insLire> ;
    if (m_lecteur.getSymbole() == "<VARIABLE>") {

        try {
            Noeud *affect = affectation();
            testerEtAvancer(";");
            return affect;
        } catch (SyntaxeException s) {
            cout << "Exception declenchée : " << s.what() << endl;
            m_erreurExistante = true;
            return nullptr;
        }

    } else if (m_lecteur.getSymbole() == "si")
        return instSiRiche();

        // Compléter les alternatives chaque fois qu'on rajoute une nouvelle instruction
    else if (m_lecteur.getSymbole() == "tantque")
        return instTantQue();

    else if (m_lecteur.getSymbole() == "repeter") {

        try {
            Noeud* repet = instRepeter();
            testerEtAvancer(";");
            return repet;
        } catch (SyntaxeException s) {
            cout << "Exception declenchée : " << s.what() << endl;
            m_erreurExistante = true; //POSE PROBLEME
            return nullptr;
        }

    } else if (m_lecteur.getSymbole() == "pour")
        return instPour();

    else if (m_lecteur.getSymbole() == "ecrire") {

        try {
            Noeud* ecrire = instEcrire();
            testerEtAvancer(";");
            return ecrire;
        } catch (SyntaxeException s) {
            cout << "Exception declenchée : " << s.what() << endl;
            m_erreurExistante = true;
            return nullptr;
        }


    } else if (m_lecteur.getSymbole() == "lire") {

        try {
            Noeud* lire = instLire();
            testerEtAvancer(";");
            return lire;
        } catch (SyntaxeException s) {
            cout << "Exception declenchée : " << s.what() << endl;
            m_erreurExistante = true;
            return nullptr;
        }

    } else erreur("Instruction incorrecte");
}

Noeud* Interpreteur::affectation() {
    // <affectation> ::= <variable> = <expression> 
    tester("<VARIABLE>");
    Noeud* var = m_table.chercheAjoute(m_lecteur.getSymbole()); // La variable est ajoutée à la table eton la mémorise
    m_lecteur.avancer();
    testerEtAvancer("=");
    Noeud* exp = expression(); // On mémorise l'expression trouvée
    return new NoeudAffectation(var, exp); // On renvoie un noeud affectation
}

Noeud* Interpreteur::expression() {
    //  <expression> ::= <terme> { + <terme> | - <terme> }
    Noeud* term = terme();
    
    while (m_lecteur.getSymbole() == "+" || m_lecteur.getSymbole() == "-") {
        Symbole operateur = m_lecteur.getSymbole(); // On mémorise le symbole de l'opérateur
        m_lecteur.avancer();
        Noeud* termDroit = terme(); // On mémorise l'opérande droit
        term = new NoeudOperateurBinaire(operateur, term, termDroit); // Et on construuit un noeud opérateur binaire
    }
    return term; // On renvoie terme qui pointe sur la racine de l'expression
}

Noeud* Interpreteur::terme() {
    //       <terme> ::= <facteur> { * <facteur> | / <facteur> }
    Noeud* fact = facteur();
    
    while (m_lecteur.getSymbole() == "*" || m_lecteur.getSymbole() == "/") {
        Symbole operateur = m_lecteur.getSymbole(); // On mémorise le symbole de l'opérateur
        m_lecteur.avancer();
        Noeud* factDroit = facteur(); // On mémorise l'opérande droit
        fact = new NoeudOperateurBinaire(operateur, fact, factDroit); // Et on construuit un noeud opérateur binaire
    }
    return fact;
}

Noeud* Interpreteur::facteur() {
    //     <facteur> ::= <entier>  |  <variable>  |  - <expBool>  | non <expBool> | ( <expBool> ) 
    Noeud* fact = nullptr;
    if (m_lecteur.getSymbole() == "<VARIABLE>" || m_lecteur.getSymbole() == "<ENTIER>") {
        fact = m_table.chercheAjoute(m_lecteur.getSymbole()); // on ajoute la variable ou l'entier à la table
        m_lecteur.avancer();
    } else if (m_lecteur.getSymbole() == "-") { // - <expBool>
        m_lecteur.avancer();
        // on représente le moins unaire (- facteur) par une soustraction binaire (0 - facteur)
        fact = new NoeudOperateurBinaire(Symbole("-"), m_table.chercheAjoute(Symbole("0")), facteur());
    } else if (m_lecteur.getSymbole() == "non") { // non <expBool>
        m_lecteur.avancer();
        // on représente le moins unaire (- facteur) par une soustractin binaire (0 - facteur)
        fact = new NoeudOperateurBinaire(Symbole("non"), facteur(), nullptr);
    } else if (m_lecteur.getSymbole() == "(") { // <expBool>
        m_lecteur.avancer();
        fact = expBool();
        testerEtAvancer(")");
    } else
        erreur("Facteur incorrect");
    return fact;    
}

Noeud* Interpreteur::expBool() {
    //     <expBool> ::= <relationEt> { ou <relationEt> }
    Noeud* relEt = relationEt();
    while (m_lecteur.getSymbole() == "ou") {
        Symbole operateur = m_lecteur.getSymbole(); // On mémorise le symbole de l'opérateur
        m_lecteur.avancer();
        Noeud* relEtDroit = relationEt(); // On mémorise l'opérande droit
        relEt = new NoeudOperateurBinaire(operateur, relEt, relEtDroit); // Et on construuit un noeud opérateur binaire
    }
    return relEt;
}

Noeud* Interpreteur::relationEt() {
    //  <relationEt> ::= <relation> { et <relation> }
    Noeud* rel = relation();
    while (m_lecteur.getSymbole() == "et") {
        Symbole operateur = m_lecteur.getSymbole(); // On mémorise le symbole de l'opérateur
        m_lecteur.avancer();
        Noeud* relDroit = relation(); // On mémorise l'opérande droit
        rel = new NoeudOperateurBinaire(operateur, rel, relDroit); // Et on construuit un noeud opérateur binaire
    }
    return rel;
}

Noeud* Interpreteur::relation() {
    //    <relation> ::= <expression> { <opRel> <expression> }
    //       <opRel> ::= < | > | <= | >= | == | !=
    Noeud* expr = expression();
    while (m_lecteur.getSymbole() == "<" || m_lecteur.getSymbole() == ">"
                                         || m_lecteur.getSymbole() == "<="
                                         || m_lecteur.getSymbole() == ">="
                                         || m_lecteur.getSymbole() == "=="
                                         || m_lecteur.getSymbole() == "!=") {
        Symbole operateur = m_lecteur.getSymbole(); // On mémorise le symbole de l'opérateur
        m_lecteur.avancer();
        Noeud* exprDroit = expression(); // On mémorise l'opérande droit
        expr = new NoeudOperateurBinaire(operateur, expr, exprDroit); // Et on construuit un noeud opérateur binaire
    }
    return expr;
}

Noeud* Interpreteur::instSiRiche() {
    // <instSiRiche> ::= si (<expression>) <seqInst> { sinonsi (<expression>) <seqInst> } [sinon <seqInst>] finsi

    try {

        vector<Noeud*> vCond;
        vector<Noeud*> vSeq;

        testerEtAvancer("si");
        testerEtAvancer("(");
        Noeud* condition = expression(); // On mémorise la condition
        vCond.push_back(condition);
        testerEtAvancer(")");

        Noeud* sequence = seqInst();
        vSeq.push_back(sequence);

        while (m_lecteur.getSymbole() == "sinonsi") {
            m_lecteur.avancer();
            testerEtAvancer("(");

            Noeud* condition = expression(); // On mémorise la condition
            vCond.push_back(condition);
            testerEtAvancer(")");
            sequence = seqInst(); // On mémorise la séquence d'instruction
            vSeq.push_back(sequence);
        }

        if (m_lecteur.getSymbole() == "sinon") {
            m_lecteur.avancer();
            sequence = seqInst(); // On mémorise la séquence d'instruction
            vSeq.push_back(sequence);
        }
        testerEtAvancer("finsi");

        return new NoeudInstSiRiche(vCond, vSeq);

    } catch (SyntaxeException s) {
        cout << "Exception declenchée : " << s.what() << endl;
        while (m_lecteur.getSymbole() != "finsi" && m_lecteur.getSymbole() != "<FINDEFICHIER>") {
            m_lecteur.avancer(); //hmm
        }
        m_lecteur.avancer(); //Demander au prof si nécessaire
        m_erreurExistante = true;
        return nullptr;
    }



}

Noeud* Interpreteur::instTantQue() {
    //   <instTantQue> ::= tantque ( <expression> ) <seqInst> fintantque
    try {
        testerEtAvancer("tantque");
        testerEtAvancer("(");
        Noeud* condition = expression(); //on mémorise condition
        testerEtAvancer(")");
        Noeud* sequence = seqInst();
        testerEtAvancer("fintantque");
        return new NoeudInstTantQue(condition, sequence); // Et on renvoie un noeud Instruction TantQue

    } catch (SyntaxeException s) {
        cout << "Exception declenchée : " << s.what() << endl;
        while (m_lecteur.getSymbole() != "fintantque" && m_lecteur.getSymbole() != "<FINDEFICHIER>") {
            m_lecteur.avancer(); //l'idée c'est de sauter le bloc fintantque
        }
        m_lecteur.avancer(); //Demander au prof si nécessaire
        m_erreurExistante = true;
        return nullptr;
    }

}

Noeud* Interpreteur::instRepeter() {
    // <instRepeter> ::= repeter <seqInst> jusqua ( <expression>)

    try {
        testerEtAvancer("repeter");
        Noeud* sequence = seqInst();
        testerEtAvancer("jusqua");
        testerEtAvancer("(");
        Noeud* condition = expression(); //on mémorise condition
        testerEtAvancer(")");
        return new NoeudInstRepeter(condition, sequence); //return Noeud repter

    } catch (SyntaxeException s) {
        cout << "Exception declenchée : " << s.what() << endl;
        while (m_lecteur.getSymbole() != "<FINDEFICHIER>") {
            m_lecteur.avancer();
        }
        m_erreurExistante = true;
        return nullptr;
    }
}

Noeud* Interpreteur::instPour() {
    // <instPour> ::= pour ( [ <affectation> ] ; <expression> ; [ <affectation> ] ) <seqInst> finpour

    try {
        testerEtAvancer("pour");
        testerEtAvancer("(");
        Noeud* initialisation = affectation(); // i = 0
        testerEtAvancer(";");
        Noeud* condition = expression(); // i<10
        testerEtAvancer(";");
        Noeud* incrementation = affectation(); // i = i + 1
        testerEtAvancer(")");
        Noeud* sequence = seqInst();
        testerEtAvancer("finpour");
        return new NoeudInstPour(initialisation, condition, incrementation, sequence);

    } catch (SyntaxeException s) {
        cout << "Exception declenchée : " << s.what() << endl;
        while (m_lecteur.getSymbole() != "finpour" && m_lecteur.getSymbole() != "<FINDEFICHIER>") {
            m_lecteur.avancer();
        }
        m_lecteur.avancer();
        m_erreurExistante = true;
        return nullptr;
    }

}

Noeud* Interpreteur::instEcrire() {
    // <instEcrire> ::= ecrire ( <expression> | <chaine> { , <expression> | <chaine> } )

    try {
        bool lecturePremierTerme = true; //le but étant de lire le premier terme sans prêter attention aux virgules
        NoeudInstEcrire* ecrireNoeud = new NoeudInstEcrire();

        testerEtAvancer("ecrire");
        testerEtAvancer("(");

        do { //do while pour lire le premier terme qui sera potentiellement présent

            if (!lecturePremierTerme) { //si c'est pas le premier terme,
                testerEtAvancer(","); //il faut attendre une virgule entre chaque instruction
            }

            if (m_lecteur.getSymbole() == "<CHAINE>") {
                Noeud* chaine = m_table.chercheAjoute(m_lecteur.getSymbole()); //cherche Symbole dans la table de symbole, si il le trouve pas il l'ajoute à la fin et renvoie son pointeur (si il le trouve il renvoie pointeur aussi)
                ecrireNoeud->ajoute(chaine); //ajoute chaine au noeud (NoeudInstEcrire)
                //testerEtAvancer("<CHAINE>");
                m_lecteur.avancer(); //on passe la "<CHAINE>"

            } else {
                Noeud* expr = expression();
                ecrireNoeud->ajoute(expr); //ajoute expr au noeud (NoeudInstEcrire)
            }
            lecturePremierTerme = false;

        } while (m_lecteur.getSymbole() != ")");
        m_lecteur.avancer(); //on passe la paranthèse fermante ")"

        return ecrireNoeud;
    } catch (SyntaxeException s) {
        cout << "Exception levée : " << s.what() << endl;

        do {
            m_lecteur.avancer();
        } while (!((m_lecteur.getSymbole() == "<VARIABLE>"
                || m_lecteur.getSymbole() == "si"
                || m_lecteur.getSymbole() == "tantque"
                || m_lecteur.getSymbole() == "repeter"
                || m_lecteur.getSymbole() == "pour"
                || m_lecteur.getSymbole() == "ecrire"
                || m_lecteur.getSymbole() == "lire")
                || m_lecteur.getSymbole() == "<FINDEFICHIER>"));
        m_erreurExistante = true;
        return nullptr;
    }


}

Noeud* Interpreteur::instLire() {
    // <instLire> ::= lire( <variable> {, <variable> } )

    try {
        bool lecturePremierTerme = true; //le but étant de lire le premier terme sans prêter attention aux virgules
        NoeudInstLire* lireNoeud = new NoeudInstLire();

        testerEtAvancer("lire");
        testerEtAvancer("(");

        do { //do while pour lire le premier terme qui sera potentiellement présent

            if (!lecturePremierTerme) { //si c'est pas le premier terme,
                testerEtAvancer(","); //il faut attendre une virgule entre chaque instruction
            }

            Noeud* var = m_table.chercheAjoute(m_lecteur.getSymbole()); // La variable est ajoutée à la table et on la mémorise

            lireNoeud->ajoute(var); //ajoute variable au noeud (NoeudInstEcrire)
            m_lecteur.avancer(); //on passe la "<VARIABLE>"   

            lecturePremierTerme = false;

        } while (m_lecteur.getSymbole() != ")");
        m_lecteur.avancer(); //on passe la parenthèse fermante ")"

        return lireNoeud;
    } catch (SyntaxeException s) {
        cout << "Exception declenchée : " << s.what() << endl;

        do {
            m_lecteur.avancer();
        } while (!((m_lecteur.getSymbole() == "<VARIABLE>"
                || m_lecteur.getSymbole() == "si"
                || m_lecteur.getSymbole() == "tantque"
                || m_lecteur.getSymbole() == "repeter"
                || m_lecteur.getSymbole() == "pour"
                || m_lecteur.getSymbole() == "ecrire"
                || m_lecteur.getSymbole() == "lire")
                || m_lecteur.getSymbole() == "<FINDEFICHIER>"));
        m_erreurExistante = true;
        return nullptr;
    }

}

