
#include "NDSSound.h"
#include <nds.h>
#include "System/System/System.h"
#include "System/io/Trace.h"


// Called when we need to fill the buffer

void SoundBufferCallback() { 
	NDSSound *sound=(NDSSound *)Sound::GetInstance() ; ;
	sound->onChunkDone() ;
} ;

// The FIFO recieve handler

void FifoInterruptHandler() {
     
  // We don't recieve data from the ARM7 so it is U32 based
       u32 value=REG_IPC_FIFO_RX ;
		switch(value)
		{
		case ENGINE_CALLBACK:
			SoundBufferCallback();
			break;
		default:
            Trace::Dump("rogue return from arm7") ;
			break;
		}        
} ;

// Initialize data structures

bool NDSSound::Init() {

    // Creates the ring buffer for stereo sound playback
 
    ringBufferL_=(unsigned short *)SYS_MALLOC(RING_SAMPLE_SIZE*2*RING_BUFFER_COUNT);
    ringBufferR_=(unsigned short *)SYS_MALLOC(RING_SAMPLE_SIZE*2*RING_BUFFER_COUNT);

    // initialize boundaries
 
    for (int i=0;i<RING_BUFFER_COUNT;i++) {
        ringBoundaries_[0][i]=ringBufferL_+RING_SAMPLE_SIZE*i ;
        ringBoundaries_[1][i]=ringBufferR_+RING_SAMPLE_SIZE*i ;
    } ;

  // initialise secondary ring accumulation buffer
   
    unalignedMain_=(char *)SYS_MALLOC(RING_SAMPLE_SIZE*2+SOUND_BUFFER_MAX) ;
    
  // Make sure the buffer is aligned
  
     mainBuffer_=(char *)((((int)unalignedMain_)+1)&(0xFFFFFFFC)) ;

  // Create mini blank buffer in case of underruns

     miniBlank_=(char *)malloc(RING_SAMPLE_SIZE*4) ;
     SYS_MEMSET(miniBlank_,0,RING_SAMPLE_SIZE*4) ;

	// Clear all pool buffers
	
       for (int i=0;i<SOUND_BUFFER_COUNT;i++) {
    	     pool_[i]=0 ;
       } ;
    
    // Finished
       

// Start a timer for performance counting

       TIMER1_DATA =65535;
       TIMER1_CR = TIMER_ENABLE | TIMER_DIV_1024  ;

       TIMER2_DATA = 0;
	   TIMER2_CR = TIMER_ENABLE | TIMER_CASCADE | TIMER_DIV_1;
           
       return true ;
} ; 


void NDSSound::Close() {
     
    TIMER2_CR=0 ;
    TIMER1_CR=0 ;
    
  // free miniblank
  
	SAFE_FREE(miniBlank_) ;

  // free secondary ring
  
    SAFE_FREE(unalignedMain_) ;
	
  // free main ring

   SAFE_FREE(ringBufferL_) ;
   SAFE_FREE(ringBufferR_) ;
} ;


bool NDSSound::Start() {

    isPlaying_=true ;

    // fills right with zero to start with
 
    SYS_MEMSET(ringBufferL_,0,RING_SAMPLE_SIZE*4*RING_BUFFER_COUNT) ;
    SYS_MEMSET(ringBufferR_,0,RING_SAMPLE_SIZE*4*RING_BUFFER_COUNT) ;
    
    // first boundary as far as possible from the playhead
 
    currentBoundary_=RING_BUFFER_COUNT/2 ;

   // free any buffers that might not have been freed yet (should better be allocated once for all anyway)
    
	 for (int i=0;i<SOUND_BUFFER_COUNT;i++) {
         SAFE_FREE(pool_[i]) ;
	 } ;

    poolQueuePosition_=0 ;
    poolPlayPosition_=0 ;

  // Queue one blank to let engine start & ibitiate the callback mechanism
  
    char blank[RING_SAMPLE_SIZE*4] ;
	SYS_MEMSET(blank,0,RING_SAMPLE_SIZE*4) ;
	bufferPos_=0 ;
	bufferSize_=0 ;

	QueueBuffer(blank,00) ;

 // Initializes IRQ to recieve callback
     
 	irqSet(IRQ_FIFO_NOT_EMPTY,FifoInterruptHandler);
	irqEnable(IRQ_FIFO_NOT_EMPTY);
	
	REG_IPC_FIFO_CR = IPC_FIFO_ENABLE | IPC_FIFO_SEND_CLEAR | IPC_FIFO_RECV_IRQ;
                     
// Send start command to Arm7
     
    startARM7();
	return 1 ;
	
} ; 

void NDSSound::Stop() {
   irqDisable(IRQ_FIFO_NOT_EMPTY) ;
   stopARM7() ;     
   isPlaying_=false ;
} ;


 // Queues a buffer sent by the app, waiting for it to be send to
 // ring buffer
 // length here is in bytes
 
void NDSSound::QueueBuffer(char *buffer,int len) {

//     Trace::Dump("Qb") ;
	if (!isPlaying_) return ;

     if (len>SOUND_BUFFER_MAX) {
          Trace::Debug("Alert: buffer size exceeded") ;
     }
     
    if (pool_[poolQueuePosition_]!=0) {
      SYS_FREE(pool_[poolQueuePosition_]) ;
      pool_[poolQueuePosition_]=0 ;
      return ;
    }	

    pool_[poolQueuePosition_]=(char*) ((short *)SYS_MALLOC(len)) ;
    SYS_MEMCPY(pool_[poolQueuePosition_],buffer,len) ;
    poolSize_[poolQueuePosition_]=len ;
    poolQueuePosition_=(poolQueuePosition_+1)%SOUND_BUFFER_COUNT ; 
}


int NDSSound::GetPlayedBufferPercentage() {
    
    int current=TIMER2_DATA-lastTimer_ ;
    if(current < 0) current += 65536;
    current=(current*100)/RING_SAMPLE_SIZE ;
	return current ;
} ;

int NDSSound::GetSampleRate(){
	return 32768 ;
} ;

void NDSSound::onChunkDone() {
     
     lastTimer_=TIMER2_DATA ;
     
//  Trace::Dump("OCD") ;   

     if (!isPlaying_) {
            Trace::Debug("caught rogue callback") ;
            return ;          
     } 
     bool notify=false ;
     
//      Trace::Debug("preparing next chunk") ;

      // Look if we have enough data in main buffer
      
       int len=RING_SAMPLE_SIZE*4 ; // Length in bytes of data to provide for both channels
       
       // we need at least len data from the app
       
       while (bufferSize_-bufferPos_<len) {

//      Trace::Debug("need more data") ;

          // First move remaining bytes at the front of the ring buffer

          SYS_MEMCPY(mainBuffer_,mainBuffer_+bufferPos_,bufferSize_-bufferPos_) ;

         // then get next queued buffer and copy data from it to the ring

    	 if (pool_[poolPlayPosition_]==0) {
             Trace::Debug("Underrun playing audio") ;
    		 SYS_MEMCPY(mainBuffer_+bufferSize_-bufferPos_, miniBlank_,len);
    		 bufferSize_=bufferSize_-bufferPos_+len ;
             bufferPos_=0 ;
         } else { 
                
    		 SYS_MEMCPY(mainBuffer_+bufferSize_-bufferPos_, pool_[poolPlayPosition_],poolSize_[poolPlayPosition_]);
    
             // Adapt buffer variables
    
    		 bufferSize_=bufferSize_-bufferPos_+poolSize_[poolPlayPosition_] ;
             bufferPos_=0 ;
           
             SYS_FREE( pool_[poolPlayPosition_]) ;
    
             pool_[poolPlayPosition_]=0 ;
             poolPlayPosition_=(poolPlayPosition_+1)%SOUND_BUFFER_COUNT ;

    	     SetChanged() ;
    	     NotifyObservers() ;

        }    	 
//         Trace::Debug("data buffered") ;
      }
//     Trace::Debug("sending data") ;      

    // Now we got enough byte, send them to the main audio ring
    
       unsigned short *ptrL=ringBoundaries_[0][currentBoundary_] ;
       unsigned short *ptrR=ringBoundaries_[1][currentBoundary_] ;
       unsigned short *src=(unsigned short *)(mainBuffer_+bufferPos_) ;

/*       SYS_MEMCPY(ptrL,src,RING_SAMPLE_SIZE*2) ;
       SYS_MEMCPY(ptrR,src+RING_SAMPLE_SIZE,RING_SAMPLE_SIZE*2) ;
*/
       for (int i=0;i<RING_SAMPLE_SIZE;i++) {
           *ptrL++=*src++ ;
           *ptrR++=*src++ ;
       };
        DC_FlushRange(ringBoundaries_[0][currentBoundary_],RING_SAMPLE_SIZE*2) ;
        DC_FlushRange(ringBoundaries_[1][currentBoundary_],RING_SAMPLE_SIZE*2) ;
       
        currentBoundary_=(currentBoundary_+1)%RING_BUFFER_COUNT ;    
        bufferPos_+=len ;
//  Trace::Dump("~OCD") ;   

    int current=TIMER2_DATA-lastTimer_ ;
    if(current < 0) current += 65536;
    current=(current*100)/RING_SAMPLE_SIZE ;
   Trace::Debug("perfs=%d %%",current) ;
}


// Sends a command from here to the Arm7

void SendCommandToArm7(Command *command)
{
    DC_FlushRange(command,sizeof(Command)) ;
    REG_IPC_FIFO_TX =(unsigned int)command;
}

Command gStartCommand ;

void NDSSound::startARM7() {

	gStartCommand.commandType = ENGINE_START;
	gStartCommand.startCommand.left = ringBufferL_;
	gStartCommand.startCommand.right = ringBufferR_;
	gStartCommand.startCommand.length = RING_SAMPLE_SIZE*2*RING_BUFFER_COUNT;
    SendCommandToArm7(&gStartCommand) ;
}

Command gStopCommand ;

void NDSSound::stopARM7() {

// send command to arm7 we should stop the sound engine

	gStopCommand.commandType = ENGINE_STOP;
       
    SendCommandToArm7(&gStopCommand) ;
 
}
