#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* COSTANTI GLOBALI */

#define INITIAL_CAPACITY 10

/* STRUTTURE */

struct Relation_type {
    char *relation_name;
    struct Relation_type* next_relation;
    struct Relation *relations;
};

struct Relation {
    char *name;
    struct Relation *next;
};

typedef struct {
    char *name;
    struct Relation_type *relations;
} Entity;

/* VARIABILI GLOBALI */

// Il vettore dinamico ordinato contenente tutte le entità
Entity *entities;

// La dimensione corrente del vettore di entità
int entities_size = INITIAL_CAPACITY;

// Il numero corrente di entità monitorate
int entities_number = 0;


/* FUNZIONI */

Entity search_entity(char* name) {
    //TODO: Implement this function
}

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

int main() {

    entities = malloc(INITIAL_CAPACITY * sizeof(Entity));
    //aggiunta prima entità
    Entity first;
    first.name = malloc(30);
    strcpy(first.name, "Ciao");
    first.relations = NULL;
    entities[0] = first;

    free(first.name);
    free(entities);

    return 0;
}