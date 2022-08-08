#include "minish.h"


int main() {
    bool should_run = true;
    
    char* buffer;
    list* arg_list;
    
    set_sig();

    while (should_run) {
        printf("minish> ");
        fflush(stdout);
        
        buffer = get_input();
        arg_list = handle_input(buffer);
        should_run = invoke(arg_list);
        hist_record(buffer);
        clean_up(arg_list, buffer);
    }
    
    clear_history();
    
    return 0;
}
