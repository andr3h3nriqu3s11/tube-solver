#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <ncurses.h>

#define ESC "\033"
#define CSI ESC "["

#define ESC_CURSOR_MOVE_FORWARD(n)  CSI #n "C"
#define ESC_CURSOR_MOVE_BACK(n)     CSI #n "D"
#define ESC_CURSOR_MOVE_UP(n)       CSI #n "A"
#define ESC_CURSOR_MOVE_DOWN(n)     CSI #n "B"
#define ESC_CURSOR_MOVE_DOWNB(n)    ESC_CURSOR_MOVE_DOWN(n) ESC_CURSOR_MOVE_BACK(n)
#define ESC_CURSOR_MOVE_UPB(n)      ESC_CURSOR_MOVE_UP(n) ESC_CURSOR_MOVE_BACK(n)
#define ESC_CURSOR_COLUMN(n)        CSI #n "G"

#define BGBLACK           CSI "40m"
#define BGRED             CSI "41m"
#define BGGREEN           CSI "42m"
#define BGYELLOW          CSI "43m"
#define BGBLUE            CSI "44m"
#define BGMAGENTA         CSI "45m"
#define BGCYAN            CSI "46m"
#define BGWHITE           CSI "47m"
#define BGDEFAULT         CSI "49m"

#define RESET "\x1B[0m"
#define FGRED  "\x1B[31m"
#define FGBLUE  "\x1B[34m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

typedef enum {
    RED,
    BLUE,
    EMPTY,
} Color;

typedef struct Tube {
    unsigned int id;
    Color pos1;
    Color pos2;
    Color pos3;
    Color pos4;
} Tube;

typedef struct TubeNode {
    Tube* tube;
    struct TubeNode *next;
    struct TubeNode *last;
} TubeNode;

typedef struct {
    unsigned int size; 
    struct TubeNode *start;
    struct TubeNode *end;
} TubeArray;

bool tubeValidate(Tube *tube) {
    return tube->pos1 == tube->pos2 && tube->pos2 == tube->pos3 && tube->pos3 == tube->pos4;
}

bool tubeIsInitial(Tube *t) {
    return t->pos1 != EMPTY && t->pos2 != EMPTY && t->pos3 != EMPTY && t->pos4 != EMPTY;
}

bool tubeIsEmpty(Tube *t) {
    return t->pos1 == EMPTY && t->pos2 == EMPTY && t->pos3 == EMPTY && t->pos4 == EMPTY;
}

void tubeArrayAdd(TubeArray *self, Tube *tube) {
    if (self->start != NULL) {
        TubeNode *newNode = (TubeNode *) malloc(sizeof(TubeNode));
        newNode->tube = tube;
        newNode->last = self->end;
        newNode->next = NULL;
        self->end->next = newNode;
        self->end = newNode;
    } else {
        TubeNode *newNode = (TubeNode *) malloc(sizeof(TubeNode));
        newNode->tube = tube;
        newNode->last = NULL;
        newNode->next = NULL;
        self->start = newNode;
        self->end = newNode;
    }
    self->size += 1;
}

Tube* createTube(int id) {
    Tube *tube = (Tube *) malloc(sizeof(Tube));
    tube->id = id;
    tube->pos1 = EMPTY;
    tube->pos2 = EMPTY;
    tube->pos3 = EMPTY;
    tube->pos4 = EMPTY;
    return tube;
}

int tubeAddColor(Tube* t, Color c) {
    if (t->pos1 == EMPTY) {
        t->pos1 = c;
        return 1;
    }
    if (t->pos2 == EMPTY) {
        t->pos2 = c;
        return 1;
    }
    if (t->pos3 == EMPTY) {
        t->pos3 = c;
        return 1;
    }
    if (t->pos4 == EMPTY) {
        t->pos4 = c;
        return 1;
    }
    return 0;
}

char * getColorFromColor(Color c) {
    if (c == RED) {
        return BGRED;
    }
    if (c == BLUE) {
        return BGBLUE;
    }
    return BGDEFAULT;
} 

void printTube(Tube *t) {
    printf(ESC_CURSOR_MOVE_FORWARD(1)
           "┃"ESC_CURSOR_MOVE_DOWNB(1)
           "┃"ESC_CURSOR_MOVE_DOWNB(1)
           "┃"ESC_CURSOR_MOVE_DOWNB(1)
           "┃"ESC_CURSOR_MOVE_DOWNB(1)
           "┗"ESC_CURSOR_MOVE_UP(4)
           "%s "BGDEFAULT ESC_CURSOR_MOVE_DOWNB(1)
           "%s "BGDEFAULT ESC_CURSOR_MOVE_DOWNB(1)
           "%s "BGDEFAULT ESC_CURSOR_MOVE_DOWNB(1)
           "%s "BGDEFAULT ESC_CURSOR_MOVE_DOWNB(1)
           "━"ESC_CURSOR_MOVE_UP(4)
           "┃"ESC_CURSOR_MOVE_DOWNB(1)
           "┃"ESC_CURSOR_MOVE_DOWNB(1)
           "┃"ESC_CURSOR_MOVE_DOWNB(1)
           "┃"ESC_CURSOR_MOVE_DOWNB(1)
           "┛"ESC_CURSOR_MOVE_UP(4) 
           ESC_CURSOR_MOVE_FORWARD(1)
           ,
           getColorFromColor(t->pos4),
           getColorFromColor(t->pos3),
           getColorFromColor(t->pos2),
           getColorFromColor(t->pos1)
    );
}

void printTubeArray(TubeArray *a, Tube *ct) {
    // Get terminal size
    struct winsize w;
    ioctl(0, TIOCGWINSZ, &w);
    int columns = w.ws_col;
    int maxTubes = (columns - 2) / 5;

    TubeNode *node = a->start;

    int tubec = maxTubes;

    while( node != NULL) {
        if (maxTubes == tubec) {
            printf(ESC_CURSOR_MOVE_DOWN(4) "\n\n\n\n\n" ESC_CURSOR_MOVE_UP(4) ESC_CURSOR_MOVE_FORWARD(1) );
            tubec = 0;
        }
        printTube(node->tube);
        tubec += 1;
        node = node->next;
    }
    if (maxTubes == tubec) {
        printf(ESC_CURSOR_MOVE_DOWN(4) "\n\n\n\n\n" ESC_CURSOR_MOVE_UP(4) ESC_CURSOR_MOVE_FORWARD(1) );
        tubec = 0;
    }
    printTube(ct);
    printf(ESC_CURSOR_MOVE_DOWN(4)"\n");
}


int main()
{

    char str[100];

    printf("Tube Simulator enter initial state\n");

    int loadingTube = 0;
    
    TubeArray a = { .size = 0};

    Tube* t = createTube(loadingTube);

    //Load initial state
    while (1) {

        printTubeArray(&a, t);

        printf("Loading Tube %i: ", loadingTube);
        //fgets(str, 100, stdin);
        scanf("%s", str);
        if (strcmp(str, "r") == 0 || strcmp(str, "red") == 0 ) {
            printf("Color" FGRED " red"RESET"\n");
            tubeAddColor(t, RED);
        } else if (strcmp(str, "b") == 0 || strcmp(str, "blue") == 0 ) {
            printf("Color" FGBLUE " blue"RESET"\n");
            tubeAddColor(t, BLUE);
        } else if (strcmp(str, "e") == 0) {
            printf("Finished Inserting Tubes\n");
            if(tubeIsEmpty(t) && a.size > 1) {
                printf("Tubes are valid\n");
                break;
            } else {
                printf("Last tube is not filled\n");
            }
        } else {
            printf("Color not found\n");
        }
        
        if (tubeIsInitial(t)) {
            printf("Tube filled. ");
            if(tubeValidate(t)) {
                printf("Tube can not be filled with the same color at the start\n");
                t = createTube(loadingTube);
            } else {
                tubeArrayAdd(&a, t);
                printf("Tube added to list\n");
                loadingTube += 1;
                t = createTube(loadingTube);
            }
        }

    }

    printf("Start processing");

    return 0;
}
