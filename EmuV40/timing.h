#ifndef FAKE86_TIMING_H_INCLUDED
#define FAKE86_TIMING_H_INCLUDED

//uint64_t sampleticks, lastsampletick, ssourceticks, lastssourcetick, adlibticks, lastadlibtick, lastblastertick, gensamplerate;
extern uint64_t gensamplerate;
extern uint64_t hostfreq;
extern uint64_t sampleticks;
extern uint64_t tickgap;
extern uint64_t lasttick;

extern void timing(void);
extern void inittiming(void);

#endif