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
#define BGORANGE          CSI "48;5;202m"
#define BGPINK            CSI "48;5;165m"
#define BGGREY            CSI "48;5;240m"
#define BGBLUE            CSI "48;5;033m"
#define BGGREEN           CSI "42m"
#define BGLBLUE           CSI "44m"
#define BGYELLOW          CSI "43m"

#define BGMAGENTA         CSI "45m"
#define BGCYAN            CSI "46m"
#define BGWHITE           CSI "47m"
#define BGDEFAULT         CSI "49m"

#define RESET     "\x1B[0m"
#define FGRED     "\x1B[31m"
#define FGGREEN   "\x1B[32m"
#define FGLBLUE   "\x1B[34m"
#define FGORANGE  "\x1B[38;5;202m"
#define FGPINK            CSI "38;5;165m"
#define FGGREY            CSI "38;5;240m"
#define FGBLUE           CSI "38;5;033m"
#define FGYELLOW  "\x1B[33m"

#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

typedef enum {
    RED,
    BLUE,
    GREEN,
    ORANGE,
    PINK,
    GREY,
    LBLUE,
    YELLOW,
    EMPTY,
} Color;

typedef struct Tube {
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

typedef struct TubeArray {
    unsigned int size; 
    struct TubeNode *start;
    struct TubeNode *end;
} TubeArray;

typedef struct TubePath {
    int depth;
    TubeArray* current;
    struct TubePath* parent;
} TubePath;

typedef struct TubeListNode {
    struct TubeListNode* next;
    TubePath* tubePath;
} TubeListNode;

bool tubeValidate(Tube *tube) {
    return tube->pos1 == tube->pos2 && tube->pos2 == tube->pos3 && tube->pos3 == tube->pos4;
}

bool tubeIsInitial(Tube *t) {
    return t->pos1 != EMPTY && t->pos2 != EMPTY && t->pos3 != EMPTY && t->pos4 != EMPTY;
}

bool tubeIsEmpty(Tube *t) {
    return t->pos1 == EMPTY && t->pos2 == EMPTY && t->pos3 == EMPTY && t->pos4 == EMPTY;
}

bool tubecompare(Tube * t1, Tube * t2) {
    return t1->pos1 == t2->pos1 &&
           t1->pos2 == t2->pos2 &&
           t1->pos3 == t2->pos3 &&
           t1->pos4 == t2->pos4;
}

void tubeArrayAdd(TubeArray *self, Tube *tube) {
    if (self == NULL) {
        return;
    }
    if (tube == NULL) {
        printf("\n\n\n\nNo Tube\n\n\n\n");
        return;
    }
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

bool tubeArrayRemoveTube(TubeArray *self, Tube *tube) {
    if (self == NULL || tube == NULL || self->size == 0) {
        return false;
    }
    TubeNode* travess = self->start;
    while(travess != NULL) {
        if(travess->tube != NULL && tubecompare(travess->tube, tube)) {
            if (travess->last != NULL) {
                travess->last->next = travess->next;
            }
            if (travess->next != NULL) {
                travess->next->last = travess->last;
            }
            if(travess == self->start) {
                self->start = travess->next;
            }
            if(travess == self->end) {
                self->end = travess->last;
            }
            self->size--;
            //free(travess);
            return true;
        }
        travess = travess->next;
    }
    return false;
}

void tubeListAdd(TubeListNode *self, TubePath *tubepath) {
    if (self->next == NULL) {
        TubeListNode *newNode = (TubeListNode *) malloc(sizeof(TubeListNode));
        newNode->tubePath = tubepath;
        newNode->next = NULL;
        self->next = newNode;
        return;
    }

    TubeListNode* next = self->next;
    while(next->next != NULL ) {
        next = next->next;
    }
    TubeListNode *newNode = (TubeListNode *) malloc(sizeof(TubeListNode));
    newNode->tubePath = tubepath;
    newNode->next=NULL;
    next->next = newNode;
}

Tube* createTube() {
    Tube *tube = (Tube *) malloc(sizeof(Tube));
    tube->pos1 = EMPTY;
    tube->pos2 = EMPTY;
    tube->pos3 = EMPTY;
    tube->pos4 = EMPTY;
    return tube;
}

Tube* cloneTube(Tube * t) {
    Tube *tube = (Tube *) malloc(sizeof(Tube));
    memcpy(tube, t, sizeof(Tube));
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
    if (c == GREY) {
        return BGGREY;
    }
    if (c == LBLUE) {
        return BGLBLUE;
    }
    if (c == PINK) {
        return BGPINK;
    }
    if (c == BLUE) {
        return BGBLUE;
    }
    if (c == GREEN) {
        return BGGREEN;
    }
    if (c == ORANGE) {
        return BGORANGE;
    }
    if (c == YELLOW) {
        return BGYELLOW;
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
    if (ct != NULL)  {
        if (maxTubes == tubec) {
            printf(ESC_CURSOR_MOVE_DOWN(4) "\n\n\n\n\n" ESC_CURSOR_MOVE_UP(4) ESC_CURSOR_MOVE_FORWARD(1) );
        }
        printTube(ct);
    }
    printf(ESC_CURSOR_MOVE_DOWN(4)"\n");
}

Tube * get(TubeArray *a, int i) {
    if (a->size <= i) return NULL;
    TubeNode* node = a->start;
    for (int j = i; j > 0; j--) {
        node = node->next;
    }
    return node->tube;
}

void clearArray(TubeArray *a) {
    if (a == NULL) {
        return;
    }
    TubeNode* end = a->end;
    if (end == NULL) {
        free(a);
        return;
    }
    while(end != NULL) {
        TubeNode* newEnd = end->last;
        free(end->tube);
        free(end);
        end=newEnd;
    }
    free(a);
}

Color getTop(Tube* b) {
    if (b->pos4 != EMPTY) {
        return b->pos4;
    }
    if (b->pos3 != EMPTY) {
        return b->pos3;
    }
    if (b->pos2 != EMPTY) {
        return b->pos2;
    }
    if (b->pos1 != EMPTY) {
        return b->pos1;
    }
    return EMPTY;
}

Color getAndRemoveTop(Tube* b) {
    Color r = EMPTY;
    if (b->pos4 != EMPTY) {
        r = b->pos4;
        b->pos4 = EMPTY;
    } else if (b->pos3 != EMPTY) {
        r = b->pos3;
        b->pos3 = EMPTY;
    } else if (b->pos2 != EMPTY) {
        r = b->pos2;
        b->pos2 = EMPTY;
    } else if (b->pos1 != EMPTY) {
        r = b->pos1;
        b->pos1 = EMPTY;
    }
    return r;
}

typedef enum moveReturnResult {
    ERROR,
    NOTHINGTODO,
    SUCCESS
} moveReturnResult;

moveReturnResult moveTube(TubeArray *a, int start, int end) {
    Tube* to = get(a, end);
    Tube* from = get(a, start);
    if (to == NULL || from == NULL) {
        return ERROR;
    }
    Color topFrom = getTop(from);
    Color topTo = getTop(to);
    if (topFrom == EMPTY || 
        to->pos4 != EMPTY || 
        start == end || 
        ( topTo != EMPTY && topFrom != topTo) ||
        tubeValidate(from)
    ) {
        return NOTHINGTODO;
    }
    if (to->pos4 == EMPTY && 
        to->pos3 == EMPTY &&
        to->pos2 == EMPTY &&
        to->pos1 == EMPTY) {
        to->pos1 = getAndRemoveTop(from);
    }
    if (to->pos4 == EMPTY && 
        to->pos3 == EMPTY &&
        to->pos2 == EMPTY) {
        if (topFrom != getTop(from))  {
            return SUCCESS;
        }
        to->pos2 = getAndRemoveTop(from);
    }
    if (to->pos4 == EMPTY && 
        to->pos3 == EMPTY) {
        if (topFrom != getTop(from))  {
            return SUCCESS;
        }
        to->pos3 = getAndRemoveTop(from);
    }
    if (to->pos4 == EMPTY) {
        if (topFrom != getTop(from))  {
            return SUCCESS;
        }
        to->pos4 = getAndRemoveTop(from);
    }
    return SUCCESS;
}

TubeArray * cloneTubeArray(TubeArray * original) {
    TubeArray * newAr = (TubeArray *) malloc(sizeof(TubeArray));
    newAr->size = 0;
    newAr->start = NULL;
    newAr->end = NULL;
    TubeNode* toCopy = original->start;
    while(toCopy != NULL) {
        Tube* tube = cloneTube(toCopy->tube);
        tubeArrayAdd(newAr, tube);
        toCopy = toCopy->next;
    }
    return newAr;
}

bool compareTubeArray(TubeArray * a, TubeArray * b) {
    if (a->size != b->size) {
        return false;
    }
    TubeArray * aC = cloneTubeArray(a);
    TubeNode * t = b->start;
    while(t != NULL) {
        if (!tubeArrayRemoveTube(aC, t->tube)) {
            clearArray(aC);
            return false;
        }
        t = t->next;
    } 
    bool r = aC->size == 0;
    clearArray(aC);
    return r;
}

bool listHasTubeArray(TubeListNode * nodeIn, TubeArray * t) {
    TubeListNode * node = nodeIn;
    while(node != NULL) {
        if(compareTubeArray(node->tubePath->current, t)) {
            return true;
        }
        node = node->next;
    }
    return false;
}

bool isArrayBalanced(TubeArray * self) {
    int red = 0;
    int blue = 0;
    int green = 0;
    int orange = 0;
    int pink = 0;
    int grey = 0;
    int yellow = 0;
    int lblue = 0;

    TubeNode * node = self->start;

    while (node != NULL) {
        if (node->tube->pos1 == RED) {
            red++;
        } else if (node->tube->pos1 == BLUE) {
            blue++;
        } else if (node->tube->pos1 == GREEN) {
            green++;
        } else if (node->tube->pos1 == PINK) {
            pink++;
        } else if (node->tube->pos1 == GREY) {
            grey++;
        } else if (node->tube->pos1 == LBLUE) {
            lblue++;
        } else if (node->tube->pos1 == ORANGE) {
            orange++;
        } else if (node->tube->pos1 == YELLOW) {
            yellow++;
        }

        if (node->tube->pos2 == RED) {
            red++;
        } else if (node->tube->pos2 == BLUE) {
            blue++;
        } else if (node->tube->pos2 == GREEN) {
            green++;
        } else if (node->tube->pos2 == PINK) {
            pink++;
        } else if (node->tube->pos2 == GREY) {
            grey++;
        } else if (node->tube->pos2 == LBLUE) {
            lblue++;
        } else if (node->tube->pos2 == ORANGE) {
            orange++;
        } else if (node->tube->pos2 == YELLOW) {
            yellow++;
        }

        if (node->tube->pos3 == RED) {
            red++;
        } else if (node->tube->pos3 == BLUE) {
            blue++;
        } else if (node->tube->pos3 == GREEN) {
            green++;
        } else if (node->tube->pos3 == PINK) {
            pink++;
        } else if (node->tube->pos3 == GREY) {
            grey++;
        } else if (node->tube->pos3 == LBLUE) {
            lblue++;
        } else if (node->tube->pos3 == ORANGE) {
            orange++;
        } else if (node->tube->pos3 == YELLOW) {
            yellow++;
        }

        if (node->tube->pos4 == RED) {
            red++;
        } else if (node->tube->pos4 == BLUE) {
            blue++;
        } else if (node->tube->pos4 == GREEN) {
            green++;
        } else if (node->tube->pos4 == PINK) {
            pink++;
        } else if (node->tube->pos4 == GREY) {
            grey++;
        } else if (node->tube->pos4 == LBLUE) {
            lblue++;
        } else if (node->tube->pos4 == ORANGE) {
            orange++;
        } else if (node->tube->pos4 == YELLOW) {
            yellow++;
        }

        node = node->next;
    }

    return (red % 4) == 0 && 
           (blue % 4) == 0 && 
           (green % 4) == 0 && 
           (grey % 4) == 0 && 
           (lblue % 4) == 0 && 
           (yellow % 4) == 0 && 
           (orange % 4) == 0;
}

bool isArrayCompleted(TubeArray * self) {
    TubeNode * node = self->start;
    while(node != NULL) {
        if (!tubeValidate(node->tube)) {
            return false;
        }
        node = node->next;
    }
    return true;
}

int main()
{

    char str[100];

    printf("Tube Simulator enter initial state\n");

    int loadingTube = 0;
    
    TubeArray * a = (TubeArray *) malloc(sizeof(TubeArray));
    a->size = 0;

    Tube* t = createTube();

    //Load initial state
    while (1) {

        printTubeArray(a, t);

        printf("Loading Tube %i: ", loadingTube);
        //fgets(str, 100, stdin);
        scanf("%s", str);
        if (strcmp(str, "r") == 0 || strcmp(str, "red") == 0 ) {
            printf("Color" FGRED " red"RESET"\n");
            tubeAddColor(t, RED);
        } else if (strcmp(str, "b") == 0 || strcmp(str, "blue") == 0 ) {
            printf("Color" FGBLUE " blue"RESET"\n");
            tubeAddColor(t, BLUE);
        } else if (strcmp(str, "p") == 0 || strcmp(str, "pink") == 0 ) {
            printf("Color" FGPINK " pink"RESET"\n");
            tubeAddColor(t, PINK);
        } else if (strcmp(str, "g") == 0 || strcmp(str, "green") == 0 ) {
            printf("Color" FGGREEN " green"RESET"\n");
            tubeAddColor(t, GREEN);
        } else if (strcmp(str, "gr") == 0 || strcmp(str, "grey") == 0 ) {
            printf("Color" FGGREY " grey"RESET"\n");
            tubeAddColor(t, GREY);
        } else if (strcmp(str, "lb") == 0 || strcmp(str, "ligth blue") == 0 ) {
            printf("Color" FGBLUE " ligth blue"RESET"\n");
            tubeAddColor(t, LBLUE);
        } else if (strcmp(str, "o") == 0 || strcmp(str, "orange") == 0 ) {
            printf("Color" FGORANGE " orange"RESET"\n");
            tubeAddColor(t, ORANGE);
        } else if (strcmp(str, "y") == 0 || strcmp(str, "yellow") == 0 ) {
            printf("Color" FGYELLOW " yellow"RESET"\n");
            tubeAddColor(t, YELLOW);
        } else if (strcmp(str, "empty") == 0 ) {
            if(tubeIsEmpty(t)) {
                tubeArrayAdd(a, t);
                printf("Tube added to list\n");
                loadingTube += 1;
                t = createTube();
            }
        } else if (strcmp(str, "e") == 0) {
            printf("Finished Inserting Tubes\n");
            if(tubeIsEmpty(t) && a->size > 1) {
                if(!isArrayBalanced(a)) {
                    printf("Tubes are not balanced\n");
                    clearArray(a);
                    a = (TubeArray *) malloc(sizeof(TubeArray));
                    a->size = 0;
                    loadingTube = 0;
                    free(t);
                    t = createTube();
                } else {
                    printf("Tubes are valid\n");
                    tubeArrayAdd(a, t);
                    break;
                }
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
                free(t);
                t = createTube();
            } else {
                tubeArrayAdd(a, t);
                printf("Tube added to list\n");
                loadingTube += 1;
                t = createTube();
            }
        }
    }
    printf("Start processing\n");
    
    TubePath init = {
        .current = a,
        .depth = 0,
    };

    TubeListNode *all = (TubeListNode *) malloc(sizeof(TubeListNode));
    all->tubePath = &init;
    all->next = NULL;
    TubeListNode *list = (TubeListNode *) malloc(sizeof(TubeListNode));
    list->tubePath = &init;
    list->next = NULL;

    int size = 1;
    int success = 0;
    int alreadyin = 0;

    TubePath * final = NULL;
    
    while (list != NULL) {
        size--;
        TubePath* p = list->tubePath;
        TubeArray* a = p->current;
        int max = a->size;
        for (int i = 0; i < max; i ++) {
            for (int j = 0; j < max; j++) {
                TubeArray* c = cloneTubeArray(a);
                moveReturnResult r = moveTube(c, i, j);
                if (r == SUCCESS) {
                    if (!listHasTubeArray(all, c)) {
                        if (alreadyin != 0) {
                            printf("Already in x%i size: %i\n", alreadyin, size);
                            alreadyin = 0;
                        }
                        success++;
                        TubePath *path = (TubePath *) malloc(sizeof(TubePath));
                        path->depth = p->depth + 1;
                        path->current = c;
                        path->parent = p;
                        tubeListAdd(all, path);
                        tubeListAdd(list, path);
      //                  printTubeArray(a, NULL);
       //                 printf(ESC_CURSOR_MOVE_DOWN(4)"\\/\n");
        //                printTubeArray(c, NULL);
                        if(isArrayCompleted(c)) {
                            printf("Found Complete");
                            final = path;
                            goto endBigLoop;
                        }
                        size++;
                    } else {
                        clearArray(c);
                        if (success != 0) {
                            printf("success x%i, size: %i\n", success, size);
                            success = 0;
                        }
                        alreadyin++;
                    }
                } else {
                    clearArray(c);
                }
            }
        }

        list = list->next;
//        printf("\nloopcomplete\n");
    }
//    printf("size: %i\n", size);
    endBigLoop:

    if(final == NULL) {
        printf("No solution found");
    } else {
        printf("\n\n===========================\n\n");
        TubePath * b = NULL;
        while (final != NULL) {
            //printf(ESC_CURSOR_MOVE_DOWN(4)"/\\\n");
            //printTubeArray(final->current, NULL);
            TubePath * nf = final->parent;
            if(b == NULL) {
                b = final;
                b -> parent =  NULL;
            } else {
                final->parent = b;
                b = final;
            }
            final = nf;

        }
    
        while (b != NULL) {
            printf(ESC_CURSOR_MOVE_DOWN(4)"\\/\n");
            printTubeArray(b->current, NULL);
            b = b -> parent;
            getchar();
        }

    }
    return 0;
}
