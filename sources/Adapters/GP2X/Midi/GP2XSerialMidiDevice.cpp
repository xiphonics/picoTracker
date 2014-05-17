
#include "GP2XSerialMidiDevice.h"
#include "System/Console/Trace.h"
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <memory.h>

const char *port="/dev/ttyS0" ;
//const char *port="/dev/usb/tts/0" ;
#define BAUDRATE B38400


GP2XSerialMidiDevice::GP2XSerialMidiDevice():MidiOutDevice("GP2X Serial") {
    fd_=0 ;
}

bool GP2XSerialMidiDevice::Init(){

     struct termios newtio;

     Trace::Debug("about to open port") ;
     fd_ = open(port, O_RDWR /*| O_NOCTTY | O_NDELAY*/ ); 
     if (fd_ <=0) {Trace::Error("Failed to open %",port); fd_=0 ;return false ; }
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
       Trace::Error("Failed to set attributes") ;
    } 

 	return (code>=0)   ;
}

void GP2XSerialMidiDevice::Close(){

     if (fd_>0) {
        tcsetattr(fd_,TCSANOW,&oldtio_);
        close(fd_) ;
        fd_=0 ;
     }
}  ;

bool GP2XSerialMidiDevice::Start(){
	return true ;
}  ;

void GP2XSerialMidiDevice::Stop(){
}


void GP2XSerialMidiDevice::SendMessage(MidiMessage &msg) {
  unsigned char buffer[3] ;
  if (fd_>0) {
    buffer[0]=msg.status_ ;
    int len=1 ;

    if (msg.status_<0xF0) {
          buffer[1]=msg.data1_;
      len++;
      if (buffer[2] != MidiMessage::UNUSED_BYTE)
      {
      buffer[2]=msg.data2_ ;
      len++ ;
      }
    }
    write(fd_,buffer,len);
  }   
}
