//
// file :         readthread.h
//
// description :  reading thread
//
// project :      TANGO Device Server
//
// $Author: graziano $ 
//
// $Revision: 1.1 $
//
// $Log: sequencerthread.h,v $
// Revision 1.1  2013-11-15 08:15:09  graziano
// first commit
//
//
//
//
//
//
//
// copyleft :   Sincrotrone Trieste S.C.p.A.
//              Strada Statale 14 - km 163,5 in AREA Science Park
//              34012 Basovizza, Trieste ITALY
//


#ifndef SEQUENCERTHREAD_H
#define SEQUENCERTHREAD_H

#include <omnithread.h>
#include <tango.h>
#include <inttypes.h>

namespace SequencerConf_ns
{

class readthread : public omni_thread, public Tango::LogAdapter
{	
	private:
		Tango::DeviceImpl* device;
		void abort_sleep(double time); 

				
	public:
		
		readthread(Tango::DeviceImpl* devImpl);
		~readthread();

		bool abortflag;
					
	protected:
		void run(void *);
		
};	/* end class readthread() */

}

#endif

/* EOF */
