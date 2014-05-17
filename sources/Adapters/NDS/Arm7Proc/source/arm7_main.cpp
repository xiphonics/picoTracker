#include <nds.h>
#include "../../Command/command.h"

void FifoInterruptHandler() {
     
	Command *command =(Command *)REG_IPC_FIFO_RX;
	switch(command->commandType)
	{
	case ENGINE_START:
		CommandStartEngine(&command->startCommand);
		break;
	case ENGINE_STOP:
		CommandStopEngine(&command->stopCommand);
		break;
       
	}
} ;

// Everything we don't need from the VBlank handler is commented out, so the
// recording won't be interrupted.

void VblankHandler()
{
	static int heartbeat = 0;

	uint16 but=0, x=0, y=0, xpx=0, ypx=0, z1=0, z2=0, batt=0, aux=0;

	// Read the X/Y buttons and the /PENIRQ line
	but = REG_KEYXY;
	if (!(but & 0x40)) {
		// Read the touch screen
		touchPosition tempPos = touchReadXY();

		x = tempPos.x;
		y = tempPos.y;
		xpx = tempPos.px;
		ypx = tempPos.py;

	}

	// Update the IPC struct

	IPC->buttons   = but;
	IPC->touchX    = x;
	IPC->touchY    = y;
	IPC->touchXpx  = xpx;
	IPC->touchYpx  = ypx;
	IPC->touchZ1   = z1;
	IPC->touchZ2   = z2;
	IPC->battery   = batt;
	IPC->aux       = aux;

}


//////////////////////////////////////////////////////////////////////


int main(int argc, char **argv)
{
    REG_IPC_FIFO_CR = IPC_FIFO_ENABLE | IPC_FIFO_SEND_CLEAR ;
    
    u32 old_reg = readPowerManagement(PM_CONTROL_REG);
    writePowerManagement(PM_CONTROL_REG, old_reg |PM_LED_ON); // enable fast
    
	// Reset the clock if needed
	rtcReset();

	// Turn on Sound
	powerON(POWER_SOUND);

	// Set up sound defaults.
	SOUND_CR = SOUND_ENABLE | SOUND_VOL(0x7F);
	IPC->soundData = 0;

	// Set up the interrupt handler
	irqInit();
	irqSet(IRQ_VBLANK, VblankHandler);
	irqEnable(IRQ_VBLANK);

     // Setup FIFO
     
 	irqSet(IRQ_FIFO_NOT_EMPTY,FifoInterruptHandler);
	irqEnable(IRQ_FIFO_NOT_EMPTY);     
    REG_IPC_FIFO_CR =  IPC_FIFO_ENABLE | IPC_FIFO_SEND_CLEAR | IPC_FIFO_RECV_IRQ ;
    
	// Keep the ARM7 out of main RAM
	while (1)
		swiWaitForVBlank();
	
	return 0;
}
