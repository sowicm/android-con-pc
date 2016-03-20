
#ifndef common_h
#define common_h

#define UseSendInput 1


#define LittlePath      "http://my.zi-jin.com/little/"

#define IGNORED 0

extern bool bWorkThreadRun;
extern bool bScreenThreadRun;
extern SOCKET sck;
extern SOCKET sckScreen;
extern int nWorkThreadWorkClock;

void workthreadStart(void *lpThreadParameter);
void supervisethreadStart(void *lpThreadParameter);
void screenthreadStart(void *lpThreadParameter);


#endif