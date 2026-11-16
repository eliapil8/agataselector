#include "interface/shared/i2oEVBMsgs.h"
#include "rubuilder/lfcaendgtz/exception/Exception.h"
#include "rubuilder/lfcaendgtz/ptServer.h"
#include "rubuilder/utils/CreateStrings.h"
#include "rubuilder/utils/ForceFailedEvent.h"
#include "xcept/tools.h"

// BLOCKSIZE deve essere almeno uguale alla size del blocco dati che arriva via socket dalla RU

//const int BLOCKSIZE = 2048;
//const int BLOCKSIZE = 131072;

// MessageSize field on 16 bit (i2o header)
//const int BLOCKSIZE = 262140;

const int BLOCKSIZE = 262144;


rubuilder::filter::ptServer::ptServer
(
   xdaq::Application *app,
   int port,
   log4cplus::Logger& logger
):
MTask( "ptServer" ),
app_( app ),
logger_( log4cplus::Logger::getInstance( logger.getName() + ".ptServer" ) )
{
   std::string xmlClass;

   LOG4CPLUS_INFO( logger_, "CTOR" );


   //std::string loggerName = logger.getName() + ".ptServer";

   //logger_ = log4cplus::Logger::getInstance( loggerName );


   xmlClass = app_->getApplicationDescriptor()->getClassName();

   // Application instance number
   instance_ = app_->getApplicationDescriptor()->getInstance();


   poolFactory_ = toolbox::mem::getMemoryPoolFactory();

   std::string poolName = rubuilder::utils::generatePoolName( xmlClass, instance_ );

   try
   {
      toolbox::net::URN urn( "toolbox-mem-pool", poolName );

      pool_ = poolFactory_->findPool( urn );
   }
   catch( xcept::Exception &e )
   {
      const std::string msg = "Memory pool not found: " + poolName;

      LOG4CPLUS_ERROR( logger_,
         msg << xcept::stdformat_exception_history(e) );
   }


   LOG4CPLUS_INFO( logger_, "port: " << port );

   port_ = port;

   addr_ = INADDR_ANY;

   g_sock_ = 0;

   FD_ZERO( &recv_fd_set_ );


   if( socketpair( AF_UNIX, SOCK_STREAM, 0, &sig_sock_[0] ) != 0 )
   {
      LOG4CPLUS_INFO( logger_, "socketpair errno: " << errno );

      XCEPT_RAISE( rubuilder::filter::exception::socket, "socketpair" );
   }


   listen_sock_ = socket( AF_INET, SOCK_STREAM, 0 );

   if( listen_sock_ == -1 )
   {
      LOG4CPLUS_INFO( logger_, "socket errno: " << errno );

      XCEPT_RAISE( rubuilder::filter::exception::socket, "socketpair" );
   }

   LOG4CPLUS_INFO( logger_, "socket listen_sock: " << listen_sock_ );


   int sock_opt = 1;

   int r = setsockopt( listen_sock_, SOL_SOCKET, SO_REUSEADDR, &sock_opt, sizeof(int) );

   if( r == -1 )
   {
      LOG4CPLUS_INFO( logger_, "bind errno: " << errno );

      XCEPT_RAISE( rubuilder::filter::exception::socket, "setsockopt" );
   }


   struct sockaddr_in srv_addr;

   srv_addr.sin_family = AF_INET;

   srv_addr.sin_addr.s_addr = htonl( addr_ );

   srv_addr.sin_port = htons( port_ );

   r = bind( listen_sock_, ( struct sockaddr* )&srv_addr, sizeof( srv_addr ) );

   if( r == -1 )
   {
      LOG4CPLUS_INFO( logger_, "bind errno: " << errno );

      XCEPT_RAISE( rubuilder::filter::exception::socket, "bind" );
   }


   if( listen_sock_ > 0 )
   {
      r = listen( listen_sock_, 5 );

      if( r == -1 )
      {
         LOG4CPLUS_INFO( logger_, "listen errno: " << errno );

         XCEPT_RAISE( rubuilder::filter::exception::socket, "listen" );
      }

      if( r == 0 ) {
         this->activate();
      }
   }
}


rubuilder::filter::ptServer::~ptServer()
{

   int n = send( sig_sock_[0], "9", 1, 0 );

   this->join();

   for( int i=0; i<g_sock_; i++ )
   {
      if( FD_ISSET( i, &recv_fd_set_ ) )
      {
         LOG4CPLUS_INFO( logger_, "close recv socket: " << i );

	 close( i );
      }
   }

/*
   if( ref != 0)
   {
      ref->release();
      ref = NULL;
   }
*/

   close( sig_sock_[0] );

   close( sig_sock_[1] );

   close( listen_sock_ );
}


void rubuilder::filter::ptServer::clear_socket( int socket )
{

   if( FD_ISSET( socket, &recv_fd_set_ ) )
   {
      FD_CLR( socket, &recv_fd_set_ );

      int i = g_sock_ - 1;

      g_sock_ = 0;

      for ( ; i>=0; i-- )
      {
         if( FD_ISSET( i, &recv_fd_set_ ) )
         {
	    g_sock_ = i + 1;

	    break;
         }
      }
   };
}


void rubuilder::filter::ptServer::add_socket( int socket )
{

   FD_SET( socket, &recv_fd_set_ );

   if( socket >= g_sock_ ) {
      g_sock_ = socket + 1;
   }
}


int rubuilder::filter::ptServer::svc()
{

   LOG4CPLUS_INFO( logger_, "start svc" );

   for( ;; )
   {
      fd_set sel_recv_fd_set;
      int g_sock = g_sock_;

      FD_ZERO( &sel_recv_fd_set );

      memcpy( (char*)&sel_recv_fd_set, (char*)&recv_fd_set_, sizeof( fd_set ) );

      FD_SET( sig_sock_[1], &sel_recv_fd_set );

      FD_SET( listen_sock_, &sel_recv_fd_set );
 
      if( sig_sock_[1] >= g_sock ) {
         g_sock = sig_sock_[1] + 1;
      }

      if ( listen_sock_ >= g_sock ) {
         g_sock = listen_sock_ + 1;
      }

      //LOG4CPLUS_INFO( logger_, "select g_sock: " << g_sock );
      //LOG4CPLUS_INFO( logger_, "select sig_sock_[1]: " << sig_sock_[1] );
      //LOG4CPLUS_INFO( logger_, "select listen_sock_: " << listen_sock_ );

      for( int i=0; i<g_sock; i++ )
      {
         if( FD_ISSET( i, &sel_recv_fd_set ) ) {
            //LOG4CPLUS_INFO( logger_, "select recv socket: " << i );
         }
      }


      int sel_ret = select( g_sock, &sel_recv_fd_set, NULL, NULL, NULL );

      //LOG4CPLUS_INFO( logger_, "select return: " << sel_ret );

      if( sel_ret > 0 )
      {
         for( int i=0; i<g_sock; i++ )
         {
            if( FD_ISSET( i, &sel_recv_fd_set ) )
            {
               if( i == sig_sock_[1] )
               {
	          char sig_buf;

		  int n = recv( sig_sock_[1], &sig_buf, 1, 0 );
		  //LOG4CPLUS_INFO( logger_, "read sig_sock_[1] return: " << n );

		  if( sig_buf == '9' )
                  {
		     LOG4CPLUS_INFO( logger_, "ptServer end" );

		     this->exit( NULL );

                     //join thread in ~ptServer()
		  }
               }
               else if( i == listen_sock_ )
               {
	          LOG4CPLUS_INFO( logger_, "accept listen_sock_: " << listen_sock_ );

		  struct sockaddr_in from;
		  int fromlen = sizeof( from );

		  int new_sock = accept( listen_sock_, (struct sockaddr *)&from, (socklen_t *)&fromlen );

		  if( new_sock == -1 ) {
		     LOG4CPLUS_INFO( logger_, "accept errno: "<< errno );
		  }
	          LOG4CPLUS_INFO( logger_, "accept new_sock: "<< new_sock );

		  if( new_sock > 0 )
                  {
		     //fcntl( new_sock , F_SETFL, O_NONBLOCK );

		     add_socket( new_sock );
		  }
               }
               else
               {
                  //LOG4CPLUS_INFO( logger_, "recv_fd_set " );

                  toolbox::mem::Reference *ref=0;
                  
                  ref = poolFactory_->getFrame( pool_, BLOCKSIZE );

                  int nb = reads( i, (char*)ref->getDataLocation(), sizeof(I2O_MESSAGE_FRAME) );

                  if( nb <= 0 )
                  {
                     close( i );
                     clear_socket( i );

                     if( ref != 0)
                     {
                        ref->release();
                        ref = NULL;
                     }

                     break;
                  }


                  swapL32( (U32*)ref->getDataLocation(), sizeof(I2O_MESSAGE_FRAME) );

                  PI2O_MESSAGE_FRAME header = (PI2O_MESSAGE_FRAME)( ref->getDataLocation() );

                  int length = header->MessageSize << 2;

                  //std::cout << "length: " << length << std::endl;

                  nb = reads( i, (char*)( (char*)ref->getDataLocation() + sizeof(I2O_MESSAGE_FRAME) ),
                             length - sizeof(I2O_MESSAGE_FRAME) );

                  if( nb <= 0 )
                  {
                     close( i );
                     clear_socket( i );
                     
                     if( ref != 0)
                     {
                        ref->release();
                        ref = NULL;
                     }

                     break;
                  }


                  // If BIG-ENDIAN order swap data
                  swapL32( (U32*)( (char*)ref->getDataLocation() + sizeof(I2O_MESSAGE_FRAME) ),
                          length - sizeof(I2O_MESSAGE_FRAME) );


                  l_->I2O_FILTER_COLLECT_Callback( ref );
               }
            }
         }
      }//if( sel_ret > 0 )
   }
}


int rubuilder::filter::ptServer::reads( int sd, char *buf, int maxb )
{
   int nleft;
   int nb;

   nleft = maxb;

   //std::cout << "nleft: " << nleft << std::endl;

   while( nleft > 0 )
   {
      nb = recv( sd, buf, nleft, 0 );

      //std::cout << "nb: " << nb << std::endl;

      if( nb < 0 )
      {
         LOG4CPLUS_INFO( logger_, "Error receiving data: "<< strerror(errno) );

         return nb;
      }
      else if( nb == 0 )
      {
         LOG4CPLUS_INFO( logger_, "Connection closed by client " );

         return nb;
      }

      nleft -= nb;

      buf += nb;
   }

   return( maxb - nleft );
}


void rubuilder::filter::ptServer::addListener( ptSrvListener *l )
{

   l_ = l;
}

