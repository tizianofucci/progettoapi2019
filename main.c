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


// Il puntatore a T_NIL_ENTITY
struct Entity_node *T_NIL_ENTITY;

// Il puntatore a T_NIL_RELATION
struct Relation_node *T_NIL_RELATION;

// Il nome dell'entità che si sta eliminando
char eliminating_entity_name[ENTITY_NAME_LENGTH];

// Se un'istanza di relazione era presente nell'albero dove si è cercato
bool FOUND;

// Variabile globale per evitare passaggi di parametri inutili in funzione ricorsiva
struct Relation_node **CURRENT_ROOT;

// Sto facendo debug?
bool DEBUG;

/* FUNZIONI PER LA GESTIONE DEGLI ALBERI */

// Stampa tutte le entità monitorate
void inorder_entity_tree_walk(struct Entity_node *root) {

    if (root != T_NIL_ENTITY) {
        inorder_entity_tree_walk(root->left);
        puts(root->key->name);
        inorder_entity_tree_walk(root->right);
    }
}

// Stampa in ordine tutte le relazioni di un certo tipo
void inorder_relation_tree_walk(struct Relation_node *root) {
    if (root != T_NIL_RELATION) {
        inorder_relation_tree_walk(root->left);
        printf(" %s, ", root->sender);
        inorder_relation_tree_walk(root->right);
    }
}

// Stampa le entità con almeno una relazione
void inorder_complete_tree_walk(struct Entity_node *root) {
    if (root == T_NIL_ENTITY)
        return;
    inorder_complete_tree_walk(root->left);
    if(root->key->relations != NULL) {
        printf("%s", root->key->name);
        struct Relation_type *rel = root->key->relations;
        while (rel != NULL) {
            printf(" {%s}: ", rel->relation_name);
            inorder_relation_tree_walk(rel->relations_root);
            putchar('\n');
            rel = rel->next_relation;
        }
    }
    inorder_complete_tree_walk(root->right);
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
struct Relation_node *relation_search(struct Relation_node *root, char *name) {

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
void relation_insert_fixup(struct Relation_node **root, struct Relation_node *z) {

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
                    relation_left_rotate(root, z);
                }
                z->p->color = BLACK;
                z->p->p->color = RED;
                relation_right_rotate(root, z->p->p);
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
                    relation_right_rotate(root, z);
                }
                z->p->color = BLACK;
                z->p->p->color = RED;
                relation_left_rotate(root, z->p->p);
            }
        }
    }
    (*root)->color = BLACK;
}

// Inserimento di una istanza di relazione nell'albero
void relation_instance_insert(struct Relation_node **root, char *name) {

    struct Relation_node *x, *y, *z;

    //alloca il nodo
    z = malloc(sizeof(struct Relation_node));
    z->sender = malloc(strlen(name) + 1);
    strcpy(z->sender, name);
    z->color = BLACK;

    y = T_NIL_RELATION;
    x = *root;

    while(x != T_NIL_RELATION) {
        y = x;
        if (strcmp(z->sender, x->sender) < 0)
            x = x->left;
        else x = x->right;
    }

    z->p = y;
    if (y == T_NIL_RELATION)
        *root = z;
    else {
        if (strcmp(z->sender, y->sender) < 0)
            y->left = z;
        else y->right = z;
    }
    z->left = T_NIL_RELATION;
    z->right = T_NIL_RELATION;
    z->color = RED;
    relation_insert_fixup(root, z);
}

// Distrugge un intero albero di relazioni
void relation_tree_destroy(struct Relation_node *root) {
    if (root != T_NIL_RELATION) {
        relation_tree_destroy(root->left);
        relation_tree_destroy(root->right);
        root->left = T_NIL_RELATION;
        root->right = T_NIL_RELATION;
        free(root->sender);
        free(root);
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
            //dealloca l'intero albero
            relation_tree_destroy(curr->relations_root);
            free(curr->relation_name);
            free(curr);
            curr = next;
        }
    }
    //dealloca il nodo
    free(x->key);
    free(x);
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
    if (strcmp(record->most_popular->name, name) == 0)
        //trovato
        return;
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
        if (strcmp(curr->name, name) == 0)
            //trovato
            return;
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

// Inserisce nel record un tipo di relazione da monitorare, se non è già presente
struct Relation_record *add_relation_record(char rel_name[RELATION_NAME_LENGTH]) {

    struct Relation_record *curr, *prev = NULL;

    curr = record_root;

    if (record_root == NULL) {
        //il record era vuoto
        record_root = malloc(sizeof(struct Relation_record));
        strcpy(record_root->relation_name, rel_name);
        record_root->most_popular = NULL;
        record_root->relations = 0;
        record_root->next = NULL;
        return record_root;
    }
    if (strcmp(curr->relation_name, rel_name) == 0) {
        //esiste in prima posizione
        return curr;
    }
    if (strcmp(curr->relation_name, rel_name) > 0) {
        //inserimento in testa
        record_root = malloc(sizeof(struct Relation_record));
        record_root->next = curr;
        record_root->most_popular = NULL;
        record_root->relations = 0;
        strcpy(record_root->relation_name, rel_name);
        return record_root;
    }
    if (record_root->next == NULL) {
        //se c'è solo una relazione e non si deve inserire prima, inserisce dopo
        record_root->next = malloc(sizeof(struct Relation_record));
        strcpy(record_root->next->relation_name, rel_name);
        record_root->next->relations = 0;
        record_root->next->most_popular = NULL;
        record_root->next->next = NULL;
        return record_root->next;

    } else {
        while (curr != NULL) {
            if (strcmp(curr->relation_name, rel_name) == 0)
                //trovata
                return curr;
            if (strcmp(curr->relation_name, rel_name) > 0) {
                //inserisce prima
                prev->next = malloc(sizeof(struct Relation_record));
                strcpy(prev->next->relation_name, rel_name);
                prev->next->relations = 0;
                prev->next->most_popular = NULL;
                prev->next->next = curr;
                return prev->next;
            }
            prev = curr;
            curr = curr->next;
        }
        //sono arrivato in fondo
        prev->next = malloc(sizeof(struct Relation_record));
        strcpy(prev->next->relation_name, rel_name);
        prev->next->relations = 0;
        prev->next->most_popular = NULL;
        prev->next->next = NULL;
        return prev->next;
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
        } else if (strcmp(curr->relation_name, name) > 0) {
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

// Cerca un tipo di relazione tra quelle monitorate
struct Relation_record *relation_record_search(char *rel_name) {

    if (record_root == NULL)
        return NULL;

    struct Relation_record *curr = record_root;

    if (curr == NULL)
        //record vuoto
        return NULL;

    while (curr != NULL) {
        if (strcmp(curr->relation_name, rel_name) == 0)
            return curr;
        curr = curr->next;
    }
    return NULL;
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
        if (rel != NULL) {
            do {
                if (rel->number > 0) {
                    //inserisce la relazione nel record delle relazioni monitorate
                    struct Relation_record *found;
                    found = add_relation_record(rel->relation_name);
                    //incrementa contatore nel record
                    record_counter_increase(found, root->key->name, rel->number);
                }

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

// Funzione ricorsiva che elimina tutte le istanze di relazione con l'entità eliminating_entity_name
void clean_relations(struct Entity_node *e_root) {

    if (e_root == T_NIL_ENTITY)
        return;

    clean_relations(e_root->left);

    //corpo della funzione
    struct Relation_type *type = e_root->key->relations;
    struct Relation_node *found;
    //scorre tutte le relazioni
    while (type != NULL && type->number > 0) {
        found = relation_search(type->relations_root, eliminating_entity_name);
        if (found != T_NIL_RELATION) {
            relation_delete(&type->relations_root, found);
            free(found->sender);
            free(found);
            type->number --;
        }
        type = type->next_relation;
    }

    clean_relations(e_root->right);
}

// Cerca un'entità tra le più popolari in una data relazione monitorata
bool search_popular(struct Relation_record *record, char *name) {

    if(record == NULL)
        return false;
    if(strcmp(record->most_popular->name, name) == 0)
        return true;
    struct Entity_name *curr = record->most_popular;
    while(curr != NULL && strcmp(curr->name, name) != 0) {
        if (strcmp(curr->name, name) == 0)
            return true;
        curr = curr->next;
    }
    return false;
}

void remove_from_popular(struct Relation_record *record, char *name) {

    struct Entity_name *curr, *prev;
    curr = record->most_popular;
    prev = curr;

    if(strcmp(record->most_popular->name, name) == 0) {
        //cancella il primo
        record->most_popular = curr->next;
        free(curr);
        return;
    }
    while (curr != NULL) {

        if(strcmp(curr->name, name) == 0) {
            //elimina curr dalla lista
            prev->next = curr->next;
            free(curr);
        }
        prev = curr;
        curr = curr->next;
    }
}

// Distrugge l'albero delle entità
void entities_tree_destroy(struct Entity_node *root) {

    if(root == T_NIL_ENTITY)
        return;
    entities_tree_destroy(root->left);
    entities_tree_destroy(root->right);
    entity_destroy(root);
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

    struct Entity_node *entity = entity_search(name);

    if (entity == T_NIL_ENTITY)
        //non esiste
        return;

    entity_node_delete(entity);
    //elimina il nodo e le relazioni destinazione
    entity_destroy(entity);

    strcpy(eliminating_entity_name, name);
    //elimina le relazioni origine, visitando tutte le entità
    clean_relations(entities_root);

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

    struct Relation_type *relationType;
    //cerca l'albero della specifica relazione
    relationType = search_root(destination, rel_name);
    //cerca il tipo di relazione nel record
    found = add_relation_record(rel_name);

    //inserisce la relazione entrante nell'entità destinazione, se non esiste già
    if (relation_search(relationType->relations_root, orig) == T_NIL_RELATION) {
        relation_instance_insert(&relationType->relations_root, orig);
        //incrementa contatore nell'entità
        relationType->number++;
        record_counter_increase(found, dest, relationType->number);
    }
}


// Elimina la relazione identificata da "id_rel" tra le entità "id_orig" e "id_dest"
// (laddove "id_dest" è il ricevente della relazione);
// se non c'è relazione "id_rel" tra "id_orig" e "id_dest" (con "id_dest" come ricevente), non fa nulla
void delrel(char *orig, char *dest, char *rel_name) {

    struct Entity_node *destination;
    if (entity_search(orig) == T_NIL_ENTITY)
        return;
    destination = entity_search(dest);
    if (destination == T_NIL_ENTITY)
        return;

    struct Relation_record *found;

    found = relation_record_search(rel_name);
    if (found == NULL || found->relations == 0 || found->most_popular == NULL)
        return;

    struct Relation_type *type;

    type = destination->key->relations;

    //cerca il tipo di relazione nell'entità destinazione
    while (type != NULL && strcmp(type->relation_name, rel_name) != 0)
        type = type->next_relation;

    if (type == NULL)
        return;

    struct Relation_node *node = relation_search(type->relations_root, orig);
    //cerca se l'istanza esiste
    if (node == T_NIL_RELATION)
        return;
    relation_delete(&type->relations_root, node);
    free(node->sender);
    free(node);
    type->number --;
    if (type->number == 0)
        type->relations_root = T_NIL_RELATION;

    if(search_popular(found, dest) == false)
        //l'entità non era tra le più popolari
        return;
    //se c'era un ex aequo basta togliere il nome dai più popolari
    if(found->most_popular->next != NULL) {
        remove_from_popular(found, dest);
        return;
    }
    //bisogna ricostruire da capo il record
    //TODO: Ricostruire solo la relazione e non tutte (inutile)
    record_destroy();
    record_create();
}



// Emette in output l’elenco delle relazioni, riportando per ciascuna le entità con il maggior numero di relazioni entranti
void report() {

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
        do {
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
        while (entity != NULL);
        //stampa il numero di relazioni
        printf(" %i", curr->relations);

        putchar(';');
        putchar(' ');
        curr = curr->next;
    }
    //fine report
    putchar('\n');
}

// Fine del cinema
void end() {
    puts("");
    record_destroy();
    entities_tree_destroy(entities_root);
    free(T_NIL_RELATION);
    free(T_NIL_ENTITY);
}

int main() {

    T_NIL_ENTITY = malloc(sizeof(struct Entity_node));
    T_NIL_RELATION = malloc(sizeof(struct Relation_node));

    T_NIL_ENTITY->key = NULL;
    T_NIL_ENTITY->right = T_NIL_ENTITY;
    T_NIL_ENTITY->left = T_NIL_ENTITY;
    T_NIL_ENTITY->p = T_NIL_ENTITY;
    T_NIL_ENTITY->color = BLACK;
    //inizializza albero
    entities_root = T_NIL_ENTITY;

    T_NIL_RELATION->sender = NULL;
    T_NIL_RELATION->right = T_NIL_RELATION;
    T_NIL_RELATION->left = T_NIL_RELATION;
    T_NIL_RELATION->p = T_NIL_RELATION;
    T_NIL_RELATION->color = BLACK;

    //TODO: togliere

    //freopen("in.txt", "r", stdin);
    //freopen("output.txt", "w", stdout);

    // Buffer per la lettura da stdin
    char entity1[ENTITY_NAME_LENGTH], entity2[ENTITY_NAME_LENGTH];
    char relation[RELATION_NAME_LENGTH];
    char ch, command[COMMAND_NAME_LENGTH];


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
            delrel(entity1, entity2, relation);
        }
        else if (strcmp(command, "report") == 0) {
            //TODO: BUG HERE
            report();
        }
        else if (strcmp(command, "here") == 0) {
            //solo per debug
            report();
        }
        else {
            end();
            break;
        }

    }
    return 0;
}

