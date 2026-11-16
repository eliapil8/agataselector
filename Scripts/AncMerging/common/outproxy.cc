#include "interface/shared/i2oEVBMsgs.h"
#include "interface/shared/i2oXFunctionCodes.h"
#include "rubuilder/lfcaendgtz/outproxy.h"
#include "rubuilder/utils/CreateStrings.h"
#include "rubuilder/utils/ForceFailedEvent.h"
#include "xcept/tools.h"


rubuilder::filter::outproxy::outproxy
(
   xdaq::Application *app,
   toolbox::BSem *applicationBSem,
   log4cplus::Logger& logger
):
app_( app ),
state_(16),
logger_( log4cplus::Logger::getInstance( logger.getName() + ".outproxy" ) )
{

   LOG4CPLUS_INFO( logger_, "START of CTOR" );

   appContext_ = app_->getApplicationContext();

   // Application instance number
   instance_ = app_->getApplicationDescriptor()->getInstance();

   // Application id
   id_ = app_->getApplicationDescriptor()->getLocalId();

   applicationBSem_ = applicationBSem;

   timeout_ = TIMEOUT_OUTPROXY_GET;


   try
   {
      buf_refq_ = new BufRefQ( OUTPROXY_QUEUE_DEPTH );
   }
   catch(...)
   {
      throw;
   }


   nbBuffs_ = 0;

   sumOfSizes_ = 0;


   state_.set( STATE_DISABLED );


   startProcessingWorkLoop();

   LOG4CPLUS_INFO( logger_, "END of CTOR" );
}


rubuilder::filter::outproxy::~outproxy()
{

   LOG4CPLUS_INFO( logger_, "START of DTOR" );

   if( buf_refq_ )
      delete buf_refq_;

   LOG4CPLUS_INFO( logger_, "END of DTOR" );
}


int rubuilder::filter::outproxy::put( toolbox::mem::Reference *bufRef, int timeout )
{
   int retcode=0;

   try
   {
      // BufRefQ.h
      // timeout=0, call bloccante se non c'è spazio
      //buf_refq_->put( bufRef );
      //retcode = buf_refq_->put( bufRef, 0 );

      // BufRefQ.cc 
      // timeout=0, call bloccante se non c'è spazio
      // Exit if "buf_refq_->stop()"
      retcode = buf_refq_->put1( bufRef, timeout );

      return retcode;
   }
   catch( BufRefQ::Stopped& e )
   {
      //throw BufRefQ::Stopped();
      throw;
   }
}


void rubuilder::filter::outproxy::configure
(
   xdata::Vector<xdata::UnsignedInteger32> buInstances,
   xdata::Vector<xdata::UnsignedInteger32> filterInstances,
   xdata::Vector<xdata::UnsignedInteger32> mergerInstances
)
{
   
   LOG4CPLUS_INFO( logger_, "configure fired" );

   buInstances_ = buInstances;

   filterInstances_ = filterInstances;

   mergerInstances_ = mergerInstances;

   this->getAppDescriptorsAndTids();
}


void rubuilder::filter::outproxy::enable()
{

   LOG4CPLUS_INFO( logger_, "enable fired" );

   nbBuffs_ = 0;

   sumOfSizes_ = 0;

   wl_->submit( outproxyAction_ );

   state_.set( STATE_ENABLED );
}


void rubuilder::filter::outproxy::disable()
{

   LOG4CPLUS_INFO( logger_, "disable fired" );

   // Dopo lo stop non vengono accettati altri buffer nella coda.
   buf_refq_->stop();


   // OUT loop
   if( state_.test_set( STATE_RUNNING, STATE_STOPPING ) )
   {
      std::cout << "outproxy loop is waiting for STATE_STOPPED... " << std::endl;

      state_.wait( STATE_STOPPED );
   }

   state_.set( STATE_DISABLED );
}


void rubuilder::filter::outproxy::getAppDescriptorsAndTids()
{

   try
   {
      // tid is the I2O target identifier assigned to this class instance
      tid_ = i2o::utils::getAddressMap()->
         getTid( app_->getApplicationDescriptor() );
   }
   catch( xcept::Exception &e )
   {
      const std::string msg = "Failed to get the I2O TID of this application ";

      LOG4CPLUS_ERROR( logger_,
         msg << xcept::stdformat_exception_history( e ) );
        
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
      const std::string msg = "Failed to fill internal descriptor list of MERGERs destination ";

      LOG4CPLUS_ERROR( logger_,
         msg << xcept::stdformat_exception_history( e ) );

      XCEPT_RETHROW( rubuilder::filter::exception::Configuration, msg, e );
   }
}


void rubuilder::filter::outproxy::fillDestinationBUs()
{

   try
   {
      // Check participating bu instance is not a duplicate
      checkBuInstances();
   }
   catch( xcept::Exception &e )
   {
      const std::string msg = "Exported parameter \"buInstances\" is invalid";

      LOG4CPLUS_ERROR( logger_,
         msg << xcept::stdformat_exception_history( e ) );
        
      XCEPT_RETHROW( rubuilder::filter::exception::Configuration, msg, e );
   }

   // Clear list of destination bu
   destinationBUs_.clear();

   // Fill list of bu destination.
   // Dalle istanze delle bu prese in configurazione dall'array buInstances_,-
   //-si ricavano bu.descriptor e bu.tid e si filla participatingBUs_.
   for( int i=0; ( (xdata::Vector<xdata::UnsignedInteger32>::size_type)i ) < buInstances_.size(); i++ )
   {
      rubuilder::utils::ApplicationDescriptorAndTid bu;

      std::cout << "buInstances_.size(): " << buInstances_.size() << '\n';
      std::cout << "Destination bu instance: " << buInstances_[i] << '\n';

      try
      {
         bu.descriptor =
            app_->getApplicationContext()->
            getDefaultZone()->
            getApplicationDescriptor( "rubuilder::bu::Application", buInstances_[i] );
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
         bu.tid = i2o::utils::getAddressMap()->
            getTid( bu.descriptor );

         //LOG4CPLUS_INFO( logger_, "bu.tid: " << bu.tid );
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


void rubuilder::filter::outproxy::checkBuInstances()
{
   unsigned int i = 0;
   unsigned int j = 0;

   for( i=0; i<buInstances_.size(); i++ )
   {
      // Check participating bu instance is not a duplicate
      for( j=0; j<buInstances_.size(); j++ )
      {
         if( (i != j) && ( buInstances_[i] == buInstances_[j] ) )
         {
            std::stringstream oss;

            oss << "Participating bu instance is a duplicate.";
            oss << " Instance:" << i;

            LOG4CPLUS_ERROR( logger_, oss.str() );

            XCEPT_RAISE( rubuilder::filter::exception::Configuration, oss.str() );
         }
      }
   }
}


void rubuilder::filter::outproxy::fillDestinationFILTERs()
{

   try
   {
      // Check participating filter instance is not a duplicate
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
   //-si ricavano filter.descriptor e filter.tid e si filla participatingFILTERs_.
   for( int i=0; ( (xdata::Vector<xdata::UnsignedInteger32>::size_type)i ) < filterInstances_.size(); i++ )
   {
      rubuilder::utils::ApplicationDescriptorAndTid filter;

      //std::cout << "filterInstances_.size(): " << filterInstances_.size() << std::endl;
      //std::cout << "DEstination filter instance: " << filterInstances_[i] << std::endl;

      try
      {
/*
         filter.descriptor =
            app_->getApplicationContext()->
            getDefaultZone()->
            getApplicationDescriptor( "rubuilder::filter::Application", filterInstances_[i] );
*/

         filter.descriptor =
            app_->getApplicationContext()->
            getDefaultZone()->
            getApplicationDescriptor( "LocalFilter", filterInstances_[i] );
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
         filter.tid = i2o::utils::getAddressMap()->
            getTid( filter.descriptor );

         //LOG4CPLUS_INFO( logger_, "filter.tid: " << filter.tid );
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


void rubuilder::filter::outproxy::checkFilterInstances()
{
   unsigned int i = 0;
   unsigned int j = 0;

   for( i=0; i<filterInstances_.size(); i++ )
   {
      // Check participating filter instance is not a duplicate
      for( j=0; j<filterInstances_.size(); j++ )
      {
         if( (i != j) && ( filterInstances_[i] == filterInstances_[j] ) )
         {
            std::stringstream oss;

            oss << "Participating filter instance is a duplicate.";
            oss << " Instance:" << i;

            LOG4CPLUS_ERROR( logger_, oss.str() );

            XCEPT_RAISE( rubuilder::filter::exception::Configuration, oss.str() );
         }
      }
   }
}


void rubuilder::filter::outproxy::fillDestinationMERGERs()
{

   try
   {
      // Check destination merger instance is not a duplicate
      checkMergerInstances();
   }
   catch( xcept::Exception &e )
   {
      const std::string msg = "Exported parameter \"mergerInstances\" is invalid ";

      LOG4CPLUS_ERROR( logger_,
         msg << xcept::stdformat_exception_history( e ) );

      XCEPT_RETHROW( rubuilder::filter::exception::Configuration, msg, e );
   }

   // Clear list of participating merger
   destinationMERGERs_.clear();

   // Fill list of merger destination.
   // Dalle istanze delle application merger prese in configurazione dall'array mergerInstances_,-
   //-si ricavano merger.descriptor e merger.tid e si filla destinationMERGERs_.
   for( int i=0; ( (xdata::Vector<xdata::UnsignedInteger32>::size_type)i ) < mergerInstances_.size(); i++ )
   {
      rubuilder::utils::ApplicationDescriptorAndTid merger;

      LOG4CPLUS_INFO( logger_, "mergerInstances_.size(): " << mergerInstances_.size() );
      LOG4CPLUS_INFO( logger_, "mergerInstance: " << mergerInstances_[i] );

      try
      {
         merger.descriptor =
            app_->getApplicationContext()->
            getDefaultZone()->
            getApplicationDescriptor( "rubuilder::merger::Application", mergerInstances_[i] );
      }
      catch( xcept::Exception &e )
      {
         std::stringstream oss;

         oss << "Failed to get application descriptor for merger";
         oss << mergerInstances_[i];

         LOG4CPLUS_ERROR( logger_,
            oss.str() << xcept::stdformat_exception_history( e ) );

         XCEPT_RETHROW( rubuilder::filter::exception::Configuration, oss.str(), e );
      }

      try
      {
         merger.tid = i2o::utils::getAddressMap()->
            getTid( merger.descriptor );

         std::cout << "merger.tid: " << merger.tid << std::endl;
      }
      catch( xcept::Exception &e )
      {
         std::stringstream oss;

         oss << "Failed to get I2O TID for merger";
         oss << mergerInstances_[i];

         LOG4CPLUS_ERROR( logger_,
            oss.str() << xcept::stdformat_exception_history( e ) );

         XCEPT_RETHROW( rubuilder::filter::exception::I2o, oss.str(), e );
      }

      destinationMERGERs_.push_back( merger );
   }
}


void rubuilder::filter::outproxy::checkMergerInstances()
{
   unsigned int i = 0;
   unsigned int j = 0;

   for( i=0; i<mergerInstances_.size(); i++ )
   {
      // Check destination merger instance is not a duplicate
      for( j=0; j<mergerInstances_.size(); j++ )
      {
         if( (i != j) && ( mergerInstances_[i] == mergerInstances_[j] ) )
         {
            std::stringstream oss;

            oss << "Destination merger instance is a duplicate.";
            oss << " Instance:" << i;

            LOG4CPLUS_ERROR( logger_, oss.str() );

            XCEPT_RAISE( rubuilder::filter::exception::Configuration, oss.str() );
         }
      }
   }
}


void rubuilder::filter::outproxy::startProcessingWorkLoop()
{

   try
   {
      const std::string identifier = rubuilder::utils::getIdentifier( app_->getApplicationDescriptor() );

      wl_ = toolbox::task::getWorkLoopFactory()->
         getWorkLoop( identifier + "Out", "waiting" );

      outproxyAction_ = toolbox::task::bind( this, &rubuilder::filter::outproxy::outproxyWorkLoop,
         identifier + "outproxyWorkLoop" );

      if( !wl_->isActive() ) {
         wl_->activate();
      }
   }
   catch( xcept::Exception& e )
   {
      std::string msg = "Failed configuring workloop ";

      LOG4CPLUS_ERROR( logger_,
         msg << xcept::stdformat_exception_history( e ) );

      XCEPT_RETHROW( exception::WorkLoop, msg, e );
   }
}


bool rubuilder::filter::outproxy::outproxyWorkLoop( toolbox::task::WorkLoop *wl )
{
   toolbox::mem::Reference *buf_ref;
   I2O_EVENT_DATA_BLOCK_MESSAGE_FRAME *block  = NULL;
   int stopBlock=0;

   int empty=0;

   try
   {
      for( ;; )
      {
         std::cout << "outproxy loop is starting ... " << '\n';

         state_.wait_set( STATE_ENABLED, STATE_RUNNING );

         std::cout << "outproxy loop is running " << '\n';


         while( state_.test( STATE_RUNNING ) )
         {
            //refq_mutex_->take();

            // buf_ref NULL if timeout
	    buf_ref = buf_refq_->get( timeout_ );

            if( buf_ref )
            {
               //std::cout << "buf_ref: " << std::hex << buf_ref << std::dec << std::endl;

               //this->push_I2O_DATA_TO_BU( buf_ref );
                  
               try
               {
                  // Potrebbe essere necessario gestire un loop di ritrasmissione del buff-
                  //-ad es. nella condizione di atcp queue full
                  //for(;;)
                  //{
                     this->push_I2O_DATA_TO_DST( buf_ref );

                     //break;
                  //}
               }
               catch( xcept::Exception &e )
               {
                  LOG4CPLUS_ERROR( logger_, e.what() );

                  XCEPT_RETHROW( rubuilder::filter::exception::Exception,
                     "Failed to send data to destination ", e );
               }
            }

            //refq_mutex_->give();
         }


         std::cout << "stopping ..." << std::endl;

         // In condizioni normali voglio spedire tutti i blocchi, rimango in STOPPING fino-
         //-al detect dello StopBlock.
         while( state_.test( STATE_STOPPING ) )
         {
            // buf_ref NULL if timeout
	    buf_ref = buf_refq_->get( timeout_ );

            //std::cout << "buf_ref: " << std::hex << buf_ref << std::dec << std::endl;

            //std::cout << "stopping ..." << std::endl;

            if( buf_ref )
            {
               empty = 0;            

               //std::cout << "buf_ref: " << std::hex << buf_ref << std::dec << std::endl;

               block = ( I2O_EVENT_DATA_BLOCK_MESSAGE_FRAME* )buf_ref->getDataLocation();

               stopBlock = block->stopBlock;
                  

               this->push_I2O_DATA_TO_DST( buf_ref );


               if( stopBlock ) break;
            }
            else
            {
               empty++;

               //std::cout << "empty: " << empty << std::endl;

               // Non è stato trovato lo STOP block!
               if( empty == 3 ) break;
            }
         }

         std::cout << "stopBlock: " << stopBlock << std::endl;


         state_.set( STATE_STOPPED );

         std::cout << "outproxy loop normally stopped " << std::endl;

         // Don't reschedule loop
         return false;
      }
   }
   catch( xcept::Exception &e )
   {
      try
      {
         // Move to the failed state
         XCEPT_DECLARE_NESTED( rubuilder::filter::exception::WorkLoop,
            forceFailedException, "Failed to send data ", e );

         toolbox::Event::Reference evtRef(
            new rubuilder::utils::ForceFailedEvent( forceFailedException, this ) );

         state_.set( STATE_STOPPED );

         applicationBSem_->take();

         //fsm_.fireEvent( evtRef );

         applicationBSem_->give();
      }
      catch( xcept::Exception &e )
      {
         applicationBSem_->give();

         LOG4CPLUS_FATAL( logger_, "Failed to move to the failed state: " << xcept::stdformat_exception_history(e) );
      }
   }

   std::cout << "outproxy loop exit with errors " << std::endl;

   // Don't reschedule loop
   return false;
}


void rubuilder::filter::outproxy::push_I2O_DATA_TO_DST( toolbox::mem::Reference *bufRef )
{
   toolbox::mem::Reference *headRef;
   toolbox::mem::Reference *bref;

   std::list<rubuilder::utils::ApplicationDescriptorAndTid>::const_iterator itor;

   uint32_t cs=0;
   int nb=0;
   int dup;
   int tsize;

   I2O_EVENT_DATA_BLOCK_MESSAGE_FRAME *block=0;
   I2O_MESSAGE_FRAME *stdMsg=0;
   I2O_PRIVATE_MESSAGE_FRAME *pvtMsg=0;

   //std::cout << "Push to destination" << std::endl;

   // Send chain of buffers to BUs destination
   for( dup=0, itor=destinationBUs_.begin(); itor!=destinationBUs_.end(); dup++, itor++ )
   {
      rubuilder::utils::ApplicationDescriptorAndTid bu = *itor;

      tsize = 0;

      //std::cout << "Send to BUs destination" << std::endl;

      // Se non viene fatto il zero-copy duplicate il post libera la chain
      bref = bufRef->duplicate();

      // Save the reference of the first block of the chain.
      headRef = bref;

      while( bref != 0 )
      {
         block  = ( I2O_EVENT_DATA_BLOCK_MESSAGE_FRAME* )bref->getDataLocation();
         stdMsg = ( I2O_MESSAGE_FRAME* )block;
         pvtMsg = ( I2O_PRIVATE_MESSAGE_FRAME* )block;

         pvtMsg->OrganizationID = XDAQ_ORGANIZATION_ID;

         stdMsg->MsgFlags = 0;
         stdMsg->VersionOffset = 0;
         stdMsg->Function = I2O_PRIVATE_MESSAGE;

         // Modify the source and destination so that the message frame can-
         //-be sent from the RU to the BU.
         //std::cout << "InitiatorAddress: " << tid_ << std::endl;
         stdMsg->InitiatorAddress = tid_;

         stdMsg->TargetAddress = bu.tid;
         //std::cout << "TargetAddress: " << bu.tid << std::endl;

         // Modify the XFunctionCode to be compatible with the BU
         pvtMsg->XFunctionCode = I2O_BU_COLLECT;

         // id_ specifica il tipo di sorgente
         block->sourceId = id_;

         // instance_ univoco per applicazione
         block->sourceInstance = instance_;

         nb++;
         cs += ( stdMsg->MessageSize << 2 );

         tsize += ( stdMsg->MessageSize << 2 );

         //std::cout << "bu tsize:" << tsize << std::endl;

         bref->setDataSize( stdMsg->MessageSize << 2 );

         bref = bref->getNextReference();
      }

      //headRef->setDataSize( tsize );

      try
      {
         appContext_->postFrame
         (
            headRef,
            app_->getApplicationDescriptor(),
            bu.descriptor//,
            //i2oExceptionHandler_,
            //bu.descriptor
         );
      }
      catch( xcept::Exception &e )
      {
         std::string s = e.what();

         LOG4CPLUS_ERROR( logger_, s );

         // In caso di errore queste free mandano l'applicazione in segmentation fault
         //headRef->release();
         //bufRef->release();

         XCEPT_RETHROW( rubuilder::filter::exception::Exception,
            "postFrame failed ", e );
      }
   }


   // Send chain of buffers to MERGER destination
   for( dup=0, itor=destinationMERGERs_.begin(); itor!=destinationMERGERs_.end(); dup++, itor++ )
   {
      rubuilder::utils::ApplicationDescriptorAndTid merger = *itor;

      tsize = 0;

      //std::cout << "Send to MERGER destination" << std::endl;

      // Se non viene fatto il zero-copy duplicate il post libera la chain
      bref = bufRef->duplicate();

      // Save the reference of the first block of the chain.
      headRef = bref;

      while( bref != 0 )
      {
         block  = ( I2O_EVENT_DATA_BLOCK_MESSAGE_FRAME* )bref->getDataLocation();
         stdMsg = ( I2O_MESSAGE_FRAME* )block;
         pvtMsg = ( I2O_PRIVATE_MESSAGE_FRAME* )block;

         pvtMsg->OrganizationID = XDAQ_ORGANIZATION_ID;

         stdMsg->MsgFlags = 0;
         stdMsg->VersionOffset = 0;
         stdMsg->Function = I2O_PRIVATE_MESSAGE;

         // Modify the source and destination so that the message frame can-
         //-be sent from the BU to the MERGER.
         //std::cout << "InitiatorAddress: " << tid_ << std::endl;
         stdMsg->InitiatorAddress = tid_;

         stdMsg->TargetAddress = merger.tid;
         //std::cout << "TargetAddress: " << merger.tid << std::endl;

         // Modify the XFunctionCode to be compatible with the MERGER
         pvtMsg->XFunctionCode = I2O_MERGER_COLLECT;

         // id_ specifica il tipo di sorgente
         block->sourceId = id_;

         // instance_ univoco per applicazione
         block->sourceInstance = instance_;

         nb++;
         cs += ( stdMsg->MessageSize << 2 );

         tsize += ( stdMsg->MessageSize << 2 );

         //std::cout << "merger tsize:" << tsize << std::endl;

         bref->setDataSize( stdMsg->MessageSize << 2 );

         bref = bref->getNextReference();
      }

      //headRef->setDataSize( tsize );

      try
      {
         appContext_->postFrame
         (
            headRef,
            app_->getApplicationDescriptor(),
            merger.descriptor//,
            //i2oExceptionHandler_,
            //merger.descriptor
         );
      }
      catch( xcept::Exception &e )
      {
         std::string s = e.what();

         LOG4CPLUS_ERROR( logger_, s );

         // In caso di errore queste free mandano l'applicazione in segmentation fault
         //headRef->release();
         //bufRef->release();

         XCEPT_RETHROW( rubuilder::filter::exception::Exception,
            "postFrame failed ", e );
      }
   }


   // Send chain of buffers to FILTERs destination
   for( dup=0, itor=destinationFILTERs_.begin(); itor!=destinationFILTERs_.end(); dup++, itor++ )
   {
      rubuilder::utils::ApplicationDescriptorAndTid filter = *itor;

      tsize = 0;

      //std::cout << "Send to FILTERs destination" << std::endl;

      // Se non viene fatto il zero-copy duplicate il post libera la chain di buffer
      bref = bufRef->duplicate();

      // Save the reference of the first block of the chain.
      headRef = bref;

      while( bref != 0 )
      {
         //std::cout << "filter bref:" << std::hex << bref << std::dec << std::endl;

         block  = ( I2O_EVENT_DATA_BLOCK_MESSAGE_FRAME* )bref->getDataLocation();
         stdMsg = ( I2O_MESSAGE_FRAME* )block;
         pvtMsg = ( I2O_PRIVATE_MESSAGE_FRAME* )block;

         pvtMsg->OrganizationID = XDAQ_ORGANIZATION_ID;

         stdMsg->MsgFlags = 0;
         stdMsg->VersionOffset = 0;
         stdMsg->Function = I2O_PRIVATE_MESSAGE;

         // Modify the source and destination so that the message frame can-
         //-be sent from the RU to the BU.
         //std::cout << "InitiatorAddress: " << tid_ << std::endl;
         stdMsg->InitiatorAddress = tid_;

         stdMsg->TargetAddress = filter.tid;
         //std::cout << "TargetAddress: " << bu.tid << std::endl;

         // Modify the XFunctionCode to be compatible with the FILTER
         pvtMsg->XFunctionCode = I2O_FILTER_COLLECT;

         // id_ specifica il tipo di sorgente
         block->sourceId = id_;

         // instance_ univoco per applicazione
         block->sourceInstance = instance_;

         nb++;
         cs += ( stdMsg->MessageSize << 2 );

         tsize += ( stdMsg->MessageSize << 2 );

         //std::cout << "filter tsize:" << tsize << std::endl;

         bref->setDataSize( stdMsg->MessageSize << 2 );

         bref = bref->getNextReference();
      }

      //headRef->setDataSize( tsize );

      try
      {
         appContext_->postFrame
         (
            headRef,
            app_->getApplicationDescriptor(),
            filter.descriptor//,
            //i2oExceptionHandler_,
            //filter.descriptor
         );
      }
      catch( xcept::Exception &e )
      {
         std::string s = e.what();

         LOG4CPLUS_ERROR( logger_, s );

         // In caso di errore queste free mandano l'applicazione in segmentation fault
         //headRef->release();
         //bufRef->release();

         XCEPT_RETHROW( rubuilder::filter::exception::Exception,
            "postFrame failed ", e );
      }
   }

   // For monitoring
   //monitoringInfoSpace_->lock(); appInfoSpace_->lock();

   nbBuffs_ += nb;

   sumOfSizes_ += cs;

   //monitoringInfoSpace_->unlock(); appInfoSpace_->unlock();

   if( bufRef )
      bufRef->release();
}


uint32_t rubuilder::filter::outproxy::get_tbuff()
{

   return nbBuffs_;
}


uint32_t rubuilder::filter::outproxy::get_tsize()
{

   return sumOfSizes_;
}


int rubuilder::filter::outproxy::qfull()
{

   return buf_refq_->qfull();
}


void rubuilder::filter::outproxy::qstop()
{

   buf_refq_->stop();
}

