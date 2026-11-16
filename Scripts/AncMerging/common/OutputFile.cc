#include "rubuilder/lfcaendgtz/OutputFile.h"
#include "rubuilder/lfcaendgtz/exception/Exception.h"

#include <errno.h>
#include <sstream>
#include <string>
	

rubuilder::filter::OutputFile::OutputFile( log4cplus::Logger& logger ) :
logger_( log4cplus::Logger::getInstance( logger.getName() + ".OutputFile" ) )
{

   LOG4CPLUS_INFO( logger_, "START of CTOR" );

   // Set exception mask
   file_.exceptions( std::ofstream::eofbit | std::ofstream::failbit | std::ofstream::badbit );

   LOG4CPLUS_INFO( logger_, "END of CTOR" );  
}


rubuilder::filter::OutputFile::OutputFile( const char *path, log4cplus::Logger& logger ) :
logger_( log4cplus::Logger::getInstance( logger.getName() + ".OutputFile" ) )
{
   std::stringstream fss;

   LOG4CPLUS_INFO( logger_, "START of CTOR" );

   // Set exception mask
   file_.exceptions( std::ofstream::eofbit | std::ofstream::failbit | std::ofstream::badbit );

   if( file_.is_open() )
   {
      fss << "File: " << path << " is already open ";

      XCEPT_RAISE( rubuilder::filter::exception::File, fss.str() );
   }
   
   try
   {
      //file_.open( path, std::ios::out | std::ios::app | std::ios::binary );
      file_.open( path, std::ios::out | std::ios::binary );
   }
   catch( const std::ofstream::failure& e )
   {
      std::stringstream oss;
      std::string s;

      oss << "Error opening outputFile "<< path
         << ": " << strerror( errno ) << std::endl;
      
      LOG4CPLUS_ERROR( logger_, oss.str() );

      XCEPT_RAISE( rubuilder::filter::exception::File, oss.str() );
   }
   
   LOG4CPLUS_INFO( logger_, "File " << path << " opened" );

   LOG4CPLUS_INFO( logger_, "END of CTOR" );
}


rubuilder::filter::OutputFile::~OutputFile()
{

   LOG4CPLUS_INFO( logger_, "DTOR" );

   if( file_.is_open() ) {
      this->close();
   }
}


void rubuilder::filter::OutputFile::close()
{
	
   try
   {
      if( file_.is_open() )
      {
         file_.flush();

         file_.close();

         file_.clear();
      }
      else
      {
         LOG4CPLUS_INFO( logger_, "File has been already closed" );
      }
   }
   catch( const std::ofstream::failure& e )
   {
      std::stringstream oss;

      oss << "Error closing file: " << strerror( errno ) << std::endl;
      
      LOG4CPLUS_ERROR( logger_, oss.str() );

      XCEPT_RAISE( rubuilder::filter::exception::File, oss.str() );
   }
      
   LOG4CPLUS_INFO( logger_, "File closed" );
}

	
void rubuilder::filter::OutputFile::set_path( const char *path )
{
   std::stringstream fss;

   if( file_.is_open() )
   {
      fss << "File: " << path << " is already open ";

      XCEPT_RAISE( rubuilder::filter::exception::File, fss.str() );
   }
   
   try
   {
      //file_.open( path, std::ios::out | std::ios::app | std::ios::binary );
      file_.open( path, std::ios::out | std::ios::binary );
   }
   catch( const std::ofstream::failure& e )
   {
      std::stringstream oss;

      oss << "Error opening file " << path
         << ": " << strerror( errno ) << std::endl;
      
      LOG4CPLUS_ERROR( logger_, oss.str() );

      XCEPT_RAISE( rubuilder::filter::exception::File, oss.str() );
   }
   
   LOG4CPLUS_INFO( logger_, "File " << path << " opened" );
}


void rubuilder::filter::OutputFile::write( char *buf, uint32_t buf_len )
{

   //std::cout << "buf_len: " << std::hex << buf_len << std::dec << '\n';

   if( !file_.is_open() ) {
       XCEPT_RAISE( rubuilder::filter::exception::File, "File is not open " );
   }

   try
   {
     if( buf_len )
         file_.write( buf, buf_len );
   }
   catch( const std::ofstream::failure& e )
   {
      std::stringstream oss;
         
      oss << "Error writing file: " << strerror(errno) << " ( buf_len = " << buf_len << ") "<< std::endl;
      
      LOG4CPLUS_ERROR( logger_, oss.str() );

      XCEPT_RAISE( rubuilder::filter::exception::File, oss.str() );
   }
}


void rubuilder::filter::OutputFile::checkOutputFileSize( uint64_t &pos )
{

   // "tellp" is used to get the position in the stream after the writing operation
   // Concurrent access to the same stream object may cause data races.
   if( file_.is_open() ) {
      pos = file_.tellp();
   }
}


std::ofstream *rubuilder::filter::OutputFile::getStream() {
   return( &file_ );
}
