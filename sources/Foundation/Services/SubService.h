#ifndef _SUBSERVICE_H_
#define _SUBSERVICE_H_

class SubService {
public:
	SubService(int fourCC);
	virtual ~SubService() ;
	int GetFourCC() { return fourCC_ ; } ;
private:
	int fourCC_ ;
} ;
#endif
