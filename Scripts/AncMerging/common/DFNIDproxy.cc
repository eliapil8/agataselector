#include "interface/shared/i2oEVBMsgs.h"
#include "interface/shared/i2oXFunctionCodes.h"
#include "rubuilder/lfcaendgtz/DFNIDproxy.h"
#include "rubuilder/utils/CreateStrings.h"
#include "rubuilder/utils/ForceFailedEvent.h"

const int BLOCKSIZE = 262144;

rubuilder::filter::DFNIDproxy::DFNIDproxy
(
   xdaq::Application *app,
   toolbox::BSem *applicationBSem,
   std::string outputFilepath,
   std::string outputFileExtension,
   uint32_t RunNum,
   uint64_t outFileMaxSize,
   int outFileCycleCounter,
   log4cplus::Logger& logger
):
app_( app ),
state_(16),
logger_( log4cplus::Logger::getInstance( logger.getName() + ".DFNIDproxy" ) )
{
   time_t rawtime;
   struct tm *timeinfo;

   LOG4CPLUS_INFO( logger_, "START of CTOR" );

   outputFilepath_ = outputFilepath;
   
   outputFileExt_ = outputFileExtension;

   RunNum_ = RunNum;

   outFileMaxSize_ = outFileMaxSize;

   cycnt_ = 0;

   outFileCycleCounter_ = outFileCycleCounter;

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


   // Only for not Idle event
   output_buffer_ = new char[ BLOCKSIZE ];


   state_.set( STATE_DISABLED );

   startProcessingWorkLoop();

   LOG4CPLUS_INFO( logger_, "END of CTOR" );
}


rubuilder::filter::DFNIDproxy::~DFNIDproxy()
{

   LOG4CPLUS_INFO( logger_, "START of DTOR" );

   if( buf_refq_ )
      delete buf_refq_;

   if( output_buffer_ )
      delete[] output_buffer_;

   LOG4CPLUS_INFO( logger_, "END of DTOR" );
}


int rubuilder::filter::DFNIDproxy::put( toolbox::mem::Reference *bufRef, int timeout )
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
      // timeout=0, call bloccante se non c'è spazio
      // Exit if "buf_refq_->stop()"
      retcode = buf_refq_->put1( bufRef, timeout );

      return retcode;
   }
   catch( BufRefQ::Stopped& e )
   {
      //re-throw BufRefQ::Stopped();
      throw;
   }
}


void rubuilder::filter::DFNIDproxy::enable()
{

   wl_->submit( fileAction_ );

   state_.set( STATE_ENABLED );
}


void rubuilder::filter::DFNIDproxy::disable()
{

   // Dopo lo stop non vengono accettati altri buffer nella coda.
   buf_refq_->stop();


   if( state_.test_set( STATE_RUNNING, STATE_STOPPING ) )
   {
      LOG4CPLUS_INFO( logger_, "File loop is waiting for STATE_STOPPED ... " );

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


void rubuilder::filter::DFNIDproxy::startProcessingWorkLoop()
{

   try
   {
      const std::string identifier = rubuilder::utils::getIdentifier( app_->getApplicationDescriptor() );

      wl_ = toolbox::task::getWorkLoopFactory()->
         getWorkLoop( identifier + "File", "waiting" );

      fileAction_ = toolbox::task::bind( this, &rubuilder::filter::DFNIDproxy::outWorkLoopFile,
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


bool rubuilder::filter::DFNIDproxy::outWorkLoopFile( toolbox::task::WorkLoop *wl )
{
   toolbox::mem::Reference *buf_ref;
   I2O_EVENT_DATA_BLOCK_MESSAGE_FRAME *block=NULL;
   int stopBlock=0;

   int empty=0;

   try
   {
      for( ;; )
      {
         std::cout << "File loop is starting ... " << '\n';

         state_.wait_set( STATE_ENABLED, STATE_RUNNING );

         std::cout << "File loop is running ... " << '\n';

         while( state_.test( STATE_RUNNING ) )
         {
            //refq_mutex_->take();

            // buf_ref is NULL if timeout
	    buf_ref = buf_refq_->get( timeout_ );

            //std::cout << "buf_ref: " << std::hex << buf_ref << std::dec << std::endl;

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


         // In condizioni normali voglio spedire tutti i blocchi, STOPPING until stopBlock.
         while( state_.test( STATE_STOPPING ) )
         {
            LOG4CPLUS_INFO( logger_, "File loop stopping ... " );

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

   std::cout << "File loop exit with errors " << '\n';

   // Don't reschedule loop
   return false;
}


void rubuilder::filter::DFNIDproxy::write_I2O_DATA_TO_OutputFile( toolbox::mem::Reference *bufRef )
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

   int packet_ID;

   int evLenByte;

   char *srcBuff;
   char *outBuff;

   uint32_t wrLen=0;
   int idleEv;


   // Save the reference of the first block of the chain.
   headRef = bufRef;

   while( bufRef != 0 )
   {
      block  = (I2O_EVENT_DATA_BLOCK_MESSAGE_FRAME *)bufRef->getDataLocation();
      stdMsg = (I2O_MESSAGE_FRAME *)block;
      pvtMsg = (I2O_PRIVATE_MESSAGE_FRAME *)block;
      dbuf = (char *)block;

      // packet_ID only for GGP: 0/Short, 0x50/Long, 0x60/Spectra
      packet_ID = block->packetType;

      //LOG4CPLUS_INFO( logger_, "Instance: " << src );

      //std::cout << "msgLen: " << (stdMsg->MessageSize << 2) << std::endl;
      msgLen = stdMsg->MessageSize << 2;
		
      userDataLen = msgLen - sizeof( I2O_EVENT_DATA_BLOCK_MESSAGE_FRAME );

      //std::cout << "userDataLen: " << userDataLen << std::endl;
	   
      dbuf = &dbuf[sizeof( I2O_EVENT_DATA_BLOCK_MESSAGE_FRAME )];


      srcBuff = dbuf;

      outBuff = output_buffer_;

      wrLen = userDataLen;

      // Purge Idle event
      wrLen = SetOutputBuffer( srcBuff, outBuff, userDataLen );

      srcBuff = output_buffer_;


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
            if( wrLen ) {
               out_file_->write( srcBuff, wrLen );
            }

            // For monitoring
            //monitoringInfoSpace_->lock(); appInfoSpace_->lock();

            sumOfSizes_ += wrLen;

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
		     outputFilepath_, outputFileExt_,  RunNum_, ++cycnt_
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
	    } //end if (outFileMaxSize_)
         }//End if( out_file_ )
      }//end try
      catch( xcept::Exception &e )
      {
         LOG4CPLUS_ERROR( logger_, e.what() );

         XCEPT_RETHROW( rubuilder::filter::exception::File, "Error on write_I2O_DATA_TO_File ", e );
      }
      
      //applicationBSem_->give();
	
 
      bufRef = bufRef->getNextReference();
   }
      
   // Release frame
   if( headRef )
      headRef->release();
}


inline int rubuilder::filter::DFNIDproxy::SetOutputBuffer( char *inpBuff, char *outBuff, uint32_t inLen )
{
   agataKey *evH;
   int evLenByte;
   int wrLen=0;
   //   int prevLenByte=-1024;
     
//   uint32_t *intBuff = (uint32_t*)inpBuff;
//   printf("\n NEW BUFFER of size = %i\n",inLen);
//   for(int j = 0 ; j < inLen/sizeof(uint32_t) ; j++ ){
//     printf("%08X ",intBuff[j]);
//     if(j%4==0 && j!=0) printf("\n");
//   }
//   printf("\n END OF BUFFER\n");
   while( inLen )
   {
      evH  = (agataKey*)inpBuff;

      evLenByte = evH->size;//*sizeof( uint32_t );
      /*   
	   if(evLenByte != 52 && evLenByte!= 24 && evLenByte!= 28){
	   LOG4CPLUS_INFO( logger_, "Warning ... evLenByte not the proper size " << evLenByte << " previous event size = " << prevLenByte );
	   //	printf("Warning ... evLenByte not the proper size %08X previous event size = %08X\n",
	   //	       evLenByte, prevLenByte);
	   //	printf("\t -size = %08X\n",evH->size);
	   //	printf("\t -key  = %08X\n",evH->key);
	   //	printf("\t -eNum = %08X\n",evH->evnum);
	   //	printf("\t -t0   = %08X\n",evH->ts_0);
	   //	printf("\t -t1   = %08X\n",evH->ts_1);
	   }
      */
      //prevLenByte = evLenByte; 

      // Not full event in the buff
      if( evLenByte > inLen ) {
	LOG4CPLUS_INFO( logger_, "Warning ... evLenByte ("<<std::hex << evLenByte << std::dec <<") > inLen ("<<inLen<<")" );
	break;
      }
      
      if( !evH->IsIdle() )
      {
         memcpy( (void*)outBuff, (void*)inpBuff, evLenByte );
         outBuff += evLenByte;
         wrLen += evLenByte;
      }

      inpBuff += evLenByte;

      inLen -= evLenByte;
 
      //std::cout << "evLenByte: " << evLenByte << std::endl;
      //std::cout << "inLen: " << inLen << std::endl;

      //assert( !((inLen & 0xffff0000)==0xffff0000 ));
   }

   return wrLen;
}


uint32_t rubuilder::filter::DFNIDproxy::get_tsize()
{

   return sumOfSizes_;
}


uint64_t rubuilder::filter::DFNIDproxy::get_dfsize()
{

   return outFileSize_;
}


int rubuilder::filter::DFNIDproxy::qfull()
{

   return buf_refq_->qfull();
}


void rubuilder::filter::DFNIDproxy::qstop()
{

   buf_refq_->stop();
}

