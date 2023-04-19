//   ____                                      
//  |  _ \ _ __ ___   __ _ _ __ __ _ _ __ ___  
//  | |_) | '__/ _ \ / _` | '__/ _` | '_ ` _ \
//  |  __/| | | (_) | (_| | | | (_| | | | | | |
//  |_|   |_|  \___/ \__, |_|  \__,_|_| |_| |_|
//                   |___/                    
// Program object
// intended to implement a linked-list of blocks

#ifndef PROGRAM_H
#define PROGRAM_H

#include "defines.h"
#include "block.h"
#include "machine.h"

//   _____                      
//  |_   _|   _ _ __   ___  ___ 
//    | || | | | '_ \ / _ \/ __|
//    | || |_| | |_) |  __/\__ \
//    |_| \__, | .__/ \___||___/
//        |___/|_|              

// Opaque structure:
typedef struct program program_t;



//   _____                 _   _                 
//  |  ___|   _ _ __   ___| |_(_) ___  _ __  ___ 
//  | |_ | | | | '_ \ / __| __| |/ _ \| '_ \/ __|
//  |  _|| |_| | | | | (__| |_| | (_) | | | \__ \
//  |_|   \__,_|_| |_|\___|\__|_|\___/|_| |_|___/
                                              
// Lifecycle functions =========================================================
program_t *program_new(char const *filename);
void program_free(program_t *program);
void program_print(program_t *program, FILE *output);


// Accessors ===================================================================
size_t program_length(program_t const *p);
block_t *program_current(program_t const *p);
block_t *program_first(program_t const *p);
block_t *program_last(program_t const *p);
char *program_filename(program_t const *p);


// Processing ==================================================================
int program_parse(program_t *program, machine_t *machine);
block_t *program_next(program_t *program);
void program_reset(program_t *program);



#endif // PROGRAM_H

