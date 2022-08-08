#include <stdlib.h>


const char* option[] = {
    "help",
    "history",
    "!!",
    "!",
    ";",
    "|",
    "&",
    "&&",
    "||",
    ">",
    "<",
    ">>",
    "set",
    "cd",
    NULL
};

const char* description[] = {
    "--------------  ---------------------------------------------\n",
    "\toption\tdescription\n",
    "Print help",
    "Print history",
    "Execute most recent command in history",
    "Execute n'th command in history",
    "Execute command continuously",
    "Pipe commands",
    "Execute command concurrously",
    "Execute next command on succes",
    "Stop on success",
    "Redirect file as output",
    "Redirect file as input",
    "Redirect file as output(Append)",
    "Set variables('-o' to show all)",
    "Change directory",
    "",
    NULL
};

const char* int_str[] = {
    "line_length",
    "divide_size",
    "history_size",
    NULL
};

const char* bool_str[] = {
    "verbose",
    "verbose_s",
    NULL
};

const char* operator[] = {
    ">>",
    ">",
    "<",
    "||",
    "&&",
    ";",
    "|",
    "&",
    NULL
};

const int operator_val[] = {
    0x82,
    0x80,
    0x81,
    0x30,
    0x20,
    0x50,
    0x10,
    0x40,
    0x00
};

const char* set_op[] = {
    "-v",
    "-vs",
    NULL
};

const char* unset_op[] = {
    "+v",
    "+vs",
    NULL
};

