#if !defined(COMMAND_H)
#define COMMAND_H

 
#define RING_SAMPLE_SIZE 1024
#define RING_BUFFER_COUNT 8

/*
  Structures and functions to allow the ARM9 to send commands to the
  ARM7. Based on code from the MOD player example posted to the GBADEV
  forums.
*/


/* Enumeration of commands that the ARM9 can send to the ARM7 */
enum CommandType {
  ENGINE_START=10,
  ENGINE_STOP,
  ENGINE_CALLBACK
};

/* Command parameters for playing a sound sample */
struct StartEngineCommand
{
       unsigned short *left ;
       unsigned short *right ;
       int length ;
};

/* Command parameters for starting to record from the microphone */
struct StopEngineCommand
{
       bool dummy ;
};

struct EngineCallback {
       bool dummy ;
} ;

/* The ARM9 fills out values in this structure to tell the ARM7 what
   to do. */

struct Command {
  CommandType commandType;
  union {
    void* data;
    StartEngineCommand startCommand ;
    StopEngineCommand stopCommand ;
    EngineCallback callbackCommand ;
  };
};

#if defined(ARM7)
void CommandStartEngine(StartEngineCommand *cmd) ;
void CommandStopEngine(StopEngineCommand *cmd);
#endif

#if defined(ARM9)
void CommandEngineCallback() ;
#endif

#endif
