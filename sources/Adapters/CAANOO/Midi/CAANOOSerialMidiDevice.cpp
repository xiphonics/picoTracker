
#include "CAANOOSerialMidiDevice.h"
#include "Application/Model/Config.h"
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <memory.h>

#define BAUDRATE B38400


CAANOOSerialMidiDevice::CAANOOSerialMidiDevice():MidiOutDevice("CAANOO Serial") {
	fd_=0 ;

	Config *config=Config::GetInstance() ;
	port_=config->GetValue("CAANOO_MIDIDEVICE") ;
	if (!port_)
	{
		port_ = "/dev/ttyS0";
	}

	Trace::Log("MIDI","using MIDI port %s",port_) ;
	SetName(port_);

} ;

bool CAANOOSerialMidiDevice::Init(){

    struct termios newtio;

     fd_ = open(port_, O_RDWR /*| O_NOCTTY | O_NDELAY*/ ); 
     if (fd_ <=0) {Trace::Error("Failed to open %",port_); fd_=0 ;return false ; }
     Trace::Log("MIDI","opened serial successfully %x",fd_) ;
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
} ;

void CAANOOSerialMidiDevice::Close(){

     if (fd_>0) {
        tcsetattr(fd_,TCSANOW,&oldtio_);
        close(fd_) ;
        fd_=0 ;
     }
}  ;

bool CAANOOSerialMidiDevice::Start(){
	return true ;
}  ;

void CAANOOSerialMidiDevice::Stop(){
} ;


void CAANOOSerialMidiDevice::SendMessage(MidiMessage &msg) {
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
 //      Trace::Debug("Sending 0x%x 0x%x 0x%x",buffer[0],buffer[1],buffer[2]) ;
    }
     
} ;
