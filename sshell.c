#include<fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include<sys/wait.h>
#include<sys/stat.h>
#include <unistd.h>

#define CMDLINE_MAX 512
#define ARG_MAX 16
#define TOKEN_MAX 32
#define STRTOKSIZE 2
#define DELIM_O "< >"
#define DELIM_PIPE "|"
#define BUFFER_CAPACITY 256
typedef struct _Command COMMAND;
typedef struct _Command_Collection CMDCOLLECTION;
struct _Command{
    size_t argc;
    char *argvs[ARG_MAX];
    int whether_redirect;
    char redir_sign[2];
    char * redirection;
};
//this struction is for executing the pipeline
struct _Command_Collection{
    //size_t argc;
    size_t num_of_commands;
    char ** commands;
};

/*Stack by partner*/
typedef struct node node;
struct node 
{
    void *data_p;
    node *next;
};

typedef struct Stack Stack;
struct Stack
{
    node *head;
    size_t size;
};


Stack *initializeStack()
{
    Stack *stack_p = malloc(sizeof(Stack));
    node *node_p = malloc(sizeof(node));

    stack_p->head = node_p;
    stack_p->size = 0;

    return stack_p;
}


void printContents(Stack *stack)
{
    int counter = stack->size;
    node *ptr = stack->head;

    while (counter--)
    {
        printf("%s\n", (char *)ptr->data_p);
        ptr = ptr->next;
    }
}


void push(Stack *stack, void *data)
{
    if (!stack->size) 
    {
        stack->head->data_p = data;
        stack->size++;
    }
    else 
    {
        node *node_p = malloc(sizeof(node));
        node_p->data_p = data;
        node_p->next = stack->head;
        stack->head = node_p;
        stack->size++;
    }
}


void *pop(Stack *stack)
{
    node *node_p = stack->head;
    void *return_data = stack->head->data_p;

    stack->head = node_p->next;
    stack->size--;

    free(node_p);

    return return_data;
}


size_t getStackSize(Stack *stack)
{
    return stack->size;
}


void clearStack(Stack *stack)
{
    int counter = stack->size;
    node *head_ptr = stack->head;
    node *tmp;

    while (counter--)
    {
        tmp = head_ptr;
        head_ptr = head_ptr->next;
        free(tmp);
    }
    free(stack);
}

void parse_command(char * cmd);
void execute_cmd(char * cmd);
int execute_single_cmd(char * cmd);
void parse_single_command(char * cmd, COMMAND * cmd_coll);
void pipeline(int * return_ptr1, int * return_ptr2, char * cmd1, char * cmd2);

//stack code
char dir_buffer[BUFFER_CAPACITY];
Stack *dirStack;
int main(void)
{
        char cmd[CMDLINE_MAX];
        dirStack=initializeStack();
        push(dirStack, getcwd(dir_buffer, sizeof(dir_buffer)));
        while (1) {
                char *nl;
                //int retval;

                /* Print prompt */
                printf("sshell$ ");
                fflush(stdout);

                /* Get command line */
                fgets(cmd, CMDLINE_MAX, stdin);

                /* Print command line if stdin is not provided by terminal */
                if (!isatty(STDIN_FILENO)) {
                        printf("%s", cmd);
                        fflush(stdout);
                }


                /* Remove trailing newline from command line */
                nl = strchr(cmd, '\n');
                if (nl)
                        *nl = '\0';

                /* Builtin command */
                if (!strcmp(cmd, "exit")) {
                        fprintf(stderr, "+ complete 'exit' [0]\n");
                        break;
                }
                
                
                if(strlen(cmd)==0){
                    continue;
                }
                /*program start here*/
                execute_cmd(cmd);
                /* Regular command */
                //retval = system(cmd);
                //fprintf(stdout, "Return status value for '%s': %d\n",
                //        cmd, retval);
                
        }

        return EXIT_SUCCESS;
}
void execute_cmd(char * cmd){
    char original_cmd[CMDLINE_MAX];
    strcpy(original_cmd, cmd);
    if(cmd[0]==DELIM_PIPE[0]){
        fprintf(stderr, "missing command\n");
        return ;
    }
    if(cmd[strlen(cmd)-1]==DELIM_PIPE[0]){
        fprintf(stderr, "missing command\n");
        return ;
    }
    CMDCOLLECTION * total_coll=malloc(sizeof(CMDCOLLECTION));
    total_coll->num_of_commands=0;
    total_coll->commands=malloc(sizeof(char*));
    char * token=strtok(cmd, DELIM_PIPE);
    //printf("%s\n", token);
    while(token!=NULL){
        //printf("%s\n", token);
        if(total_coll->num_of_commands!=0){
            total_coll->commands=realloc(total_coll->commands, (total_coll->num_of_commands+1)*sizeof(char*));
        }
        total_coll->commands[total_coll->num_of_commands]=malloc(sizeof(char)*(strlen(token)+1));
        strcpy(total_coll->commands[total_coll->num_of_commands++], token);
        token=strtok(NULL, DELIM_PIPE);
    }
    int * return_val=malloc(sizeof(int)*total_coll->num_of_commands);
    if(total_coll->num_of_commands==1){
        return_val[0]=execute_single_cmd(cmd);
        
    }
    else{
        int first_val;
        int second_val;
        for(size_t i=0;i<total_coll->num_of_commands-1;i++){
            pipeline(&first_val, &second_val, total_coll->commands[i], total_coll->commands[i+1]);
            return_val[i]=first_val;
            return_val[i+1]=second_val;
        }
    }
    printf("+ complete '%s' ", original_cmd);
    for(size_t i=0;i<total_coll->num_of_commands;i++){
        printf("[%d]", return_val[i]);
    }
    printf("\n");
}
void pipeline(int * return_ptr1, int * return_ptr2, char * cmd1, char * cmd2){
    int fd[2];
    pipe(fd);
    if(fork()!=0){
        close(fd[0]);
        dup2(fd[1], STDOUT_FILENO);
        *return_ptr1=execute_single_cmd(cmd1);
    }
    else{
        close(fd[1]);
        dup2(fd[0], STDIN_FILENO);
        close(fd[0]);
        *return_ptr2=execute_single_cmd(cmd2);
    }
}
char * find_root_user(char * dir){
    int root_mark_count=0;
    int end_idx=strlen(dir);
    for(size_t i=0;i<strlen(dir);i++){
        if(dir[i]=='/'){
            root_mark_count++;
        }
        if(root_mark_count==3){
            end_idx=i;
            break;
        }
    }
    char * ret=malloc(sizeof(char)*(end_idx+1));
    for(int i=0;i<end_idx;i++){
        ret[i]=dir[i];
    }
    return ret;
}
int execute_single_cmd(char * cmd){
    //char argvs[ARG_MAX][TOKEN_MAX];

    int status;
    COMMAND * cmd_coll=malloc(sizeof(COMMAND));
    cmd_coll->argc=0;
    cmd_coll->whether_redirect=0;
    cmd_coll->redirection=NULL;
    parse_single_command(cmd, cmd_coll);
    
    if(cmd_coll->argc>ARG_MAX){
        fprintf(stderr, "Error: too many process arguments\n");
        return 2;
    }
    if(cmd_coll->argc==0){
        fprintf(stderr, "missing command\n");
        return 2;
    }
    //printf("%s", cmd_coll->redirection);
    if(cmd_coll->whether_redirect&&cmd_coll->redirection==NULL){
        fprintf(stderr, "no output file\n");
        return 2;
    }
    if(!strcmp(cmd_coll->argvs[0], "pwd")){
        getcwd(dir_buffer, sizeof(dir_buffer));
        printf("%s\n", dir_buffer);
        return 0;
    }
    if(!strcmp(cmd_coll->argvs[0], "cd")){
        if(cmd_coll->argc>2){
                fprintf(stderr, "cd: too many arguments\n");
                return 2;
            }
        else if(cmd_coll->argc==1){
            getcwd(dir_buffer, sizeof(dir_buffer));
            char * target_dir=find_root_user(dir_buffer);
            chdir(target_dir);
            while(dirStack->size>2){
                pop(dirStack);
                //dirStack->size--;
            }
        }
        else{
            char *cd;
            char *arg;

            if (!strcmp(cmd_coll->argvs[1], ".."))
            {
                arg = pop(dirStack);
                chdir(arg);
            }
            else
            {
                cd = getcwd(dir_buffer, sizeof(dir_buffer));
                push(dirStack, cd);
                chdir(cmd_coll->argvs[1]);
            }
        }
        return 0;
    }
    if (!fork()) {
        int fd;
        if(!strcmp(cmd_coll->redir_sign, ">")){
            fd=open(cmd_coll->redirection, O_WRONLY| O_CREAT, 0644);
            dup2(fd, STDOUT_FILENO);
        }
        else {
            fd=open(cmd_coll->redirection, O_RDONLY, 0);
            dup2(fd, STDIN_FILENO);
        }
        close(fd);
        execvp(cmd_coll->argvs[0], cmd_coll->argvs);
        
        
        perror("execv");
        return 1;
        //exit(1);
    } else {
        // Parent 
        waitpid(-1, &status, 0);
        return 0;
    }
    return 0;
}
char * parse_redirection(int index, char * cmd){
    if(index==(int)strlen(cmd)-1)return NULL;
    int end_idx=strlen(cmd);
    int start_idx=index;
    int start=0;
    for(size_t i=index+1;i<strlen(cmd);i++){
        if(cmd[i]!=' '&&!start){
            start=1;
            start_idx=i;
        }
        if(cmd[i]==DELIM_O[0]||cmd[i]==DELIM_O[2]){
            end_idx=i;
            break;
        }
    }
    char * ret=malloc(sizeof(char)*(end_idx-start_idx+1));
    for(int i=start_idx;i<end_idx;i++){
        ret[i-start_idx]=cmd[i];
    }
    return ret;
}

void parse_single_command(char * cmd, COMMAND * cmd_coll){
    size_t space_idx=0;
    int whether_space=1;
    //cmd_coll->redir=malloc(sizeof(char)*STRTOKSIZE);
    for(size_t i=0;i<strlen(cmd);i++){
        if(cmd[i]==DELIM_O[0]||cmd[i]==DELIM_O[2]){
            cmd_coll->whether_redirect=1;
            if(i==0){
                //strcpy(cmd, "");
                return;
            }
            cmd_coll->redirection=parse_redirection(i, cmd);
            if(cmd[i]==DELIM_O[0]){
                cmd_coll->redir_sign[0]=DELIM_O[0];
            }
            else{
                cmd_coll->redir_sign[0]=DELIM_O[2];
            }
            cmd=strtok(cmd, cmd_coll->redir_sign);
            //char * token=strtok(cmd, cmd_coll->redir_sign);
            //printf("%s\n", token);
            //strcpy(cmd, token);
            break;
        }
    }
    //printf("%d\n",(int) strlen(cmd));
    //if(strlen(cmd)==0)return;
    //printf("%s\n", cmd);
    for(size_t i=0;i<strlen(cmd);i++){
        if(cmd[i]==' '&&!whether_space){
            whether_space=1;
            
            cmd_coll->argvs[cmd_coll->argc]=malloc(sizeof(char)*(i-space_idx+1));

            
            for(size_t j=space_idx;j<i;j++){
                cmd_coll->argvs[cmd_coll->argc][j-space_idx]=cmd[j];    
            }
            cmd_coll->argc++;
        }
        if(cmd[i]!=' '&&whether_space){
            whether_space=0;
            space_idx=i;
        }
        //if(cmd)
    }
    if(cmd[strlen(cmd)-1]!=' '){
        
            cmd_coll->argvs[cmd_coll->argc]=malloc(sizeof(char)*(strlen(cmd)-space_idx+1));
        

        for(size_t j=space_idx;j<strlen(cmd);j++){
            
            cmd_coll->argvs[cmd_coll->argc][j-space_idx]=cmd[j];
            
        }
        cmd_coll->argc++;
    }
}