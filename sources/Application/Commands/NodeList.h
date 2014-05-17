#ifndef _NODE_LIST_H_
#define _NODE_LIST_H_

#define URL_VOLUME_INCREASE "/mixer/volume/increase"
#define URL_VOLUME_DECREASE "/mixer/volume/decrease"
#define URL_EVENT_A "/event/a"
#define URL_EVENT_B "/event/b"
#define URL_EVENT_UP "/event/up"
#define URL_EVENT_DOWN "/event/down"
#define URL_EVENT_LEFT "/event/left"
#define URL_EVENT_RIGHT "/event/right"
#define URL_EVENT_LSHOULDER "/event/lshoulder"
#define URL_EVENT_RSHOULDER "/event/rshoulder"
#define URL_EVENT_START "/event/start"

#define URL_TEMPO_TAP "/tempo/tap"
#define URL_QUEUE_ROW "/sequencer/current/all/queue"

#define TRIG_VOLUME_INCREASE MAKE_FOURCC('V','L','U','P') 
#define TRIG_VOLUME_DECREASE MAKE_FOURCC('V','L','D','N') 
#define TRIG_EVENT_A MAKE_FOURCC('E','V','A','_')
#define TRIG_EVENT_B MAKE_FOURCC('E','V','B','_')
#define TRIG_EVENT_UP MAKE_FOURCC('E','V','U','P')
#define TRIG_EVENT_DOWN MAKE_FOURCC('E','V','D','N')
#define TRIG_EVENT_LEFT MAKE_FOURCC('E','V','L','F')
#define TRIG_EVENT_RIGHT MAKE_FOURCC('E','V','R','T')
#define TRIG_EVENT_LSHOULDER MAKE_FOURCC('E','V','L','S')
#define TRIG_EVENT_RSHOULDER MAKE_FOURCC('E','V','R','S')
#define TRIG_EVENT_START MAKE_FOURCC('E','V','S','T')

#define TRIG_TEMPO_TAP MAKE_FOURCC('T','T','A','P')
#define TRIG_SEQ_QUEUE_ROW MAKE_FOURCC('T','S','Q','R')
#endif
