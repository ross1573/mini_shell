#include "minish.h"

extern const char* option[];
extern const char* description[];
extern const char* int_str[];
extern const char* bool_str[];
extern const char* operator[];
extern const int operator_val[];
extern const char* set_op[];
extern const char* unset_op[];


// ============================================================================================================================================ //

bool invoke(list* args) {
    if (!args) return true;                                 // avoid null ptr
    if (!args->tag) return true;                            // avoid null input
    char** arg = args->data;
    
    if (var_bool[0]) print_args(args);                      // print arg(node) on verbose mode
    if (var_bool[1]) print_arg(args);                       // print arg on verbose simple mode
    if (!strcmp(arg[0], "exit")) return false;              // exit
    
    if (invoke_history(args)) return true;                  // invoke history functions
    if (invoke_builtin(args)) return true;                  // invoke builtin funcions
    
    return exec_cmd(args, NULL);                            // execute command, none exec command should be run before this
}

bool invoke_history(list* args) {
    if (exec_mrecent(args)) return true;                    // execute most recent command
    if (exec_recent(args))  return true;                    // execute recent command
    
    return false;
}

bool invoke_builtin(list* args) {
    if (!args) return true;
    if (!args->tag) return true;
        
    if (cd(args))        return true;                       // change directory
    if (set(args->data)) return true;                       // set variables
    
    return false;
}

// ============================================================================================================================================ //

bool exec_cmd(list* args, int* fd) {
    if (!args) return true;
    if (!args->tag) return true;
    char** arg = args->data;
    int fd2[2];
    int c_status;
        
    int flag = chk_type(args);                              // get flag
    int* fd_ptr = build_pipe(fd2, flag);                    // build pipe
    
    pid_t pid = fork();
    switch (pid) {
        case -1:
            perror("fork");
            return true;                                    // exit on fork fail
        case  0:
            conn_pipe(fd, fd2, flag);                       // connect pipe
            if (redirection(args->next, &flag))             // open file for redirection
                exit(EXIT_FAILURE);
            _exec(arg[0], arg);                             // invoke execvp
            exit(EXIT_FAILURE);
        default:
            close_pipe(fd);                                 // close pipe
            if ((flag >> 4) & 0x8) {
                args = args->next;                          // on redirection, next arg is file path
                flag = (flag & 0x8F) ^ flag;                // remove redirection bit
            }
            if (exec_conc(args, flag, pid)) return true;
            waitpid(pid, &c_status, 0);
            if (chk_stat(c_status, flag)) return true;      // check child status
            return exec_cmd(args->next, fd_ptr);            // run command recursively
    }
}

bool exec_conc(list* args, int flag, int pid) {
    if (flag == 0x40) {
        if (!args) return true;
        if (exec_cmd(args->next, NULL)) {
            waitpid(pid, NULL, 0);
            return true;
        }
        waitpid(pid, NULL, 0);
        return true;
    }
    return false;
}

void _exec(char* cmd, char** param) {
    signal(SIGINT, SIG_DFL);                                // child process should not ignore signal
    signal(SIGQUIT, SIG_DFL);
    signal(SIGSTOP, SIG_DFL);
    
    if (hist_print(cmd)) exit(0);                           // print history
    if (print_help(cmd)) exit(0);                           // print help
    
    execvp(cmd, param);
    switch (errno) {
        case ENOENT:
            printf("Unknown command %s\n", cmd);
            break;
        default:
            perror("execvp");
            break;
    }
    exit(EXIT_FAILURE);                                     // notice parent on failure;
}

// ============================================================================================================================================ //

char* get_input() {
    char* buf = malloc(var_int[0]);
    fgets(buf, var_int[0], stdin);
    fflush(stdin);
    buf[var_int[0]-1] = '\0';
    return buf;
}

list* handle_input(char* src) {
    char* mod = pre_handle(src);
    if (src[0] == '\0') return NULL;
    if (mod) src = mod;
    char buf[strlen(src)];
    strcpy(buf, src);
    free(mod);
    
    return split_cmd(NULL, buf);                            // split command
}

char* pre_handle(char* src) {
    long len = strlen(src)-1;
    if (src[len] == '\n') src[len] = '\0';                  // remove newline
    
    char* ptr = strchr(src, '~');
    if (ptr) {
        char* path = get_env("HOME");
        len = strlen(src)+strlen(path);
        char* buf = malloc(len);
        strncpy(buf, src, ptr-src);
        strcat(buf, path);
        strcat(buf, ptr+1);
        buf[len-1] = '\0';
        
        return buf;
    }
    
    ptr = strchr(src, '$');
    if (ptr) {
        char* ptr_e = ptr;
        for (; *ptr_e != '\0'; ptr_e++) {}
        len = ptr_e-ptr-1;
        char str[len+1];
        strcpy(str, ptr+1);
        
        char* env = get_env(str);
        if (!env) return NULL;
        len = strlen(src)+strlen(env)-len;
        char* buf = malloc(len+1);
        strncpy(buf, src, ptr-src);
        strcat(buf, env);
        strcat(buf, ptr_e);
        buf[len] = '\0';
        
        return buf;
    }
    return NULL;
}

list* split_cmd(list* args, char* src) {
    if (!src) return args;
    int pos;
    char* ptr = find_operator(src, &pos);                   // search for operators
    if (ptr) {
        ptr = strtok(src, operator[pos]);                   // split by operator
        long len = strlen(ptr)+strlen(operator[pos]);
        args = split_arg(args, ptr, operator[pos]);         // split arguments
        ptr = src+len;
        return split_cmd(args, ptr);                        // recursive call
    }
    
    return split_arg(args, src, NULL);
}

list* split_arg(list* args, char* src, const char* op) {
    char* cmd[strlen(src)/var_int[1]+1];                    // temporary buffer
    char* str = strtok(src, " ");                           // split by " "
    
    for (int i = 0; ; i++) {
        if (!str) {
            if (op) {
                cmd[i] = malloc(strlen(op));                // add operator
                strcpy(cmd[i], op);
                i++;
            }
            
            long len = sizeof(char*)*(i+1);
            char** ptr = malloc(len);
            for (int j = 0; j < i; j++) {
                ptr[j] = cmd[j];
            }
            ptr[i] = NULL;
            return insert_end(ptr, i, args);                // store command
        }

        cmd[i] = malloc(strlen(str));                       // store argument
        strcpy(cmd[i], str);
        str = strtok(NULL, " ");
    }
    
    return NULL;
}

void update_input(list* dst, char* src) {
    if (!dst) return;
    clear_args(dst->next);
    clear_arg(dst->data);
    dst->next = NULL;
    dst->data = NULL;
    
    list* args = handle_input(src);                         // handle new input
    dst->data = args->data;                                 // connect input on old ptr
    dst->tag = args->tag;
    dst->next = args->next;
    free(args);                                             // delete temporary node
}

char* find_operator(char* src, int* pos) {
    char* end = src+strlen(src);
    char* first = end;
    *pos = -1;
    int i = 0;
    
    for ( ; operator[i]; i++) {
        char* ptr = strstr(src, operator[i]);
        if (ptr && (first > ptr)) {                         // find operator that comes first
            first = ptr;
            *pos = i;
        }
    }
    
    if (*pos == -1) *pos = i;                               // if none operator is found, op == NULL
    if (first >= end) return NULL;                          // return NULL when ptr is out of range
    return first;
}

// ============================================================================================================================================ //

int chk_type(list* args) {
    char** arg = args->data;
    char* str = arg[args->tag-1];
    int flag = 0;
    
    for (int i = 0; operator[i]; i++) {                     // check every operator
        if (!strcmp(str, operator[i])) {
            flag = operator_val[i];
            break;
        }
    }
    
    if (flag) {                                             // delete operator arg
        free(str);
        str = NULL;
        args->tag--;
        arg[args->tag] = NULL;
        
        if ((flag >> 4) & 0x8) {
            flag |= chk_type(args->next);
        }
    }
    
    return flag;
}

bool chk_stat(int status, int flag) {
    int stat = WEXITSTATUS(status);                         // get exit status
    if      ((flag == 0x20) &&  stat) return true;          // && operator
    else if ((flag == 0x30) && !stat) return true;          // || operator
    return false;
}

int* build_pipe(int* fd, int flag) {
    if (flag == 0x10) {                                     // | operator
        if (pipe(fd) < 0) {                                 // make pipe
            perror("pipe");
        }
        return fd;
    }
    return NULL;
}

void conn_pipe(int* fd, int* fd2, int flag) {
    if (fd) {
        dup2(fd[0], STDIN_FILENO);                          // stdin for old pipe out
        close(fd[0]);
        close(fd[1]);
    }
    if (flag == 0x10) {
        close(fd2[0]);
        dup2(fd2[1], STDOUT_FILENO);                        // stdout for new pipe in
        close(fd2[1]);
    }
}

void close_pipe(int* fd) {
    if (fd) {
        close(fd[0]);
        close(fd[1]);
    }
}

bool redirection(list* path, int* flag_ptr) {
    int flag = *flag_ptr;
    if (!((flag >> 4) & 0x8)) return false;                 // redirection operator 0xE*
    
    char* str = ((char**)path->data)[0];
    char* full_path = get_path(str);
    if (!full_path) return true;
    
    int op, io;
    int fd;
    
    flag &= 0x8F;
    *flag_ptr = *flag_ptr ^ flag;
    switch (flag) {
        case 0x80:                                          // > operator
            op = O_RDWR | O_CREAT;
            io = STDOUT_FILENO;
            break;
        case 0x82:                                          // >> operator
            op = O_RDWR | O_APPEND;
            io = STDOUT_FILENO;
            break;
        case 0x81:                                          // < operator
            op = O_RDONLY;
            io = STDIN_FILENO;
            break;
        default: op = io = -1;
    }
    
    fd = open(full_path, op, 0644);
    if (fd < 0) {
        perror("open");
        return true;
    }
    dup2(fd, io);
    close(fd);
    free(full_path);
    
    return false;
}

// ============================================================================================================================================ //

bool hist_print(char* cmd) {
    if (strcmp(cmd, "history")) return false;
    list* node = history;
    for (; node; node = node->next) {
        printf("%d %s\n", node->tag, (char*)node->data);
    }
    return true;
}

void hist_record(char* cmd) {
    if (cmd[0] == '\0') return;
    char* dst = malloc(strlen(cmd));                        // copy command
    strcpy(dst, cmd);
    history = insert(dst, hist_cnt, history);
    hist_cnt++;
    if (count(history) > var_int[2]) {                      // History size is adjustable
        rm_end(history);                                    // older history is deleted
    }
}

bool exec_mrecent(list* args) {
    char** arg = args->data;
    if (strcmp(arg[0], "!!")) return false;
    if (!history) {
        printf("No commands in history\n");
        return true;
    }

    update_input(args, history->data);                      // replace command to history data
    
    if (var_bool[0] || var_bool[1])                         // print new command in verbose mode
        printf("\t>>>\n");
    if (var_bool[0]) print_arg(args);
    if (var_bool[1]) print_args(args);
    return false;
}

bool exec_recent(list* args) {
    char** arg = args->data;
    if (arg[0][0] != '!') return false;

    int pos = args->tag;
    char* str = ((char**)(args->data))[pos-1];
    char num[strlen(str)-1];                                // get n from argument
    strcpy(num, str+1);
    int no = atoi(num);
    
    if (!no) {
        printf("Invalid input");
        return true;
    }
    list* node = find(no, history);                         // find n'th history
    if (!node) {
        printf("No command %d in history", no);
        return true;
    }
    
    update_input(args, node->data);                         // replace command to history data
    
    if (var_bool[0] || var_bool[1])                         // print new command in verbose mode
        printf("\t>>>\n");
    if (var_bool[0]) print_args(args);
    if (var_bool[1]) print_arg(args);
    return false;
}

// ============================================================================================================================================ //

bool cd(list* args) {
    char** arg = args->data;
    if (strcmp(arg[0], "cd")) {
        return false;
    }
    if (!arg[1]) {
        printf("Usage : cd [path]\n");
        return true;
    }
        
    char* path = get_path(arg[1]);
    if (!path) return true;
        
    if (chdir(path)) {
        perror("chdir");
        free(path);
        return true;
    }
    
    free(path);
    return true;
}

char* get_path(char* path_in) {
    switch (path_in[0]) {
//        case '~': return get_home(path_in+1);             // '~' is automatically changed to home dir
        case '/': return get_absol(path_in);
        default : return get_curr(path_in);
    }
    
    return NULL;
}

char* get_env(char* env) {
    char* buf = getenv(env);
    if (!buf) {
        perror("getenv");
        return NULL;
    }
    return buf;
}

char* get_absol(char* path_in) {
    long len;
    char* path;
    len = strlen(path_in);
    path = malloc(len+1);
    strcpy(path, path_in);
    return path;
}

char* get_curr(char* path_in) {
    long len;
    char* path;
    char* buf = getcwd(NULL, 0);
    if (!buf) {
        perror("getcwd");
        return NULL;
    }
    len = strlen(buf)+strlen(path_in);
    path = malloc(len+2);
    strcpy(path, buf);
    strcat(path, "/");
    strcat(path, path_in);
    path[len+1] = '\0';
    free(buf);
    return path;
}

// ============================================================================================================================================ //

void set_sig() {
    signal(SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    signal(SIGSTOP, SIG_IGN);
}

bool print_help(char* cmd) {
    if (strcmp(cmd, "help")) return false;
    printf("%s", description[0]);
    printf("%s", description[1]);
    printf("%s", description[0]);
    for (int i = 0; option[i]; i++) {
        printf("%14s\t%s\n", option[i], description[i+2]);
    }
    printf("%s", description[0]);
    return true;
}

bool set(char** arg) {
    if (strcmp(arg[0], "set")) return false;
    if (!arg[1]) {
        printf("Usage : set [option] [value]\n");
        return true;
    }
    
    if (set_vbool(arg[1])) {
        return true;
    }
    if (!set_vint(arg)) {
        printf("Invalid value\n");
    }
    return true;
}

bool set_vint(char** arg) {
    if (!arg[2]) return false;
    int val = atoi(arg[2]);
    if (!val) return false;
    
    if (!strcmp(arg[1], int_str[0])) {
        var_int[0] = val;
    }
    else if (!strcmp(arg[1], int_str[2])) {
        var_int[2] = val;
        int diff = count(history) - var_int[2];
        if (diff > 0) {
            for (; diff; diff--) {
                rm_end(history);
            }
        }
    }
    else if (!strcmp(arg[1], int_str[2])) {
        var_int[2] = val;
    }
    else {
        printf("Invalid option\n");
    }
    return true;
}

bool set_vbool(char* op) {
    if (op[0] == '-') {
        if (!strcmp(op, "-o")) {
            print_value();
            return true;
        }
        for (int i = 0; set_op[i]; i++) {
            if (!strcmp(op,  set_op[i])) {
                var_bool[i] = true;
                return true;
            }
        }
        printf("Invalid option");
        return true;
    }
    else if (op[0] == '+') {
        for (int i = 0; unset_op[i]; i++) {
            if (!strcmp(op,  unset_op[i])) {
                var_bool[i] = false;
                return true;;
            }
        }
        printf("Invalid option");
        return true;
    }
    return false;
}

void print_value() {
    printf("%s", description[0]);
    printf("%s", description[1]);
    printf("%s", description[0]);
    for (int i = 0;  int_str[i]; i++) printf("%14s\t%d\n",  int_str[i],  var_int[i]);
    for (int i = 0; bool_str[i]; i++) printf("%14s\t%d\n", bool_str[i], var_bool[i]);
    printf("%s", description[0]);
}

void print_args(list* args) {
    int cnt = 0;
    for (; args; args = args->next) {
        char** data = args->data;
        int i = 0;
        for (; data[i]; i++) {
            printf("node[%d]::arg[%d] : %s\n", cnt, i, data[i]);
        }
        cnt++;
    }
}

void print_arg(list* args) {
    int cnt = 0;
    for (; args; args = args->next) {
        char** data = args->data;
        int i = 0;
        for (; data[i]; i++) {
            printf("arg[%d] : %s\n", cnt, data[i]);
            cnt++;
        }
    }
}

// ============================================================================================================================================ //

void clean_up(list* args, char* str) {
    clear_args(args);
    free(str);
}

void clear_args(list* args) {
    if (!args) return;
    for (;args;) {
        clear_arg(args->data);
        args = rm_node(args);
    }
}

void clear_arg(char** arg) {
    if (!arg) return;
    for (int i = 0; arg[i]; i++) {
        free(arg[i]);
        arg[i] = NULL;
    }
    free(arg);
}

void clear_history() {
    clear_list(history);
}

// ============================================================================================================================================ //
