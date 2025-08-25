#ifndef FAKE86_PARSECL_H_INCLUDED
#define FAKE86_PARSECL_H_INCLUDED

#include "config.h"
#include <SDL.h>

extern uint16_t constanth;
extern uint16_t constantw;
extern uint8_t slowsystem;
extern char* biosfile;
extern uint32_t speed;
extern uint8_t verbose;
extern uint8_t useconsole;
extern uint8_t usessource;

extern uint8_t dohardreset;

extern void parsecl(int argc, char* argv[]);
extern uint32_t loadrom(uint32_t addr32, const char* filename, uint8_t failure_fatal);

#endif
