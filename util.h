
#define TRUE 1
#define FALSE !TRUE
void signal_child(int p);
void signal_int(int p);
int changeDir(char * args[]);
int promptNum;
static pid_t G_PID;
static pid_t G_PGID;
static int G_INTERACTIVE;
static struct termios G_TMODES;
static char* currentDir;
extern char** environ;
struct sigaction act_child;
struct sigaction act_int;
pid_t pid;

