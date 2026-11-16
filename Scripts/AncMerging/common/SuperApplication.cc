#include "cgicc/HTMLClasses.h"
#include "cgicc/HTTPHTMLHeader.h"
#include "cgicc/HTTPPlainHeader.h"
#include "interface/shared/i2oEVBMsgs.h"
#include "interface/shared/i2oXFunctionCodes.h"
#include "rubuilder/lfcaendgtz/SuperApplication.h"
#include "rubuilder/lfcaendgtz/version.h"
#include "rubuilder/lfcaendgtz/exception/Exception.h"
#include "rubuilder/utils/CreateStrings.h"
#include "rubuilder/utils/ForceFailedEvent.h"
#include "rubuilder/utils/XoapUtils.h"
#include "rubuilder/utils/StopWatch.h"
#include "toolbox/utils.h"
#include "toolbox/fsm/FailedEvent.h"
#include "xcept/tools.h"
#include "xdaq/exception/ApplicationNotFound.h"
#include "xdata/Double.h"
#include "xdata/InfoSpaceFactory.h"
#include "xgi/Method.h"
#include "xgi/Utils.h"
#include "xoap/domutils.h"
#include "xoap/MessageFactory.h"
#include "xoap/MessageReference.h"
#include "xoap/Method.h"
#include "xoap/SOAPEnvelope.h"

#include <algorithm>
#include <math.h>
#include <pthread.h>
#include <sstream>
#include <stdint.h>
#include <unistd.h>

uint32_t rubuilder::BufRefQ::frame_released_=0;


/*
 * An XDAQ application is a program written in the C++ programming language that can be loaded into an XDAQ executive
 * at run-time.
 * Unlike ordinary C++ applications, an XDAQ application does not have a main method as an entry point, but instead,
 * has several methods to control specific aspects of its execution.
 * XDAQ applications are implemented as C++ classes, that extend the class xdaq::Application through inheritance mechanism. 
 */
rubuilder::filter::SuperApplication::SuperApplication( xdaq::ApplicationStub *s ):
xdaq::WebApplication( s ),
state_(16),
logger_( getApplicationLogger() ),
rcmsStateNotifier_( logger_, getApplicationDescriptor(), getApplicationContext() ),
soapParameterExtractor_( this ),
applicationBSem_( toolbox::BSem::FULL ),
monitoringBSem_( toolbox::BSem::FULL )
{
   const char *hostn;
   const char *home;
   size_t pos;

   LOG4CPLUS_INFO( logger_, "START of CTOR" );


   hostn = getenv( "HOSTNAME" );
   
   if( hostn != NULL ) 
      hostname_ = std::string( hostn ); 
   else
   {
      LOG4CPLUS_ERROR( logger_, "HOSTNAME cannot be found in the environment!" );
      
      //XCEPT_RAISE( xdaq::exception::Exception, "HOSTNAME cannot be found in the environment!" );
   }
      
   if( hostname_.empty() )
   {
      LOG4CPLUS_ERROR( logger_, "hostname_ is an empty string!" );
      
      //XCEPT_RAISE( xdaq::exception::Exception, "hostname_ is an empty string!!" );
   }
   
   pos = hostname_.find_first_of(".");
    
   if( pos < hostname_.size() )
      hostname_.erase( hostname_.begin() + pos, hostname_.end() );
      
   LOG4CPLUS_INFO( logger_, "hostname: " << hostname_ );


   home = getenv( "HOME" );
   
   if( home != NULL ) 
      homedir_ = std::string( home ) + "/"; 
   else
   {
      LOG4CPLUS_ERROR( logger_, "HOME cannot be found in the environment!" );
      
      //XCEPT_RAISE( xdaq::exception::Exception, "HOME cannot be found in the environment!" );
   }
      
   if( homedir_.empty() )
   {
      LOG4CPLUS_ERROR( logger_, "homedir_ is an empty string!" );
      
      //XCEPT_RAISE( xdaq::exception::Exception, "homedir_ is an empty string!!" );
   }


   tid_ = 0;

   AutoRefresh_ = false;
	
   AutoRefresh_def_ = false;

   AutoRefresh_dbg_ = false;


/*
 * A XDAQ application can make various data elements visible to external systems.
 * Both simple built-in data types as well as user-defined composite data structures are supported and the set of data types
 * that are exported can be extended by the user.
 * The application properties can be set at configuration time and at run-time by using the xdata::package.
 * XDAQ application properties are placed in a memory structure called Application InfoSpace associated with the application.
 */
   appInfoSpace_ = getApplicationInfoSpace();

   i2oAddressMap_ = i2o::utils::getAddressMap();

   // xml configuration scope
   appDescriptor_ = getApplicationDescriptor();

   appContext_ = getApplicationContext();

   poolFactory_ = toolbox::mem::getMemoryPoolFactory();

   xmlClass_ = appDescriptor_->getClassName();

   // Application instance number
   instance_ = appDescriptor_->getInstance();
   xinstance_ = instance_;

   // Application urn
   urn_ = appDescriptor_->getURN();


   // Sets the value of an application property.
#ifdef x86_64_centos7
   s->getDescriptor()->setAttribute( "icon", APP_ICON );
#else
   appDescriptor_->setAttribute( "icon", APP_ICON );
#endif


   // Bind application web pages
   this->bindXgiCallbacks();


   block_seq_nb_ = 0;

   out_bf_ = NULL;

   out_df_ = NULL;

   
   Output_ = "";


   postbuf_len_ = 0;

   postbufRef_ = NULL;


   stopBlockExpected_ = 0;


   output_buffer_ = NULL;


   pool_ = NULL;


   ptSrv_ = NULL;


   i2oExceptionHandler_ = toolbox::exception::bind( this,
      &rubuilder::filter::SuperApplication::onI2oException, "onI2oException" );


   // Generate Application memory pool name
   poolName_ = rubuilder::utils::generatePoolName( xmlClass_, instance_ );


   // Application Finite State Machine
   try
   {
      defineFsm();
   }
   catch( xcept::Exception &e )
   {
      const std::string msg = "Failed to define the finite state machine ";

      LOG4CPLUS_ERROR( logger_,
         msg << xcept::stdformat_exception_history(e) );
        
      XCEPT_RETHROW( rubuilder::filter::exception::FSM, msg, e );
   }


   bindSoapCallbacks();

   bindI2oCallbacks();

   
   // Initialise and group parameters
   stdConfigParams_  = initAndGetStdConfigParams();

   stdMonitorParams_ = initAndGetStdMonitorParams();


   // Create info space for monitoring
   monitoringInfoSpaceName_ =
      rubuilder::utils::generateMonitoringInfoSpaceName( xmlClass_, instance_ );

   toolbox::net::URN urn =
      this->createQualifiedInfoSpace( monitoringInfoSpaceName_ );

   monitoringInfoSpace_ = xdata::getInfoSpaceFactory()->get( urn.toString() );
   

   // Put parameters into InfoSpaces
   try
   {
      putParamsIntoInfoSpace( stdConfigParams_ , appInfoSpace_ );

      putParamsIntoInfoSpace( stdMonitorParams_, appInfoSpace_ );
   
      putParamsIntoInfoSpace( stdMonitorParams_, monitoringInfoSpace_ );
      
      // Attach listeners to default appInfoSpace_
      //addInfoSpaceListeners( appInfoSpace_ );
      
      // Attach listener to monitoring infoSpace
      //monitoringInfoSpace_->addGroupRetrieveListener( this );
   }
   catch( xcept::Exception &e )
   {
      const std::string msg = "Failed to put parameters into info spaces ";

      LOG4CPLUS_ERROR( logger_,
         msg << xcept::stdformat_exception_history( e ) );
        
      XCEPT_RETHROW( rubuilder::filter::exception::Monitoring, msg, e );
   }


   clearMonitoringVars();

   try
   {
      startMonitoringCalculations();
   }
   catch( xcept::Exception &e )
   {
      const std::string msg = "Failed to start monitoring work loop ";

      LOG4CPLUS_ERROR( logger_,
         msg << xcept::stdformat_exception_history(e) );
        
      XCEPT_RETHROW( rubuilder::filter::exception::Monitoring, msg, e );
   }


   refq_mutex_ = new toolbox::BSem( toolbox::BSem::FULL );


/*
The properties tag in the xml configuration file, provide default values for the exported parameters-
-which are set immediately after a successful creation of the application instance.

An Application can be identified within a context using one of the following combinations:
1) class name + instance number unique within context
2) numeric identifier unique within context ( 10 <= id <= 0xFFFF )


tid is the I2O target identifier assigned to this class instance ( 0 <= tid <= 2048 )

There are two ways to declare a target identifier (tid):
1) class + instance
class is the C++ class name (including namespace if present) of the application
instance is the partition wide unique instance number
Example: <i2o:target class="rubuilder::ru::Application"  instance="0" tid="25"/>

2) context + id
Context is the context identifier URL of the XDAQ process
id is the context wide unique numeric identifier of the target application
Example: <i2o:target context="http://lxcmd102:40000" id="11" tid="24"/>
*/

   rcmsStateNotifier_.subscribeToChangesInRcmsStateListener( appInfoSpace_ );

   rcmsStateNotifier_.findRcmsStateListener();


   state_.set( STATE_DISABLED );


   LOG4CPLUS_INFO( logger_, "END of CTOR" );
}


rubuilder::filter::SuperApplication::~SuperApplication()
{

   LOG4CPLUS_INFO( logger_, "START of DTOR" );

   if( refq_mutex_ ) {
      delete refq_mutex_;
   }

   if( output_buffer_ ) {
      delete [] output_buffer_;
      output_buffer_ = NULL;
   }

   LOG4CPLUS_INFO( logger_, "END of DTOR" );
}


void rubuilder::filter::SuperApplication::input( const std::string& in )
{

   try
   {
      toolbox::Event::Reference e( new toolbox::Event( in, this ) );

      fsm_.fireEvent(e);
   }
   catch( toolbox::fsm::exception::Exception & e )
   {
      std::cout << "Invalid input " << in << ", error: " << e.what() << std::endl;
   }
}


void rubuilder::filter::SuperApplication::getAppDescriptorsAndTids()
{

   try
   {
      // This field holds the numeric identifier of the application that will receive the message.
      // Used for the location lookup (address resolution).
      tid_ = i2oAddressMap_->getTid( appDescriptor_ );
   }
   catch( xcept::Exception &e )
   {
      const std::string msg = "Failed to get the I2O TID of this application ";

      LOG4CPLUS_ERROR( logger_,
         msg << xcept::stdformat_exception_history(e) );
       
      XCEPT_RETHROW( rubuilder::filter::exception::I2o, msg, e );
   }

   // Fill internal list of BUs destination
   try
   {
      fillDestinationBUs();
   }
   catch( xcept::Exception &e )
   {
      const std::string msg = "Failed to fill internal descriptor list of BUs destination ";

      LOG4CPLUS_ERROR( logger_,
         msg << xcept::stdformat_exception_history( e ) );
        
      XCEPT_RETHROW( rubuilder::filter::exception::Configuration, msg, e );
   }

   // Fill internal list of FILTERs destination
   try
   {
      fillDestinationFILTERs();
   }
   catch( xcept::Exception &e )
   {
      const std::string msg = "Failed to fill internal descriptor list of FILTERs destination ";

      LOG4CPLUS_ERROR( logger_,
         msg << xcept::stdformat_exception_history( e ) );
        
      XCEPT_RETHROW( rubuilder::filter::exception::Configuration, msg, e );
   }

   // Fill internal list of MERGERs destination
   try
   {
      fillDestinationMERGERs();
   }
   catch( xcept::Exception &e )
   {
      const std::string msg = "Failed to fill internal descriptor list of MERGERs destination";

      LOG4CPLUS_ERROR( logger_,
         msg << xcept::stdformat_exception_history( e ) );
        
      XCEPT_RETHROW( rubuilder::filter::exception::Configuration, msg, e );
   }
}


void rubuilder::filter::SuperApplication::checkBuInstances()
{
   unsigned int i = 0;
   unsigned int j = 0;

   for( i=0; i<buInstances_.size(); i++ )
   {
      // Check destination bu instance is not a duplicate
      for( j=0; j<buInstances_.size(); j++ )
      {
         if( (i != j) && ( buInstances_[i] == buInstances_[j] ) )
         {
            std::stringstream oss;

            oss << "Destination bu instance is a duplicate.";
            oss << " Instance:" << i;

            LOG4CPLUS_ERROR( logger_, oss.str() );

            XCEPT_RAISE( rubuilder::filter::exception::Configuration, oss.str() );
         }
      }
   }
}


void rubuilder::filter::SuperApplication::fillDestinationBUs()
{

   try
   {
      // Check destination bu instance is not a duplicate
      checkBuInstances();
   }
   catch( xcept::Exception &e )
   {
      const std::string msg = "Exported parameter \"buInstances\" is invalid ";

      LOG4CPLUS_ERROR( logger_,
         msg << xcept::stdformat_exception_history( e ) );
        
      XCEPT_RETHROW( rubuilder::filter::exception::Configuration, msg, e );
   }

   // Clear list of destination bu
   destinationBUs_.clear();

   // Fill list of bu destination.
   // Dalle istanze delle bu prese in configurazione dall'array buInstances_,-
   //-si ricavano bu.descriptor e bu.tid e si filla destinationBUs_.
   for( int i=0; ( (xdata::Vector<xdata::UnsignedInteger32>::size_type)i ) < buInstances_.size(); i++ )
   {
      rubuilder::utils::ApplicationDescriptorAndTid bu;

      //std::cout << "buInstances_.size(): " << buInstances_.size() << std::endl;
      std::cout << "Destination bu instance: " << buInstances_[i] << std::endl;

      try
      {
         bu.descriptor =
            getApplicationContext()->
            getDefaultZone()->
            getApplicationDescriptor( "rubuilder::bu::Application",
               buInstances_[i] );
      }
      catch( xcept::Exception &e )
      {
         std::stringstream oss;

         oss << "Failed to get application descriptor for bu";
         oss << buInstances_[i];

         LOG4CPLUS_ERROR( logger_,
            oss.str() << xcept::stdformat_exception_history( e ) );

         XCEPT_RETHROW( rubuilder::filter::exception::Configuration, oss.str(), e );
      }

      try
      {
         bu.tid = i2oAddressMap_->getTid( bu.descriptor );

         std::cout << "bu.tid: " << bu.tid << std::endl;
      }
      catch( xcept::Exception &e )
      {
         std::stringstream oss;

         oss << "Failed to get I2O TID for bu";
         oss << buInstances_[i];

         LOG4CPLUS_ERROR( logger_,
            oss.str() << xcept::stdformat_exception_history( e ) );

         XCEPT_RETHROW( rubuilder::filter::exception::I2o, oss.str(), e );
      }

      destinationBUs_.push_back(bu);
   }
}


void rubuilder::filter::SuperApplication::checkFilterInstances()
{
   unsigned int i = 0;
   unsigned int j = 0;

   for( i=0; i<filterInstances_.size(); i++ )
   {
      // Check destination filter instance is not a duplicate
      for( j=0; j<filterInstances_.size(); j++ )
      {
         if( (i != j) && ( filterInstances_[i] == filterInstances_[j] ) )
         {
            std::stringstream oss;

            oss << "Destination filter instance is a duplicate.";
            oss << " Instance:" << i;

            LOG4CPLUS_ERROR( logger_, oss.str() );

            XCEPT_RAISE( rubuilder::filter::exception::Configuration, oss.str() );
         }
      }
   }
}


void rubuilder::filter::SuperApplication::fillDestinationFILTERs()
{

   try
   {
      // Check destination filter instance is not a duplicate
      checkFilterInstances();
   }
   catch( xcept::Exception &e )
   {
      const std::string msg = "Exported parameter \"LocalFilterInstances\" is invalid ";

      LOG4CPLUS_ERROR( logger_,
         msg << xcept::stdformat_exception_history( e ) );
        
      XCEPT_RETHROW( rubuilder::filter::exception::Configuration, msg, e );
   }

   // Clear list of destination filter
   destinationFILTERs_.clear();

   // Fill list of filter destination.
   // Dalle istanze delle application filter prese in configurazione dall'array filterInstances_,-
   //-si ricavano filter.descriptor e filter.tid e si filla destinationFILTERs_.
   for( int i=0; ( (xdata::Vector<xdata::UnsignedInteger32>::size_type)i ) < filterInstances_.size(); i++ )
   {
      rubuilder::utils::ApplicationDescriptorAndTid filter;

      //LOG4CPLUS_INFO( logger_, "filterInstances_.size(): " << filterInstances_.size() );
      LOG4CPLUS_INFO( logger_, "Destination filter Instance: " << filterInstances_[i] );

      try
      {
/*
         filter.descriptor =
            getApplicationContext()->
            getDefaultZone()->
            getApplicationDescriptor( "rubuilder::filter::Application",
               filterInstances_[i] );
*/

         filter.descriptor =
            getApplicationContext()->
            getDefaultZone()->
            getApplicationDescriptor( "LocalFilter",
               filterInstances_[i] );
      }
      catch( xcept::Exception &e )
      {
         std::stringstream oss;

         oss << "Failed to get application descriptor for filter";
         oss << filterInstances_[i];

         LOG4CPLUS_ERROR( logger_,
            oss.str() << xcept::stdformat_exception_history( e ) );

         XCEPT_RETHROW( rubuilder::filter::exception::Configuration, oss.str(), e );
      }

      try
      {
         filter.tid = i2oAddressMap_->getTid( filter.descriptor );
      }
      catch( xcept::Exception &e )
      {
         std::stringstream oss;

         oss << "Failed to get I2O TID for filter";
         oss << filterInstances_[i];

         LOG4CPLUS_ERROR( logger_,
            oss.str() << xcept::stdformat_exception_history( e ) );

         XCEPT_RETHROW( rubuilder::filter::exception::I2o, oss.str(), e );
      }

      destinationFILTERs_.push_back(filter);
   }
}


void rubuilder::filter::SuperApplication::fillDestinationMERGERs()
{

   try
   {
      // Check destination merger instance is not a duplicate
      checkMergerInstances( mergerInstances_ );
   }
   catch( xcept::Exception &e )
   {
      const std::string msg = "Exported parameter \"MergerInstances\" is invalid";

      LOG4CPLUS_ERROR( logger_,
         msg << xcept::stdformat_exception_history( e ) );
        
      XCEPT_RETHROW( rubuilder::filter::exception::Configuration, msg, e );
   }

   // Clear list of destinatiom merger
   destinationMERGERs_.clear();

   // Fill list of filter destination.
   // Dalle istanze delle application filter prese in configurazione dall'array filterInstances_,-
   //-si ricavano filter.descriptor e filter.tid e si filla destinationFILTERs_.
   for( int i=0; ( (xdata::Vector<xdata::UnsignedInteger32>::size_type)i ) < mergerInstances_.size(); i++ )
   {
      rubuilder::utils::ApplicationDescriptorAndTid merger;

      //LOG4CPLUS_INFO( logger_, "mergerInstances_.size(): " << mergerInstances_.size() );
      LOG4CPLUS_INFO( logger_, "Destination merger instance: " << mergerInstances_[i] );

      try
      {
         merger.descriptor =
            getApplicationContext()->
            getDefaultZone()->
            getApplicationDescriptor( "rubuilder::merger::Application",
               mergerInstances_[i] );
      }
      catch( xcept::Exception &e )
      {
         std::stringstream oss;

         oss << "Failed to get application descriptor for merger";
         oss << filterInstances_[i];

         LOG4CPLUS_ERROR( logger_, oss.str() );

         XCEPT_RETHROW( rubuilder::filter::exception::Configuration, oss.str(), e );
      }

      try
      {
         merger.tid = i2oAddressMap_->getTid( merger.descriptor );
         LOG4CPLUS_INFO( logger_, "Destination merger tid: " << merger.tid );
      }
      catch( xcept::Exception &e )
      {
         std::stringstream oss;

         oss << "Failed to get I2O TID for merger";
         oss << mergerInstances_[i];

         LOG4CPLUS_ERROR( logger_, oss.str() );

         XCEPT_RETHROW( rubuilder::filter::exception::I2o, oss.str(), e );
      }

      destinationMERGERs_.push_back(merger);
   }
}


void rubuilder::filter::SuperApplication::checkMergerInstances( xdata::Vector<xdata::UnsignedInteger32> &mergerInst )
{
   unsigned int i = 0;
   unsigned int j = 0;

   for( i=0; i<mergerInst.size(); i++ )
   {
      // Check participating filter instance is not a duplicate
      for( j=0; j<mergerInst.size(); j++ )
      {
         if( (i != j) && ( mergerInst[i] == mergerInst[j] ) )
         {
            std::stringstream oss;

            oss << "merger instance is a duplicate.";
            oss << " Instance:" << i;

            LOG4CPLUS_ERROR( logger_, oss.str() );

            XCEPT_RAISE( rubuilder::filter::exception::Configuration, oss.str() );
         }
      }
   }
}


void rubuilder::filter::SuperApplication::defineFsm()
{

   try
   {
      // Define Fsm states
      fsm_.addState( 'H', "Halted", this,
         &rubuilder::filter::SuperApplication::stateChanged );

      fsm_.addState( 'C', "Configured", this,
         &rubuilder::filter::SuperApplication::stateChanged );

      fsm_.addState( 'R', "Running", this,
         &rubuilder::filter::SuperApplication::stateChanged );

      fsm_.addState( 'P', "Paused", this,
         &rubuilder::filter::SuperApplication::stateChanged );
   }
   catch( xcept::Exception &e )
   {
      const std::string msg = "Failed to define FSM states ";

      LOG4CPLUS_ERROR( logger_,
         msg << xcept::stdformat_exception_history(e) );

      XCEPT_RETHROW( rubuilder::filter::exception::FSM, msg, e );
   }
      

   try
   {
      // Explicitly set the name of the Failed state
      fsm_.setStateName( 'F', "Failed" );
   }
   catch( xcept::Exception &e )
   {
      const std::string msg = "Failed to explicitly set the name of the Failed state ";

      LOG4CPLUS_ERROR( logger_,
         msg << xcept::stdformat_exception_history(e) );

      XCEPT_RETHROW( rubuilder::filter::exception::FSM, msg, e );
   }


   try
   {
      // Define Fsm transitions
      fsm_.addStateTransition( 'H', 'C', "Configure",	this,
         &rubuilder::filter::SuperApplication::configureAction );

      fsm_.addStateTransition( 'C', 'H', "Halt",	this,
         &rubuilder::filter::SuperApplication::haltAction );

      fsm_.addStateTransition( 'C', 'R', "Enable",	this,
         &rubuilder::filter::SuperApplication::enableAction );

      fsm_.addStateTransition( 'R', 'C', "Disable",	this,
         &rubuilder::filter::SuperApplication::disableAction );

      fsm_.addStateTransition( 'R', 'H', "Halt",	this,
         &rubuilder::filter::SuperApplication::haltActionFromRunning );

      fsm_.addStateTransition( 'R', 'P', "Pause",	this,
         &rubuilder::filter::SuperApplication::pauseAction );

      fsm_.addStateTransition( 'P', 'R', "Resume",	this,
         &rubuilder::filter::SuperApplication::resumeAction );

      fsm_.addStateTransition( 'P', 'R', "Halt",	this,
         &rubuilder::filter::SuperApplication::haltAction );

      fsm_.addStateTransition( 'R', 'F', "Fail",	this,
         &rubuilder::filter::SuperApplication::failAction );
   }
   catch( xcept::Exception &e )
   {
      const std::string msg = "Failed to define FSM states ";

      LOG4CPLUS_ERROR( logger_,
         msg << xcept::stdformat_exception_history(e) );

      XCEPT_RETHROW( rubuilder::filter::exception::FSM, msg, e );
   }


   try
   {
      fsm_.setFailedStateTransitionAction( this,
         &rubuilder::filter::SuperApplication::failAction );
   }
   catch( xcept::Exception &e )
   {
      const std::string msg = "Failed to set action for failed state transition ";

      LOG4CPLUS_ERROR( logger_,
         msg << xcept::stdformat_exception_history(e) );

      XCEPT_RETHROW( rubuilder::filter::exception::FSM, msg, e );
   }


   try
   {
      fsm_.setFailedStateTransitionChanged( this,
         &rubuilder::filter::SuperApplication::stateChanged );
   }
   catch( xcept::Exception &e )
   {
      const std::string msg = "Failed to set state changed callback for failed state transition ";

      LOG4CPLUS_ERROR( logger_,
         msg << xcept::stdformat_exception_history(e) );

      XCEPT_RETHROW( rubuilder::filter::exception::FSM, msg, e );
   }


   try
   {
      fsm_.setInitialState( 'H' );
   }
   catch( xcept::Exception &e )
   {
      const std::string msg = "Failed to set state initial state of FSM";

      LOG4CPLUS_ERROR( logger_,
         msg << xcept::stdformat_exception_history(e) );

      XCEPT_RETHROW( rubuilder::filter::exception::FSM, msg, e );
   }


   try
   {
      fsm_.reset();
   }
   catch( xcept::Exception &e )
   {
      const std::string msg = "Failed to reset FSM ";

      LOG4CPLUS_ERROR( logger_,
         msg << xcept::stdformat_exception_history(e) );

      XCEPT_RETHROW( rubuilder::filter::exception::FSM, msg, e );
   }
}


void rubuilder::filter::SuperApplication::stateChanged
(
   toolbox::fsm::FiniteStateMachine &fsm
)
{
   toolbox::fsm::State state = fsm.getCurrentState();

   try
   {
      monitoringInfoSpace_->lock(); appInfoSpace_->lock();

      stateName_ = fsm.getStateName( state );

      monitoringInfoSpace_->unlock(); appInfoSpace_->unlock();
   }
   catch( xcept::Exception &e )
   {
      monitoringInfoSpace_->unlock(); appInfoSpace_->unlock();

      const std::string msg = "Failed to set exported parameter stateName ";

      LOG4CPLUS_ERROR( logger_,
         msg << xcept::stdformat_exception_history(e) );

      XCEPT_RETHROW( rubuilder::filter::exception::FSM, msg, e );
   }

   // Send notification if current state is an error state
   if( state == 'F' ) // Failed
   {
      std::string reason = "";

      switch( state )
      {
         case 'F':
            // Used to display on the application web-pages the reason why the-
            //-finite state machine moved to the failed state if it did.
            reason = reasonForFailed_.getValue();
            break;

         default:
            reason = "";
      }

      try
      {
         rcmsStateNotifier_.stateChanged( stateName_.value_, reason );
      }
      catch( xcept::Exception &e )
      {
         const std::string msg = "Failed to notify state change ";

         LOG4CPLUS_ERROR( logger_,
            msg << xcept::stdformat_exception_history(e) );

         XCEPT_RETHROW( rubuilder::filter::exception::FSM, msg, e );
      }
   }
}


void rubuilder::filter::SuperApplication::bindSoapCallbacks()
{
   
   xoap::bind( this, &rubuilder::filter::SuperApplication::processSoapFsmEvent,
      "Configure", XDAQ_NS_URI );
      
   xoap::bind( this, &rubuilder::filter::SuperApplication::processSoapFsmEvent,
      "Enable", XDAQ_NS_URI );
   
   xoap::bind( this, &rubuilder::filter::SuperApplication::processSoapFsmEvent,
      "Disable", XDAQ_NS_URI );
	    
   xoap::bind( this, &rubuilder::filter::SuperApplication::processSoapFsmEvent,
      "Pause", XDAQ_NS_URI );
	    
   xoap::bind( this, &rubuilder::filter::SuperApplication::processSoapFsmEvent,
      "Resume", XDAQ_NS_URI );
	    
   xoap::bind( this, &rubuilder::filter::SuperApplication::processSoapFsmEvent,
      "Halt", XDAQ_NS_URI );
}


xoap::MessageReference rubuilder::filter::SuperApplication::processSoapFsmEvent
(
   xoap::MessageReference msg
)
{
   std::string event = "";

   try
   {
      event = soapParameterExtractor_.extractParameters( msg );
   }
   catch( xcept::Exception &e )
   {
      std::string msg = "Failed to extract FSM event and parameters from SOAP message ";

      LOG4CPLUS_ERROR( logger_,
         msg << xcept::stdformat_exception_history(e) );

      XCEPT_RETHROW( rubuilder::filter::exception::FSM, msg, e );
   }

   try
   {
      toolbox::Event::Reference evtRef( new toolbox::Event( event, this ) );

      // L'esecuzione della routine relativa al singolo comando invocato da rc-
      //-è protetta da applicationBSem_.
      applicationBSem_.take();

      fsm_.fireEvent( evtRef );

      applicationBSem_.give();
   }
   catch( xcept::Exception &e )
   {
      applicationBSem_.give();

      std::stringstream oss;

      oss << "Failed to process FSM event: " << event;

      LOG4CPLUS_ERROR( logger_,
         oss.str() << xcept::stdformat_exception_history(e) );

      //XCEPT_RETHROW( rubuilder::filter::exception::FSM, oss.str(), e );

      XCEPT_RETHROW( xoap::exception::Exception, oss.str(), e );
   }

   if( fsm_.getCurrentState() == 'F' ) {
      XCEPT_RAISE( xoap::exception::Exception, reasonForFailed_.getValue() );
   }

   return rubuilder::utils::createFsmSoapResponseMsg( event, stateName_.toString() );
}


void rubuilder::filter::SuperApplication::failAction( toolbox::Event::Reference event )
{
   std::string reason = "";
   xcept::Exception exception;

   // Determine reason for failure if there is one
   if( typeid(*event) == typeid( toolbox::fsm::FailedEvent ) )
   {
      toolbox::fsm::FailedEvent &failedEvent =
         dynamic_cast<toolbox::fsm::FailedEvent&>(*event);

      exception = failedEvent.getException();

      std::stringstream oss;

      oss << "Failure occurred when performing transition from: ";
      oss << failedEvent.getFromState();
      oss <<  " to: ";
      oss << failedEvent.getToState();

      reason = oss.str();
   }
   else if( typeid(*event) == typeid( rubuilder::utils::ForceFailedEvent ) )
   {
      rubuilder::utils::ForceFailedEvent &forceFailedEvent =
         dynamic_cast<rubuilder::utils::ForceFailedEvent&>(*event);

      reason = forceFailedEvent.getReason();

      exception = forceFailedEvent.getException();
   }

   // Update web-page data
   reasonForFailed_.setValue( reason );

   // Prepare error message for logging and the sentinel
   if( reason != "" )
   {
      std::string errorMessage = "Moved to failed state: " + reason;

      LOG4CPLUS_FATAL( logger_, errorMessage
         << xcept::stdformat_exception_history( exception ) );
   }
   else
   {
      std::string errorMessage = "Moved to failed state";

      LOG4CPLUS_FATAL( logger_, errorMessage );
   }
}


void rubuilder::filter::SuperApplication::configureAction( toolbox::Event::Reference e )
{
   int error_code;
   const char *path;

   LOG4CPLUS_INFO( logger_, "configureAction fired" );


/*
   std::cout << "output_buffer_size_: " << output_buffer_size_ << std::endl;

   output_buffer_ = new char[ output_buffer_size_ ];

   if( (uint32_t)output_buffer_size_ < BLOCKSIZE )
   {
      const std::string msg = "output_buffer size < input_buffer size: ";

      LOG4CPLUS_ERROR( logger_, msg << output_buffer_size_ << "," << BLOCKSIZE );

      XCEPT_RAISE( rubuilder::filter::exception::Configuration, msg );
   }
*/


   try
   {
      this->getAppDescriptorsAndTids();

      // Create Application Memory Pool
      //int committedPoolSize = 0xC800000; // 200Mb
      //pool_ = createApplicationMemoryPool( committedPoolSize );

      pool_ = createApplicationMemoryPool( COMMITTED_POOL_SIZE );

      LOG4CPLUS_INFO( logger_, "Application Memory Pool instantiated" );

      // Config file path
      configFilepath_ = xconfigFilepath_.toString();

      path = configFilepath_.c_str();

      this->process_config( path, &error_code );
   }
   catch( xcept::Exception &e )
   {
      const std::string msg = "configureAction failed ";

      LOG4CPLUS_ERROR( logger_,
         msg << xcept::stdformat_exception_history( e ) );

      XCEPT_RETHROW( toolbox::fsm::exception::Exception, msg, e );
   }
}


void rubuilder::filter::SuperApplication::haltAction( toolbox::Event::Reference e )
{

   LOG4CPLUS_INFO( logger_, "haltAction fired" );


   try
   {
      if( output_buffer_ ) {
         delete [] output_buffer_;
         output_buffer_ = NULL;
      }

      this->clearMonitoringVars();

      toolbox::net::URN urn( "toolbox-mem-pool", poolName_ );

      poolFactory_->destroyPool( urn );

      if( allocator_ ) {
         delete allocator_;
      }

      pool_ = NULL;


      if( ptSrv_ )
      {
         delete ptSrv_;
         ptSrv_ = NULL;
      }


      state_.set( STATE_DISABLED );
   }
   catch( xcept::Exception &e )
   {
      const std::string msg = "haltAction failed ";

      LOG4CPLUS_ERROR( logger_,
         msg << xcept::stdformat_exception_history( e ) );

      XCEPT_RETHROW( toolbox::fsm::exception::Exception, msg, e );
   }
}


void rubuilder::filter::SuperApplication::enableAction( toolbox::Event::Reference e )
{
   time_t rawtime;
   //struct tm *timeinfo;
   std::string str;
   size_t pos;
   char buffer[80];
   struct tm *timeinfo;

   int error_code;

   std::stringstream stmss;

   std::stringstream fnss;
   std::string outputFileStr;

   LOG4CPLUS_INFO( logger_, "enableAction fired" );
   firstBlock=true;

   try
   {
      std::cout << "output_buffer_size_: " << output_buffer_size_ << std::endl;

      output_buffer_ = new char[ output_buffer_size_ ];

      if( (uint32_t)output_buffer_size_ < BLOCKSIZE )
      {
         const std::string msg = "output_buffer size < input_buffer size: ";

         LOG4CPLUS_ERROR( logger_, msg << output_buffer_size_ << "," << BLOCKSIZE );

         XCEPT_RAISE( rubuilder::filter::exception::Configuration, msg );
      }


      std::cout << "chain_buffer_size_: " << chain_buffer_size_ << std::endl;

      chain_buffer_ = new char[ chain_buffer_size_ ];

      chain_size_ = 0;

      max_chain_size_ = 0;

      postbuf_len_ = 0;

      postbufRef_ = NULL;

      // Il filtro ha un sola sorgente in input
      stopBlockExpected_ = 1;

      // Input data file path
      inputFilepath_ = xinputFilepath_.toString();

      // runNumber_ modifications take effect only after enableAction
      RunNum_ = runNumber_;

      //cycnt_ = (uint32_t)xoutputFileCycleCounter_;
      cycnt_ = 0;

      // MB
      outFileMaxSize_ = (uint64_t)xoutputFileSizeLimit_;

      // Bytes
      outFileMaxSize_ = outFileMaxSize_*1048576;

      ancillaryTcpPort_ = (uint32_t)xancillaryTcpPort_;

      // Commentato il 25/05/2016.
      // Perchè la porta veniva ridefinita staticamente?
      //ancillaryTcpPort_ = 9999;

      //ptSrv_ = new ptServer( this, ancillaryTcpPort_, logger_ );

      if( ptSrv_ )
         ptSrv_->addListener( this );


/*
 Ogni istanza di RU può gestire output diversi:
 1 - Merge
 2 - Output File
 3 - Spy socket
 Scelta e configurazione degli output da RCMS o in via preliminare
 usando variabili d'ambiente o text file.
*/

      num_out_ = 0;

      out_[num_out_] = NULL;

      // Output
      for( unsigned int i=0; i<xOutput_.size(); i++ )
      {
         // To upper
         Output_ = (std::string)xOutput_[i];
         std::transform( Output_.begin(), Output_.end(), Output_.begin(), ::toupper );

         std::cout << "Output_:" << Output_ << std::endl;

         if( Output_ == "NETWORK" )
         {
            if( buInstances_.size() || filterInstances_.size() || mergerInstances_.size() )
            {
               out_bf_ = new outproxy( this, &applicationBSem_, logger_ );

               out_bf_->configure( buInstances_, filterInstances_, mergerInstances_ );

               out_[num_out_++] = out_bf_;
            }

/*
            if( buInstances_.size() )
            {
               out_bu_ = new BUproxy( this, &applicationBSem_, logger_ );

               out_bu_->configure( buInstances_ );

               out_[num_out_++] = out_bu_;
            }

            if( filterInstances_.size() )
            {
               out_filter_ = new FILTERproxy( this, &applicationBSem_, logger_ );

               out_filter_->configure( filterInstances_ );

               out_[num_out_++] = out_filter_;
            }
*/

         }


         if( Output_ == "LOCAL" )
         {

//
            std::fstream iofile;

            std::stringstream fss;

            std::string appName;


            if( xmlClass_ == "LocalFilter" )
               appName = ".lf";
            else if( xmlClass_ == "GlobalFilter" )
               appName = ".gf";
            else
               std::cout << "Application not recognized" << std::endl;

            fss << homedir_ << appName << instance_;


            std::string ofileIdpath = fss.str();

            LOG4CPLUS_INFO( logger_, "ofileIdpath: " << ofileIdpath.c_str() );

            iofile.clear();

            int lsid = 0;

            iofile.open( ofileIdpath.c_str() );

            if( !iofile.is_open() )
            {
               iofile.clear();
            
               // Create file
               iofile.open( ofileIdpath.c_str(), std::ios::out );

               iofile << lsid << std::endl;
 
               iofile.close();

               iofile.open( ofileIdpath.c_str() );
            }

            iofile >> lsid;

            std::cout << "ofsid: " << lsid << std::endl;

            iofile.close();

            // ofsid update
            iofile.open( ofileIdpath.c_str(), std::ios::out | std::ios::trunc );

            iofile << (lsid+1) << std::endl;

            iofile.close();
//

            // Data file path
            outputFilepath_ = xoutputFilepath_.toString();

            outputFileStr = outputFilepath_;

            // Get file name only
            // position of "/" in outputFilepath_
            str = outputFilepath_;
            pos = outputFilepath_.find("/");
            while( pos != std::string::npos )
            {
               // Get after "/" to the end
               str = str.substr (pos+1); 
               pos = str.find("/");
            }

            std::cout << "Filename only: " << str << std::endl;


/*
            // Path + filename
            // Path e filename separati come forniti da rcms
            fnss << outputFilepath_.toString() << outputFilename_.toString();

            outputFileStr = fnss.str();
*/


/*
            // Generate data file full path
            outputFullFilepath_= rubuilder::utils::generateOutputFileName
            (
               outputFilepath_,
               xmlClass_, instance_,
               RunNum_,
               cycnt_
            );
*/

            // Generate data file full path
            // Galileo format: filename + hostname + appl-name + appl-instance + file seq-id + Run-number + cycle-counter
            // output string in Agata format: filename + _i<file seq-id> + _<Run-number> + _<cycle-counter> + .<data-type-ext>
	    // cycnt_ = 0
            outputFullFilepath_= rubuilder::utils::generateOutputFileName
            (
               outputFilepath_,
               hostname_,
               xmlClass_,
               instance_,
               lsid,
               RunNum_,
               cycnt_
            );

/* outputFullFilepath_: output from rubuilder::utils::generateOutputFileName
ffnss << outputFilepath
      << "_i" << lsid 
      << "_"  << std::setfill('0') << std::setw(4) << runNum 
      << "_"  << std::setfill('0') << std::setw(4) << index 
      <<  "." << ext;
*/
            LOG4CPLUS_INFO( logger_, "outputFullFilepath_: " << outputFullFilepath_ );

            LOG4CPLUS_INFO( logger_, "xwriteDataFile_: " << xwriteDataFile_  );
	    LOG4CPLUS_INFO( logger_, "xidleEventOnFile_: " << xidleEventOnFile_  );


            if( xwriteDataFile_ && (outputFilepath_ != "") )
            {
               // output string in Agata format: filename + _i<file seq-id>
               // outputFileExt filled in generateOutputFileName
               // Run-number, cycle-counter and .<outputFileExt> added in DFproxy
	       std::string outputFileExt;
               outputFileStr = rubuilder::utils::generateOutputFileName
               (
                  outputFilepath_,
                  xmlClass_,
                  lsid,
		  &outputFileExt
               );

	       std::cout << "File base name = " << outputFileStr << std::endl;
	       std::cout << "file extension = " << outputFileExt << std::endl;

	       if( xidleEventOnFile_ )
	       {

		  // IDLE events on disk file
		  out_df_id_ = new DFproxy
		  (
		     this, &applicationBSem_,
		     outputFileStr, outputFileExt, RunNum_, outFileMaxSize_, xoutputFileCycleCounter_,
		     logger_
		  );

		  out_[num_out_++] = out_df_id_;
		   
		  out_df_ = out_df_id_;
	       }
	       else
	       {
	          // NO-IDLE events on disk file                                           
	          out_df_nid_ = new DFNIDproxy
		  (
		     this, &applicationBSem_,
		     outputFileStr, outputFileExt, RunNum_, outFileMaxSize_, xoutputFileCycleCounter_,
		     logger_
		  );
		   
		  out_[num_out_++] = out_df_nid_;
		   
		  out_df_ = out_df_nid_;
               }
	    }//if( (uint32_t)xwriteDataFile_ && (outputFilepath_ != "") )
     
	 }//if( Output_ == "LOCAL" )

      }//for( unsigned int i=0; i<xOutput_.size(); i++ )

      out_[num_out_] = NULL;

      block_seq_nb_ = 0;

      this->clearMonitoringVars();

      for( int i=0; i<num_out_; i++ ) {
         out_[i]->enable();
      }


      //void LocalFilter::process_config(const char* fname, int *error_code)
      //worker = new LocalWorkerPRISMA(cServer, tcpPortno);
      this->process_start( RunNum_, &error_code );


      state_.set( STATE_ENABLED );


      std::time( &rawtime );

      timeinfo = localtime( &rawtime );

      strftime( buffer, 80, "%b %d %T %Y", timeinfo );

      //stmss << std::asctime( std::localtime(&rawtime) );

      stmss << buffer;

      startTimeOfRunString_ = stmss.str();

      stopTimeOfRunString_ = "---";

      LOG4CPLUS_INFO( logger_, "enableAction complete" );
   }
   catch( xcept::Exception &e )
   {
      const std::string msg = "enableAction failed ";

      LOG4CPLUS_ERROR( logger_,
         msg << xcept::stdformat_exception_history( e ) );

      XCEPT_RETHROW( toolbox::fsm::exception::Exception, msg, e );
   }
}


void rubuilder::filter::SuperApplication::disableAction( toolbox::Event::Reference e )
{
   //uint32_t timeout=10000; // msec
   uint32_t timeout=2000; // msec
   uint32_t referenceValue = 0;
   int ret;
   int error_code;

   time_t rawtime;
   std::stringstream stmss;
   char buffer[80];
   struct tm * timeinfo;


   LOG4CPLUS_INFO( logger_, "disableAction fired" );


   // pollMethod = POLL_UNTIL_EQUAL
   ret = pollItem( &stopBlockExpected_, referenceValue, timeout );

   if( ret ) {
      LOG4CPLUS_INFO( logger_, "Timeout elapsed waiting for stopBlock... " );
   }
   else {
      LOG4CPLUS_INFO( logger_, "stopBlock detected... " );
   }


   // TAKE
   monitoringBSem_.take();

   try
   {
      for( int i=0; i<num_out_; i++ ) {
         out_[i]->disable();
      }

      LOG4CPLUS_INFO( logger_, "Disable out, stop queue " );


      for( int i=0; i<num_out_; i++ )
      {
         if( out_[i] )
         {
            delete out_[i];
            out_[i] = NULL;
         }
      }

      LOG4CPLUS_INFO( logger_, "Delete out, delete queue " );

      out_bf_ = NULL;

      out_df_ = NULL;


      num_out_ = 0;


      this->process_stop( &error_code );


      if( ptSrv_ )
      {
         delete ptSrv_;
         ptSrv_ = NULL;
      }


      if( output_buffer_ )
      {
         delete [] output_buffer_;
         output_buffer_ = NULL;
      }


      if( chain_buffer_ )
      {
         delete [] chain_buffer_;
         chain_buffer_ = NULL;
      }


      state_.set( STATE_DISABLED );

      // GIVE	
      monitoringBSem_.give();


      this->clearMonitoringVars();


      std::time( &rawtime );

      timeinfo = localtime( &rawtime );

      strftime( buffer, 80, "%b %d %T %Y", timeinfo );

      //stmss << std::asctime( std::localtime(&rawtime) );

      stmss << buffer;

      stopTimeOfRunString_ = stmss.str();

      LOG4CPLUS_INFO( logger_, "disableAction done " );
   }
   catch( xcept::Exception &e )
   {
      const std::string msg = "disableAction failed ";

      LOG4CPLUS_ERROR( logger_,
         msg << xcept::stdformat_exception_history( e ) );

      // GIVE	
      monitoringBSem_.give();

      XCEPT_RETHROW( toolbox::fsm::exception::Exception, msg, e );
   }
}


void rubuilder::filter::SuperApplication::haltActionFromRunning( toolbox::Event::Reference e )
{
   uint32_t timeout = 10000; // msec
   uint32_t referenceValue = 0;
   int ret;
   int error_code;

   time_t rawtime;
   std::stringstream stmss;
   char buffer [80];
   struct tm * timeinfo;


   LOG4CPLUS_INFO( logger_, "haltActionFromRunning fired" );


   // pollMethod = POLL_UNTIL_EQUAL
   ret = pollItem( &stopBlockExpected_, referenceValue, timeout );

   if( ret ) {
      LOG4CPLUS_INFO( logger_, "Timeout waiting for stopBlock..." );
   }
   else {
      LOG4CPLUS_INFO( logger_, "stopBlock detected..." );
   }


   // TAKE
   monitoringBSem_.take();

   try
   {
      for( int i=0; i<num_out_; i++ ) {
         out_[i]->disable();
      }

      LOG4CPLUS_INFO( logger_, "out_[i]->disable() exit" );


      for( int i=0; i<num_out_; i++ )
      {
         if( out_[i] )
         {
            delete out_[i];
            out_[i] = NULL;
         }
      }

      LOG4CPLUS_INFO( logger_, "delete out_[i] exit" );

      out_bf_ = NULL;

      out_df_ = NULL;


      num_out_ = 0;


      this->process_stop( &error_code );


      toolbox::net::URN urn( "toolbox-mem-pool", poolName_ );

      poolFactory_->destroyPool( urn );

      if( allocator_ ) {
         delete allocator_;
      }

      pool_ = NULL;


      if( output_buffer_ )
      {
         delete [] output_buffer_;
         output_buffer_ = NULL;
      }


      if( chain_buffer_ )
      {
         delete [] chain_buffer_;
         chain_buffer_ = NULL;
      }


      if( ptSrv_ )
      {
         delete ptSrv_;
         ptSrv_ = NULL;
      }


      state_.set( STATE_DISABLED );

      // GIVE	
      monitoringBSem_.give();


      this->clearMonitoringVars();


      std::time( &rawtime );

      timeinfo = localtime( &rawtime );

      strftime( buffer, 80, "%b %d %T %Y", timeinfo );

      //stmss << std::asctime( std::localtime(&rawtime) );

      stmss << buffer;

      stopTimeOfRunString_ = stmss.str();

      LOG4CPLUS_INFO( logger_, "haltActionFromRunning done" );
   }
   catch( xcept::Exception &e )
   {
      const std::string msg = "haltActionFromRunning failed ";

      LOG4CPLUS_ERROR( logger_,
         msg << xcept::stdformat_exception_history( e ) );

      // GIVE	
      monitoringBSem_.give();

      XCEPT_RETHROW( toolbox::fsm::exception::Exception, msg, e );
   }
}


void rubuilder::filter::SuperApplication::pauseAction( toolbox::Event::Reference e )
{
}


void rubuilder::filter::SuperApplication::resumeAction( toolbox::Event::Reference e )
{
}


void rubuilder::filter::SuperApplication::bindI2oCallbacks()
{

   i2o::bind
   (
      this,
      &rubuilder::filter::SuperApplication::I2O_FILTER_COLLECT_Callback,
      I2O_FILTER_COLLECT,
      XDAQ_ORGANIZATION_ID
   );

   i2o::bind
   (
      this,
      &rubuilder::filter::SuperApplication::I2O_FILTER_COLLECT_CHAIN_Callback,
      I2O_FILTER_COLLECT_CHAIN,
      XDAQ_ORGANIZATION_ID
   );
}


/*
 * All memory used for message passing is organized around pools of toolbox::mem::References,-
 *-toolbox::mem::Buffer structures and associated memory blocks.
 */
void rubuilder::filter::SuperApplication::I2O_FILTER_COLLECT_Callback( toolbox::mem::Reference *bufRef )
{
   I2O_EVENT_DATA_BLOCK_MESSAGE_FRAME *block=NULL;
   I2O_MESSAGE_FRAME *stdMsg=NULL;

   I2O_EVENT_DATA_BLOCK_MESSAGE_FRAME *postblock=NULL;
   I2O_MESSAGE_FRAME *poststdMsg=NULL;

   int src;
   int id;

   char *input_buffer;
   int input_size;

   U32 *frg;
   int frg_len;

   int used_size_of_output_buffer=0;
   int error_code=0;

   char *postbuf;

   int stopBlock;

   int packet_ID;


   //LOG4CPLUS_INFO( logger_, "I2O_FILTER_COLLECT_Callback" );


   block  = (I2O_EVENT_DATA_BLOCK_MESSAGE_FRAME *)bufRef->getDataLocation();
   stdMsg = (I2O_MESSAGE_FRAME *)block;

   stopBlock = block->stopBlock;


   // TAKE
   monitoringBSem_.take();

   // if the state machine is not running 
   if( (fsm_.getCurrentState() != 'R') || state_.test(STATE_DISABLED) )		
   {
      LOG4CPLUS_INFO( logger_, "fsm currentState not R " );

      if( stopBlock ) {
         LOG4CPLUS_INFO( logger_, "fsm currentState not R, stopBlock incoming " );
      }

      if( bufRef )
      {
         bufRef->release();
         bufRef = NULL;

         //std::cout << "fsm currentState not R" << std::endl;
      }

      // GIVE
      monitoringBSem_.give();

      return;
   }


   try
   {
      // id sorgente che ha spedito il frame
      id = block->sourceId;
      //LOG4CPLUS_INFO( logger_, "id: " << id );

      // instance sorgente che ha spedito il frame
      src = block->sourceInstance;
      //LOG4CPLUS_INFO( logger_, "Instance: " << src );

      // packetID: 0/Short, 0x50/Long, 0x60/Spectra
      packet_ID = block->packetType;
      //LOG4CPLUS_INFO( logger_, "Instance: " << src );


      if( stopBlock )
      {
         LOG4CPLUS_INFO( logger_, "stopBlock coming from readout-unit " );

         // Flush last data buffer (dati già copiati)
         if( postbufRef_ != NULL )
         {
            postblock  = (I2O_EVENT_DATA_BLOCK_MESSAGE_FRAME *)postbufRef_->getDataLocation();

            poststdMsg = (I2O_MESSAGE_FRAME *)postblock;

            poststdMsg->MessageSize = ( BLOCKSIZE - postbuf_len_ ) >> 2;

            std::cout << "poststdMsg->MessageSize: " << poststdMsg->MessageSize << std::endl;

            try
            {
               LOG4CPLUS_INFO( logger_, "Last buff must be dumped " );

               postDataForward( postbufRef_ );

               postbufRef_ = NULL;
            }
            catch( xcept::Exception& e )
            {
               if( bufRef )
               {
                  bufRef->release();
                  bufRef = NULL;
               }
               // GIVE
               monitoringBSem_.give();

               XCEPT_RETHROW( rubuilder::filter::exception::Exception,
                  "Failed forwarding last postbufRef_ ", e );
            }
         } // if( postbufRef_ != NULL )


         // Stop Block
         try
         {
            postDataForward( bufRef );

            LOG4CPLUS_INFO( logger_, "stopBlock forwarded " );
         }
         catch( xcept::Exception& e )
         {
            // GIVE
            monitoringBSem_.give();

            XCEPT_RETHROW( rubuilder::filter::exception::Exception,
               "Failed forwarding stopBlock buffer ", e );
         }

         stopBlockExpected_--;

         // GIVE
         monitoringBSem_.give();

         return;
      }//if( stopBlock )


      // Vars for monitoring
      nbuffs_++;
      sumOfSizes_ += ( stdMsg->MessageSize << 2 );


      input_buffer = (char *)block;

      input_buffer = &input_buffer[sizeof( I2O_EVENT_DATA_BLOCK_MESSAGE_FRAME )];

      input_size = ( stdMsg->MessageSize << 2 ) - sizeof( I2O_EVENT_DATA_BLOCK_MESSAGE_FRAME );

      //std::cout << "input_size: " << input_size << std::endl;
      if( firstBlock )
      {
	FILE* fout = fopen( "/onlineANCILLARY/xDaqCurrentRun/Data/caen_digitizer/firstBlock_SA.caendat", "wb" );
	fwrite( input_buffer, 1, input_size, fout );
	fclose( fout );
	firstBlock = false;
      }

      //LOG4CPLUS_INFO( logger_, "Calling LocalFilter::process_block with input size = " << input_size << std::endl );

      process_block( (void *)input_buffer, input_size, packet_ID, 
		     (void *)output_buffer_, output_buffer_size_, &used_size_of_output_buffer, 
		     &error_code );


      chain_size_ = input_size;

      // Save chain max size
      if( max_chain_size_ < chain_size_ )
         max_chain_size_ = chain_size_;

      // 07/06/2023: Fix data corruption when a single block is followed by a chain
      chain_size_ = 0;
      
      // Copia di output_buffer sapendo che contiene solo eventi completi. L'eventuale evento non completo dovrebbe essere gestito-
      //-in process_block. Al return di process_block il buffer di input è stato esaurito.
      // Formato eventi dopo process_block?, prima word è size evento.

      //std::cout << "used_size_of_output_buffer: " << used_size_of_output_buffer << std::endl;

      frg = ( U32* )output_buffer_;

      while( used_size_of_output_buffer )
      {
         // Length in byte dell' evento letto dal buf di input, prima word evento
         frg_len = *frg;
         //std::cout << "frg_len: 0x" << std::hex << frg_len << std::dec << std::endl;

         if( postbuf_len_ >= frg_len )
         {
            //std::cout << "postbuf_len:" << std::dec << postbuf_len_ << " frg_len:" << frg_len << std::endl;

	    // Copy evt by evt from output_buffer to postbuf
	    for( int j=0; j<frg_len/4; j++ ) {
               //std::cout << std::hex << *inFrg << std::dec << std::endl;
	       *postfrg_++ = *frg++;
	    }

            postbuf_len_ -= frg_len;

            used_size_of_output_buffer -= frg_len;
         }
         else  // ( postbuf_len_ < frg_len )
         {
            if( postbufRef_ != NULL )
            {
               block  = ( I2O_EVENT_DATA_BLOCK_MESSAGE_FRAME* )postbufRef_->getDataLocation();

               stdMsg = ( I2O_MESSAGE_FRAME* )block;

               stdMsg->MessageSize = ( BLOCKSIZE - postbuf_len_ ) >> 2;

               try
               {
                  postDataForward( postbufRef_ );

                  postbufRef_ = NULL;
               }
               catch( xcept::Exception& e )
               {
                  if( bufRef )
                  {
                     bufRef->release();
                     bufRef = NULL;
                  }

                  // GIVE
                  monitoringBSem_.give();

                  XCEPT_RETHROW( rubuilder::filter::exception::Exception,
                     "Failed forwarding data ", e );
               }
            }


            // Allocazione di un nuovo postbuf
            try
            {        
               postbufRef_ = poolFactory_->getFrame( pool_, BLOCKSIZE );

               block  = ( I2O_EVENT_DATA_BLOCK_MESSAGE_FRAME* )postbufRef_->getDataLocation();

               block->packetType = packet_ID;

               block->stopBlock = 0;

               stdMsg = ( I2O_MESSAGE_FRAME* )block;

               postbuf = (char* )block;

               postbuf = &postbuf[sizeof( I2O_EVENT_DATA_BLOCK_MESSAGE_FRAME )];

               postfrg_ = ( U32* )postbuf;

               //sizeof( I2O_EVENT_DATA_BLOCK_MESSAGE_FRAME ) = 64
               postbuf_len_ = BLOCKSIZE - sizeof( I2O_EVENT_DATA_BLOCK_MESSAGE_FRAME );
            }
            catch( toolbox::mem::exception::Exception& e )
            {
	       //LOG4CPLUS_INFO( logger_, "Failed to allocate memory: " << xcept::stdformat_exception_history(e) );

               if( postbufRef_ )
               {
                  postbufRef_->release();
                  postbufRef_ = NULL;
               }

                if( bufRef )
                {
                   bufRef->release();
                   bufRef = NULL;
                }

               // GIVE
               monitoringBSem_.give();

               XCEPT_RETHROW( toolbox::mem::exception::Exception, "Failed to allocate memory: ",  e );
            }
         }
      } // while( used_size_of_output_buffer )

      if( bufRef )
      {
         bufRef->release();

         bufRef = NULL;
      }

      // GIVE
      monitoringBSem_.give();
   }
   catch( xcept::Exception& e )
   {
      std::string msg = "Error on I2O_FILTER_COLLECT_Callback function ";

      if( bufRef )
      {
         bufRef->release();
         bufRef = NULL;
      }

      // GIVE
      monitoringBSem_.give();

      LOG4CPLUS_ERROR( logger_, msg << xcept::stdformat_exception_history( e ) );
   }
}


void rubuilder::filter::SuperApplication::I2O_FILTER_COLLECT_CHAIN_Callback( toolbox::mem::Reference *bufRef )
{
   I2O_EVENT_DATA_BLOCK_MESSAGE_FRAME *block=NULL;
   I2O_MESSAGE_FRAME *stdMsg=NULL;

   I2O_EVENT_DATA_BLOCK_MESSAGE_FRAME *postblock=NULL;
   I2O_MESSAGE_FRAME *poststdMsg=NULL;

   int src;
   int id;

   char *input_buffer;
   int input_size;

   U32 *frg;
   int frg_len;

   int used_size_of_output_buffer=0;
   int error_code=0;

   char *postbuf;

   int stopBlock;

   int packet_ID;

   uint32_t chain;


   //LOG4CPLUS_INFO( logger_, "I2O_FILTER_COLLECT_CHAIN_Callback" );


   block  = ( I2O_EVENT_DATA_BLOCK_MESSAGE_FRAME* )bufRef->getDataLocation();
   stdMsg = ( I2O_MESSAGE_FRAME* )block;

   stopBlock = block->stopBlock;


   // TAKE
   monitoringBSem_.take();

   // if the state machine is not running 
   if( (fsm_.getCurrentState() != 'R') || state_.test(STATE_DISABLED) )		
   {
      LOG4CPLUS_INFO( logger_, "fsm currentState not R " );

      if( stopBlock ) {
         LOG4CPLUS_INFO( logger_, "fsm currentState not R, stopBlock incoming " );
      }

      if( bufRef )
      {
         bufRef->release();
         bufRef = NULL;

         //std::cout << "fsm currentState not R" << std::endl;
      }

      // GIVE
      monitoringBSem_.give();

      return;
   }


   try
   {
      // id sorgente che ha spedito il frame
      id = block->sourceId;
      //LOG4CPLUS_INFO( logger_, "id: " << id );

      // instance sorgente che ha spedito il frame
      src = block->sourceInstance;
      //LOG4CPLUS_INFO( logger_, "Instance: " << src );

      // packetID: 0/Short, 0x50/Long, 0x60/Spectra
      packet_ID = block->packetType;
      //LOG4CPLUS_INFO( logger_, "Instance: " << src );

      // chain: 0/Single block, 1/Chain block, 2/Chain last block
      chain = block->chain;
      //LOG4CPLUS_INFO( logger_, "chain: " << chain );


      // Vars for monitoring
      nbuffs_++;
      sumOfSizes_ += ( stdMsg->MessageSize << 2 );


      input_buffer = ( char* )block;

      input_buffer = &input_buffer[sizeof( I2O_EVENT_DATA_BLOCK_MESSAGE_FRAME )];

      input_size = ( stdMsg->MessageSize << 2 ) - sizeof( I2O_EVENT_DATA_BLOCK_MESSAGE_FRAME );

      //std::cout << "input_size: " << input_size << std::endl;


      if( firstBlock )
      {
	FILE* fout = fopen( "/onlineANCILLARY/xDaqCurrentRun/Data/caen_digitizer/firstBlock_SA.caendat", "wb" );
	fwrite( input_buffer, 1, input_size, fout );
	fclose( fout );
	firstBlock = false;
      }

 
      memcpy( &chain_buffer_[chain_size_], input_buffer, input_size );

      chain_size_ += input_size;

      // Save chain max size
      if( max_chain_size_ < chain_size_ )
         max_chain_size_ = chain_size_;

      if( chain != 2 )	// Chain Block, not last
      {
         if( bufRef )
         {
            bufRef->release();
            bufRef = NULL;
         }

         // GIVE
         monitoringBSem_.give();

         return;
      }


      // ONLY IF LAST BLOCK OF THE CHAIN


      process_block( (void *)chain_buffer_, chain_size_, packet_ID, (void *)output_buffer_, output_buffer_size_, &used_size_of_output_buffer, &error_code );


      chain_size_ = 0;

      // Copia di output_buffer sapendo che contiene solo eventi completi. L'eventuale evento non completo dovrebbe essere gestito-
      //-in process_block. Al return di process_block il buffer di input è stato esaurito.
      // Formato eventi dopo process_block?, prima word è size evento.

      //std::cout << "used_size_of_output_buffer: " << used_size_of_output_buffer << std::endl;

      frg = ( U32* )output_buffer_;

      while( used_size_of_output_buffer )
      {
         // Length in byte dell' evento letto dal buf di input, prima word evento
         frg_len = *frg;
         //std::cout << "frg_len: 0x" << std::hex << frg_len << std::dec << std::endl;

         if( postbuf_len_ >= frg_len )
         {
            //std::cout << "postbuf_len:" << std::dec << postbuf_len_ << " frg_len:" << frg_len << std::endl;

	    // Copy from output_buffer to postbuf
	    for( int j=0; j<frg_len/4; j++ ) {
               //std::cout << std::hex << *inFrg << std::dec << std::endl;
	       *postfrg_++ = *frg++;
	    }

            postbuf_len_ -= frg_len;

            used_size_of_output_buffer -= frg_len;
         }
         else  // ( postbuf_len_ < frg_len )
         {
            if( postbufRef_ != NULL )
            {
               block  = ( I2O_EVENT_DATA_BLOCK_MESSAGE_FRAME* )postbufRef_->getDataLocation();

               stdMsg = ( I2O_MESSAGE_FRAME* )block;

               stdMsg->MessageSize = ( BLOCKSIZE - postbuf_len_ ) >> 2;

               try
               {
                  postDataForward( postbufRef_ );

                  // postbufRef_ set to NULL solo dopo postDataForward
                  postbufRef_ = NULL;
               }
               catch( xcept::Exception &e )
               {
                  if( bufRef )
                  {
                     bufRef->release();
                     bufRef = NULL;
                  }

                  // GIVE
                  monitoringBSem_.give();

                  XCEPT_RETHROW( rubuilder::filter::exception::Exception,
                     "Failed forwarding postbufRef_ ", e );
               }
            }


            // Allocazione di un nuovo postbuf
            try
            {        
               postbufRef_ = poolFactory_->getFrame( pool_, BLOCKSIZE );

               block  = ( I2O_EVENT_DATA_BLOCK_MESSAGE_FRAME* )postbufRef_->getDataLocation();

               block->stopBlock = 0;

               stdMsg = ( I2O_MESSAGE_FRAME* )block;

               postbuf = ( char* )block;

               postbuf = &postbuf[sizeof( I2O_EVENT_DATA_BLOCK_MESSAGE_FRAME )];

               postfrg_ = ( U32* )postbuf;

               //sizeof( I2O_EVENT_DATA_BLOCK_MESSAGE_FRAME ) = 48
               postbuf_len_ = BLOCKSIZE - sizeof( I2O_EVENT_DATA_BLOCK_MESSAGE_FRAME );
            }
            catch( toolbox::mem::exception::Exception& e )
            {
	       //LOG4CPLUS_INFO( logger_, "Failed to allocate memory: " << xcept::stdformat_exception_history(e) );

               if( postbufRef_ )
               {
                  postbufRef_->release();
                  postbufRef_ = NULL;
               }

                if( bufRef )
                {
                   bufRef->release();
                   bufRef = NULL;
                }

               // GIVE
               monitoringBSem_.give();

               XCEPT_RETHROW( toolbox::mem::exception::Exception, "Failed to allocate memory: ",  e );
            }
         }
      } // while( used_size_of_output_buffer )

      if( bufRef )
      {
         bufRef->release();

         bufRef = NULL;
      }

      // GIVE
      monitoringBSem_.give();
   }
   catch( xcept::Exception &e )
   {
      std::string msg = "Error on I2O_FILTER_COLLECT_CHAIN_Callback function ";

      if( bufRef )
      {
         bufRef->release();
         bufRef = NULL;
      }

      // GIVE
      monitoringBSem_.give();

      LOG4CPLUS_ERROR( logger_, msg << xcept::stdformat_exception_history( e ) );
   }
}


void rubuilder::filter::SuperApplication::postDataForward( toolbox::mem::Reference *buf_ref )
{
   toolbox::mem::Reference *bufRef=0;

   if( buf_ref )
   {
      try
      {
         // timeout=0, call bloccante se non c'è spazio
         // Exit if "buf_refq_->stop()"
         for( int i=0; i<num_out_; i++ )
         {
            bufRef = buf_ref->duplicate();

            if( out_[i] ) {
               out_[i]->put( bufRef, 0 );
            }
         }

         if( buf_ref )
         {
            buf_ref->release();
            buf_ref = NULL;
         }
      }
      catch( BufRefQ::Stopped )
      {
         LOG4CPLUS_INFO( logger_, "BufRefQ::Stopped, free buf_ref and bufRef " );

         if( buf_ref )
         {
            buf_ref->release();
            buf_ref = NULL;
         }

/* 
 * Dangerous, SEGSEV
         if( bufRef )
         {
            bufRef->release();
            bufRef = NULL;
         }
*/
      }
      catch( xcept::Exception &e )
      {
         LOG4CPLUS_ERROR( logger_, e.what() );

         if( buf_ref )
         {
            buf_ref->release();
            buf_ref = NULL;
         }

/* 
 * Dangerous, SEGSEV
         if( bufRef )
         {
            bufRef->release();
            bufRef = NULL;
         }
*/

         XCEPT_RETHROW( rubuilder::filter::exception::Exception,
            "Failed forwarding data ", e );
      }
   }
}


toolbox::mem::Pool* rubuilder::filter::SuperApplication::createApplicationMemoryPool( size_t committedPoolSize )
{

   try
   {
      toolbox::net::URN urn( "toolbox-mem-pool", poolName_ );

      allocator_ = new toolbox::mem::CommittedHeapAllocator( committedPoolSize );

      pool_ = poolFactory_->createPool( urn, allocator_ );

      return pool_;
   }
   catch( xcept::Exception &e )
   {
      const std::string msg = "Failed to create memory pool: " + poolName_;

      LOG4CPLUS_FATAL( logger_,
         msg << xcept::stdformat_exception_history(e) );

      XCEPT_RETHROW( rubuilder::filter::exception::Memory, msg, e );
   }
}


void rubuilder::filter::SuperApplication::putParamsIntoInfoSpace
(
    std::vector< std::pair<std::string, xdata::Serializable*> > &params,
    xdata::InfoSpace *s
)
{
   std::vector< std::pair<std::string, xdata::Serializable*> >::const_iterator itor;

   for(itor=params.begin(); itor!=params.end(); itor++)
   {
      try
      {
         s->fireItemAvailable(itor->first, itor->second);
      }
      catch( xcept::Exception &e )
      {
         std::stringstream oss;

         oss << "Failed to put " << itor->first;
         oss << " into infospace " << s->name();

         LOG4CPLUS_ERROR( logger_,
            oss.str() << xcept::stdformat_exception_history(e) );

         XCEPT_RETHROW( rubuilder::filter::exception::Monitoring, oss.str(), e );
      }
   }
}


void rubuilder::filter::SuperApplication::bindXgiCallbacks()
{

   xgi::bind
   (
      this,
      &rubuilder::filter::SuperApplication::css,
      "styles.css"
   );

   xgi::bind
   (
      this,
      &rubuilder::filter::SuperApplication::defaultWebPage,
      "Default"
   );

   xgi::bind
   (
      this,
      &rubuilder::filter::SuperApplication::cgiRefreshDefaultWebPage,
      "RefreshDefaultWebPage"
   );
}


std::vector< std::pair<std::string, xdata::Serializable*> >
rubuilder::filter::SuperApplication::initAndGetStdConfigParams()
{
   std::vector< std::pair<std::string, xdata::Serializable*> > params;

// L'init di alcuni parametri viene fatto usando variabili d'ambiente

   const char* env_var;
   std::string host;

   env_var = ::getenv( "RU_FILE_PATH" );
   if( env_var ) {
      outputFilepath_ = env_var;
   }
   else {
      outputFilepath_ = "";
   }


   env_var = ::getenv( "HOSTNAME" );
   if( env_var ) {
      host = env_var;
   }
   else {
      host = "";
   }


   xhost_ = host;

   xxmlClass_ = xmlClass_;

   xApplicationUrl_ = appDescriptor_->getContextDescriptor()->getURL();

   xwriteDataFile_ = false;

   xidleEventOnFile_ = false;

   // Limite di size imposto al data file
   xoutputFileSizeLimit_ = 4096;

   // Numero data file scritti di size predefinita
   xoutputFileCycleCounter_ = 1;

   // Nella GUI sono previsti campi diversi per path e nome data file
   xoutputFilepath_ = outputFilepath_;

   monitoringSleepSec_ = 5;

   output_buffer_size_ = OUTPUT_BUFFER_SIZE;

   chain_buffer_size_ = CHAIN_BUFFER_SIZE;

   xancillaryTcpPort_ = 9999;


   params.push_back( std::pair<std::string, xdata::Serializable *>
      ("host", &xhost_) );

   params.push_back( std::pair<std::string, xdata::Serializable *>
      ("class", &xxmlClass_) );

   params.push_back( std::pair<std::string, xdata::Serializable *>
      ("instance", &xinstance_) );

   params.push_back( std::pair<std::string, xdata::Serializable *>
      ("url", &xApplicationUrl_) );

   params.push_back( std::pair<std::string, xdata::Serializable *>
      ("dataOutput", &xOutput_) );

   params.push_back( std::pair<std::string, xdata::Serializable *>
      ("buInstances", &buInstances_) );

   params.push_back( std::pair<std::string, xdata::Serializable *>
      ("LocalFilterInstances", &filterInstances_) );

   params.push_back( std::pair<std::string,xdata::Serializable *>
      ( "mergerInstances", &mergerInstances_ ) );

   params.push_back( std::pair<std::string, xdata::Serializable *>
      ("writeDataFile", &xwriteDataFile_) );

   params.push_back( std::pair<std::string, xdata::Serializable *>  
      ("idleEventOnFile", &xidleEventOnFile_) );                    

   params.push_back( std::pair<std::string, xdata::Serializable *>
      ("outputFilepath", &xoutputFilepath_) );

   params.push_back( std::pair<std::string,xdata::Serializable *>
      ("outputFilename", &xoutputFilename_) );
      
   params.push_back( std::pair<std::string, xdata::Serializable *>
      ("outputFileSizeLimit_MB", &xoutputFileSizeLimit_) );
      
   params.push_back( std::pair<std::string, xdata::Serializable *>
      ("outputFileCycleCounter", &xoutputFileCycleCounter_) );

   params.push_back( std::pair<std::string, xdata::Serializable *>
      ("inputFilepath", &xinputFilepath_) );

   params.push_back( std::pair<std::string, xdata::Serializable *>
      ("configFilepath", &xconfigFilepath_) );

   //params.push_back( std::pair<std::string, xdata::Serializable *>
   //   ("statisticsUpdate", &monitoringSleepSec_) );

   params.push_back( std::pair<std::string, xdata::Serializable *>
      ("output_buffer_size", &output_buffer_size_) );

   params.push_back( std::pair<std::string, xdata::Serializable *>
      ("chain_buffer_size", &chain_buffer_size_) );

   params.push_back( std::pair<std::string, xdata::Serializable *>
      ("ancillaryTcpPort", &xancillaryTcpPort_) );

   params.push_back(std::pair<std::string,xdata::Serializable *>
      ("rcmsStateListener", rcmsStateNotifier_.getRcmsStateListenerParameter()));

   return params;
}


std::vector< std::pair<std::string, xdata::Serializable*> >
rubuilder::filter::SuperApplication::initAndGetStdMonitorParams()
{
   std::vector< std::pair<std::string, xdata::Serializable*> > params;

   runNumber_ = 0;
   deltaT_ = 0.0;
   deltaN_ = 0;
   deltaSumOfSizes_	= 0;
   xoutputBandw_ = "0.0";
   xinputBandw_ = "0.0";
   xfileBandw_ = "0.0";
   xoutputFileSize_ = 0;
   max_chain_size_ = 0;
   stateName_ = "Halted";

   startTimeOfRunString_= "---";

   stopTimeOfRunString_ = "---";


/*
   std::time( &rawtime );
   stmss << std::asctime( std::localtime(&rawtime) );
   startTimeOfRunString_ = stmss.str();

   stopTimeOfRunString_ = stmss.str();
*/


   xoutputFullFilepath_ = outputFullFilepath_;

   params.push_back( std::pair<std::string, xdata::Serializable *>
      ("outputFullFilepath", &xoutputFullFilepath_) );

   params.push_back( std::pair<std::string,xdata::Serializable *>
      ("runNumber", &runNumber_) );

   params.push_back( std::pair<std::string,xdata::Serializable *>
      ("stateName", &stateName_) );

   params.push_back( std::pair<std::string, xdata::Serializable *>
      ("inputBufferRate", &inrate_) );

   params.push_back( std::pair<std::string, xdata::Serializable *>
      ("inputBandw", &xinputBandw_) );

   params.push_back( std::pair<std::string, xdata::Serializable *>
      ("outputBufferRate", &outrate_) );

   params.push_back( std::pair<std::string, xdata::Serializable *>
      ("outputBandw", &xoutputBandw_) );

   params.push_back( std::pair<std::string, xdata::Serializable *>
      ("fileBandw", &xfileBandw_) );

   params.push_back( std::pair<std::string,xdata::Serializable *>
      ("outputFileSize_MB", &xoutputFileSize_) );
   
   params.push_back( std::pair<std::string,xdata::Serializable *>
      ("maxChainSize_B", &max_chain_size_) );

   params.push_back(std::pair<std::string,xdata::Serializable *>
      ("foundRcmsStateListener", rcmsStateNotifier_.getFoundRcmsStateListenerParameter()));

   return params;
}


toolbox::mem::Pool* rubuilder::filter::SuperApplication::getApplicationMemoryPool() {
   return pool_;
}


toolbox::fsm::FiniteStateMachine* rubuilder::filter::SuperApplication::getApplicationFiniteStateMachine() {
   return &fsm_;
}


bool rubuilder::filter::SuperApplication::onI2oException
(
    xcept::Exception &exception, void *context
)
{
   xdaq::ApplicationDescriptor *destDescriptor = (xdaq::ApplicationDescriptor *)context;

   std::string errorMessage =
      rubuilder::utils::createI2oErrorMsg(appDescriptor_, destDescriptor);

   LOG4CPLUS_ERROR( logger_,
      errorMessage << " : " << xcept::stdformat_exception_history( exception ) );

/*
   XCEPT_DECLARE_NESTED( rubuilder::ru::exception::I2o, sentinelException,
      errorMessage, exception );

   this->notifyQualified( "error",sentinelException );
*/

   return true;
}


void rubuilder::filter::SuperApplication::startMonitoringCalculations()
{

   monitoringWorkLoopName_ = rubuilder::utils::generateMonitoringWorkLoopName( xmlClass_, instance_ );

   monitoringWorkLoop_ = toolbox::task::getWorkLoopFactory()->
      getWorkLoop( monitoringWorkLoopName_, "waiting" );

   monitoringActionName_ = rubuilder::utils::generateMonitoringActionName( xmlClass_, instance_ );

   monitoringActionSignature_ = toolbox::task::bind
   (
      this,
      &rubuilder::filter::SuperApplication::updateMonitoringInfo,
      monitoringActionName_
   );

   struct timezone timezone;
   gettimeofday( &monitoringStartTime_, &timezone );

   try
   {
      monitoringWorkLoop_->submit( monitoringActionSignature_ );
   }
   catch( xcept::Exception &e )
   {
      const std::string msg = "Failed to submit action to work loop: " + monitoringWorkLoopName_;

      LOG4CPLUS_ERROR( logger_,
         msg << xcept::stdformat_exception_history(e) );

      XCEPT_RETHROW( rubuilder::filter::exception::WorkLoop, msg, e );
   }

   try
   {
      monitoringWorkLoop_->activate();
   }
   catch( xcept::Exception &e )
   {
      const std::string msg = "Failed to activate work loop: " + monitoringWorkLoopName_;

      LOG4CPLUS_ERROR( logger_,
         msg << xcept::stdformat_exception_history(e) );

      XCEPT_RETHROW( rubuilder::filter::exception::WorkLoop, msg, e );
    }
}


bool rubuilder::filter::SuperApplication::updateMonitoringInfo
(
   toolbox::task::WorkLoop *wl
)
{
   struct timeval monitoringEndTime;
   struct timezone timezone;
   std::stringstream ibwss;
   std::stringstream obwss;
   std::stringstream fbwss;

   unsigned int inputMonitoringEndSumOfSizes=0;
   unsigned int inputMonitoringEndN=0;

   unsigned int outMonitoringEndN=0;
   unsigned int outMonitoringEndSumOfSizes=0;

   unsigned int fileMonitoringEndSumOfSizes=0;


   // Sample
   gettimeofday( &monitoringEndTime, &timezone );

   //monitoringInfoSpace_->lock(); appInfoSpace_->lock();

   // Calculate the delta time and prepare for the next monitoring interval
   deltaT_ = rubuilder::utils::calcDeltaTime( &monitoringStartTime_, &monitoringEndTime );
   monitoringStartTime_ = monitoringEndTime;


   // filter input
   inputMonitoringEndN = nbuffs_;
   inputMonitoringEndSumOfSizes = sumOfSizes_;

   // Calculate the delta nbuffs_ and prepare for the next monitoring interval
   // Wrapping taken care of by using usigned longs
   deltaN_ = inputMonitoringEndN - inputMonitoringStartN_;
   inputMonitoringStartN_ = inputMonitoringEndN;

   // buffer rate
   if( deltaT_ != 0 )
   {
      inrate_ = deltaN_ / deltaT_;
   }
   else
   {
      inrate_ = 0;
   }


   // Calculate the delta sumOfSizes_ and prepare for the next monitoring interval
   // Wrapping taken care of by using usigned longs
   deltaSumOfSizes_ = inputMonitoringEndSumOfSizes - inputMonitoringStartSumOfSizes_;
   inputMonitoringStartSumOfSizes_ = inputMonitoringEndSumOfSizes;

   // throughput
   if( deltaT_ != 0 )
   {
      inBandw_ = deltaSumOfSizes_ / deltaT_;
   }
   else
   {
      inBandw_ = 0.0;
   }

   ibwss.precision(3);
   
   ibwss << std::fixed << ( (double)inBandw_/1048576 );

   xinputBandw_ = ibwss.str();

   std::cout << "IN bandw[MB/s]: " << ibwss.str() << "\t IN buff-rate[Hz]: " << inrate_ << std::endl;


   monitoringBSem_.take();

   if( out_bf_ )
   {
      // Output
      outMonitoringEndN = out_bf_->get_tbuff();
      outMonitoringEndSumOfSizes = out_bf_->get_tsize();

      // Calculate the delta nbBuffs_ and prepare for the next monitoring interval
      // Wrapping taken care of by using usigned longs
      deltaN_ = outMonitoringEndN - outMonitoringStartN_;
      outMonitoringStartN_ = outMonitoringEndN;

      // buffer rate
      if( deltaT_ != 0 )
      {
         outrate_ = deltaN_ / deltaT_;
      }
      else
      {
         outrate_ = 0;
      }

      // Calculate the delta sumOfSizes_ and prepare for the next monitoring interval
      // Wrapping taken care of by using usigned longs
      deltaSumOfSizes_ = outMonitoringEndSumOfSizes - outMonitoringStartSumOfSizes_;
      outMonitoringStartSumOfSizes_ = outMonitoringEndSumOfSizes;

      // throughput
      if( deltaT_ != 0 )
      {
         outBandw_ = deltaSumOfSizes_ / deltaT_;
      }
      else
      {
         outBandw_ = 0.0;
      }

      obwss.precision(3);
   
      obwss << std::fixed << ( (double)outBandw_/1048576 );

      xoutputBandw_ = obwss.str();

      std::cout << "OUT  bandw[MB/s]: " << obwss.str() << "\t OUT buff-rate[Hz]: " << outrate_ << std::endl;
   }


   if( out_df_ )
   {
      // Output
      fileMonitoringEndSumOfSizes = out_df_->get_tsize();

      // Calculate the delta sumOfSizes_ and prepare for the next monitoring interval
      // Wrapping taken care of by using usigned longs
      deltaSumOfSizes_ = fileMonitoringEndSumOfSizes - fileMonitoringStartSumOfSizes_;
      fileMonitoringStartSumOfSizes_ = fileMonitoringEndSumOfSizes;

      // throughput
      if( deltaT_ != 0 )
      {
         fileBandw_  = deltaSumOfSizes_ / deltaT_;
      }
      else
      {
         fileBandw_  = 0.0;
      }

      fbwss.precision(3);
   
      fbwss << std::fixed << ( (double)fileBandw_/1048576 );

      xfileBandw_ = fbwss.str();

      std::cout << "file bandw[MB/s]: " << fbwss.str() << std::endl;


      // outputFile size
      xoutputFileSize_ = out_df_->get_dfsize();

      xoutputFileSize_ = (uint64_t)xoutputFileSize_/1048576;
   }

   monitoringBSem_.give();


   ::sleep( monitoringSleepSec_.value_ );

   // Reschedule this action code
   return true;
}


void rubuilder::filter::SuperApplication::clearMonitoringVars()
{

   // input
   inputMonitoringStartN_ = 0;
   inputMonitoringStartSumOfSizes_ = 0;

   // out
   outMonitoringStartN_ = 0;
   outMonitoringStartSumOfSizes_ = 0;

   // file
   fileMonitoringStartSumOfSizes_ = 0;

   nbuffs_ = 0;
   sumOfSizes_ = 0;

   inrate_ = 0;

   outrate_ = 0;

   outBandw_ = 0.0;

   xoutputBandw_ = "0.0";

   inBandw_ = 0.0;
}


int rubuilder::filter::SuperApplication::pollItem
(
   uint32_t *item,
   uint32_t compareValue,
   uint32_t timeout,
   PollMethod pollMethod
)
{
   rubuilder::utils::StopWatch watch(0);
   uint32_t loopCounter = 1;
   bool pollCondition = false;
   watch.start();
   uint32_t time;

   // here the poll loop starts
   while( !pollCondition )
   {
      if( (pollMethod == POLL_UNTIL_EQUAL) ) 
         pollCondition = (compareValue == *item);
      else 
         pollCondition = (compareValue != *item );

      loopCounter++;

      if( loopCounter%1000 == 0 )
      {
         watch.stop();
         time = watch.read();

         if( time/1000 >= timeout )
         {
            std::stringstream text;

            text << "Timeout polling item \n"
            	<< std::dec << time/1000
            	<< "ms have passed wheras the timeout has been set to "
            	<< timeout << "ms\n"
            	<< std::dec << loopCounter << " polls have been carried out \n"
            	<< std::ends;

            LOG4CPLUS_INFO( logger_, text.str() );

            return 1;
         }
      }
      //::usleep( 100000 ); // limit the number of accesses
   }

   return 0;
}










/**
 * FILTER application WEB routines.
 * xgi is a C++ package based on cgicc to support the dynamic creation of HTML pages
 * that can be served through the pthttp peer transport of a running XDAQ process. 
 */

void rubuilder::filter::SuperApplication::css
(
   xgi::Input  *in,
   xgi::Output *out
)
{

   out->getHTTPResponseHeader().addHeader("Content-Type", "text/css");

   *out << "body" << std::endl;
   *out << "{" << std::endl;
   *out << "background-image: url('/rubuilder/images/sfondo.jpg');" << std::endl;
   *out << "}" << std::endl;

   *out << "table.head" << std::endl;
   *out << "{" << std::endl;
   *out << "width: 100%;" << std::endl;
   *out << "height: 120px;" << std::endl;
   *out << "background-color: #A8A8A8;" << std::endl;
   *out << "border: 2px solid black" << std::endl;
   *out << "}" << std::endl;

   // Logo Galileo
   *out << ".head td.cell_01" << std::endl;
   *out << "{" << std::endl;
   *out << "width: 10%;" << std::endl;
   //*out << "background-color: #A8A8A8;" << std::endl;
   *out << "}" << std::endl;

   // Logo applicazione
   *out << ".head td.cell_02" << std::endl;
   *out << "{" << std::endl;
   *out << "width: 5%;" << std::endl;
   //*out << "background-color: #A8A8A8;" << std::endl;
   *out << "text-align: center;" << std::endl;
   *out << "}" << std::endl;

   // Status applicazione
   *out << ".head td.cell_03" << std::endl;
   *out << "{" << std::endl;
   *out << "width: 10%;" << std::endl;
   *out << "font-size: 140%;" << std::endl;
   *out << "font-weight: 900" << std::endl;
   *out << "}" << std::endl;

   // Status applicazione in Running
   *out << ".head td.cell_03.green" << std::endl;
   *out << "{" << std::endl;
   *out << "width: 10%;" << std::endl;
   *out << "font-size: 140%;" << std::endl;
   *out << "font-weight: 900;" << std::endl;
   *out << "color: green;" << std::endl;
   *out << "}" << std::endl;

   // bw
   *out << ".head td.cell_04" << std::endl;
   *out << "{" << std::endl;
   *out << "width: 10%;" << std::endl;
   *out << "background-color: #D3D3D3;" << std::endl;
   *out << "}" << std::endl;

   // bw
   *out << ".head td.cell_05" << std::endl;
   *out << "{" << std::endl;
   *out << "width: 10%;" << std::endl;
   *out << "background-color: #D3D3D3;" << std::endl;
   *out << "}" << std::endl;

   *out << ".head td.cell_06" << std::endl;
   *out << "{" << std::endl;
   *out << "width: 55%;" << std::endl;
   *out << "}" << std::endl;

   *out << "table.bw" << std::endl;
   *out << "{" << std::endl;
   *out << "border:1px solid black;" << std::endl;
   *out << "width: 100%;" << std::endl;
   *out << "height: 100%;" << std::endl;
   *out << "}" << std::endl;

   *out << ".bw td" << std::endl;
   *out << "{" << std::endl;
   *out << "border:1px solid black;" << std::endl;
   *out << "width: 100%;" << std::endl;
   *out << "height: 25%;" << std::endl;
   *out << "font-size: 90%;" << std::endl;
   *out << "text-align: center;" << std::endl;
   *out << "}" << std::endl;

   *out << ".bw td.green" << std::endl;
   *out << "{" << std::endl;
   *out << "background-color: #32CD32;" << std::endl;
   *out << "border:1px solid black;" << std::endl;
   *out << "width: 100%;" << std::endl;
   *out << "height: 25%;" << std::endl;
   *out << "font-size: 90%;" << std::endl;
   *out << "text-align: center;" << std::endl;
   *out << "}" << std::endl;

   *out << ".bw td img" << std::endl;
   *out << "{" << std::endl;
   *out << "width: 80%;" << std::endl;
   *out << "height: 80%;" << std::endl;
   *out << "max-height:25px;" << std::endl;
   *out << "text-align: center;" << std::endl;
   *out << "}" << std::endl;

   *out << "table.updateTime" << std::endl;
   *out << "{" << std::endl;
   *out << "background-image: url('/rubuilder/images/sfondo.jpg');" << std::endl;
   //*out << "background-color: #D3D3D3;" << std::endl;
   //*out << "border: 1px solid black;" << std::endl;
   *out << "width: 100%;" << std::endl;
   *out << "height: 8%;" << std::endl;
   *out << "}" << std::endl;

   *out << ".updateTime td.cell_01" << std::endl;
   *out << "{" << std::endl;
   *out << "width: 49%;" << std::endl;
   *out << "font-family: arial;" << std::endl;
   *out << "font-size: 100%;" << std::endl;
   *out << "font-style: italic;" << std::endl;
   *out << "text-align: right;" << std::endl;
   *out << "color: black;" << std::endl;
   *out << "}" << std::endl;

   *out << ".updateTime td.cell_02" << std::endl;
   *out << "{" << std::endl;
   *out << "width: 2%;" << std::endl;
   *out << "font-family: arial;" << std::endl;
   *out << "font-size: 100%;" << std::endl;
   *out << "font-style: italic;" << std::endl;
   *out << "text-align: center;" << std::endl;
   *out << "color: gray;" << std::endl;
   *out << "}" << std::endl;

   *out << ".updateTime td.cell_03" << std::endl;
   *out << "{" << std::endl;
   *out << "width: 49%;" << std::endl;
   *out << "font-family: arial;" << std::endl;
   *out << "font-size: 100%;" << std::endl;
   *out << "font-style: italic;" << std::endl;
   *out << "text-align: left;" << std::endl;
   *out << "vertical-align: middle;" << std::endl;
   *out << "color: gray;" << std::endl;
   *out << "}" << std::endl;

   *out << "table.info" << std::endl;
   *out << "{" << std::endl;
   *out << "background-image: url('/rubuilder/images/sfondo.jpg');" << std::endl;
   //*out << "background-color: #D3D3D3;" << std::endl;
   //*out << "border: 1px solid black;" << std::endl;
   *out << "width: 100%;" << std::endl;
   *out << "height: 100%;" << std::endl;
   *out << "}" << std::endl;

   *out << ".info td.cell_01" << std::endl;
   *out << "{" << std::endl;
   //*out << "background-color: #A8A8A8;" << std::endl;
   //*out << "border: 1px solid black;" << std::endl;
   *out << "width: 3%;" << std::endl;
   *out << "height: 100%;" << std::endl;
   *out << "}" << std::endl;

   *out << ".info td.cell_02" << std::endl;
   *out << "{" << std::endl;
   //*out << "background-color: #A8A8A8;" << std::endl;
   //*out << "border: 1px solid black;" << std::endl;
   *out << "width: 47%;" << std::endl;
   *out << "height: 100%;" << std::endl;
   *out << "vertical-align:top;" << std::endl;
   *out << "}" << std::endl;

   *out << ".info td.cell_03" << std::endl;
   *out << "{" << std::endl;
   //*out << "background-color: #A8A8A8;" << std::endl;
   //*out << "border: 1px solid black;" << std::endl;
   *out << "width: 94%;" << std::endl;
   *out << "height: 100%;" << std::endl;
   *out << "vertical-align:top;" << std::endl;
   *out << "}" << std::endl;

   *out << "table.paramsTable" << std::endl;
   *out << "{" << std::endl;
   *out << "background-image: url('/rubuilder/images/sfondo.jpg');" << std::endl;
   *out << "border: 1px solid black;" << std::endl;
   *out << "width: 90%;" << std::endl;
   *out << "margin-top:auto;" << std::endl;
   *out << "margin-left:auto;" << std::endl; 
   *out << "margin-right:auto;" << std::endl;
   *out << "font-family: Arial;" << std::endl;
   *out << "}" << std::endl;

   *out << ".paramsTable th" << std::endl;
   *out << "{" << std::endl;
   *out << "height: 40px;" << std::endl;
   *out << "color: white;" << std::endl;
   *out << "background-color: #66F;" << std::endl;
   *out << "font-size: 20px;" << std::endl;
   *out << "}" << std::endl;

   *out << ".paramsTable tr" << std::endl;
   *out << "{" << std::endl;
   *out << "line-height: 35px;" << std::endl;
   *out << "}" << std::endl;

   *out << ".paramsTable td.cell_01" << std::endl;
   *out << "{" << std::endl;
   *out << "width: 50%;" << std::endl;
   *out << "height: 20px;" << std::endl;
   *out << "font-weight: 700;" << std::endl;
   *out << "font-size: 110%;" << std::endl;
   *out << "padding-left: 0.1cm;" << std::endl;
   *out << "border: 1px solid black;" << std::endl;
   *out << "}" << std::endl;

   *out << ".paramsTable td.cell_02" << std::endl;
   *out << "{" << std::endl;
   *out << "width: 50%;" << std::endl;
   *out << "padding-left: 0.1cm;" << std::endl;
   *out << "font-size: 110%;" << std::endl;
   *out << "border: 1px solid black;" << std::endl;
   *out << "}" << std::endl;

   *out << ".paramsTable td.cell_02.bold" << std::endl;
   *out << "{" << std::endl;
   *out << "width: 50%;" << std::endl;
   *out << "padding-left: 0.1cm;" << std::endl;
   *out << "font-size: 110%;" << std::endl;
   *out << "font-weight: 700;" << std::endl;
   *out << "border: 1px solid black;" << std::endl;
   *out << "color: blue;" << std::endl;
   *out << "}" << std::endl;

   // paramsTable Status cell
   *out << ".paramsTable td.cell_03" << std::endl;
   *out << "{" << std::endl;
   *out << "width: 50%;" << std::endl;
   *out << "padding-left: 0.1cm;" << std::endl;
   *out << "font-size: 130%;" << std::endl;
   *out << "font-weight: 700;" << std::endl;
   *out << "border: 1px solid black;" << std::endl;
   *out << "}" << std::endl;

   // paramsTable Status cell (Green in Running)
   *out << ".paramsTable td.cell_03.green" << std::endl;
   *out << "{" << std::endl;
   *out << "width: 50%;" << std::endl;
   *out << "padding-left: 0.1cm;" << std::endl;
   *out << "font-size: 130%;" << std::endl;
   *out << "font-weight: 700;" << std::endl;
   *out << "color: green;" << std::endl;
   *out << "border: 1px solid black;" << std::endl;
   *out << "}" << std::endl;

   *out << "</style>" << std::endl;
}


// It's the callback that is executed when the application is accessed through the Web browser.
void rubuilder::filter::SuperApplication::defaultWebPage
(
   xgi::Input  *in,
   xgi::Output *out
)
{
   // Time at which the page is being updated
   std::time_t rawtime;
   std::time( &rawtime );

   std::string cmd_url;
   std::string imgSrc;

   *out << "<html>" << std::endl;
   *out << "<head>" << std::endl;

   *out << "<link type=\"text/css\" rel=\"stylesheet\"";
   *out << " href=\"/" << urn_ << "/styles.css\"/>" << std::endl;

   if( AutoRefresh_def_ )
   {
      *out << "<meta HTTP-EQUIV=\"Refresh\" CONTENT=\"5; URL="
         << "Default" <<"\">" << std::endl;
   }

   *out << "<title>" << xmlClass_ << instance_ << " MAIN</title>" << std::endl;

   *out << "<body>" << std::endl;

   *out << "<table class=\"head\">" << std::endl;

   *out << "<tr>" << std::endl;

//
   *out << "<td class=\"cell_01\">" << std::endl;
   *out << "<img" << std::endl;
   imgSrc = IMAGE_DIR + "/galileoLogo.jpg";
   *out << "src=\"" << imgSrc << "\"" << std::endl;
   *out << "alt=\"Galileo logo\"" << std::endl;
   *out << "width=\"100%\"" << std::endl;
   *out << "height=\"auto\"" << std::endl;
   *out << "</img>" << std::endl;
   *out << "</td>" << std::endl;
//

//
   *out << "<td class=\"cell_02\">" << std::endl;
   *out << "<a href=\"/" << urn_ << "/Default\">" << std::endl;
   *out << "<img" << std::endl;
   imgSrc = IMAGE_DIR + "/fu64x64.gif";
   *out << "src=\"" << imgSrc << "\"" << std::endl;
   *out << "alt=\"ru64x64.gif\"" << std::endl;
   *out << "width=\"64\"" << std::endl;
   *out << "height=\"64\"" << std::endl;
   *out << "</a>" << std::endl;
   *out << "</td>" << std::endl;
//

//
   if( stateName_.toString() == "Running" )
   {
      *out << "<td class=\"cell_03 green\">" << std::endl;
      *out << stateName_.toString() << std::endl;
      *out << "</td>" << std::endl;
   }
   else
   {
      *out << "<td class=\"cell_03\">" << std::endl;
      *out << stateName_.toString() << std::endl;
      *out << "</td>" << std::endl;
   }
//

//
   *out << "<td class=\"cell_04\">" << std::endl;
   *out << "<table class=\"bw\">" << std::endl;
   *out << "<tr>" << std::endl;
   *out << "<td title=\"Input buffer rate\">" << std::endl;

   if( (uint32_t)inrate_ == 0 )
      *out << "---" << std::endl;
   else
      *out << inrate_ << " " << "Hz" << std::endl;

   *out << "</td>" << std::endl;
   *out << "</tr>" << std::endl;
   *out << "<tr>" << std::endl;
   *out << "<td title=\"Input bandwidth\" >" << std::endl;

   if( (double)inBandw_ == 0 )
      *out << "---" << std::endl;
   else
      *out << xinputBandw_.toString()<< " " << "MB/s" << std::endl;

   *out << "</td>" << std::endl;
   *out << "</tr>" << std::endl;
   *out << "<tr>" << std::endl;
   *out << "<td title=\"Output buffer rate\">" << std::endl;

   if( (uint32_t)outrate_ == 0 )
      *out << "---" << std::endl;
   else
      *out << outrate_ << " " << "Hz" << std::endl;

   *out << "</td>" << std::endl;
   *out << "</tr>" << std::endl;
   *out << "<tr>" << std::endl;
   *out << "<td title=\"Output bandwidth\">" << std::endl;

   if( (double)outBandw_ == 0 )
      *out << "---" << std::endl;
   else
      *out << xoutputBandw_.toString()<< " " << "MB/s" << std::endl;

   *out << "</td>" << std::endl;
   *out << "</tr>" << std::endl;
   *out << "</table>" << std::endl;
   *out << "</td>" << std::endl;
//

//
   *out << "<td class=\"cell_05\">" << std::endl;
   *out << "<table class=\"bw\">" << std::endl;
   *out << "<tr>" << std::endl;
   *out << "<td title=\"Run Number\">" << std::endl;
   *out << "#" << RunNum_ << std::endl;
   *out << "</td>" << std::endl;
   *out << "</tr>" << std::endl;
   *out << "<tr>" << std::endl;
   *out << "<td title=\"Start time of run\">" << std::endl;

   //std::cout << "startTimeOfRunString_: " << startTimeOfRunString_.toString() << std::endl;
   *out << startTimeOfRunString_.toString() << std::endl;

   *out << "</td>" << std::endl;
   *out << "</tr>" << std::endl;
   *out << "<tr>" << std::endl;

   if( stateName_.toString() == "Running" )
   {
      *out << "<td class=\"green\" title=\"Stop time of run\">" << std::endl;
      // In HTML the <img> tag has no end tag
      *out << "<img" << std::endl;
      imgSrc = IMAGE_DIR + "/counter.gif";
      *out << "src=\"" << imgSrc << "\"" << std::endl;
      *out << "alt=\"counter.gif\"" << std::endl;
   }
   else
   {
      *out << "<td title=\"Stop time of run\">" << std::endl;
      //std::cout << "stopTimeOfRunString_: " << stopTimeOfRunString_.toString() << std::endl;
      *out << stopTimeOfRunString_.toString() << std::endl;
   }

   *out << "</td>" << std::endl;
   *out << "</tr>" << std::endl;
   *out << "<tr>" << std::endl;
   *out << "<td title=\"OutputFile size\">" << std::endl;

   if( (uint64_t)xoutputFileSize_ == 0 )
      *out << "---" << std::endl;
   else
      *out << (uint64_t)xoutputFileSize_ << " " << "MB" << std::endl;

   *out << "</td>" << std::endl;
   *out << "</tr>" << std::endl;
   *out << "</table>" << std::endl;
   *out << "</td>" << std::endl;
//

//
   *out << "<td class=\"cell_06\">" << std::endl;
   *out << "</td>" << std::endl;
//

   *out << "</tr>" << std::endl;

   *out << "</table>" << std::endl;

   *out << "<hr/>" << std::endl;


   *out << "<table class=\"updateTime\">" << std::endl;
   *out << "<tr>" << std::endl;
   *out << "<i>" << std::endl;
   *out << "<td class=\"cell_01\">" << std::endl;
   *out << "Page last updated: ";
   *out << std::asctime( std::localtime(&rawtime) ) << std::endl;
   *out << "</i>" << std::endl;
   *out << "</td>" << std::endl;

   *out << "<td class=\"cell_02\">" << std::endl;
   *out << "</td>" << std::endl;

   *out << "<td class=\"cell_03\">" << std::endl;
   // AUTO REFRESH BUTTON:
   cmd_url = "/";
   cmd_url += appDescriptor_->getURN();
   cmd_url += "/RefreshDefaultWebPage";
   *out << cgicc::form().set("method","get").set("action", cmd_url).set("enctype","multipart/form-data") << std::endl;

   *out << cgicc::input().set("type", "submit").set("name", "RefreshDefaultWebPage").set("value",
      (AutoRefresh_def_ ? "AutoRefresh OFF" : "AutoRefresh"));

   *out << cgicc::form();
   *out << "</td>" << std::endl;

   *out << "</tr>" << std::endl;
   *out << "</table>" << std::endl;


//
   *out << "<table class=\"info\">" << std::endl;

   *out << "<tr>" << std::endl;

   *out << "<td class=\"cell_01\">" << std::endl;
   *out << "</td>" << std::endl;

   *out << "<td class=\"cell_02\">" << std::endl;
   //
   printParamsTable( in, out, "DAQ Standard Configuration", stdConfigParams_ );
   //
   *out << "</td>" << std::endl;

   *out << "<td class=\"cell_02\">" << std::endl;
   //
   printParamsTable( in, out, "DAQ Standard Monitoring", stdMonitorParams_ );
   //
   *out << "</td>" << std::endl;

   *out << "<td class=\"cell_01\">" << std::endl;
   *out << "</td>" << std::endl;

   *out << "</tr>" << std::endl;

   *out << "</table>" << std::endl;
//

   *out << "</body>" << std::endl;

   *out << "</html>" << std::endl;
}


// It's the callback that is executed when the application is accessed through the Web browser.
void rubuilder::filter::SuperApplication::debugWebPage
(
   xgi::Input  *in,
   xgi::Output *out
)
{
   // Time at which the page is being updated
   std::time_t rawtime;
   std::time( &rawtime );

   std::string cmd_url;
   std::string imgSrc;

   *out << "<html>" << std::endl;
   *out << "<head>" << std::endl;

   *out << "<link type=\"text/css\" rel=\"stylesheet\"";
   *out << " href=\"/" << urn_ << "/styles.css\"/>" << std::endl;

   if( AutoRefresh_dbg_ )
   {
      *out << "<meta HTTP-EQUIV=\"Refresh\" CONTENT=\"5; URL="
         << "Debug" <<"\">" << std::endl;
   }

   *out << "<title>" << xmlClass_ << instance_ << " DEBUG</title>" << std::endl;

   *out << "<body>" << std::endl;

   *out << "<table class=\"head\">" << std::endl;

   *out << "<tr>" << std::endl;

//
   *out << "<td class=\"cell_01\">" << std::endl;
   *out << "<img" << std::endl;
   imgSrc = IMAGE_DIR + "/galileoLogo.jpg";
   *out << "src=\"" << imgSrc << "\"" << std::endl;
   *out << "alt=\"Galileo logo\"" << std::endl;
   *out << "width=\"100%\"" << std::endl;
   *out << "height=\"auto\"" << std::endl;
   *out << "</img>" << std::endl;
   *out << "</td>" << std::endl;
//

//
   *out << "<td class=\"cell_02\">" << std::endl;
   *out << "<a href=\"/" << urn_ << "/Debug\">" << std::endl;
   *out << "<img" << std::endl;
   imgSrc = IMAGE_DIR + "/fu64x64.gif";
   *out << "src=\"" << imgSrc << "\"" << std::endl;
   *out << "alt=\"ru64x64.gif\"" << std::endl;
   *out << "width=\"64\"" << std::endl;
   *out << "height=\"64\"" << std::endl;
   *out << "</a>" << std::endl;
   *out << "</td>" << std::endl;
//

//
   if( stateName_.toString() == "Running" )
   {
      *out << "<td class=\"cell_03 green\">" << std::endl;
      *out << stateName_.toString() << std::endl;
      *out << "</td>" << std::endl;
   }
   else
   {
      *out << "<td class=\"cell_03\">" << std::endl;
      *out << stateName_.toString() << std::endl;
      *out << "</td>" << std::endl;
   }
//

//
   *out << "<td class=\"cell_04\">" << std::endl;
   *out << "<table class=\"bw\">" << std::endl;
   *out << "<tr>" << std::endl;
   *out << "<td title=\"Input buffer rate\">" << std::endl;

   if( (uint32_t)inrate_ == 0 )
      *out << "---" << std::endl;
   else
      *out << inrate_ << " " << "Hz" << std::endl;

   *out << "</td>" << std::endl;
   *out << "</tr>" << std::endl;
   *out << "<tr>" << std::endl;
   *out << "<td title=\"Input bandwidth\" >" << std::endl;

   if( (double)inBandw_ == 0 )
      *out << "---" << std::endl;
   else
      *out << xinputBandw_.toString()<< " " << "MB/s" << std::endl;

   *out << "</td>" << std::endl;
   *out << "</tr>" << std::endl;
   *out << "<tr>" << std::endl;
   *out << "<td title=\"Output buffer rate\">" << std::endl;

   if( (uint32_t)outrate_ == 0 )
      *out << "---" << std::endl;
   else
      *out << outrate_ << " " << "Hz" << std::endl;

   *out << "</td>" << std::endl;
   *out << "</tr>" << std::endl;
   *out << "<tr>" << std::endl;
   *out << "<td title=\"Output bandwidth\">" << std::endl;

   if( (double)outBandw_ == 0 )
      *out << "---" << std::endl;
   else
      *out << xoutputBandw_.toString()<< " " << "MB/s" << std::endl;

   *out << "</td>" << std::endl;
   *out << "</tr>" << std::endl;
   *out << "</table>" << std::endl;
   *out << "</td>" << std::endl;
//

//
   *out << "<td class=\"cell_05\">" << std::endl;
   *out << "<table class=\"bw\">" << std::endl;
   *out << "<tr>" << std::endl;
   *out << "<td title=\"Run Number\">" << std::endl;
   *out << "#" << RunNum_ << std::endl;
   *out << "</td>" << std::endl;
   *out << "</tr>" << std::endl;
   *out << "<tr>" << std::endl;
   *out << "<td title=\"Start time of run\">" << std::endl;

   //std::cout << "startTimeOfRunString_: " << startTimeOfRunString_.toString() << std::endl;
   *out << startTimeOfRunString_.toString() << std::endl;

   *out << "</td>" << std::endl;
   *out << "</tr>" << std::endl;
   *out << "<tr>" << std::endl;

   if( stateName_.toString() == "Running" )
   {
      *out << "<td class=\"green\" title=\"Stop time of run\">" << std::endl;
      // In HTML the <img> tag has no end tag
      *out << "<img" << std::endl;
      imgSrc = IMAGE_DIR + "/counter.gif";
      *out << "src=\"" << imgSrc << "\"" << std::endl;
      *out << "alt=\"counter.gif\"" << std::endl;
   }
   else
   {
      *out << "<td title=\"Stop time of run\">" << std::endl;
      //std::cout << "stopTimeOfRunString_: " << stopTimeOfRunString_.toString() << std::endl;
      *out << stopTimeOfRunString_.toString() << std::endl;
   }

   *out << "</td>" << std::endl;
   *out << "</tr>" << std::endl;
   *out << "<tr>" << std::endl;
   *out << "<td title=\"OutputFile size\">" << std::endl;

   if( (uint64_t)xoutputFileSize_ == 0 )
      *out << "---" << std::endl;
   else
      *out << (uint64_t)xoutputFileSize_ << " " << "MB" << std::endl;

   *out << "</td>" << std::endl;
   *out << "</tr>" << std::endl;
   *out << "</table>" << std::endl;
   *out << "</td>" << std::endl;
//

//
   *out << "<td class=\"cell_06\">" << std::endl;
   *out << "</td>" << std::endl;
//

   *out << "</tr>" << std::endl;

   *out << "</table>" << std::endl;

   *out << "<hr/>" << std::endl;


   *out << "<table class=\"updateTime\">" << std::endl;
   *out << "<tr>" << std::endl;
   *out << "<i>" << std::endl;
   *out << "<td class=\"cell_01\">" << std::endl;
   *out << "Page last updated: ";
   *out << std::asctime( std::localtime(&rawtime) ) << std::endl;
   *out << "</i>" << std::endl;
   *out << "</td>" << std::endl;

   *out << "<td class=\"cell_02\">" << std::endl;
   *out << "</td>" << std::endl;

   *out << "<td class=\"cell_03\">" << std::endl;
   // AUTO REFRESH BUTTON:
   cmd_url = "/";
   cmd_url += appDescriptor_->getURN();
   cmd_url += "/RefreshDebugWebPage";
   *out << cgicc::form().set("method","get").set("action", cmd_url).set("enctype","multipart/form-data") << std::endl;

   *out << cgicc::input().set("type", "submit").set("name", "RefreshDebugWebPage").set("value",
      (AutoRefresh_dbg_ ? "AutoRefresh OFF" : "AutoRefresh"));

   *out << cgicc::form();
   *out << "</td>" << std::endl;

   *out << "</tr>" << std::endl;
   *out << "</table>" << std::endl;


//
   *out << "<table class=\"info\">" << std::endl;

   *out << "<tr>" << std::endl;

   *out << "<td class=\"cell_01\">" << std::endl;
   *out << "</td>" << std::endl;

   *out << "<td class=\"cell_03\">" << std::endl;
   //
   printDebugTable( in, out, "DAQ Debug" );
   //	
   *out << "</td>" << std::endl;

   *out << "<td class=\"cell_01\">" << std::endl;
   *out << "</td>" << std::endl;

   *out << "</tr>" << std::endl;

   *out << "</table>" << std::endl;
//

   *out << "</body>" << std::endl;

   *out << "</html>" << std::endl;
}


void rubuilder::filter::SuperApplication::printDebugTable
(
   xgi::Input        *in,
   xgi::Output       *out,
   const std::string name
)
{
   uint64_t usedMem;
   
   *out << "<table class=\"paramsTable\">" << std::endl;

   *out << "<tr>" << std::endl;
   *out << "<th colspan=\"2\">" << std::endl;
   *out << name << std::endl;
   *out << "</th>" << std::endl;
   *out << "</tr>" << std::endl;


   // "Committed Pool Size"
   *out << "<tr>" << std::endl;
   *out << "<td class=\"cell_01\">" << std::endl;
   *out << "Committed Pool Size" << std::endl;
   *out << "</td>" << std::endl;
	    
   *out << "<td class=\"cell_02\">" << std::endl;

   *out << COMMITTED_POOL_SIZE/1048576 << " " << "MB" << std::endl;
   
   *out << "</td>" << std::endl;
   *out << "</tr>" << std::endl;


   // "usedMem"
   *out << "<tr>" << std::endl;
   *out << "<td class=\"cell_01\">" << std::endl;
   *out << "Memory usage" << std::endl;
   *out << "</td>" << std::endl;
	    
   *out << "<td class=\"cell_02\">" << std::endl;


   applicationBSem_.take();

   if( pool_ )
   {
      toolbox::mem::Usage& usage = pool_->getMemoryUsage();
      usedMem = usage.getUsed();
      *out << usedMem/1048576 << " " << "MB" << std::endl;
   }
   else
   {
      *out << "---" << std::endl;
   }

   applicationBSem_.give();

   
   *out << "</td>" << std::endl;
   *out << "</tr>" << std::endl;


   // "FILTER_MAX_OUT"
   *out << "<tr>" << std::endl;
   *out << "<td class=\"cell_01\">" << std::endl;
   *out << "FILTER_MAX_OUT" << std::endl;
   *out << "</td>" << std::endl;
	    
   *out << "<td class=\"cell_02\">" << std::endl;

   *out << FILTER_MAX_OUT << std::endl;
   
   *out << "</td>" << std::endl;
   *out << "</tr>" << std::endl;


   *out << "</table>" << std::endl;
}


void rubuilder::filter::SuperApplication::printParamsTable
(
   xgi::Input        *in,
   xgi::Output       *out,
   const std::string name,
   std::vector< std::pair<std::string, xdata::Serializable*> > &params
)
{
   std::vector< std::pair<std::string, xdata::Serializable*> >::iterator pos;
   std::map<std::string, int> lookThisStrings;
   std::string str;
   std::size_t found;
  
   
   lookThisStrings["class"] = 1;
   lookThisStrings["url"] = 2;
   lookThisStrings["writeDataFile"] = 3;
   lookThisStrings["outputFilepath"] = 4;
   lookThisStrings["runNumber"] = 5;
   lookThisStrings["outputBufferRate"] = 6;
   lookThisStrings["outputBandw"] = 7;
   lookThisStrings["fileBandw"] = 8;
   lookThisStrings["stateName"] = 9;
   lookThisStrings["instance"] = 10;
   lookThisStrings["inputBandw"] = 11;
   lookThisStrings["inputBufferRate"] = 12;
   lookThisStrings["outputFileSizeLimit_MB"] = 13;
   lookThisStrings["outputFileSize_MB"] = 14;
   lookThisStrings["outputFileCycleCounter"] = 15;
   lookThisStrings["chain_buffer_size"] = 16;
   lookThisStrings["host"] = 17;
   lookThisStrings["maxChainSize_B"] = 18;

   
   *out << "<table class=\"paramsTable\">" << std::endl;

   *out << "<tr>" << std::endl;
   *out << "<th colspan=\"2\">" << std::endl;
   *out << name << std::endl;
   *out << "</th>" << std::endl;
   *out << "</tr>" << std::endl;


   for( pos=params.begin(); pos!=params.end(); pos++ )
   {
      
      switch( lookThisStrings[pos->first] )
      {
         // "class"
         case 1:
            *out << "<tr>" << std::endl;

	    *out << "<td class=\"cell_01\">" << std::endl;
            //*out << pos->first << std::endl;
            *out << "Application" << std::endl;
            *out << "</td>" << std::endl;

            str = pos->second->toString();

/*
            found = str.find( "ru::Application", 10 );

            if( found != std::string::npos )
               std::cout << "first 'needle' found at: " << found << '\n';
*/
	    
	    *out << "<td class=\"cell_02\">" << std::endl;
	    *out << "Filter Unit" << std::endl;
	    *out << "</td>" << std::endl;

            *out << "</tr>" << std::endl;
	 break;

         // "url"
         case 2:
            *out << "<tr>" << std::endl;

	    *out << "<td class=\"cell_01\">" << std::endl;
            //*out << pos->first << std::endl;
            *out << "Application url" << std::endl;
            *out << "</td>" << std::endl;
	    
	    *out << "<td class=\"cell_02\">" << std::endl;
	    str = pos->second->toString();
	    *out << str << std::endl;
	    *out << "</td>" << std::endl;

            *out << "</tr>" << std::endl;
	 break;

         // "writeDataFile"
         case 3:
            *out << "<tr>" << std::endl;

	    *out << "<td class=\"cell_01\">" << std::endl;
            //*out << pos->first << std::endl;
            *out << "OutputFile" << std::endl;
            *out << "</td>" << std::endl;

            str = pos->second->toString();

/*
            found = str.find( "ru::Application", 10 );

            if( found != std::string::npos )
               std::cout << "first 'needle' found at: " << found << '\n';
*/
	    
	    *out << "<td class=\"cell_02\">" << std::endl;

            if( xwriteDataFile_ )
            {
	       *out << "Enabled" << std::endl;
            }
            else
            {
	       *out << "Disabled" << std::endl;
            }

	    *out << "</td>" << std::endl;

            *out << "</tr>" << std::endl;
	 break;

         // "xoutputFilepath"
         case 4:
            *out << "<tr>" << std::endl;

	    *out << "<td class=\"cell_01\">" << std::endl;
            //*out << pos->first << std::endl;
            *out << "OutputFile path" << std::endl;
            *out << "</td>" << std::endl;

            str = pos->second->toString();

/*
            found = str.find( "ru::Application", 10 );

            if( found != std::string::npos )
               std::cout << "first 'needle' found at: " << found << '\n';
*/
	    
	    *out << "<td class=\"cell_02\">" << std::endl;

	    *out << xoutputFilepath_.toString() << std::endl;

	    *out << "</td>" << std::endl;

            *out << "</tr>" << std::endl;
	 break;

         // "RunNumber"
         case 5:
            *out << "<tr>" << std::endl;

	    *out << "<td class=\"cell_01\">" << std::endl;
            //*out << pos->first << std::endl;
            *out << "Run Number" << std::endl;
            *out << "</td>" << std::endl;
	    
	    *out << "<td class=\"cell_02\">" << std::endl;
	    str = pos->second->toString();
	    *out << str << std::endl;
	    *out << "</td>" << std::endl;

            *out << "</tr>" << std::endl;
	 break;

         // "outputBufferRate"
         case 6:
            *out << "<tr>" << std::endl;

	    *out << "<td class=\"cell_01\">" << std::endl;
            //*out << pos->first << std::endl;
            *out << "Output buffer rate" << std::endl;
            *out << "</td>" << std::endl;
	    
	    *out << "<td class=\"cell_02\">" << std::endl;
	    str = pos->second->toString();
	    *out << str << " " << "Hz" << std::endl;
	    *out << "</td>" << std::endl;

            *out << "</tr>" << std::endl;
	 break;

         // "outputBandw"
         case 7:
            *out << "<tr>" << std::endl;

	    *out << "<td class=\"cell_01\">" << std::endl;
            //*out << pos->first << std::endl;
            *out << "Output bandwidth" << std::endl;
            *out << "</td>" << std::endl;
	    
	    *out << "<td class=\"cell_02\">" << std::endl;
	    str = pos->second->toString();
	    *out << str << " " << "MB/s" << std::endl;
	    *out << "</td>" << std::endl;

            *out << "</tr>" << std::endl;
	 break;

         // "fileBandw"
         case 8:
            *out << "<tr>" << std::endl;

	    *out << "<td class=\"cell_01\">" << std::endl;
            //*out << pos->first << std::endl;
            *out << "OutputFile bandwidth" << std::endl;
            *out << "</td>" << std::endl;
	    
	    *out << "<td class=\"cell_02\">" << std::endl;
	    str = pos->second->toString();
            *out << str << " " << "MB/s" << std::endl;
	    *out << "</td>" << std::endl;

            *out << "</tr>" << std::endl;
	 break;

         // "StateName"
         case 9:
            *out << "<tr>" << std::endl;

	    *out << "<td class=\"cell_01\">" << std::endl;
            //*out << pos->first << std::endl;
            *out << "Status" << std::endl;
            *out << "</td>" << std::endl;

            if( stateName_.toString() == "Running" )
            {
	       *out << "<td class=\"cell_03 green\">" << std::endl;
	       str = pos->second->toString();
	       *out << str << std::endl;
	       *out << "</td>" << std::endl;
            }
            else
            {
	       *out << "<td class=\"cell_03\">" << std::endl;
	       str = pos->second->toString();
	       *out << str << std::endl;
	       *out << "</td>" << std::endl;
            }

            *out << "</tr>" << std::endl;
	 break;

         // "instance"
         case 10:
            *out << "<tr>" << std::endl;

	    *out << "<td class=\"cell_01\">" << std::endl;
            //*out << pos->first << std::endl;
            *out << "Application instance" << std::endl;
            *out << "</td>" << std::endl;
	    
	    *out << "<td class=\"cell_02\">" << std::endl;
	    str = pos->second->toString();
	    *out << str << std::endl;
	    *out << "</td>" << std::endl;

            *out << "</tr>" << std::endl;
	 break;

         // "inputBandw"
         case 11:
            *out << "<tr>" << std::endl;

	    *out << "<td class=\"cell_01\">" << std::endl;
            //*out << pos->first << std::endl;
            *out << "Input bandwidth" << std::endl;
            *out << "</td>" << std::endl;
	    
	    *out << "<td class=\"cell_02\">" << std::endl;
	    str = pos->second->toString();

            if( (double)inBandw_ == 0 )
               *out << "---" << std::endl;
            else
	       *out << str << " " << "MB/s" << std::endl;

	    *out << "</td>" << std::endl;

            *out << "</tr>" << std::endl;
	 break;

         // "inputBufferRate"
         case 12:
            *out << "<tr>" << std::endl;

	    *out << "<td class=\"cell_01\">" << std::endl;
            //*out << pos->first << std::endl;
            *out << "Input buffer rate" << std::endl;
            *out << "</td>" << std::endl;
	    
	    *out << "<td class=\"cell_02\">" << std::endl;
	    str = pos->second->toString();

            if( (double)inrate_ == 0 )
               *out << "---" << std::endl;
            else
	       *out << str << " " << "Hz" << std::endl;

	    *out << "</td>" << std::endl;

            *out << "</tr>" << std::endl;
	 break;

         // "outputFileSizeLimit_MB"
         case 13:
            *out << "<tr>" << std::endl;

	    *out << "<td class=\"cell_01\">" << std::endl;
            //*out << pos->first << std::endl;
            *out << "OutputFile Max size" << std::endl;
            *out << "</td>" << std::endl;
	    
	    *out << "<td class=\"cell_02\">" << std::endl;
	    str = pos->second->toString();
    
            if( outFileMaxSize_ )
               *out << str << " " << "MB" << std::endl;
            else
               *out << "n/a" << std::endl;
	    *out << "</td>" << std::endl;

            *out << "</tr>" << std::endl;
	 break;

         // "outputFileSize_MB"
         case 14:
            *out << "<tr>" << std::endl;

	    *out << "<td class=\"cell_01\">" << std::endl;
            //*out << pos->first << std::endl;
            *out << "OutputFile size" << std::endl;
            *out << "</td>" << std::endl;
	    
	    *out << "<td class=\"cell_02\">" << std::endl;
	    str = pos->second->toString();
            *out << str << " " << "MB" << std::endl;
	    *out << "</td>" << std::endl;

            *out << "</tr>" << std::endl;
	 break;

         // "outputFileCycleCounter"
         case 15:
            *out << "<tr>" << std::endl;

	    *out << "<td class=\"cell_01\">" << std::endl;
            //*out << pos->first << std::endl;
            *out << "OutputFile cycle counter" << std::endl;
            *out << "</td>" << std::endl;
	    
	    *out << "<td class=\"cell_02\">" << std::endl;
	    str = pos->second->toString();
    
            if( outFileMaxSize_ )
               *out << str << std::endl;
            else
               *out << "n/a" << std::endl;
	    *out << "</td>" << std::endl;

            *out << "</tr>" << std::endl;
	 break;

         // "chain_buffer_size"
         case 16:
            *out << "<tr>" << std::endl;

	    *out << "<td class=\"cell_01\">" << std::endl;
            //*out << pos->first << std::endl;
            *out << "Chain buffer Max size" << std::endl;
            *out << "</td>" << std::endl;
	    
	    *out << "<td class=\"cell_02\">" << std::endl;
	    str = pos->second->toString();
            *out << str << " " << "B" << std::endl;
	    *out << "</td>" << std::endl;
    
            *out << "</tr>" << std::endl;
	 break;

         // "Host"
         case 17:
            *out << "<tr>" << std::endl;

	    *out << "<td class=\"cell_01\">" << std::endl;
            //*out << pos->first << std::endl;
            *out << "Host" << std::endl;
            *out << "</td>" << std::endl;
	    
	    *out << "<td class=\"cell_02 bold\">" << std::endl;
	    str = pos->second->toString();
	    *out << str << std::endl;
	    *out << "</td>" << std::endl;

            *out << "</tr>" << std::endl;
	 break;

         // "maxChainSize_B"
         case 18:
            *out << "<tr>" << std::endl;

	    *out << "<td class=\"cell_01\">" << std::endl;
            //*out << pos->first << std::endl;
            *out << "Max chain size" << std::endl;
            *out << "</td>" << std::endl;
	    
	    *out << "<td class=\"cell_02\">" << std::endl;
	    str = pos->second->toString();
            *out << str << " " << "B" << std::endl;
	    *out << "</td>" << std::endl;

            *out << "</tr>" << std::endl;
	 break;

         default:
         break;
      }
   }

   *out << "</table>" << std::endl;
}


// Refresh "defaultWebPage"
void rubuilder::filter::SuperApplication::cgiRefreshDefaultWebPage( xgi::Input *in, xgi::Output *out )
{
   std::string command;

   try 
   {
      // Create a new Cgicc object containing all the CGI data
      cgicc::Cgicc cgi(in);

      command = cgi["RefreshDefaultWebPage"]->getValue();

      bool found = false;

      if( command == "AutoRefresh OFF" || command == "AutoRefresh" )
      {
         found = true;
	 
         AutoRefresh_def_ = !AutoRefresh_def_;
      }

      if( !found )
      {
         LOG4CPLUS_ERROR
	 (
	    logger_,
	    toolbox::toString( "cgiRefreshDefaultWebPage(): Unknown command \"%s\"", command.c_str() )
	 );
      }
   }
   catch( const std::exception& e ) 
   {
      std::cout << e.what() << std::endl;
   }
   
   // Redirect to the "defaultWebPage" web page
   std::string url = "/";
   
   url += getApplicationDescriptor()->getURN() + "/" + "Default";
      
   *out << "<meta HTTP-EQUIV=\"Refresh\" CONTENT=\"0; URL=" << url << "\">" << std::endl;
}


// Refresh "debugWebPage"
void rubuilder::filter::SuperApplication::cgiRefreshDebugWebPage( xgi::Input *in, xgi::Output *out )
{
   std::string command;

   try 
   {
      // Create a new Cgicc object containing all the CGI data
      cgicc::Cgicc cgi(in);

      command = cgi["RefreshDebugWebPage"]->getValue();

      bool found = false;

      if( command == "AutoRefresh OFF" || command == "AutoRefresh" )
      {
         found = true;
	 
         AutoRefresh_dbg_ = !AutoRefresh_dbg_;
      }

      if( !found )
      {
         LOG4CPLUS_ERROR
	 (
	    logger_,
	    toolbox::toString( "cgiRefreshDebugWebPage(): Unknown command \"%s\"", command.c_str() )
	 );
      }
   }
   catch( const std::exception& e ) 
   {
      std::cout << e.what() << std::endl;
   }
   
   // Redirect to the "debugWebPage" web page
   std::string url = "/";
   
   url += getApplicationDescriptor()->getURN() + "/" + "Debug";
      
   *out << "<meta HTTP-EQUIV=\"Refresh\" CONTENT=\"0; URL=" << url << "\">" << std::endl;
}


















