#include "Application/Application.h"
#include "Adapters/NDS/System/NDSSystem.h"
#include <string.h>
#include <nds.h>
#include <stdio.h>

int main(int argc,char *argv[]) 
{
    REG_IPC_FIFO_CR = IPC_FIFO_ENABLE | IPC_FIFO_SEND_CLEAR ;    

    defaultExceptionHandler();

	bool fullscreen=false ;
	if (argc>1) {
		fullscreen=!strcmp(argv[1],"-fullscreen") ;
	}
	
    powerON(POWER_ALL);
	irqInit();

    consoleDemoInit();
    	
    BG_PALETTE[255] = RGB15(31,31,31);
	
//	irqSet(IRQ_VBLANK, on_irq_vblank); 
    
    videoSetMode(MODE_FB0);
    vramSetBankA(VRAM_A_LCD);
	videoSetModeSub(MODE_0_2D | DISPLAY_BG0_ACTIVE); 

    printf("booting") ;
	NDSSystem::Boot() ;

 	Application::Init(fullscreen) ;

	NDSSystem::MainLoop() ;

	Application::Cleanup() ;

	return 0;
}



