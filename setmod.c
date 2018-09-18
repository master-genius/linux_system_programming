#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#define ARGS_AUTO       0
#define ARGS_RECUR      1
#define ARGS_IGNDIR     2
#define ARGS_IGNFIL     3
#define ARGS_IGNLNK     4

#define ARGS_END        8

char _args[ARGS_END] = {'\0'};


#define MAX_NAME_LEN    2048

#define MOD_UNO     0
#define MOD_DIR     1
#define MOD_FIL     2
#define MOD_REG     3

#define PCELL_LEN   8

struct path_cell {
    char path[MAX_NAME_LEN];
    int type;
}

struct path_list {
    struct path_cell pce[PCELL_LEN];
    int end_ind;
    struct path_list *next;
    struct path_list *prev;
    struct path_list *plast;
};

struct path_list *
init_path_list(struct path_list* pl) {
   pl->end_ind = 0;
   pl->next = NULL;
   pl->prev = NULL;
   pl->plast = pl;
   return pl;
}

void destroy_path_list(struct path_list * pl) {
    struct path_list * ptmp;
    ptmp = pl;

    while(pl!=NULL) {
        ptmp = pl->next;
        free(pl);
        pl = ptmp;
    }
}

struct path_list *
add_path_list(struct path_list * pl, char * path, int height);

void
del_path_list(struct path_list * pl, struct path_list * pnode);

struct path_list *
get_path_list_last(struct path_list * pl);


struct path_list *
add_path_list(struct path_list * pl, char * path, int height) {
    struct path_list * plast =  pl->plast; 
    struct path_cell * pcell = NULL;
    struct path_list * ptmp;

    if (plast->end_ind < PATH_CELL_END) {
        pcell = plast->pce+plast->end_ind;
        plast->end_ind += 1;
    } else {
        ptmp = (struct path_list*)malloc(sizeof(struct path_list));
        if (ptmp==NULL) {
            perror("malloc");
            return NULL;
        }

        plast->next = ptmp;
        ptmp->next = NULL;
        ptmp->prev = plast;
        ptmp->end_ind = 0;
        pcell = ptmp->pce;
        pl->plast = ptmp;
    }

    strcpy(pcell->path, path);
    pcell->is_root = 0;
    pcell->plen = strlen(path);

    if (height > 0) {
        pcell->height = height;
    }

    return ptmp;
}


void
del_path_list(struct path_list * pl, struct path_list * pnode) {
    if (pnode == NULL)return ;

    //如果是首节点则不释放
    if (pnode->prev == NULL)return ;

    pnode->prev->next = pnode->next;
    
    if (pnode->next)
        pnode->next->prev = pnode->prev;

    if (pl->plast == pnode) {
        pl->plast = pnode->prev;
    }
    free(pnode);
}

int recur_set_mode(int deep) {

}



int main(int argc, char *argv[]) {
	return 0;
}


