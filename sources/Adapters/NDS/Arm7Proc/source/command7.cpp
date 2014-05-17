/*
  Functions for the ARM7 to process the commands from the ARM9.Based
  on code from the MOD player example posted to the GBADEV forums.
  Chris Double (chris.double@double.co.nz)
  http://www.double.co.nz/nintendo_ds
*/
#include <nds.h>
#include "../../Command/command.h"


void SendCommandToArm9(u32 command)
{
   if (!(REG_IPC_FIFO_CR & IPC_FIFO_SEND_FULL)) {
      REG_IPC_FIFO_TX =command;
   } ;
}

Command *gCallbackCommand ;

void timerHandler() {    
   int CallbackCommand=ENGINE_CALLBACK ;
   SendCommandToArm9(CallbackCommand) ;      
} ;


void CommandStartEngine(StartEngineCommand* cmd)
{
     
u32 old_reg = readPowerManagement(PM_CONTROL_REG);
writePowerManagement(PM_CONTROL_REG, (old_reg&(~PM_LED_BLINK)) | PM_LED_BLINK); // blink + fast

     // Setup FIFO
     
   REG_IPC_FIFO_CR =  IPC_FIFO_ENABLE | IPC_FIFO_SEND_CLEAR | IPC_FIFO_RECV_IRQ ;

     
// LEFT

	SCHANNEL_CR(0) = 0;
	SCHANNEL_TIMER(0) = SOUND_FREQ(32768);
	SCHANNEL_SOURCE(0) = (u32)cmd->left ;
	SCHANNEL_LENGTH(0) = cmd->length>>2 ;
	SCHANNEL_REPEAT_POINT(0) = 0;
   	SCHANNEL_CR(0) =
	    SCHANNEL_ENABLE |
	    SOUND_REPEAT |
	    SOUND_16BIT |
	    SOUND_PAN(0) |
	    SOUND_VOL(127);

// RIGHT

	SCHANNEL_CR(1) = 0;
	SCHANNEL_TIMER(1) = SOUND_FREQ(32768);
	SCHANNEL_SOURCE(1) = (u32)cmd->right ;
	SCHANNEL_LENGTH(1) = cmd->length>>2 ;
	SCHANNEL_REPEAT_POINT(1) = 0;
   	SCHANNEL_CR(1) =
	    SCHANNEL_ENABLE |
	    SOUND_REPEAT |
	    SOUND_16BIT |
	    SOUND_PAN(127) |
	    SOUND_VOL(127);

    // Setup a timer at the same rate as the sound. The sound DAC works at 32768 which is (interestingly)
    // just the base freq of a timer with DIV_1024.
    // We kick the timer to work at a frequency equivalent to a buffer in the ring
 
    TIMER1_DATA =65536-RING_SAMPLE_SIZE;
    TIMER1_CR = TIMER_ENABLE | TIMER_DIV_1024 | TIMER_IRQ_REQ ;
    
    // Prebuffer
    
	timerHandler(); 

    irqSet(IRQ_TIMER1, timerHandler);
    irqEnable(IRQ_TIMER1);   


}

void CommandStopEngine(StopEngineCommand* cmd)
{  
u32 old_reg = readPowerManagement(PM_CONTROL_REG);
writePowerManagement(PM_CONTROL_REG, (old_reg&(~PM_LED_BLINK))); // no blinking
          
   TIMER1_CR=0 ;       
   irqDisable(IRQ_TIMER1);     
   SCHANNEL_CR(0)=0 ;
   SCHANNEL_CR(1)=0 ;
}

