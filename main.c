#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>


/* COSTANTI GLOBALI */

// Numero massimo di relazioni stimato
#define MAX_RELATIONS_NUMBER 25
// Lunghezza massima stimata del nome di una relazione
#define RELATION_NAME_LENGTH 50
// Lunghezza massima stimata del nome di un'entità
#define ENTITY_NAME_LENGTH 50
// Lunghezza del buffer per memorizzare il comando
#define COMMAND_NAME_LENGTH 15
// Dimensione del buffer di lettura
#define BUFFER_SIZE 200


// Costanti per gli alberi rosso-neri
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

// Un nodo dell'albero delle entità
struct Entity_node {
    struct Entity *key;
    struct Entity_node *left, *right, *p;
    bool color;
};

// Un nodo dell'albero delle relazioni
struct Relation_node {
    char *sender;
    struct Relation_node *left, *right, *p;
    bool color;
};

// La rappresentazione di un'entità attraverso il suo nome, utilizzata nel record delle relazioni
struct Entity_name {
    char name[ENTITY_NAME_LENGTH];
    struct Entity_name *next;
};

// La rappresentazione di una relazione monitorata, utilizzata per velocizzare la report
struct Relation_record {
    struct Entity_name *most_popular;
    char relation_name[RELATION_NAME_LENGTH];
    unsigned int relations;
    struct Relation_record *next;
};

/* VARIABILI GLOBALI */

// Per la gestione di input/output
FILE *fp1, *fp2;

// Radice della lista dei record utilizzati per velocizzare la report
struct Relation_record *record_root = NULL;

// Buffer per salvare temporaneamente stringhe
char buffer[BUFFER_SIZE];

// Radice dell'albero rosso-nero contenente tutte le entità
struct Entity_node *entities_root;

// Il nodo T_NIL_ENTITY per l'albero delle entità
struct Entity_node T_NIL_ENTITY_NODE;
// Il puntatore a T_NIL_ENTITY
struct Entity_node *T_NIL_ENTITY = &T_NIL_ENTITY_NODE;

// Il nodo T_NIL_ENTITY per l'albero delle relazioni
struct Relation_node T_NIL_RELATION_NODE;
// Il puntatore a T_NIL_ENTITY
struct Relation_node *T_NIL_RELATION = &T_NIL_RELATION_NODE;

// Il nome dell'entità che si sta eliminando
char *eliminating_entity_name;

// Se un'istanza di relazione era presente nell'albero dove si è cercato
bool FOUND;

// Variabile globale per evitare passaggi di parametri inutili in funzione ricorsiva
struct Relation_node **CURRENT_ROOT;

// Sto facendo debug?
bool DEBUG;

/* FUNZIONI PER LA GESTIONE DEGLI ALBERI */

void inorder_entity_tree_walk(struct Entity_node *root) {

    if (root != T_NIL_ENTITY) {
        inorder_entity_tree_walk(root->left);
        puts(root->key->name);
        inorder_entity_tree_walk(root->right);
    }
}

// Cerca un'entità nell'albero
struct Entity_node *entity_search(char *name) {

    if (entities_root == T_NIL_ENTITY)
        // albero vuoto
        return T_NIL_ENTITY;

    struct Entity_node *current = entities_root;

    do {
        if (strcmp(name, current->key->name) == 0)
            return current;
        if (strcmp(name, current->key->name) > 0)
            current = current->right;
        else current = current->left;

    } while (current != T_NIL_ENTITY);

    // non trovata
    return T_NIL_ENTITY;
}

// Cerca un'entità origine dato un albero di relazioni (per addrel)
struct Relation_node *relation_name_search(struct Relation_node *root, char *name) {

    if (root == T_NIL_RELATION)
        // albero vuoto
        return T_NIL_RELATION;

    struct Relation_node *current = root;

    do {
        if (strcmp(name, current->sender) == 0)
            return current;
        if (strcmp(name, current->sender) > 0)
            current = current->right;
        else current = current->left;

    } while (current != T_NIL_RELATION);

    // non trovata
    return T_NIL_RELATION;
}

struct Relation_type *search_relation_type(struct Entity *entity, char name[RELATION_NAME_LENGTH]) {
    struct Relation_type *curr = entity->relations;
    //TODO: Ottimizzare
    while (curr != NULL && strcmp(curr->relation_name, name) != 0) {
        curr = curr->next_relation;
    }
    return curr;
}

// Cerca un'istanza di relazione in un'entità (per delrel)
struct Relation_node *search_relation(struct Entity *rel_dest, char *rel_name, char *rel_orig) {


    struct Relation_type *curr_rel_type = rel_dest->relations;
    // cerca se esiste quel tipo di relazione
    //TODO: Ottimizzare
    while (curr_rel_type != NULL && strcmp(curr_rel_type->relation_name, rel_name) != 0)
        curr_rel_type = curr_rel_type->next_relation;
    // se non trovato, ritorna NIL
    if (curr_rel_type == NULL)
        return T_NIL_RELATION;

    // se quel tipo di relazione esiste, cerca la specifica istanza
    struct Relation_node *curr_rel = curr_rel_type->relations_root;

    // ricerca nell'albero
    do {
        if (strcmp(rel_orig, curr_rel->sender) == 0)
            return curr_rel;
        if (strcmp(rel_orig, curr_rel->sender) > 0)
            curr_rel = curr_rel->right;
        else curr_rel = curr_rel->left;

    } while (curr_rel != T_NIL_RELATION);

    // non trovata
    return T_NIL_RELATION;
}

// Funzione di supporto per delete
struct Entity_node *entity_successor(struct Entity_node *x) {

    if (x->right != T_NIL_ENTITY) {
        // allora il successore è il minimo del sottoalbero destro
        while (x->left != T_NIL_ENTITY) x = x->left;
        return x;
    }
    //se non c'è l'albero destro allora risalgo fino a trovare un figlio sinistro
    struct Entity_node *y;
    y = x->p;
    while (y != T_NIL_ENTITY && x == y->right) {
        x = y;
        y = x->p;
    }
    return y;
}

// Funzine di supporto per delete
struct Relation_node *relation_successor(struct Relation_node *x) {

    if (x->right != T_NIL_RELATION) {
        // allora il successore è il minimo del sottoalbero destro
        while (x->left != T_NIL_RELATION) x = x->left;
        return x;
    }
    struct Relation_node *curr;
    //se non c'è l'albero destro allora risalgo fino a trovare un figlio sinistro
    curr = x->p;
    while (curr != T_NIL_RELATION && x == curr->right) {
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
    if (y->left != T_NIL_ENTITY)
        y->left->p = x;
    //attacca il padre di x a y
    y->p = x->p;
    if (x->p == T_NIL_ENTITY) {
        entities_root = y;
    } else if (x == x->p->left) {
        x->p->left = y;
    } else x->p->right = y;
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
    if (y->right != T_NIL_ENTITY)
        y->right->p = x;
    //attacca il padre di x a y
    y->p = x->p;
    if (x->p == T_NIL_ENTITY)
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
    if (y->left != T_NIL_RELATION)
        y->left->p = x;
    //attacca il padre di x a y
    y->p = x->p;
    if (x->p == T_NIL_RELATION) {
        *tree_root = y;
    } else if (x == x->p->left) {
        x->p->left = y;
    } else x->p->right = y;
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
    if (y->right != T_NIL_RELATION)
        y->right->p = x;
    //attacca il padre di x a y
    y->p = x->p;
    if (x->p == T_NIL_RELATION)
        *tree_root = y;
    else if (x == x->p->left)
        x->p->left = y;
    else x->p->right = y;
    //mette x a destra di y
    y->right = x;
    x->p = y;
}

struct Entity_node *entity_create(char *name) {
    //l'entità non era già presente, costruisce una nuova entità a partire dal nome
    struct Entity *entity = malloc(sizeof(struct Entity));
    entity->name = malloc(strlen(name) + 1);
    strcpy(entity->name, name);
    entity->relations = NULL;

    //costruisce il nodo con la chiave
    struct Entity_node *new = malloc(sizeof(struct Entity_node));
    new->key = entity;
    return new;
}

// Funzione di supporto all'inserimento
void entity_insert_fixup(struct Entity_node *z) {

    struct Entity_node *y;

    while (z->p->color == RED) {
        if (z->p == z->p->p->left) {
            y = z->p->p->right;
            if (y->color == RED) {
                z->p->color = BLACK;
                y->color = BLACK;
                z->p->p->color = RED;
                z = z->p->p;
            } else {
                if (z == z->p->right) {
                    z = z->p;
                    entity_left_rotate(z);
                }
                z->p->color = BLACK;
                z->p->p->color = RED;
                entity_right_rotate(z->p->p);
            }
        } else {
            y = z->p->p->left;
            if (y->color == RED) {
                z->p->color = BLACK;
                y->color = BLACK;
                z->p->p->color = RED;
                z = z->p->p;
            } else {
                if (z == z->p->left) {
                    z = z->p;
                    entity_right_rotate(z);
                }
                z->p->color = BLACK;
                z->p->p->color = RED;
                entity_left_rotate(z->p->p);
            }
        }
    }
    entities_root->color = BLACK;
}

// Inserimento di un'entità nell'albero, con verifica per evitare duplicati
void entity_insert(struct Entity_node *z) {

    struct Entity_node *y = T_NIL_ENTITY,
                       *x = entities_root;
    while (x != T_NIL_ENTITY) {
        y = x;
        if (strcmp(z->key->name, x->key->name) < 0)
            x = x->left;
        else x = x->right;
    }
    z->p = y;
    if (y == T_NIL_ENTITY)
        entities_root = z;
    else if (strcmp(z->key->name, y->key->name) < 0)
        y->left = z;
    else y->right = z;
    z->left = T_NIL_ENTITY;
    z->right = T_NIL_ENTITY;
    z->color = RED;
    entity_insert_fixup(z);
}

// Funzione di supporto all'inserimento
void relation_insert_fixup(struct Relation_type *type, struct Relation_node *z) {

    struct Relation_node *y = T_NIL_RELATION;

    while (z->p->color == RED) {
        if (z->p == z->p->p->left) {
            y = z->p->p->right;
            if (y->color == RED) {
                z->p->color = BLACK;
                y->color = BLACK;
                z->p->p->color = RED;
                z = z->p->p;
            } else {
                if (z == z->p->right) {
                    z = z->p;
                    relation_left_rotate(&type->relations_root, z);
                }
                z->p->color = BLACK;
                z->p->p->color = RED;
                relation_right_rotate(&type->relations_root, z->p->p);
            }
        } else {
            y = z->p->p->left;
            if (y->color == RED) {
                z->p->color = BLACK;
                y->color = BLACK;
                z->p->p->color = RED;
                z = z->p->p;
            } else {
                if (z == z->p->left) {
                    z = z->p;
                    relation_right_rotate(&type->relations_root, z);
                }
                z->p->color = BLACK;
                z->p->p->color = RED;
                relation_left_rotate(&type->relations_root, z->p->p);
            }
        }
    }
    type->relations_root->color = BLACK;
}

// Inserimento di una istanza di relazione nell'albero, con verifica per evitare duplicati
void relation_instance_insert(struct Relation_type *type, char *name) {

    struct Relation_node *y = T_NIL_RELATION, *x = type->relations_root;
    struct Relation_node *z = malloc(sizeof(struct Entity_node));
    z->right = T_NIL_RELATION;
    z->left = T_NIL_RELATION;
    z->sender = malloc(strlen(name) + 1);
    strcpy(z->sender, name);
    z->p = T_NIL_RELATION;


    while (x != T_NIL_RELATION) {
        y = x;
        if (strcmp(z->sender, x->sender) < 0)
            x = x->left;
        else x = x->right;
    }
    z->p = y;
    if (y == T_NIL_RELATION)
        type->relations_root = z;
    else if (strcmp(z->sender, y->sender) < 0)
        y->left = z;
    else y->right = z;
    z->left = T_NIL_RELATION;
    z->right = T_NIL_RELATION;
    z->color = RED;
    relation_insert_fixup(type, z);
}

// Distrugge un intero albero di relazioni, ma non la radice
void relation_tree_destroy(struct Relation_node *root) {
    if (root != T_NIL_RELATION) {
        relation_tree_destroy(root->left);
        relation_tree_destroy(root->right);
        free(root->sender);
    }
}

// Deallocazione di un'intera entità
void entity_destroy(struct Entity_node *x) {
    free(x->key->name);
    struct Relation_type *curr, *next;
    curr = x->key->relations;
    if (curr != NULL) {
        //scorre tutti i tipi di relazione
        while (curr != NULL) {
            next = curr->next_relation;
            free(curr->relation_name);
            //dealloca l'intero albero
            relation_tree_destroy(curr->relations_root);
            //dealloca la radice
            free(curr->relations_root);
            free(curr);
            curr = next;
        }
        free(x->key->relations);
    }
    //dealloca il nodo
    free(x->key);
    free(x);
}

// Deallocazione di un'istanza di relazione
void relation_instance_destroy(struct Relation_node *x) {
    free(x->sender);
    free(x);
}

// Elimina un tipo di relazione, nei parametri va fornito anche il precedente così da avere tempo di esecuzione O(1)
void relation_destroy(struct Relation_type *relation, struct Relation_type *prev) {
    //collega la relazione precedente alla successiva di quella da eliminare
    prev->next_relation = relation->next_relation;
    relation_tree_destroy(relation->relations_root);
    free(relation->relations_root);
    free(relation->relation_name);
    free(relation);
}

// Funzione di supporto alla cancellazione
void entity_transplant(struct Entity_node *u, struct Entity_node *v) {
    if (u->p == T_NIL_ENTITY)
        entities_root = v;
    else if (u == u->p->left)
        u->p->left = v;
    else u->p->right = v;
    v->p = u->p;
}

// Funzione di supporto alla cancellazione
void relation_transplant(struct Relation_node **tree_root, struct Relation_node *u, struct Relation_node *v) {
    if (u->p == T_NIL_RELATION)
        *tree_root = v;
    else if (u == u->p->left)
        u->p->left = v;
    else u->p->right = v;
    v->p = u->p;
}

// Funzione di supporto alla cancellazione
void entity_delete_fixup(struct Entity_node *x) {

    struct Entity_node *w;

    while (x != entities_root && x->color == BLACK) {
        if (x == x->p->left) {
            w = x->p->right;
            if (w->color == RED) {
                w->color = BLACK;
                x->p->color = RED;
                entity_left_rotate(x->p);
                w = x->p->right;
            }
            if (w->left->color == BLACK && w->right->color == BLACK) {
                w->color = RED;
                x = x->p;
            } else {
                if (w->right->color == BLACK) {
                    w->left->color = BLACK;
                    w->color = BLACK;
                    entity_right_rotate(w);
                    w = x->p->right;
                }
                w->color = x->p->color;
                x->p->color = BLACK;
                w->right->color = BLACK;
                entity_left_rotate(x->p);
                x = entities_root;
            }
        } else {
            w = x->p->left;
            if (w->color == RED) {
                w->color = BLACK;
                x->p->color = RED;
                entity_right_rotate(x->p);
                w = x->p->left;
            }
            if (w->right->color == BLACK && w->left->color == BLACK) {
                w->color = RED;
                x = x->p;
            } else {
                if (w->left->color == BLACK) {
                    w->right->color = BLACK;
                    w->color = BLACK;
                    entity_left_rotate(w);
                    w = x->p->left;
                }
                w->color = x->p->color;
                x->p->color = BLACK;
                w->left->color = BLACK;
                entity_right_rotate(x->p);
                x = entities_root;
            }
        }
    }
    x->color = BLACK;
}

// Cancellazione di un'entità dall'albero, da chiamare dopo aver verificato se l'entità è presente
void entity_node_delete(struct Entity_node *z) {

    bool y_orig_color;
    struct Entity_node *x, *y;

    y = z;
    y_orig_color = y->color;
    if (z->left == T_NIL_ENTITY) {
        x = z->right;
        entity_transplant(z, z->right);
    }
    else if (z->right == T_NIL_ENTITY) {
        x = z->left;
        entity_transplant(z, z->left);
    }
    else {
        y = z->right;
        while (y->left != T_NIL_ENTITY)
            y = y->left;
        y_orig_color = y->color;
        x = y->right;
        if (y->p == z)
            x->p = y;
        else {
            entity_transplant(y, y->right);
            y->right = z->right;
            y->right->p = y;
        }
        entity_transplant(z, y);
        y->left = z->left;
        y->left->p = y;
        y->color = z->color;
    }
    if (y_orig_color == BLACK)
        entity_delete_fixup(x);
}

// Funzione di supporto alla cancellazione
void relation_delete_fixup(struct Relation_node **tree_root, struct Relation_node *x) {

    struct Relation_node *w;

    while (x != *tree_root && x->color == BLACK) {
        if (x == x->p->left) {
            w = x->p->right;
            if (w->color == RED) {
                w->color = BLACK;
                x->p->color = RED;
                relation_left_rotate(tree_root, x->p);
                w = x->p->right;
            }
            if (w->left->color == BLACK && w->right->color == BLACK) {
                w->color = RED;
                x = x->p;
            } else {
                if (w->right->color == BLACK) {
                    w->left->color = BLACK;
                    w->color = BLACK;
                    relation_right_rotate(tree_root, w);
                    w = x->p->right;
                }
                w->color = x->p->color;
                x->p->color = BLACK;
                w->right->color = BLACK;
                relation_left_rotate(tree_root, x->p);
                x = *tree_root;
            }
        } else {
            w = x->p->left;
            if (w->color == RED) {
                w->color = BLACK;
                x->p->color = RED;
                relation_right_rotate(tree_root, x->p);
                w = x->p->left;
            }
            if (w->right->color == BLACK && w->left->color == BLACK) {
                w->color = RED;
                x = x->p;
            } else {
                if (w->left->color == BLACK) {
                    w->right->color = BLACK;
                    w->color = BLACK;
                    relation_left_rotate(tree_root, w);
                    w = x->p->left;
                }
                w->color = x->p->color;
                x->p->color = BLACK;
                w->left->color = BLACK;
                relation_right_rotate(tree_root, x->p);
                x = *tree_root;
            }
        }
    }
    x->color = BLACK;
}

// Cancellazione di una relazione dall'albero e deallocazione del nodo
void relation_delete(struct Relation_node **tree_root, struct Relation_node *z) {

    bool y_orig_color;
    struct Relation_node *x, *y;

    y = z;
    y_orig_color = y->color;
    if (z->left == T_NIL_RELATION) {
        x = z->right;
        relation_transplant(tree_root, z, z->right);
    }
    else if (z->right == T_NIL_RELATION) {
        x = z->left;
        relation_transplant(tree_root, z, z->left);
    }
    else {
        y = z->right;
        while (y->left != T_NIL_RELATION)
            y = y->left;
        y_orig_color = y->color;
        x = y->right;
        if (y->p == z)
            x->p = y;
        else {
            relation_transplant(tree_root, y, y->right);
            y->right = z->right;
            y->right->p = y;
        }
        relation_transplant(tree_root, z, y);
        y->left = z->left;
        y->left->p = y;
        y->color = z->color;
    }
    if (y_orig_color == BLACK)
        relation_delete_fixup(tree_root, x);
}

void counter_decrease(struct Relation_type *relation) {
    //TODO: Implement this function
}

void record_counter_increase(struct Relation_record *record, char *name, unsigned number) {

    if (number < record->relations)
        return;

    struct Entity_name *curr, *prev;
    curr = record->most_popular;

    if (number > record->relations) {
        //libera la lista e lascia solo il nuovo
        while (curr != NULL) {
            prev = curr;
            curr = curr->next;
            free(prev);
        }
        record->most_popular = malloc(sizeof(struct Entity_name));
        record->most_popular->next = NULL;
        strcpy(record->most_popular->name, name);
        record->relations = number;
        return;
    }

    if (record->most_popular == NULL) {
        //il record era vuoto
        record->most_popular = malloc(sizeof(struct Entity_name));
        record->most_popular->next = NULL;
        strcpy(record->most_popular->name, name);
        record->relations = number;
        return;
    }
    if (strcmp(record->most_popular->name, name) > 0) {
        //inserimento in testa
        curr = record->most_popular;
        record->most_popular = malloc(sizeof(struct Entity_name));
        record->most_popular->next = curr;
        strcpy(record->most_popular->name, name);
    }
    else if (record->most_popular->next == NULL) {

        //un solo elemento in lista
        if (strcmp(record->most_popular->name, name) > 0) {
            //inserisce prima
            record->most_popular = malloc(sizeof(struct Entity_name));
            strcpy(record->most_popular->name, name);
            record->most_popular->next = curr;

        } else {
            //inserisce dopo
            record->most_popular->next = malloc(sizeof(struct Entity_name));
            record->most_popular->next->next = NULL;
            strcpy(record->most_popular->next->name, name);
        }
    } else {
        prev = curr;
        //scorre la lista
        while (curr->next != NULL && strcmp(curr->name, name) < 0) {
            prev = curr;
            curr = curr->next;
        }
        if (strcmp(curr->name, name) > 0) {
            //inserisce prima
            prev->next = malloc(sizeof(struct Entity_name));
            strcpy(prev->next->name, name);
            prev->next->next = curr;
        } else {
            //inserisce dopo
            curr->next = malloc(sizeof(struct Entity_name));
            strcpy(curr->next->name, name);
            curr->next->next = NULL;
        }
    }
    record->relations = number;
}

// Rimuove le relazioni entranti da parte di una certa entità.
void search_relation_by_name(struct Relation_node *x) {

    if (x != T_NIL_RELATION) {
        search_relation_by_name(x->left);
        search_relation_by_name(x->right);
        if (strcmp(x->sender, eliminating_entity_name) == 0) {
            relation_delete(CURRENT_ROOT, x);
            FOUND = 1;
        }
    }
}

// Cancellazione di tutte le relazioni uscenti da un'entità
void outgoing_relations_delete(struct Entity_node *root) {

    struct Entity_node *curr = root;

    if (curr != T_NIL_ENTITY) {
        //visita tutte le entità
        outgoing_relations_delete(curr->left);

        //scorre i tipi di relazione nell'entità
        struct Relation_type *rel = curr->key->relations;
        while (rel != NULL) {

            CURRENT_ROOT = &curr->key->relations->relations_root;
            FOUND = 0;
            //elimina l'istanza di relazione con l'entità da eliminare
            search_relation_by_name(rel->relations_root);
            //c'era un'istanza e bisogna decrementare il contatore
            if (FOUND == 1)
                counter_decrease(rel);
            rel = rel->next_relation;
        }

        outgoing_relations_delete(curr->right);
    }
}

struct Relation_record *add_relation_record(char rel_name[RELATION_NAME_LENGTH]) {

    struct Relation_record *curr, *prev = NULL;

    curr = record_root;

    if (curr == NULL) {
        //il record era vuoto
        record_root = malloc(sizeof(struct Relation_record));
        strcpy(record_root->relation_name, rel_name);
        record_root->most_popular = NULL;
        record_root->relations = 0;
        record_root->next = NULL;
        return record_root;
    } else if (curr->next == NULL) {
        //esiste solo una relazione
        if (strcmp(curr->relation_name, rel_name) == 0) {
            //trovata, esisteva già
            return curr;
        } else if (strcmp(curr->relation_name, rel_name) < 0) {
            //inserisce dopo
            curr->next = malloc(sizeof(struct Relation_node));
            strcpy(curr->next->relation_name, rel_name);
            curr->next->relations = 0;
            curr->next->most_popular = NULL;
            curr->next->next = NULL;
            return curr->next;
        } else {
            //inserisce prima
            record_root = malloc(sizeof(struct Relation_record));
            record_root->next = curr;
            record_root->most_popular = NULL;
            record_root->relations = 0;
            strcpy(record_root->relation_name, rel_name);
            return record_root;
        }
    } else {

        //scorre la lista
        while (curr->next != NULL && strcmp(curr->relation_name, rel_name) < 0) {
            prev = curr;
            curr = curr->next;
        }

        if (strcmp(curr->relation_name, rel_name) == 0) {
            //trovata, esisteva già
            return curr;
        }
        else if (strcmp(curr->relation_name, rel_name) > 0) {
            //inserisce prima
            if (prev != NULL) {
                prev->next = malloc(sizeof(struct Relation_record));
                strcpy(prev->next->relation_name, rel_name);
                prev->next->most_popular = NULL;
                prev->next->relations = 0;
                prev->next->next = curr;
                return prev->next;
            }
            else {
                //inserisce dopo
                curr->next = malloc(sizeof(struct Relation_node));
                strcpy(curr->next->relation_name, rel_name);
                curr->next->relations = 0;
                curr->next->most_popular = NULL;
                curr->next->next = NULL;
                return curr->next;
            }
        }
        return NULL;
    }
}

//Data un'entità destinazione, trova o crea il tipo di relazione desiderato e lo restituisce
struct Relation_type *search_root(struct Entity_node *dest, char *name) {
    struct Relation_type *curr, *prev = NULL;

    curr = dest->key->relations;
    if (dest->key->relations == NULL) {
        //non c'erano relazioni
        dest->key->relations = malloc(sizeof(struct Relation_type));
        dest->key->relations->relation_name = malloc(strlen(name) + 1);
        strcpy(dest->key->relations->relation_name, name);
        dest->key->relations->relations_root = T_NIL_RELATION;
        dest->key->relations->next_relation = NULL;
        dest->key->relations->number = 0;
        return dest->key->relations;
    }
    if (strcmp(dest->key->relations->relation_name, name) > 0) {
        //inserimento in testa
        dest->key->relations = malloc(sizeof(struct Relation_type));
        dest->key->relations->relation_name = malloc(strlen(name) + 1);
        strcpy(dest->key->relations->relation_name, name);
        dest->key->relations->relations_root = T_NIL_RELATION;
        dest->key->relations->next_relation = curr;
        dest->key->relations->number = 0;
        return dest->key->relations;
    }
    else if (dest->key->relations->next_relation == NULL) {

        //un solo elemento in lista
        if (strcmp(curr->relation_name, name) == 0) {
            //trovata
            return curr;
        }
        else if (strcmp(curr->relation_name, name) > 0) {
            //inserisce prima
            prev = dest->key->relations;
            prev->next_relation = malloc(sizeof(struct Relation_type));
            prev->next_relation->next_relation = curr;
            prev->next_relation->relation_name = malloc(strlen(name) + 1);
            strcpy(prev->next_relation->relation_name, name);
            prev->next_relation->relations_root = T_NIL_RELATION;
            prev->next_relation->number = 0;
            return prev->next_relation;
        } else {
            //inserisce dopo
            curr->next_relation = malloc(sizeof(struct Relation_type));
            curr->next_relation->next_relation = NULL;
            curr->next_relation->relations_root = T_NIL_RELATION;
            curr->next_relation->number = 0;
            curr->next_relation->relation_name = malloc(strlen(name) + 1);
            strcpy(curr->next_relation->relation_name, name);
            return curr->next_relation;
        }
    } else {
        prev = curr;
        //scorre la lista, che non è vuota
        while (curr->next_relation != NULL && strcmp(curr->relation_name, name) < 0) {
            prev = curr;
            curr = curr->next_relation;
        }
        if (strcmp(curr->relation_name, name) == 0) {
            //trovata
            return curr;
        } else if (strcmp(curr->relation_name, name) < 0) {
            //inserisce prima
            prev->next_relation = malloc(sizeof(struct Relation_type));
            prev->next_relation->next_relation = curr;
            prev->next_relation->relation_name = malloc(strlen(name) + 1);
            strcpy(prev->next_relation->relation_name, name);
            prev->next_relation->relations_root = T_NIL_RELATION;
            prev->next_relation->number = 0;
            return prev->next_relation;
        } else {
            //inserisce dopo
            curr->next_relation = malloc(sizeof(struct Relation_type));
            curr->next_relation->next_relation = NULL;
            curr->next_relation->relations_root = T_NIL_RELATION;
            curr->next_relation->number = 0;
            curr->next_relation->relation_name = malloc(strlen(name) + 1);
            strcpy(curr->next_relation->relation_name, name);
            return curr->next_relation;
        }
    }
}

struct Relation_record *relation_record_search(char *rel_name) {

    if (record_root == NULL)
        return NULL;

    struct Relation_record *curr = record_root;

    if (curr->next == NULL) {
        //una sola relazione nel record
        if (strcmp(rel_name, curr->relation_name) == 0)
            return curr;
        else return NULL;
    } else {
        if (strcmp(rel_name, curr->relation_name) == 0)
                return curr;
        else do {
            curr = curr->next;
            if (strcmp(rel_name, curr->relation_name) == 0)
                return curr;
        } while (curr->next != NULL && strcmp(curr->relation_name, rel_name) < 0);
        return NULL;
    }
}

// In seguito a una delent distrugge il record e lo ricrea
void record_destroy() {
    struct Relation_record *curr, *prev;
    struct Entity_name *c, *p;
    curr = record_root;

    if (curr == NULL)
        //record vuoto
        return;
    else if (curr->next == NULL) {
        //una sola relazione monitorata
        c = curr->most_popular;
        if (c->next == NULL) {
            //una sola entità
            free(c);
        } else {
            //scorre la lista
            do {
                p = c;
                c = c->next;
                free(p);
            } while (c != NULL);
        }
        free(curr);
        record_root = NULL;
        return;
    } else {
        //scorre la lista
        do {
            prev = curr;
            curr = curr->next;
            c = prev->most_popular;
            if (c->next == NULL) {
                //una sola entità
                free(c);
            } else {
                //scorre la lista
                do {
                    p = c;
                    c = c->next;
                    free(p);
                } while (c != NULL);
            }
            free(prev);
        } while (curr != NULL);
        record_root = NULL;
        return;
    }
}

// Per ricostruire il record, percorre l'albero al contrario in modo da avere solo inserimenti in testa
void reverse_entity_tree_walk(struct Entity_node *root) {

    if (root != T_NIL_ENTITY) {
        reverse_entity_tree_walk(root->right);

        struct Relation_type *rel = root->key->relations;

        //scorre tutte le relazioni nell'entità
        if (rel == NULL)
            return;
        else {
            do {
                //inserisce la relazione nel record delle relazioni monitorate
                struct Relation_record *found;
                found = add_relation_record(rel->relation_name);
                //incrementa contatore nel record
                record_counter_increase(found, root->key->name, rel->number);
                rel = rel->next_relation;
            } while (rel != NULL);
        }
        reverse_entity_tree_walk(root->left);
    }
}

// Ricrea da zero il record
void record_create() {
    //visita tutto l'albero delle entità
    reverse_entity_tree_walk(entities_root);
}

/* FUNZIONI */

//aggiunge un'entità identificata da "id_ent" all'insieme delle entità monitorate; se l'entità è già monitorata, non fa nulla
void addent(char *name) {

    if (entity_search(name) != T_NIL_ENTITY)
        //l'entità è già presente
        return;

    struct Entity_node *new;
    new = entity_create(name);
    //inserisce l'entità nell'albero
    entity_insert(new);
}

// Elimina l'entità identificata da "id_ent" dall'insieme delle entità monitorate;
// elimina tutte le relazioni di cui "id_ent" fa parte (sia come origine, che come destinazione)
void delent(char *name) {

    FOUND = 0;
    eliminating_entity_name = name;
    struct Entity_node *found;
    found = entity_search(name);
    if (found != T_NIL_ENTITY) {
        outgoing_relations_delete(entities_root);
        entity_node_delete(found);
        //entity_destroy(found);
    } else
        return;
    FOUND = 0;

    record_destroy();
    record_create();
}

// Aggiunge una relazione – identificata da "id_rel" – tra le entità "id_orig" e "id_dest", in cui "id_dest" è il
// ricevente della relazione. Se la relazione tra "id_orig" e "id_dest" esiste già, o se almeno una delle entità non è
// monitorata, non fa nulla.
void addrel(char *orig, char *dest, char *rel_name) {

    struct Entity_node *destination;
    struct Relation_record *found;

    FOUND = 0;
    if (entity_search(orig) == T_NIL_ENTITY)
        //l'entità non è monitorata
        return;
    destination = entity_search(dest);
    if (destination == T_NIL_ENTITY)
        return;

    //inserisce la relazione nel record delle relazioni monitorate
    found = add_relation_record(rel_name);

    struct Relation_type *relationType;
    //cerca l'albero della specifica relazione
    relationType = search_root(destination, rel_name);

    //inserisce la relazione entrante nell'entità destinazione, se non esiste già
    if (relation_name_search(relationType->relations_root, orig) == T_NIL_RELATION) {
        relation_instance_insert(relationType, orig);
        //incrementa contatore nell'entità
        relationType->number++;
        //incrementa contatore nel record
        record_counter_increase(found, dest, relationType->number);
    }
}

// Elimina la relazione identificata da "id_rel" tra le entità "id_orig" e "id_dest"
// (laddove "id_dest" è il ricevente della relazione);
// se non c'è relazione "id_rel" tra "id_orig" e "id_dest" (con "id_dest" come ricevente), non fa nulla
void delrel(char *orig, char *dest, char *rel_name) {

    struct Relation_record *found;
    struct Entity_node *destination;
    found = relation_record_search(rel_name);

    if (found == NULL)
        //relazione non monitorata
        return;

    destination = entity_search(dest);
    if (destination == T_NIL_ENTITY)
        //entità non monitorata
        return;
    if (entity_search(orig) == T_NIL_ENTITY)
        //entità non monitorata
        return;
    if (destination->key->relations == NULL)
        //nessuna relazione
        return;

    struct Relation_type *rel_type, *prev;
    rel_type = destination->key->relations;
    prev = rel_type;
    while (rel_type->next_relation != NULL && strcmp(rel_type->relation_name, rel_name) < 0) {
        prev = rel_type;
        rel_type = rel_type->next_relation;
    }
    if (strcmp(rel_type->relation_name, rel_name) != 0) {
        //non trovata
        return;
    }

    struct Relation_node *node;

    //se l'istanza esiste, la elimina e decrementa il contatore
    node = search_relation(destination->key, rel_name, orig);
    if (node != T_NIL_RELATION) {
        relation_delete(&rel_type->relations_root, node);
        relation_instance_destroy(node);
        rel_type->number --;
        //se non ci sono altre istanze, elimina il tipo di relazione dall'entità
        if (rel_type->relations_root == T_NIL_RELATION) {
            prev->next_relation = rel_type->next_relation;
            free(rel_type->relation_name);
            relation_tree_destroy(rel_type->relations_root);
            //free(rel_type->relations_root);
            free(rel_type);
        }
    }

    //TODO: Ottimizzare (molto)
    record_destroy();
    record_create();
}

// Emette in output l’elenco delle relazioni, riportando per ciascuna le entità con il maggior numero di relazioni entranti
void report() {

    if (DEBUG) {

        if (record_root == NULL) {
            //record vuoto, stampa none
            putc('n', fp2);
            putc('o', fp2);
            putc('n', fp2);
            putc('e', fp2);
            putc('\n', fp2);
            return;
        }
        struct Relation_record *curr = record_root;
        struct Entity_name *entity;
        while (curr != NULL) {
            if (curr->relations > 0) {
                int i = 0;
                //stampa il nome della relazione
                putc('"', fp2);
                while (curr->relation_name[i] != '\0') {
                    putc(curr->relation_name[i], fp2);
                    i++;
                }
                putc('"', fp2);

                //stampa il nome di tutte le entità
                entity = curr->most_popular;
                while (entity != NULL) {
                    i = 0;
                    putc(' ', fp2);
                    putc('"', fp2);
                    while (entity->name[i] != '\0') {
                        putc(entity->name[i], fp2);
                        i++;
                    }
                    putc('"', fp2);
                    entity = entity->next;
                }
                //stampa il numero di relazioni
                fprintf(fp2, " %i", curr->relations);
            }
            putc(';', fp2);
            putc(' ', fp2);
            curr = curr->next;
        }
        //fine report
        putc('\n', fp2);

    } else {

        if (record_root == NULL) {
            //record vuoto, stampa none
            putchar('n');
            putchar('o');
            putchar('n');
            putchar('e');
            putchar('\n');
            return;
        }
        struct Relation_record *curr = record_root;
        struct Entity_name *entity;
        while (curr != NULL) {
            if (curr->relations > 0) {
                int i = 0;
                //stampa il nome della relazione
                putchar('"');
                while (curr->relation_name[i] != '\0') {
                    putchar(curr->relation_name[i]);
                    i++;
                }
                putchar('"');

                //stampa il nome di tutte le entità
                entity = curr->most_popular;
                while (entity != NULL) {
                    i = 0;
                    putchar(' ');
                    putchar('"');
                    while (entity->name[i] != '\0') {
                        putchar(entity->name[i]);
                        i++;
                    }
                    putchar('"');
                    entity = entity->next;
                }
                //stampa il numero di relazioni
                printf(" %i", curr->relations);
            }
            putchar(';');
            putchar(' ');
            curr = curr->next;
        }
        //fine report
        putchar('\n');
    }
}

// Fine del cinema
void end() {
    //TODO: Implement this function
}


int main() {

    //inizializza albero
    entities_root = T_NIL_ENTITY;
    T_NIL_ENTITY_NODE.key = NULL;
    T_NIL_ENTITY_NODE.right = T_NIL_ENTITY;
    T_NIL_ENTITY_NODE.left = T_NIL_ENTITY;
    T_NIL_ENTITY_NODE.p = NULL;
    T_NIL_ENTITY_NODE.color = BLACK;

    T_NIL_RELATION_NODE.p = NULL;
    T_NIL_RELATION_NODE.right = T_NIL_RELATION;
    T_NIL_RELATION_NODE.left = T_NIL_RELATION;
    T_NIL_RELATION_NODE.p = NULL;
    T_NIL_RELATION_NODE.color = BLACK;

    // Buffer per la lettura da stdin
    char entity1[ENTITY_NAME_LENGTH], entity2[ENTITY_NAME_LENGTH];
    char relation[RELATION_NAME_LENGTH];
    char ch, command[COMMAND_NAME_LENGTH];

    DEBUG = 0;

    if (DEBUG) {

        if (!(fp1 = fopen("in.txt", "r"))) {
            puts("Errore fp1");
            return -1;
        }
        if (!(fp2 = fopen("out.txt", "w"))) {
            puts("Errore fp2");
            return -1;
        }

        while (1) {

            // Scorre il file di input fino al primo comando
            do {
                ch = getc(fp1);
            } while (ch == ' ' || ch == '\n' || ch == '\0');
            // A questo punto ho trovato il primo carattere, inizio a scrivere il comando nel vettore
            int i = 0;
            do {
                command[i] = ch;
                i++;
            }
            while ((ch = getc(fp1)) != '"' && ch != '\n' && ch != ' ');
            //fine del nome del comando
            command[i] = '\0';

            // A questo punto il comando è salvato nel buffer
            if (strcmp(command, "addent") == 0) {
                //carica il nome dell'entità
                do {
                    ch = getc(fp1);
                } while (ch != '"');
                //inizio del nome
                i = 0;
                while ((ch = getc(fp1)) != '"') {
                    entity1[i] = ch;
                    i++;
                }
                //fine del nome dell'entità
                entity1[i] = '\0';
                addent(entity1);
            }

            else if (strcmp(command, "delent") == 0) {
                //carica il nome dell'entità
                do {
                    ch = getc(fp1);
                } while (ch != '"');
                //inizio del nome
                i = 0;
                while ((ch = getc(fp1)) != '"') {
                    entity1[i] = ch;
                    i++;
                }
                //fine del nome dell'entità
                entity1[i] = '\0';
                delent(entity1);
            }
            else if (strcmp(command, "addrel") == 0) {
                //carica il nome della prima entità
                do {
                    ch = getc(fp1);
                } while (ch != '"');
                //inizio del nome
                i = 0;
                while ((ch = getc(fp1)) != '"') {
                    entity1[i] = ch;
                    i++;
                }
                //fine del nome della prima entità
                entity1[i] = '\0';

                //carica il nome della seconda entità
                do {
                    ch = getc(fp1);
                } while (ch != '"');
                //inizio del nome
                i = 0;
                while ((ch = getc(fp1)) != '"') {
                    entity2[i] = ch;
                    i++;
                }
                //fine del nome della seconda entità
                entity2[i] = '\0';

                //carica il nome della relazione
                do {
                    ch = getc(fp1);
                } while (ch != '"');
                //inizio del nome
                i = 0;
                while ((ch = getc(fp1)) != '"') {
                    relation[i] = ch;
                    i++;
                }
                //fine del nome della relazione
                relation[i] = '\0';

                addrel(entity1, entity2, relation);
            }
            else if (strcmp(command, "delrel") == 0) {
                //carica il nome della prima entità
                do {
                    ch = getc(fp1);
                } while (ch != '"');
                //inizio del nome
                i = 0;
                while ((ch = getc(fp1)) != '"') {
                    entity1[i] = ch;
                    i++;
                }
                //fine del nome della prima entità
                entity1[i] = '\0';

                //carica il nome della seconda entità
                do {
                    ch = getc(fp1);
                } while (ch != '"');
                //inizio del nome
                i = 0;
                while ((ch = getc(fp1)) != '"') {
                    entity2[i] = ch;
                    i++;
                }
                //fine del nome della seconda entità
                entity2[i] = '\0';

                //carica il nome della relazione
                do {
                    ch = getc(fp1);
                } while (ch != '"');
                //inizio del nome
                i = 0;
                while ((ch = getc(fp1)) != '"') {
                    relation[i] = ch;
                    i++;
                }
                //fine del nome della relazione
                relation[i] = '\0';
                delrel(entity1, entity2, relation);
            }
            else if (strcmp(command, "report") == 0) {
                report();
            }
            else
                // "end" oppure comportamento indefinito
                //eventualmente liberare la memoria
                break;
        }

        fclose(fp1);
        fclose(fp2);

    } else {
        while (1) {

            // Scorre il file di input fino al primo comando
            do {
                ch = getchar();
            } while (ch == ' ' || ch == '\n' || ch == '\0');
            // A questo punto ho trovato il primo carattere, inizio a scrivere il comando nel vettore
            int i = 0;
            do {
                command[i] = ch;
                i++;
            }
            while ((ch = getchar()) != '"' && ch != '\n' && ch != ' ');
            //fine del nome del comando
            command[i] = '\0';

            // A questo punto il comando è salvato nel buffer
            if (strcmp(command, "addent") == 0) {
                //carica il nome dell'entità
                do {
                    ch = getchar();
                } while (ch != '"');
                //inizio del nome
                i = 0;
                while ((ch = getchar()) != '"') {
                    entity1[i] = ch;
                    i++;
                }
                //fine del nome dell'entità
                entity1[i] = '\0';
                addent(entity1);
            }

            else if (strcmp(command, "delent") == 0) {
                //carica il nome dell'entità
                do {
                    ch = getchar();
                } while (ch != '"');
                //inizio del nome
                i = 0;
                while ((ch = getchar()) != '"') {
                    entity1[i] = ch;
                    i++;
                }
                //fine del nome dell'entità
                entity1[i] = '\0';
                delent(entity1);
            }
            else if (strcmp(command, "addrel") == 0) {
                //carica il nome della prima entità
                do {
                    ch = getchar();
                } while (ch != '"');
                //inizio del nome
                i = 0;
                while ((ch = getchar()) != '"') {
                    entity1[i] = ch;
                    i++;
                }
                //fine del nome della prima entità
                entity1[i] = '\0';

                //carica il nome della seconda entità
                do {
                    ch = getchar();
                } while (ch != '"');
                //inizio del nome
                i = 0;
                while ((ch = getchar()) != '"') {
                    entity2[i] = ch;
                    i++;
                }
                //fine del nome della seconda entità
                entity2[i] = '\0';

                //carica il nome della relazione
                do {
                    ch = getchar();
                } while (ch != '"');
                //inizio del nome
                i = 0;
                while ((ch = getchar()) != '"') {
                    relation[i] = ch;
                    i++;
                }
                //fine del nome della relazione
                relation[i] = '\0';

                addrel(entity1, entity2, relation);
            }
            else if (strcmp(command, "delrel") == 0) {
                // Da implementare
                delrel(entity1, entity2, relation);
            }
            else if (strcmp(command, "report") == 0) {
                report();
            }
            else
                // "end" oppure comportamento indefinito
                //eventualmente liberare la memoria
                break;
        }
    }
    return 0;
}