//
// file :         readthread.cpp
//
// description :  reading thread
//
// project :      TANGO Device Server
//
// $Author: graziano $ 
//
// $Revision: 1.4 $
//
// $Log: sequencerthread.cpp,v $
// Revision 1.4  2015-01-26 10:03:54  graziano
// fixed
//
// Revision 1.3  2013-11-20 13:44:42  graziano
// development
//
// Revision 1.2  2013-11-19 14:58:56  graziano
// development
//
// Revision 1.1  2013-11-15 08:15:09  graziano
// first commit
//
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

#include "sequencerthread.h"
#include "SequencerConf.h"
#include <math.h>

static const char __FILE__rev[] = __FILE__ " $Revision: 1.4 $";

namespace SequencerConf_ns
{

//+------------------------------------------------------------------
//
//	method:			readthread::readthread()
//
//	description:	readthread constructor
//
//-------------------------------------------------------------------
readthread::readthread(Tango::DeviceImpl* devImpl):Tango::LogAdapter(devImpl)
{ 
	DEBUG_STREAM << "readthread::readthread(): constructor... :" << __FILE__rev << endl;
	device = devImpl;
}

//+------------------------------------------------------------------
//
//	method:			readthread::~readthread()
//
//	description:	readthread destructor
//
//-------------------------------------------------------------------
readthread::~readthread()
{
	DEBUG_STREAM << "readthread::~readthread(): destructor... !" << endl;
}

//+------------------------------------------------------------------
//
//	method:			readthread::run()
//
//	description:	Run
//
//-------------------------------------------------------------------
void readthread::run(void *)
{
	INFO_STREAM << "readthread::run(): running... !" << endl;
	/*int pausesec,pausenano,pausesec_dfl,pausenano_dfl,
	    cnt_err = 0,
		max_cnt_err = 3,
		ret;*/

	abortflag = false;

	while(!(static_cast<SequencerConf *>(device))->created_attr && !abortflag)
	{
		sleep(1);
		DEBUG_STREAM << "readthread::run(): waiting..." << endl;
	}
	(static_cast<SequencerConf *>(device))->set_state(Tango::OFF);
	stringstream status;
	status<<"Off ("<<(static_cast<SequencerConf *>(device))->step<<"/"<<((static_cast<SequencerConf *>(device))->att_data.size())<<")";
	(static_cast<SequencerConf *>(device))->set_status(status.str());

	while (!abortflag)
	{
		if((static_cast<SequencerConf *>(device))->get_state() != Tango::RUNNING)
		{
			if((static_cast<SequencerConf *>(device))->polling_period > 0)
				abort_sleep((static_cast<SequencerConf *>(device))->polling_period);
			else
				usleep(300000);
			continue;
		}
		else
		{
			bool formula_err = false;
			bool aborted = false;
			bool timeout = false;
			while((static_cast<SequencerConf *>(device))->step >= 1 && (static_cast<SequencerConf *>(device))->step <= (static_cast<SequencerConf *>(device))->att_data.size())
			{
				stringstream stepname;
				stepname << string("step")<<(static_cast<SequencerConf *>(device))->step;
				DEBUG_STREAM << "readthread::run(): executing "<< stepname.str() << endl;
				stringstream status;
				status<<"Running ("<<(static_cast<SequencerConf *>(device))->step<<"/"<<((static_cast<SequencerConf *>(device))->att_data.size())<<")";
				(static_cast<SequencerConf *>(device))->set_status(status.str());
				formula_err = false;
				aborted = false;
				timeout = false;
				map<string,attr_val_t >::iterator it_val = (static_cast<SequencerConf *>(device))->att_value.find(stepname.str());
				if(it_val != (static_cast<SequencerConf *>(device))->att_value.end())
				{
					it_val->second.val[0] = false;
				}
				map<string,attr_desc_t>::iterator it=(static_cast<SequencerConf *>(device))->att_data.find(stepname.str());
				if(it == (static_cast<SequencerConf *>(device))->att_data.end())
				{
					stringstream status;
					status<<"Sequence exited with error ("<<(static_cast<SequencerConf *>(device))->step<<"/"<<((static_cast<SequencerConf *>(device))->att_data.size())<<")";
					INFO_STREAM << "readthread::run(): " << status.str();
					(static_cast<SequencerConf *>(device))->add_log_str("Error");
					(static_cast<SequencerConf *>(device))->set_state(Tango::FAULT);
					(static_cast<SequencerConf *>(device))->set_status(status.str());
					aborted = true;
					break;
				}
				double res = 0;	//initialize to err
				//for(vector<attr_desc_t>::iterator it_rem_attr = it->second.begin(); it_rem_attr != it->second.end(); it_rem_attr++)
				//vector<attr_desc_t>::iterator it_rem_attr = it->second.begin();
				{
					string attr_values;
					int esttime=0;
					map<string,attr_desc_t>::iterator it_data;
					for(it_data = (static_cast<SequencerConf *>(device))->att_data.begin(); it_data != (static_cast<SequencerConf *>(device))->att_data.end(); it_data++)
					{
						if(it_data->second.step < (static_cast<SequencerConf *>(device))->step || (it_data->second.step > (int)(static_cast<SequencerConf *>(device))->att_data.size()))
							continue;
						if(it_data->second.timeout != -1)
							esttime += ceil((double)it_data->second.timeout/1000);
						else
							esttime += ceil((double)(static_cast<SequencerConf *>(device))->defaultStepTimeout/1000);
					}
					(static_cast<SequencerConf *>(device))->RemainingTime = esttime;
					if(it->second.first)
					{
						clock_gettime(CLOCK_MONOTONIC,&it->second.start);
						it->second.first = false;
					}
					try
					{
						strcpy((static_cast<SequencerConf *>(device))->stepstatus_str,it->second.descr.c_str());
						res = (static_cast<SequencerConf *>(device))->eval_formula(it->second.formula_tree, stepname.str(), attr_values);
						INFO_STREAM << "readthread::run(): step="<< (static_cast<SequencerConf *>(device))->step << " formula returned: "<<res;
						if(it_val != (static_cast<SequencerConf *>(device))->att_value.end() && res > 0)
						{
							it_val->second.val[0] = true;
						}
					}
					catch(string &err)
					{
						INFO_STREAM << __func__<<": error evaluating formula="<<it->second.formula<<" err="<<err<<endl;
						formula_err = true;
						if(err == "aborted")
							aborted = true;
						if(err == "TIMEOUT")
							timeout = true;
						//break;
					}
					catch(std::out_of_range &err)
					{
						INFO_STREAM << __func__<<": out_of_range exception evaluating formula="<<it->second.formula<<" err="<<err.what()<<endl;
						formula_err = true;
						//break;
					}
					catch(...)
					{
						INFO_STREAM << __func__<<": generic exception evaluating formula="<<it->second.formula<<endl;
						formula_err = true;
						//break;
					}
				}
				if(formula_err && !aborted && !timeout)
				{
					strcpy((static_cast<SequencerConf *>(device))->steperror_str,it->second.error.c_str());
					stringstream status;
					status<<"Sequence exited with parsing error ("<<(static_cast<SequencerConf *>(device))->step<<"/"<<((static_cast<SequencerConf *>(device))->att_data.size())<<")";
					INFO_STREAM << "readthread::run(): " << status.str();
					(static_cast<SequencerConf *>(device))->set_state(Tango::FAULT);
					(static_cast<SequencerConf *>(device))->set_status(status.str());
					(static_cast<SequencerConf *>(device))->add_log_str("Formula Exception");
					break;
				}
				if(!aborted && timeout)
				{
					strcpy((static_cast<SequencerConf *>(device))->steperror_str,it->second.error.c_str());
					stringstream status;
					status<<"Sequence exited with TIMEOUT ("<<(static_cast<SequencerConf *>(device))->step<<"/"<<((static_cast<SequencerConf *>(device))->att_data.size())<<")";
					INFO_STREAM << "readthread::run(): " << status.str();
					(static_cast<SequencerConf *>(device))->set_state(Tango::FAULT);
					(static_cast<SequencerConf *>(device))->set_status(status.str());
					(static_cast<SequencerConf *>(device))->add_log_str("TIMEOUT");
					break;
				}
				else if(aborted)
				{
					INFO_STREAM << "readthread::run(): aborted";
				}
				(static_cast<SequencerConf *>(device))->step = res;
				if(res <= (static_cast<SequencerConf *>(device))->att_data.size() && res > 0)
					*(static_cast<SequencerConf *>(device))->attr_Step_read = res;
				if(res <= 0)
					strcpy((static_cast<SequencerConf *>(device))->steperror_str,it->second.error.c_str());
			}

			if(!formula_err && !aborted)
			{
				stringstream status;
				if((static_cast<SequencerConf *>(device))->step > 0)
				{
					if((static_cast<SequencerConf *>(device))->step > (static_cast<SequencerConf *>(device))->att_data.size())
						(static_cast<SequencerConf *>(device))->step = (static_cast<SequencerConf *>(device))->att_data.size();
					(static_cast<SequencerConf *>(device))->set_state(Tango::OFF);
					status<<"Sequence completed ("<<*(static_cast<SequencerConf *>(device))->attr_Step_read<<"/"<<((static_cast<SequencerConf *>(device))->att_data.size())<<")";
					(static_cast<SequencerConf *>(device))->add_log_str("End");
					strcpy((static_cast<SequencerConf *>(device))->stepstatus_str,"");
				}
				else
				{
					(static_cast<SequencerConf *>(device))->set_state(Tango::FAULT);
					status<<"Sequence exited with error ("<<*(static_cast<SequencerConf *>(device))->attr_Step_read<<"/"<<((static_cast<SequencerConf *>(device))->att_data.size())<<")";
					(static_cast<SequencerConf *>(device))->add_log_str("Error");
				}
				(static_cast<SequencerConf *>(device))->set_status(status.str());
			}
		}

		if((static_cast<SequencerConf *>(device))->polling_period > 0)
			abort_sleep((static_cast<SequencerConf *>(device))->polling_period);
	}
	INFO_STREAM << "readthread::run(): exit!!!" << endl;
}

//+------------------------------------------------------------------
//
//	method:			readthread::abort_sleep
//
//	description:	Resume from sleep if abort_flag set (sec.)
//
//-------------------------------------------------------------------
void readthread::abort_sleep(double time)
{

	for (int i = 0; i < (time/0.3); i++) {
		if (abortflag)
			break;
		omni_thread::sleep(0,300000000);
	}

}


}


