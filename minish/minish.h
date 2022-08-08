#ifndef __MINISH_H__
#define __MINISH_H__

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>

#include "list.h"


/* Shell parameters */
static int var_int[] = { 80, 2, 10 };
static bool var_bool[] = { false, false };

/* Set command */
void set_sig(void);
bool set(char** arg);
bool set_vbool(char* op);
bool set_vint(char** arg);

/* Print */
bool print_help(char* cmd);
void print_args(list* args);
void print_arg(list* args);
void print_value(void);

/* Get input */
char* get_input(void);

/* Modify input into array */
list* handle_input(char* src);
char* pre_handle(char* src);
void update_input(list* dst, char* src);
char* find_operator(char* src, int* pos);
list* split_cmd(list* args, char* src);
list* split_arg(list* args, char* src, const char* op);

/* Invoke command */
bool invoke(list* args);
bool invoke_history(list* args);
bool invoke_builtin(list* args);

/* Execute command */
bool exec_cmd(list* args, int* fd);
bool exec_conc(list* args, int flag, int pid);
void _exec(char* cmd, char** params);

/* History list variables */
static list* history = NULL;
static int hist_cnt = 1;

/* History command */
bool hist_print(char* cmd);
void hist_record(char* cmd);
bool exec_mrecent(list* args);
bool exec_recent(list* args);

/* Change directory */
bool cd(list* args);

/* Get path from directory */
char* get_path(char* path_in);
char* get_env(char* env);
char* get_absol(char* path_in);
char* get_curr(char* path_in);

/* Checkings before invoking execvp */
int chk_type(list* args);
bool chk_stat(int status, int flag);

/* Pipe functions */
int* build_pipe(int* fd, int flag);
void conn_pipe(int* fd, int* fd2, int cnt);
void close_pipe(int* fd);

/* Redirection */
bool redirection(list* path, int* flag);

/* Clean */
void clear_history(void);
void clear_arg(char** arg);
void clear_args(list* args);
void clean_up(list* args, char* str);

#endif /* __MINISH_H__ */
