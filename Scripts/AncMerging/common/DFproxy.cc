#include "interface/shared/i2oEVBMsgs.h"
#include "interface/shared/i2oXFunctionCodes.h"
#include "rubuilder/lfcaendgtz/DFproxy.h"
#include "rubuilder/utils/CreateStrings.h"
#include "rubuilder/utils/ForceFailedEvent.h"


rubuilder::filter::DFproxy::DFproxy
(
   xdaq::Application *app,
   toolbox::BSem *applicationBSem,
   std::string outputFilepath,
   std::string outputFileExtension,
   uint32_t RunNum,
   uint64_t outFileMaxSize,
   int outFileCycleCounter,
   log4cplus::Logger &logger
):
app_( app ),
state_(16),
logger_( log4cplus::Logger::getInstance( logger.getName() + ".DFproxy" ) )
{
   time_t rawtime;
   struct tm *timeinfo;

   LOG4CPLUS_INFO( logger_, "START of CTOR" );

   outputFilepath_ = outputFilepath;

   outputFileExt_ = outputFileExtension;

   RunNum_ = RunNum;

   outFileMaxSize_ = outFileMaxSize;

   outFileCycleCounter_ = outFileCycleCounter;

   cycnt_ = 0;   

   applicationBSem_ = applicationBSem;

   out_file_ = NULL;

   outFileSize_ = 0;

   timeout_ = TIMEOUT_DFPROXY_GET;

   nbBuffs_ = 0;

   sumOfSizes_ = 0;


   try
   {
      out_file_ = new OutputFile( logger_ );

      // Generate data file full path (run_num and index added)
      outputFullFilepath_= rubuilder::utils::generateOutputFileName
      (
         outputFilepath_, outputFileExt_, RunNum, cycnt_
      );

      // Open data file
      out_file_->set_path( outputFullFilepath_.c_str() );

      // Intoduce Start time of run in the data file header
      rawtime = time( NULL );

      timeinfo = localtime( &rawtime );
  
      // Run number 
      // Time info
      dfh_.RunNum = RunNum;
      dfh_.bt.tm_year = timeinfo->tm_year + 1900;
      dfh_.bt.tm_year = timeinfo->tm_mon + 1;
      dfh_.bt.tm_year = timeinfo->tm_mday;
      dfh_.bt.tm_year = timeinfo->tm_hour;
      dfh_.bt.tm_year = timeinfo->tm_sec;

      // Write header
      //out_file_->write( (char *)&dfh_, sizeof( dfh_ ) );
   }
   catch( xcept::Exception& e )
   {  
      LOG4CPLUS_ERROR( logger_, "File proxy error" );

      XCEPT_RETHROW( rubuilder::filter::exception::Exception, "File proxy error ", e );
   }


   try
   {
      buf_refq_ = new BufRefQ( DFPROXY_QUEUE_DEPTH );
   }
   catch(...)
   {
      throw;
   }

   state_.set( STATE_DISABLED );

   startProcessingWorkLoop();

   LOG4CPLUS_INFO( logger_, "END of CTOR" );
}


rubuilder::filter::DFproxy::~DFproxy()
{

   LOG4CPLUS_INFO( logger_, "START of DTOR" );

   if( buf_refq_ )
      delete buf_refq_;

   LOG4CPLUS_INFO( logger_, "END of DTOR" );
}


int rubuilder::filter::DFproxy::put( toolbox::mem::Reference *bufRef, int timeout )
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


void rubuilder::filter::DFproxy::enable()
{

   wl_->submit( fileAction_ );

   state_.set( STATE_ENABLED );
}


void rubuilder::filter::DFproxy::disable()
{

   // Dopo lo stop non vengono accettati altri buffer nella coda.
   buf_refq_->stop();


   if( state_.test_set( STATE_RUNNING, STATE_STOPPING ) )
   {

      //LOG4CPLUS_INFO( logger_, "File loop is waiting for STATE_STOPPED... " );

      state_.wait( STATE_STOPPED );
   }

   if( out_file_ )
   {
      out_file_->close();

      // Refresh size in bytes
      out_file_->checkOutputFileSize( outFileSize_ );

      delete out_file_;

      out_file_ = NULL;
   }

   state_.set( STATE_DISABLED );
}


void rubuilder::filter::DFproxy::startProcessingWorkLoop()
{

   try
   {
      const std::string identifier = rubuilder::utils::getIdentifier( app_->getApplicationDescriptor() );

      wl_ = toolbox::task::getWorkLoopFactory()->
         getWorkLoop( identifier + "File", "waiting" );

      fileAction_ = toolbox::task::bind( this, &rubuilder::filter::DFproxy::outWorkLoopFile,
         identifier + "outWorkLoopFile" );

      if( !wl_->isActive() ) {
         wl_->activate();
      }
   }
   catch( xcept::Exception& e )
   {
      std::string msg = "Failed configuring workloop";

      LOG4CPLUS_ERROR( logger_,
         msg << xcept::stdformat_exception_history( e ) );

      XCEPT_RETHROW( exception::WorkLoop, msg, e );
   }
}


bool rubuilder::filter::DFproxy::outWorkLoopFile( toolbox::task::WorkLoop *wl )
{
   toolbox::mem::Reference *buf_ref=0;
   I2O_EVENT_DATA_BLOCK_MESSAGE_FRAME *block=0;
   int stopBlock=0;

   int empty=0;

   try
   {
      for( ;; )
      {
         //LOG4CPLUS_INFO( logger_, "File loop is starting... " );
         std::cout << "File loop is starting... " << std::endl;

         state_.wait_set( STATE_ENABLED, STATE_RUNNING );

         //LOG4CPLUS_INFO( logger_, "File loop is running " );
         std::cout << "File loop is running " << std::endl;

         while( state_.test( STATE_RUNNING ) )
         {
            //refq_mutex_->take();

            // buf_ref is NULL if timeout
            // BufRefQ.h
	    buf_ref = buf_refq_->get( timeout_ );

            if( buf_ref )
            {
               block = ( I2O_EVENT_DATA_BLOCK_MESSAGE_FRAME* )buf_ref->getDataLocation();

               stopBlock = block->stopBlock;

               if( stopBlock )
               {
                  LOG4CPLUS_INFO( logger_, "stopBlock coming " );
                  state_.set( STATE_STOPPED );
                  buf_ref->release();
                  LOG4CPLUS_INFO( logger_, "File loop normally stopped " );
                  return false;
               }

	       this->write_I2O_DATA_TO_OutputFile( buf_ref );
            }

            //refq_mutex_->give();
         }


         // In condizioni normali voglio spedire tutti i blocchi, STOPPING until stopBlock
         while( state_.test( STATE_STOPPING ) )
         {
            // buf_ref NULL if timeout
	    buf_ref = buf_refq_->get( timeout_ );

            if( buf_ref )
            {
               empty = 0;            

               //std::cout << "buf_ref: " << std::hex << buf_ref << std::dec << std::endl;

               block  = ( I2O_EVENT_DATA_BLOCK_MESSAGE_FRAME* )buf_ref->getDataLocation();

               stopBlock = block->stopBlock;

               if( stopBlock )
               {
                  LOG4CPLUS_INFO( logger_, "stopBlock coming " );
                  buf_ref->release();
                  break;
               }
                  
               this->write_I2O_DATA_TO_OutputFile( buf_ref );
            }
            else
            {
               empty++;

               // No stopBlock
               if( empty == 3 ) break;
            }
         }


         state_.set( STATE_STOPPED );

         LOG4CPLUS_INFO( logger_, "File loop normally stopped " );

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
            forceFailedException, "outWorkLoopFile error ", e );

         toolbox::Event::Reference evtRef(
            new rubuilder::utils::ForceFailedEvent( forceFailedException, this ) );

         applicationBSem_->take();

         //fsm_.fireEvent( evtRef );

         applicationBSem_->give();
      }
      catch( xcept::Exception &e )
      {
         applicationBSem_->give();

         LOG4CPLUS_FATAL( logger_, "Failed to move to the failed state: " << xcept::stdformat_exception_history( e ) );
      }
   }

   //LOG4CPLUS_INFO( logger_, "File loop exit with errors " );
   std::cout << "File loop exit with errors " << std::endl;

   // Don't reschedule loop
   return false;
}


void rubuilder::filter::DFproxy::write_I2O_DATA_TO_OutputFile( toolbox::mem::Reference *bufRef )
{
   I2O_EVENT_DATA_BLOCK_MESSAGE_FRAME *block=0;
   I2O_MESSAGE_FRAME *stdMsg=0;
   I2O_PRIVATE_MESSAGE_FRAME *pvtMsg=0;
   toolbox::mem::Reference *headRef=0;

   char *dbuf;
   uint32_t msgLen;
   uint32_t userDataLen;
   std::time_t rawtime;
   struct tm *timeinfo;


   // Save the reference of the first block of the chain.
   headRef = bufRef;

   while( bufRef )
   {
      block  = ( I2O_EVENT_DATA_BLOCK_MESSAGE_FRAME* )bufRef->getDataLocation();
      stdMsg = ( I2O_MESSAGE_FRAME* )block;
      pvtMsg = ( I2O_PRIVATE_MESSAGE_FRAME* )block;
      dbuf   = ( char* )block;

      // Bytes
      msgLen = stdMsg->MessageSize << 2;
      //std::cout << "msgLen (Bytes): " << std::showbase << (stdMsg->MessageSize << 2) << std::endl;
		
      userDataLen = msgLen - sizeof( I2O_EVENT_DATA_BLOCK_MESSAGE_FRAME );
      //std::cout << "userDataLen: " << std::showbase << userDataLen << std::endl;
	   
      dbuf = &dbuf[sizeof( I2O_EVENT_DATA_BLOCK_MESSAGE_FRAME )];

      // For monitoring
      nbBuffs_++;

      //if( !out_file_ )
      //{
         //monitoringInfoSpace_->lock(); appInfoSpace_->lock();

         //sumOfSizes_ += ( stdMsg->MessageSize << 2 );

         //monitoringInfoSpace_->unlock(); appInfoSpace_->unlock();
      //}
      
      
      //applicationBSem_->take();
 
  
      try
      {
         if( out_file_ )
         {
            out_file_->write( dbuf, userDataLen );

            // For monitoring
            //monitoringInfoSpace_->lock(); appInfoSpace_->lock();

            sumOfSizes_ += userDataLen;

            //monitoringInfoSpace_->unlock(); appInfoSpace_->unlock();

            // Size in bytes
            // "tellp" is used to get the position in the stream after the writing operation.
	    out_file_->checkOutputFileSize( outFileSize_ );
	 
            // Limite imposto alla size del file
	    if( outFileMaxSize_ )
	    {
	       if( outFileSize_ > outFileMaxSize_ )
	       {
                  // Write trailer, close file

                  // Time for data file trailer
                  rawtime = time( NULL );

                  timeinfo = localtime( &rawtime );
  
                  // Time info
                  dft_.bt.tm_year = timeinfo->tm_year + 1900;
                  dfh_.bt.tm_year = timeinfo->tm_mon + 1;
                  dfh_.bt.tm_year = timeinfo->tm_mday;
                  dfh_.bt.tm_year = timeinfo->tm_hour;
                  dfh_.bt.tm_year = timeinfo->tm_sec;
	       
	          //out_file_->write( (char *)&dft_, sizeof( dft_ ) );
		  
	          // Close outputFile
	          out_file_->close();
		  
		  // Genera nome file con index aggiornato (cycnt_)
		  outputFullFilepath_= rubuilder::utils::generateOutputFileName
		  (
		     outputFilepath_, outputFileExt_, RunNum_, ++cycnt_
		  );
		     
		  // Open data file
		  out_file_->set_path( outputFullFilepath_.c_str() );
		  
		  // Run number 
		  // Time info
		  dfh_.RunNum = RunNum_;
		  dfh_.bt.tm_year = timeinfo->tm_year + 1900;
		  dfh_.bt.tm_year = timeinfo->tm_mon + 1;
		  dfh_.bt.tm_year = timeinfo->tm_mday;
		  dfh_.bt.tm_year = timeinfo->tm_hour;
		  dfh_.bt.tm_year = timeinfo->tm_sec;

		  // Write header
		  //out_file_->write( (char *)&dfh_, sizeof( dfh_ ) );
	       
               }
	    }//if( outFileMaxSize_ )
         }//End if( out_file_ )
      }
      catch( xcept::Exception &e )
      {
         LOG4CPLUS_ERROR( logger_, e.what() );

         XCEPT_RETHROW( rubuilder::filter::exception::File, "Error on write_I2O_DATA_TO_OutputFile ", e );
      }

      
      //applicationBSem_->give();
	
 
      bufRef = bufRef->getNextReference();
   }
      
   // Release frame
   if( headRef )
      headRef->release();
}


uint32_t rubuilder::filter::DFproxy::get_tsize()
{

   return sumOfSizes_;
}


uint64_t rubuilder::filter::DFproxy::get_dfsize()
{

   return outFileSize_;
}


int rubuilder::filter::DFproxy::qfull()
{

   return buf_refq_->qfull();
}


void rubuilder::filter::DFproxy::qstop()
{

   buf_refq_->stop();
}

