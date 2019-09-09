#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <linux/kdev_t.h>
#include <stdlib.h>
#include <ctype.h>
#include <getopt.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/sysmacros.h>
#define MAX_SIZE 1024
#define LINE_SIZE 256

char tty_buffer[32];
struct stat buffer;


struct procinfo{
    int pid;
    int uid;
    int gid;
    int ppid;
    int pgid;
    int sid;
    char state;
    char ttys[32];
    char cmdline[LINE_SIZE];
    char img[64];
    struct procinfo *next;
};




int check_valid(char *string){
    int i=0;
    while(string[i] != '\0'){
        if(!isdigit(string[i]))
            return 0;
        i++;
    }
    return 1;
}

void search_device(int major){ 
    char line[128];
    FILE *fp;
    char *match;
    char devices[] = "/proc/devices";
    char device_nr[8]; 
    sprintf(device_nr,"%d", major);
    
    int cmp = 3 - strlen(device_nr);
    char *pos;
    
    fp = fopen(devices, "r");
    while(fgets(line, 128, fp)!=NULL){
        if((match = strstr(line, device_nr)) != NULL)
            if((line+cmp) == match)
                if(strstr(line,"vc/") == NULL){
                    strcpy(tty_buffer, line+4);
                    if ((pos = strchr(tty_buffer, '\n')) != NULL)
                        *pos = '\0';
                    break;
            }        
    }
}



struct procinfo *head = NULL;
struct procinfo *current = NULL;


void printList(){
    struct procinfo *ptr = head;
    printf("  pid   uid   gid   ppid   pgid   sid     tty St  (img)   cmd\n");

    while (ptr != NULL){
        printf("%5d %5d %5d %6d %6d %5d %7s %2c %s  %-8s\n", 
                ptr->pid, ptr->uid, ptr->gid, ptr->ppid, 
                ptr->pgid, ptr->sid, ptr->ttys, ptr->state,
                ptr->img, ptr->cmdline);
        ptr = ptr->next;
    }
}



void insertNode(struct procinfo data){
    struct procinfo *link = (struct procinfo*) malloc(sizeof(struct procinfo));

    link->gid = data.gid;
    link->uid = data.uid;
    link->pid = data.pid;
    link->ppid = data.ppid;
    link->pgid = data.pgid;
    link->sid = data.sid;
    link->state = data.state;
    strcpy(&(link->ttys), data.ttys);
    strcpy(&(link->cmdline), data.cmdline);
    strcpy(&(link->img), data.img);
    
    link->next = head;
    head = link;
}

int list_length(void){
    int length = 0;
    struct procinfo *current;

    for(current = head; current != NULL; current = current->next)
        length++;

    return length;
}


void sort(int type){
    int i, j, k;
    int t_pid,t_uid,t_gid,t_ppid,t_pgid,t_sid;
    char t_state;
    char t_ttys[32];
    char t_cmdline[LINE_SIZE];
    char t_img[64];
    struct procinfo temp;
    struct procinfo *current;
    struct procinfo *next;
    
    int a_cmp, b_cmp;

    int size = list_length();
    k = size;
    for(i = 0; i<size-1; i++, k--){
        current = head;
        next = head->next;
        for(j=1; j<k; j++){
            switch(type){
                case 0:
                    a_cmp = current->pid; b_cmp = next->pid;
                    break;
                case 1:
                    a_cmp = current->ppid; b_cmp = next->ppid;
                    break;
                case 2:
                    a_cmp = current->pgid; b_cmp = next->pgid;
                    break;
                case 3:
                    a_cmp = current->sid; b_cmp = next->sid;
                    break;
            }

            if (a_cmp > b_cmp){
                t_pid = current->pid; t_uid = current->uid; t_gid = current->gid;
                t_ppid = current->ppid; t_pgid = current->pgid; t_sid = current->sid;
                t_state = current->state;
                strcpy(t_ttys, &(current->ttys));
                strcpy(t_cmdline, &(current->cmdline));
                strcpy(t_img, &(current->img));

                current->pid = next->pid; current->uid = next->uid; current->gid = next->gid;
                current->ppid = next->ppid; current->pgid = next->pgid; current->sid = next->sid;
                current->state = next->state;
                strcpy(&(current->ttys), &(next->ttys));
                strcpy(&(current->cmdline), &(next->cmdline));
                strcpy(&(current->img), &(next->img));
                
                next->pid = t_pid; next->uid = t_uid; next->gid = t_gid;
                next->ppid = t_ppid; next->pgid = t_pgid; next->sid = t_sid;
                next->state = t_state;
                strcpy(next->ttys, &(t_ttys));
                strcpy(next->cmdline, &(t_cmdline));
                strcpy(next->img, &(t_img));
            }

            current = current->next;
            next = next->next;
        }
    }

}



int ps(int user_flag, int terminal_flag, int sort_type){
    int pid, ppid, pgid, uid, gid, sid, tty_nr;
    char state;
    char cmdline[LINE_SIZE];
    char img[64];
    FILE *fp;
    char dp_buffer[MAX_SIZE];
    DIR *dir, *fdinfo;
    struct dirent *dirpath;
    char line[MAX_SIZE];

    int userid;
    char *tty_name = ttyname(STDIN_FILENO);
    
    userid = getuid(); 

    struct procinfo temp;

    if ((dir = opendir("/proc/")) == NULL){
        perror("Error");
        exit(-1);
    }
    while((dirpath = readdir(dir)) != NULL){
        if (check_valid(dirpath->d_name)){
            strcpy(dp_buffer, "/proc/");
            strcat(dp_buffer, dirpath->d_name);
            strcat(dp_buffer, "/status");
            fp = fopen(dp_buffer,"r");
            fgets(line, MAX_SIZE, fp);
            fgets(line, MAX_SIZE, fp);
            fgets(line, MAX_SIZE, fp);
            fgets(line, MAX_SIZE, fp);
            fgets(line, MAX_SIZE, fp);
            fgets(line, MAX_SIZE, fp);
            fgets(line, MAX_SIZE, fp);
            fgets(line, MAX_SIZE, fp);  
            fgets(line, MAX_SIZE, fp);  // Uid
            sscanf(line, "Uid:    %*d    %d    %*d", &uid);
            fgets(line, MAX_SIZE, fp);  // Gid
            sscanf(line, "Gid:    %*d    %d    %*d", &gid);
    
            strcpy(dp_buffer,"/proc/");
            strcat(dp_buffer,dirpath->d_name);
            strcat(dp_buffer,"/cmdline");
            fp = fopen(dp_buffer, "r");
            if (fgets(line, MAX_SIZE, fp)!=NULL)
                strcpy(cmdline, line);
            else
                strcpy(cmdline,"\0");
            
            //sscanf(line,"%s",cmdline);

        
                //memset(cmdline, 0, LINE_SIZE);
            strcpy(dp_buffer,"/proc/");
            strcat(dp_buffer, dirpath->d_name);
            strcat(dp_buffer,"/stat");
            fp = fopen(dp_buffer, "r");
            fgets(line,MAX_SIZE, fp);
            sscanf(line, "%d %s %c %d %d %d %d", 
                    &pid, &img, &state, &ppid, &pgid, &sid, &tty_nr);
            int major = MAJOR(tty_nr);
            int minor = MINOR(tty_nr);
            
            
            char ttys[32];
            if(major != 0){
                search_device(major);
                sprintf(ttys,"%s/%d", tty_buffer, minor);
            }
            else{
                strcpy(ttys,"-");
            }
            
            if(terminal_flag==0 && user_flag==0){
                if(strstr(tty_name, ttys)==NULL)
                    continue;
                if(userid != uid)
                    continue;
            }
            if(user_flag==1 && terminal_flag==0){
                if(strstr(ttys,"pts")==NULL && strstr(ttys,"tty")==NULL)
                    continue;
            }
            if(user_flag==0 && terminal_flag==1){
                if(userid != uid)
                    continue;
            }


            temp.pid = pid;
            temp.ppid = ppid; 
            temp.uid = uid;
            temp.gid = gid; 
            temp.pgid = pgid; 
            temp.sid = sid;
            temp.state = state;
            strcpy(temp.img, img);
            strcpy(temp.cmdline, cmdline);
            strcpy(temp.ttys, ttys);
            temp.next = NULL;

            insertNode(temp);
        }
    }
    sort(sort_type);
    printList();
    return 0;
}


void usage(){
    puts("Usage:\n ps_command [option]\n");

    puts(" -a: list processes from all users");
    puts(" -x: list processes without an associated terminal");

    puts(" -p: sort the listed processes by pid (default)");
    puts(" -q: sort the listed processes by ppid");
    puts(" -r: sort the listed processes by pgid");
    puts(" -s: sort the listed processes by sid");

}


int main(int argc, char **argv){
    int opt;
    int user_flag=0, terminal_flag=0;
    int sort_type = 0;
    int sortf = 0;

    while ((opt = getopt(argc, argv, "axpqrsh")) != -1){
        switch(opt){
            case 'a':
                user_flag = 1;
                break;
            case 'x':
                terminal_flag = 1;
                break;
            case 'p':
                if(sortf){
                    usage();
                    exit(-1);
                }
                else{
                    sort_type = 0;
                    sortf = 1;
                }
                break;
            case 'q':
                if(sortf){
                    usage();
                    exit(-1);
                }
                else{
                    sort_type = 1;
                    sortf = 1;
                }
                break;
            case 'r':
                if(sortf){
                    usage();
                    exit(-1);
                }
                else{
                    sort_type = 2;
                    sortf = 1;
                }
                break;
            case 's':
                if(sortf){
                    usage();
                    exit(-1);
                }
                else{
                    sort_type = 3;
                    sortf = 1;
                }
                break;
            case 'h':
                usage();
                exit(0);
            default:
                usage();
                exit(-1);
                break;
        }
    }
    
    ps(user_flag, terminal_flag, sort_type);
    return 0;

}


