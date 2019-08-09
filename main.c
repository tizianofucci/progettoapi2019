#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>


/* COSTANTI GLOBALI */

// Capacità iniziale del vettore dinamico
#define INITIAL_CAPACITY 10


// Costanti per gli alberi rosso-neri
#define T_NIL NULL
#define RED true
#define BLACK false

/* STRUTTURE */

// Un tipo di relazione collegato a un'entità
struct Relation_type {
    char *relation_name;
    struct Relation_type *next_relation;
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
struct Entity_node {
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
    struct Relation_node *curr_rel = curr_rel_type->relations_root;

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

// Rotazione a sinistra nell'albero delle entità
void entity_left_rotate(struct Entity_node *x) {

    struct Entity_node *y;

    y = x->right;
    //il sottoalbero sinistro di y diventa quello destro di x
    x->right = y->left;
    if (y->left == T_NIL)
        y->left->p = x;
    //attacca il padre di x a y
    y->p = x->p;
    if (x->p == T_NIL)
        entities_root = y;
    else if (x == x->p->left)
        x->p->left = y;
    else x->p->right = y;
    //mette x a sinistra di y
    y->left = x;
    x->p = y;
}

// Rotazione a destra nell'albero delle entità
void entity_right_rotate(struct Entity_node *x) {

    struct Entity_node *y;

    y = x->left;
    //il sottoalbero destro di y diventa quello sinistro di x
    x->left = y->right;
    if (y->right == T_NIL)
        y->right->p = x;
    //attacca il padre di x a y
    y->p = x->p;
    if (x->p == T_NIL)
        entities_root = y;
    else if (x == x->p->left)
        x->p->left = y;
    else x->p->right = y;
    //mette x a destra di y
    y->right = x;
    x->p = y;
}

// Rotazione a sinistra nell'albero delle relazioni
void relation_left_rotate(struct Relation_node **tree_root, struct Relation_node *x) {

    struct Relation_node *y;

    y = x->right;
    //il sottoalbero sinistro di y diventa quello destro di x
    x->right = y->left;
    if (y->left == T_NIL)
        y->left->p = x;
    //attacca il padre di x a y
    y->p = x->p;
    if (x->p == T_NIL)
        *tree_root = y;
    else if (x == x->p->left)
        x->p->left = y;
    else x->p->right = y;
    //mette x a sinistra di y
    y->left = x;
    x->p = y;
}

// Rotazione a destra nell'albero delle relazioni
void relation_right_rotate(struct Relation_node **tree_root, struct Relation_node *x) {

    struct Relation_node *y;

    y = x->left;
    //il sottoalbero destro di y diventa quello sinistro di x
    x->left = y->right;
    if (y->right == T_NIL)
        y->right->p = x;
    //attacca il padre di x a y
    y->p = x->p;
    if (x->p == T_NIL)
        *tree_root = y;
    else if (x == x->p->left)
        x->p->left = y;
    else x->p->right = y;
    //mette x a destra di y
    y->right = x;
    x->p = y;
}

// Funzione di supporto all'inserimento
void entity_insert_fixup(struct Entity_node *entity) {

    struct Entity_node *x, *y;

    if (entity == entities_root)
        entities_root->color = BLACK;
    else {
        x = entity->p;
        if (x->color == RED) {
            if (x == x->p->left) {
                //x è figlio sinistro
                y = x->p->right;
                if (y->color == RED) {
                    x->color = BLACK;
                    y->color = BLACK;
                    x->p->color =RED;
                    entity_insert_fixup(x->p);
                    //TODO: Eliminate this recursive statement
                }
                else {
                    if (entity == x->right) {
                        entity = x;
                        entity_left_rotate(entity);
                        x = entity->p;
                    }
                    x->color = BLACK;
                    x->p->color = RED;
                    entity_right_rotate(x->p);
                }
            }
            else {
                //x è figlio destro
                y = x->p->left;
                if (y->color == RED) {
                    x->color = BLACK;
                    y->color = BLACK;
                    x->p->color =RED;
                    entity_insert_fixup(x->p);
                    //TODO: Eliminate this recursive statement
                }
                else {
                    if (entity == x->left) {
                        entity = x;
                        entity_right_rotate(entity);
                        x = entity->p;
                    }
                    x->color = BLACK;
                    x->p->color = RED;
                    entity_left_rotate(x->p);
                }
            }

        }
    }
    //forse non necessario
    entities_root->color = BLACK;
}

// Inserimento di un'entità nell'albero, con verifica per evitare duplicati
void entity_insert(struct Entity entity) {

    //costruisce il nodo con la chiave
    struct Entity_node *new = malloc(sizeof(struct Entity_node));
    new->key = entity;
    new->right = T_NIL;
    new->left = T_NIL;
    new->p = T_NIL;

    struct Entity_node *x, *y;
    y = T_NIL;
    x = entities_root;
    //ricerca
    while (x != T_NIL) {
        y = x;
        if (strcmp(new->key.name, x->key.name) < 0)
            x = x->left;
        else if (strcmp(new->key.name, x->key.name) == 0)
            //esiste già
            return;
        else x = x->right;
    }
    //l'entità non era già presente
    new->p = y;
    if (y == T_NIL)
        entities_root = new;
    else if (strcmp(new->key.name, y->key.name) < 0)
        y->left = new;
    else y->right = new;
    new->color = RED;
    entity_insert_fixup(new);
}

// Funzione di supporto all'inserimento
void relation_insert_fixup(struct Relation_node *tree_root, struct Relation_node *relation) {

    struct Relation_node *x, *y;

    if (relation == tree_root)
        tree_root->color = BLACK;
    else {
        x = relation->p;
        if (x->color == RED) {
            if (x == x->p->left) {
                //x è figlio sinistro
                y = x->p->right;
                if (y->color == RED) {
                    x->color = BLACK;
                    y->color = BLACK;
                    x->p->color =RED;
                    relation_insert_fixup(tree_root, x->p);
                    //TODO: Eliminate this recursive statement
                }
                else {
                    if (relation == x->right) {
                        relation = x;
                        relation_left_rotate(&tree_root, relation);
                        x = relation->p;
                    }
                    x->color = BLACK;
                    x->p->color = RED;
                    relation_right_rotate(&tree_root, x->p);
                }
            }
            else {
                //x è figlio destro
                y = x->p->left;
                if (y->color == RED) {
                    x->color = BLACK;
                    y->color = BLACK;
                    x->p->color =RED;
                    relation_insert_fixup(tree_root, x->p);
                    //TODO: Eliminate this recursive statement
                }
                else {
                    if (relation == x->left) {
                        relation = x;
                        relation_right_rotate(&tree_root, relation);
                        x = relation->p;
                    }
                    x->color = BLACK;
                    x->p->color = RED;
                    relation_left_rotate(&tree_root, x->p);
                }
            }

        }
    }
    //forse tree_root->color = BLACK?
}

// Inserimento di una relazione nell'albero, con verifica per evitare duplicati
void relation_insert(struct Relation_node **tree_root, struct Relation relation) {

    //costruisce il nodo con la chiave
    struct Relation_node *new = malloc(sizeof(struct Relation_node));
    new->key = relation;
    new->right = T_NIL;
    new->left = T_NIL;
    new->p = T_NIL;

    struct Relation_node *x, *y;
    y = T_NIL;
    x = *tree_root;
    //ricerca
    while (x != T_NIL) {
        y = x;
        if (strcmp(new->key.name, x->key.name) < 0)
            x = x->left;
        else if (strcmp(new->key.name, x->key.name) == 0)
            //esiste già
            return;
        else x = x->right;
    }
    //l'entità non era già presente
    new->p = y;
    if (y == T_NIL)
        *tree_root = new;
    else if (strcmp(new->key.name, y->key.name) < 0)
        y->left = new;
    else y->right = new;
    new->color = RED;
    relation_insert_fixup(*tree_root, new);
}

// Funzione di supporto alla cancellazione
void entity_delete_fixup (struct Entity_node *x) {

    struct Entity_node *w;

    if (x->color == RED || x->p == T_NIL)
        x->color = BLACK;
    else if (x == x->p->left) {
        w = x->p->right;
        if (w->color == RED) {
            w->color = BLACK;
            x->p->color = RED;
            entity_left_rotate(x->p);
            w = x->p->right;
        }
        if (w->left->color == BLACK && w->right->color == BLACK) {
            w->color = RED;
            entity_delete_fixup(x->p);
            //TODO: Eliminate this recursive statement
        }
        else if (w->right->color == BLACK) {
            w->left->color = BLACK;
            w->color = RED;
            entity_right_rotate(w);
            w = x->p->right;
        }
        w->color = x->p->color;
        x->p->color = BLACK;
        w->right->color = BLACK;
        entity_left_rotate(x->p);
    }
    else {
        w = x->p->left;
        if (w->color == RED) {
            w->color = BLACK;
            x->p->color = RED;
            entity_right_rotate(x->p);
            w = x->p->left;
        }
        if (w->right->color == BLACK && w->left->color == BLACK) {
            w->color = RED;
            entity_delete_fixup(x->p);
            //TODO: Eliminate this recursive statement
        }
        else if (w->left->color == BLACK) {
            w->right->color = BLACK;
            w->color = RED;
            entity_left_rotate(w);
            w = x->p->left;
        }
        w->color = x->p->color;
        x->p->color = BLACK;
        w->left->color = BLACK;
        entity_right_rotate(x->p);
    }
    // forse x->color = BLACK?
}

// Cancellazione di un'entità dall'albero
void entity_delete (struct Entity_node *z) {

    struct Entity_node *x, *y;

    if (z->left == T_NIL || z->right == T_NIL)
        y = z;
    else y = entity_successor(z);
    if (y->left != T_NIL)
        x = y->left;
    else x = y->right;
    x->p = y->p;
    if (y->p == T_NIL)
        entities_root = x;
    else if (y == y->p->left)
        y->p->left = x;
    else y->p->right = x;
    if (y != z)
        z->key = y->key;
    if (y->color == BLACK)
        entity_delete_fixup(x);

    //TODO: fare free su tutte le cose allocate nell'entità
    free(z);
}

// Funzione di supporto alla cancellazione
void relation_delete_fixup (struct Relation_node **tree_root, struct Relation_node *x) {

    struct Relation_node *w;

    if (x->color == RED || x->p == T_NIL)
        x->color = BLACK;
    else if (x == x->p->left) {
        w = x->p->right;
        if (w->color == RED) {
            w->color = BLACK;
            x->p->color = RED;
            relation_left_rotate(tree_root, x->p);
            w = x->p->right;
        }
        if (w->left->color == BLACK && w->right->color == BLACK) {
            w->color = RED;
            relation_delete_fixup(tree_root, x->p);
            //TODO: Eliminate this recursive statement
        }
        else if (w->right->color == BLACK) {
            w->left->color = BLACK;
            w->color = RED;
            relation_right_rotate(tree_root, w);
            w = x->p->right;
        }
        w->color = x->p->color;
        x->p->color = BLACK;
        w->right->color = BLACK;
        relation_left_rotate(tree_root, x->p);
    }
    else {
        w = x->p->left;
        if (w->color == RED) {
            w->color = BLACK;
            x->p->color = RED;
            relation_right_rotate(tree_root, x->p);
            w = x->p->left;
        }
        if (w->right->color == BLACK && w->left->color == BLACK) {
            w->color = RED;
            relation_delete_fixup(tree_root, x->p);
            //TODO: Eliminate this recursive statement
        }
        else if (w->left->color == BLACK) {
            w->right->color = BLACK;
            w->color = RED;
            relation_left_rotate(tree_root, w);
            w = x->p->left;
        }
        w->color = x->p->color;
        x->p->color = BLACK;
        w->left->color = BLACK;
        relation_right_rotate(tree_root, x->p);
    }
    // forse x->color = BLACK?
}

// Cancellazione di un'entità dall'albero
void relation_delete (struct Relation_node **tree_root, struct Relation_node *z) {

    struct Relation_node *x, *y;

    if (z->left == T_NIL || z->right == T_NIL)
        y = z;
    else y = relation_successor(z);
    if (y->left != T_NIL)
        x = y->left;
    else x = y->right;
    x->p = y->p;
    if (y->p == T_NIL)
        *tree_root = x;
    else if (y == y->p->left)
        y->p->left = x;
    else y->p->right = x;
    if (y != z)
        z->key = y->key;
    if (y->color == BLACK)
        relation_delete_fixup(tree_root, x);

    //TODO: fare free su tutte le cose allocate nell'entità
    free(z);
}





int main() {

    // inizializza l'albero vuoto
    entities_root = T_NIL;

    printf("%d", sizeof(struct Relation_type));

    return 0;
}