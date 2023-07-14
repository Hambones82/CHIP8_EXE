#ifndef TIMER_H
#define TIMER_H

#define MAX_TIMER_CALLBACKS 20

typedef void (* far TimerCallback)(void);

void startTimer(void);
void endTimer(void);
void setTimer(unsigned char val);
unsigned char getTimer(void);
void setSoundTimer(unsigned char val);
unsigned char getSoundTimer(void);
unsigned char getDrawHalt(void);
void setDrawHalt(void);
void RegisterTimerCallback(TimerCallback cb);

#endif
