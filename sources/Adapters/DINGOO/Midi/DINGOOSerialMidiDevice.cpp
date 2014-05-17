
#include "DINGOOSerialMidiDevice.h"
#include "System/io/Trace.h"
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <memory.h>

const char *port="/dev/ttyS0" ;
//const char *port="/dev/usb/tts/0" ;
#define BAUDRATE B38400


DINGOOSerialMidiDevice::DINGOOSerialMidiDevice():MidiOutDevice("DINGOO Serial") {
    fd_=0 ;
} ;

bool DINGOOSerialMidiDevice::Init(){

     struct termios newtio;

     Trace::Debug("about to open port") ;
     fd_ = open(port, O_RDWR /*| O_NOCTTY | O_NDELAY*/ ); 
     if (fd_ <=0) {Trace::Dump("Failed to open %",port); fd_=0 ;return false ; }
     Trace::Dump("opened serial successfully %x",fd_) ;
     tcgetattr(fd_,&oldtio_); /* save current port settings */
    
     memset(&newtio,0,sizeof(newtio)) ;
     tcgetattr(fd_,&newtio); /* save current port settings */
     newtio.c_cflag = BAUDRATE | CS8 | CLOCAL;
     newtio.c_iflag = IGNPAR;
     newtio.c_oflag = 0;
    
    /* set input mode (non-canonical, no echo,...) */
    
    newtio.c_lflag = 0;
    
    tcflush(fd_, TCIFLUSH);
    int code=tcsetattr(fd_,TCSANOW,&newtio);
    if (code<0) {
       Trace::Dump("Failed to set attributes") ;
    } else {
       Trace::Debug("Serial attributes set") ;
    }

 	return (code>=0)   ;
} ;

void DINGOOSerialMidiDevice::Close(){

     if (fd_>0) {
        tcsetattr(fd_,TCSANOW,&oldtio_);
        close(fd_) ;
        fd_=0 ;
     }
}  ;

bool DINGOOSerialMidiDevice::Start(){
	return true ;
}  ;

void DINGOOSerialMidiDevice::Stop(){
} ;


void DINGOOSerialMidiDevice::SendMessage(MidiMessage &msg) {
     unsigned char buffer[3] ;
     if (fd_>0) {
        buffer[0]=msg.status_ ;
        int len=1 ;
 
		if (msg.status_<0xF0) {
          buffer[1]=msg.data1_;
          buffer[2]=msg.data2_ ;
          len=3 ;
         }
       write(fd_,buffer,len);
       Trace::Debug("Sending 0x%x 0x%x 0x%x",buffer[0],buffer[1],buffer[2]) ;
    }
     
} ;
