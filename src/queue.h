#ifndef QUEUE_H
#define QUEUE_H

#include "structs.h"

void initialize_queue();
void set_client_ready(client_t* client);
void cleanup_queue();

#endif
