#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>


/* COSTANTI GLOBALI */

// Capacità iniziale del vettore dinamico
#define INITIAL_CAPACITY 10



// Costanti per gli alberi rosso-neri
#define T_NIL NULL
#define RED true;
#define BLACK false;


/* STRUTTURE */

// Un tipo di relazione collegato a un'entità
struct Relation_type{
    char *relation_name;
    struct Relation_type* next_relation;
    struct Relation_node *relations_root;
    unsigned int number;
};

// Un'entità monitorata
struct Entity {
    char *name;
    struct Relation_type *relations;
};

// Un'istanza di relazione
struct Relation {
    char *name;
    struct Relation *next;
};

// Un nodo dell'albero delle entità
struct Entity_node{
    struct Entity key;
    struct Entity_node *left, *right, *p;
    bool color;
};

// Un nodo dell'albero delle relazioni
struct Relation_node {
    struct Relation key;
    struct Relation_node *left, *right, *p;
    bool color;
} ;


/* VARIABILI GLOBALI */

// Un albero rosso-nero contenente tutte le entità
struct Entity_node *entities_root;






/* FUNZIONI */

int addent(char *name) {
    //TODO: Implement this function
}

int delent(char *name) {
    //TODO: Implement this function
}

int addrel(char *orig, char *dest, char *rel_name) {
    //TODO: Implement this function
}

int delrel(char *orig, char *dest, char *rel_name) {
    //TODO: Implement this function
}

void report() {
    //TODO: Implement this function
}

int end() {
    //TODO: Implement this function
}


/* FUNZIONI PER LA GESTIONE DEGLI ALBERI */

// Cerca un'entità nell'albero
struct Entity_node *search_entity(char *name) {

    if (entities_root == T_NIL)
        // albero vuoto
        return T_NIL;

    struct Entity_node *current = entities_root;

    do {
        if (strcmp(name, current->key.name) == 0)
            return current;
        if (strcmp(name, current->key.name) > 0)
            current = current->right;
        else current = current->left;

    } while (current != T_NIL);

    // non trovata
    return  T_NIL;
}

// Cerca una relazione nell'albero
struct Relation_node *search_relation(struct Entity *rel_dest, char *rel_name, char *rel_orig) {

    if (rel_dest == T_NIL || rel_name == T_NIL || rel_orig == T_NIL)
        return T_NIL;

    struct Relation_type *curr_rel_type = rel_dest->relations;
    // cerca se esiste quel tipo di relazione
    while(curr_rel_type != T_NIL && strcmp(curr_rel_type->relation_name, rel_name) != 0)
        curr_rel_type = curr_rel_type->next_relation;
    // se non trovato, ritorna NIL
    if (curr_rel_type == T_NIL)
        return T_NIL;

    // se quel tipo di relazione esiste, cerca la specifica istanza
    struct Relation_node *curr_rel = curr_rel_type->next_relation;

    // ricerca nell'albero
    do {
        if (strcmp(rel_orig, curr_rel->key.name) == 0)
            return curr_rel;
        if (strcmp(rel_orig, curr_rel->key.name) > 0)
            curr_rel = curr_rel->right;
        else curr_rel = curr_rel->left;

    } while (curr_rel != T_NIL);

    // non trovata
    return  T_NIL;
}

// Funzione di supporto per delete
struct Entity_node *entity_successor(struct Entity_node *x) {

    struct Entity_node* curr;

    if (x->right != T_NIL) {
        // allora il successore è il minimo del sottoalbero destro
        curr = x->right;
        while (x->left != T_NIL) x = x->left;
        return x;
    }
    //se non c'è l'albero destro allora risalgo fino a trovare un figlio sinistro
    curr = x->p;
    while (curr != T_NIL && x == curr->right ) {
        x = curr;
        curr = x->p;
    }
    return curr;
}

// Funzine di supporto per delete
struct Relation_node *relation_successor(struct Relation_node *x) {

    struct Relation_node* curr;

    if (x->right != T_NIL) {
        // allora il successore è il minimo del sottoalbero destro
        curr = x->right;
        while (x->left != T_NIL) x = x->left;
        return x;
    }
    //se non c'è l'albero destro allora risalgo fino a trovare un figlio sinistro
    curr = x->p;
    while (curr != T_NIL && x == curr->right ) {
        x = curr;
        curr = x->p;
    }
    return curr;
}



int main() {

    // inizializza l'albero vuoto
    entities_root = T_NIL;

    printf("%d", sizeof(struct Relation_type));

    return 0;
}