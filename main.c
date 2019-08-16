#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>


/* COSTANTI GLOBALI */

// Capacità iniziale del vettore dinamico
#define INITIAL_CAPACITY 10
#define RELATION_NAME_LENGTH
#define BUFFER_SIZE 50


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


/* VARIABILI GLOBALI */

// Buffer per salvare temporaneamente stringhe
char buffer[BUFFER_SIZE];

// Un albero rosso-nero contenente tutte le entità
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
struct Entity_node *search_entity(char *name) {

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
void entity_insert(char *name, struct Entity_node *root) {

    struct Entity_node *x, *y;
    y = T_NIL_ENTITY;
    x = entities_root;
    //ricerca
    while (x != T_NIL_ENTITY) {
        y = x;
        if (strcmp(name, x->key->name) < 0)
            x = x->left;
        else if (strcmp(name, x->key->name) == 0)
            //esiste già
            return;
        else x = x->right;
    }
    //l'entità non era già presente, costruisce una nuova entità a partire dal nome
    struct Entity *entity = malloc(sizeof(struct Entity));
    entity->name = malloc(strlen(name) + 1);
    strcpy(entity->name, name);
    entity->relations = NULL;

    //costruisce il nodo con la chiave
    struct Entity_node *new = malloc(sizeof(struct Entity_node));
    new->key = entity;
    new->p = y;
    //inserisce il nodo nell'albero
    if (y == T_NIL_ENTITY)
        entities_root = new;
    else if (strcmp(new->key->name, y->key->name) < 0)
        y->left = new;
    else y->right = new;
    new->right = T_NIL_ENTITY;
    new->left = T_NIL_ENTITY;
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

// Inserimento di una istanza relazione nell'albero, con verifica per evitare duplicati
void relation_instance_insert(struct Relation_node **tree_root, char *name) {

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
    x = *tree_root;
    //ricerca
    while (x != T_NIL_RELATION) {
        y = x;
        if (strcmp(new->sender, x->sender) < 0)
            x = x->left;
        else if (strcmp(new->sender, x->sender) == 0) {
            //esiste già, non aumentare contatore
            FOUND = 1;
            return;
        } else x = x->right;
    }
    //l'entità non era già presente
    new->p = y;
    if (y == T_NIL_RELATION)
        *tree_root = new;
    else if (strcmp(new->sender, y->sender) < 0)
        y->left = new;
    else y->right = new;
    new->color = RED;
    relation_insert_fixup(*tree_root, new);
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

// Cancellazione di un'entità dall'albero
void entity_node_delete(struct Entity_node *z, struct Entity_node *root) {

    struct Entity_node *x, *y;

    if (z->left == T_NIL_ENTITY || z->right == T_NIL_ENTITY)
        y = z;
    else y = entity_successor(z);
    if (y->left != T_NIL_ENTITY)
        x = y->left;
    else x = y->right;
    x->p = y->p;
    if (y->p == T_NIL_ENTITY)
        entities_root = x;
    else {
        if (y == y->p->left)
            y->p->left = x;
        else y->p->right = x;
    }
    if (y != z)
        z->key = y->key;
    if (y->color == BLACK)
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

void counter_increase(struct Relation_type *relation) {
    //TODO: Implement this function
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

/* FUNZIONI */

//aggiunge un'entità identificata da "id_ent" all'insieme delle entità monitorate; se l'entità è già monitorata, non fa nulla
void addent(char *name) {

    //inserisce l'entità nell'albero
    entity_insert(name, entities_root);
}

// Elimina l'entità identificata da "id_ent" dall'insieme delle entità monitorate;
// elimina tutte le relazioni di cui "id_ent" fa parte (sia come origine, che come destinazione)
void delent(char *name) {

    FOUND = 0;
    eliminating_entity_name = name;
    struct Entity_node *found = T_NIL_ENTITY;
    found = search_entity(name);
    if (found != T_NIL_ENTITY) {
        entity_node_delete(found, entities_root);
        outgoing_relations_delete(entities_root);
    }
    FOUND = 0;
}

// Aggiunge una relazione – identificata da "id_rel" – tra le entità "id_orig" e "id_dest", in cui "id_dest" è il
// ricevente della relazione. Se la relazione tra "id_orig" e "id_dest" esiste già, o se almeno una delle entità non è
// monitorata, non fa nulla.
void addrel(char *orig, char *dest, char *rel_name) {

    FOUND = 0;
    if (search_entity(orig) == T_NIL_ENTITY)
        return;
    if (search_entity(dest) == T_NIL_ENTITY)
        return;
    //TODO: Implement this function
    //scorrere l'entità destinazione, trovare/creare il tipo di relazione giusto, chiamare
    //relation_instance_insert che si occuperà del flag, gestire il contatore
    //TODO: Incrementare contatore

    FOUND = 0;

}

// Elimina la relazione identificata da "id_rel" tra le entità "id_orig" e "id_dest"
// (laddove "id_dest" è il ricevente della relazione);
// se non c'è relazione "id_rel" tra "id_orig" e "id_dest" (con "id_dest" come ricevente), non fa nulla
void delrel(char *orig, char *dest, char *rel_name) {
    //TODO: Implement this function
}

// Emette in output l’elenco delle relazioni, riportando per ciascuna le entità con il maggior numero di relazioni entranti
void report() {
    //TODO: Implement this function
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


    addent("-");
    addent("-----");
    addent("---");
    addent("--");
    addent(".");
    addent(".....");
    addent("...");
    addent("..");

    inorder_entity_tree_walk(entities_root);
    puts("");


    delent("-");
    inorder_entity_tree_walk(entities_root);
    puts("");
    delent("-----");
    delent("---");
    delent("--");
    delent(".");
    delent(".....");
    delent("...");
    delent("..");
    delent("/");
    delent("/////");
    delent("///");
    delent("//");
    delent("0");
    delent("00000");
    delent("000");
    delent("00");
    delent("1");
    delent("11111");
    delent("111");
    delent("11");
    delent("2");
    delent("22222");
    delent("222");
    delent("22");
    delent("3");
    delent("33333");
    delent("333");
    delent("33");
    delent("4");
    delent("44444");
    delent("444");
    delent("44");
    delent("5");
    delent("55555");
    delent("555");
    delent("55");
    delent("6");
    delent("66666");
    delent("666");
    delent("66");
    delent("7");
    delent("77777");
    delent("777");
    delent("77");
    delent("8");
    delent("88888");
    delent("888");
    delent("88");
    delent("9");
    delent("99999");
    delent("999");
    delent("99");
    delent(":");
    delent(":::::");
    delent(":::");
    delent("::");
    delent(";");
    delent(";;;;;");
    delent(";;;");
    delent(";;");
    delent("<");
    delent("<<<<<");
    delent("<<<");
    delent("<<");
    delent("=");
    delent("=====");
    delent("===");
    delent("==");
    delent(">");
    delent(">>>>>");
    delent(">>>");
    delent(">>");
    delent("?");
    delent("?????");
    delent("???");
    delent("??");
    delent("@");
    delent("@@@@@");
    delent("@@@");
    delent("@@");
    delent("A");
    delent("AAAAA");
    delent("AAA");
    delent("AA");
    delent("B");
    delent("BBBBB");
    delent("BBB");
    delent("BB");
    delent("C");
    delent("CCCCC");
    delent("CCC");
    delent("CC");
    delent("D");
    delent("DDDDD");
    delent("DDD");
    delent("DD");
    delent("E");
    delent("EEEEE");
    delent("EEE");
    delent("EE");
    delent("F");
    delent("FFFFF");
    delent("FFF");
    delent("FF");
    delent("G");
    delent("GGGGG");
    delent("GGG");
    delent("GG");
    delent("H");
    delent("HHHHH");
    delent("HHH");
    delent("HH");
    delent("I");
    delent("IIIII");
    delent("III");
    delent("II");
    delent("JJJJJ");
    delent("JJJ");
    delent("JJ");
    delent("K");
    delent("KKKKK");
    delent("KKK");
    delent("KK");
    delent("L");
    delent("LLLLL");
    delent("LLL");
    delent("LL");
    delent("M");
    delent("MMMMM");
    delent("MMM");
    delent("MM");
    delent("N");
    delent("NNNNN");
    delent("NNN");
    delent("NN");
    delent("O");
    delent("OOOOO");
    delent("OOO");
    delent("OO");
    delent("P");
    delent("PPPPP");
    delent("PPP");
    delent("PP");
    delent("Q");
    delent("QQQQQ");
    delent("QQQ");
    delent("QQ");
    delent("R");
    delent("RRRRR");
    delent("RRR");
    delent("RR");
    delent("S");
    delent("SSSSS");
    delent("SSS");
    delent("SS");
    delent("T");
    delent("TTTTT");
    delent("TTT");
    delent("TT");
    delent("U");
    delent("UUUUU");
    delent("UUU");
    delent("UU");
    delent("V");
    delent("VVVVV");
    delent("VVV");
    delent("VV");
    delent("W");
    delent("WWWWW");
    delent("WWW");
    delent("WW");
    delent("X");
    delent("XXXXX");
    delent("XXX");
    delent("XX");
    delent("Y");
    delent("YYYYY");
    delent("YYY");
    delent("YY");
    delent("Z");
    delent("ZZZZZ");
    delent("ZZZ");
    delent("ZZ");

    puts("end");
    inorder_entity_tree_walk(entities_root);
    puts("end");


    return 0;
}