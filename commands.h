#ifndef COMMANDS_H
#define COMMANDS_H

#include "dlist.h"
#include "server.h"

void handle_command(char* buf, int len, cli_ctx_t* ctx_list);

#endif /*COMMANDS_H*/