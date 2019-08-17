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

//
struct Relation_node **CURRENT_ROOT;


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

struct Relation_type *search_relation_type(struct Entity *entity, char name[RELATION_NAME_LENGTH]) {
    struct Relation_type *curr = entity->relations;
    while (curr != NULL && strcmp(curr->relation_name, name) != 0) {
        curr = curr->next_relation;
    }
    return curr;
}

// Cerca un'istanza di relazione in un'entità (per delrel)
struct Relation_node *search_relation(struct Entity *rel_dest, char *rel_name, char *rel_orig) {

    if (rel_dest == NULL || rel_name == NULL || rel_orig == NULL)
        return T_NIL_RELATION;

    struct Relation_type *curr_rel_type = rel_dest->relations;
    // cerca se esiste quel tipo di relazione
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
                    x->p->color = RED;
                    relation_insert_fixup(tree_root, x->p);
                } else {
                    if (relation == x->right) {
                        relation = x;
                        relation_left_rotate(&tree_root, relation);
                        x = relation->p;
                    }
                    x->color = BLACK;
                    x->p->color = RED;
                    relation_right_rotate(&tree_root, x->p);
                }
            } else {
                //x è figlio destro
                y = x->p->left;
                if (y->color == RED) {
                    x->color = BLACK;
                    y->color = BLACK;
                    x->p->color = RED;
                    relation_insert_fixup(tree_root, x->p);
                } else {
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

// Inserimento di una istanza di relazione nell'albero, con verifica per evitare duplicati
unsigned relation_instance_insert(struct Relation_type *type, char *name) {

    //flag per verifica duplicati
    FOUND = 0;

    //costruisce il nodo con la chiave
    struct Relation_node *new = malloc(sizeof(struct Relation_node));
    new->sender = name;
    new->right = T_NIL_RELATION;
    new->left = T_NIL_RELATION;
    new->p = T_NIL_RELATION;

    struct Relation_node *x, *y;
    y = T_NIL_RELATION;
    x = type->relations_root;
    //ricerca
    while (x != T_NIL_RELATION) {
        y = x;
        if (strcmp(new->sender, x->sender) < 0)
            x = x->left;
        else if (strcmp(new->sender, x->sender) == 0) {
            //esiste già, non aumentare contatore
            FOUND = 1;
            return 0;
        } else x = x->right;
    }
    //l'entità non era già presente
    new->p = y;
    if (y == T_NIL_RELATION)
        type->relations_root = new;
    else if (strcmp(new->sender, y->sender) < 0)
        y->left = new;
    else y->right = new;
    new->color = RED;
    relation_insert_fixup(type->relations_root, new);
    //incrementa contatore nell'entità
    type->number++;
    return type->number;
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

    if (x->color == RED || x->p == T_NIL_RELATION)
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
        } else {
            if (w->right->color == BLACK) {
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
            relation_delete_fixup(tree_root, x->p);
        } else {
            if (w->left->color == BLACK) {
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
    }
}

// Cancellazione di un'entità dall'albero e deallocazione del nodo
void relation_delete(struct Relation_node **tree_root, struct Relation_node *z) {

    struct Relation_node *x, *y;

    if (z->left == T_NIL_RELATION || z->right == T_NIL_RELATION)
        y = z;
    else y = relation_successor(z);
    if (y->left != T_NIL_RELATION)
        x = y->left;
    else x = y->right;
    x->p = y->p;
    if (y->p == T_NIL_RELATION)
        *tree_root = x;
    else if (y == y->p->left)
        y->p->left = x;
    else y->p->right = x;
    if (y != z)
        z->sender = y->sender;
    if (y->color == BLACK)
        relation_delete_fixup(tree_root, x);

    relation_instance_destroy(z);
}

void counter_decrease(struct Relation_type *relation) {
    //TODO: Implement this function
}

void record_counter_increase(struct Relation_record *record, char *name, unsigned number) {
    if (number < record->relations)
        return;

    if (number == record->relations) {
        //si crea un ex aequo
        struct Entity_name *curr, *prev = NULL;
        curr = record->most_popular;
        //inserimento in lista ordinata non vuota
        while (curr->next != NULL && strcmp(curr->name, name) < 0) {
            prev = curr;
            curr = curr->next;
        }

        if (strcmp(curr->name, name) == 0) {
            //trovata
            return;
        }
        if (strcmp(curr->name, name) > 0) {
            //non esisteva, inserimento in ordine
            if (prev != NULL) {
                prev->next = malloc(sizeof(struct Entity_name));
                strcpy(prev->next->name, name);
                prev->next->next = curr;
                return;
            }
            else {
                //esisteva un'unica relazione e la nuova va inserita prima
                curr = malloc(sizeof(struct Relation_record));
                strcpy(curr->name, name);
                curr->next = record->most_popular;
                record->most_popular = curr;
                return;
            }
        }
    }
    if (record->most_popular == NULL) {
        //il record era vuoto
        record->most_popular = malloc(sizeof(struct Entity_name));
        record->most_popular->next = NULL;
        strcpy(record->most_popular->name, name);
        record->relations = 1;
        return;
    }
    if (strcmp(record->most_popular->name, name) == 0 && record->most_popular->next == NULL) {
        //l'entità era l'unica con più relazioni di tutti
        record->relations++;
        return;
    }
    if (strcmp(record->most_popular->name, name) == 0 && record->most_popular->next != NULL) {
        //c'era un ex aequo e adesso c'è un unico più popolare
        struct Entity_name *curr, *prev;
        //salva la lista
        curr = record->most_popular;
        record->most_popular = malloc(sizeof(struct Entity_name));
        record->most_popular->next = NULL;
        strcpy(record->most_popular->name, name);
        record->relations++;
        //libera la lista
        while (curr->next != NULL) {
            prev = curr;
            curr = curr->next;
            free(prev);
        }
        return;
    }

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
        outgoing_relations_delete(curr->right);

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
    }
}

struct Relation_record *add_relation_record(char rel_name[RELATION_NAME_LENGTH]) {

    struct Relation_record *curr, *prev = NULL;

    curr = record_root;
    //scorre la lista
    while (curr != NULL && curr->next != NULL && strcmp(curr->relation_name, rel_name) < 0) {
        prev = curr;
        curr = curr->next;
    }
    if (curr == NULL) {
        //il record era vuoto
        record_root = malloc(sizeof(struct Relation_record));
        strcpy(record_root->relation_name, rel_name);
        record_root->most_popular = NULL;
        record_root->relations = 0;
        record_root->next = NULL;
        return record_root;
    }
    else {
        //esisteva almeno una relazione nel record
        if (strcmp(curr->relation_name, rel_name) == 0) {
            //trovata, esisteva già
            return curr;
        }
        else if (strcmp(curr->relation_name, rel_name) > 0) {
            //non esisteva, inserimento in ordine
            if (prev != NULL) {
                prev->next = malloc(sizeof(struct Relation_record));
                strcpy(prev->next->relation_name, rel_name);
                prev->next->most_popular = NULL;
                prev->next->relations = 0;
                prev->next->next = curr;
                return prev->next;
            }
            else {
                //esisteva un'unica relazione e la nuova va inserita prima
                curr = malloc(sizeof(struct Relation_record));
                strcpy(curr->relation_name, rel_name);
                curr->most_popular = NULL;
                curr->relations = 0;
                curr->next = record_root;
                record_root = curr;
                return curr;
            }
        }
        return NULL;
    }
}

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
    else {
        //scorre la lista, che non è vuota
        while (curr->next_relation != NULL && strcmp(curr->relation_name, name) < 0) {
            prev = curr;
            curr = curr->next_relation;
        }

        if (strcmp(curr->relation_name, name) == 0) {
            //trovata
            return curr;
        }
        if (strcmp(curr->relation_name, name) > 0) {
            //non esisteva, inserimento in ordine
            if (prev != NULL) {
                prev->next_relation = malloc(sizeof(struct Relation_record));
                prev->next_relation->relations_root = T_NIL_RELATION;
                prev->next_relation->relation_name = malloc(strlen(name) + 1);
                strcpy(prev->next_relation->relation_name, name);
                prev->next_relation->next_relation = curr;
                return prev->next_relation;
            }
            else {
                //esisteva un'unica relazione e la nuova va inserita prima
                curr = malloc(sizeof(struct Relation_record));
                curr->relation_name = malloc(strlen(name) + 1);
                strcpy(curr->relation_name, name);
                curr->relations_root = T_NIL_RELATION;
                curr->next_relation = dest->key->relations;
                dest->key->relations = curr;
                return curr;
            }
        }

    }
    return NULL;
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
    struct Entity_node *found = T_NIL_ENTITY;
    found = entity_search(name);
    if (found != T_NIL_ENTITY) {
        entity_node_delete(found);
        entity_destroy(found);
        outgoing_relations_delete(entities_root);
    }
    FOUND = 0;

    //TODO: Ricreare per intero il record
}

// Aggiunge una relazione – identificata da "id_rel" – tra le entità "id_orig" e "id_dest", in cui "id_dest" è il
// ricevente della relazione. Se la relazione tra "id_orig" e "id_dest" esiste già, o se almeno una delle entità non è
// monitorata, non fa nulla.
void addrel(char *orig, char *dest, char *rel_name) {

    struct Entity_node *destination;
    struct Relation_record *found;
    unsigned new_number;

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

    //inserisce la relazione entrante nell'entità destinazione
    new_number = relation_instance_insert(relationType, orig);

    if (FOUND == 0) {
        //incrementa contatore nel record
        record_counter_increase(found, dest, new_number);
    }

    FOUND = 0;

}

// Elimina la relazione identificata da "id_rel" tra le entità "id_orig" e "id_dest"
// (laddove "id_dest" è il ricevente della relazione);
// se non c'è relazione "id_rel" tra "id_orig" e "id_dest" (con "id_dest" come ricevente), non fa nulla
void delrel(char *orig, char *dest, char *rel_name) {
    //TODO: Implement this function
    //distruggere il record
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


    addent("Giovanni");
    addent("Marco");
    addent("Maria");


    addrel("Giovanni", "Giovanni", "fidanzato_di");
    addrel("Maria", "Giovanni", "amico_di");
    addrel("Giovanni", "Marco", "amico_di");
    addrel("Marco", "Marco", "amico_di");
    addrel("Maria", "Marco", "amico_di");
    report();

    return 0;
}