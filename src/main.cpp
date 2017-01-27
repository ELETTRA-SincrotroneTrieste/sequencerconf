/*PROTECTED REGION ID(FormulaConf::main.cpp) ENABLED START*/
static const char *RcsId = "$Id: main.cpp,v 1.1 2013-11-15 08:15:09 graziano Exp $";
//=============================================================================
//
// file :        main.cpp
//
// description : C++ source for the FormulaConf device server main.
//               The main rule is to initialise (and create) the Tango
//               system and to create the DServerClass singleton.
//                The main should be the same for every Tango device server.
//
// project :     Configurable Sequencer.
//
// $Author: graziano $
//
// $Revision: 1.1 $
// $Date: 2013-11-15 08:15:09 $
//
// SVN only:
// $HeadURL:  $
//
// CVS only:
// $Source: /home/cvsadm/cvsroot/fermi/servers/sequencerconf/src/main.cpp,v $
// $Log: main.cpp,v $
// Revision 1.1  2013-11-15 08:15:09  graziano
// first commit
//
//
//
//
//
//=============================================================================
//                This file is generated by POGO
//        (Program Obviously used to Generate tango Object)
//=============================================================================

#include <tango.h>


int main(int argc,char *argv[])
{
	Tango::Util *tg = NULL;
	try
	{
		// Initialise the device server
		//----------------------------------------
		tg = Tango::Util::init(argc,argv);

		// Create the device server singleton 
		//	which will create everything
		//----------------------------------------
		tg->server_init(false);

		// Run the endless loop
		//----------------------------------------
		cout << "Ready to accept request" << endl;
		tg->server_run();
	}
	catch (bad_alloc)
	{
		cout << "Can't allocate memory to store device object !!!" << endl;
		cout << "Exiting" << endl;
	}
	catch (CORBA::Exception &e)
	{
		Tango::Except::print_exception(e);
		
		cout << "Received a CORBA_Exception" << endl;
		cout << "Exiting" << endl;
	}
	if (tg!=NULL)
		tg->server_cleanup();
	return(0);
}
/*PROTECTED REGION END*/
//========================================================