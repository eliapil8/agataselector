#ifndef __MISC__
#define __MISC__

#include <string>
#include <iostream>

//terminal colors
#define RESET   "\033[0m"
#define BLACK   "\033[30m"      /* Black */
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */
#define BLUE    "\033[34m"      /* Blue */
#define MAGENTA "\033[35m"      /* Magenta */
#define CYAN    "\033[36m"      /* Cyan */
#define WHITE   "\033[37m"      /* White */
#define BOLDBLACK   "\033[1m\033[30m"      /* Bold Black */
#define BOLDRED     "\033[1m\033[31m"      /* Bold Red */
#define BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
#define BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */
#define BOLDBLUE    "\033[1m\033[34m"      /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m"      /* Bold Magenta */
#define BOLDCYAN    "\033[1m\033[36m"      /* Bold Cyan */
#define BOLDWHITE   "\033[1m\033[37m"      /* Bold White */


static inline void PrintError(std::string msg)
  {std::cout << BOLDRED << " --- ERROR: " << RESET << RED << msg << RESET << std::endl;}
static inline void PrintError_continue(std::string msg)
  {std::cout  << "            " << RED << msg << RESET << std::endl;}
static inline void PrintWarning(std::string msg)
  {std::cout << BOLDYELLOW << " --- Warning: " << YELLOW << msg << RESET << std::endl;}
static inline void PrintWarning_continue(std::string msg)
  {std::cout  << "              " << YELLOW << msg << RESET << std::endl;}
static inline void PrintInfo_important(std::string msg)
  {std::cout << BOLDGREEN << "INFO: " << RESET << GREEN << msg << RESET << std::endl;}
static inline void PrintInfo_normal(std::string msg)
  {std::cout << BOLDBLUE << "INFO: " << RESET << BLUE << msg << RESET << std::endl;}


static inline void Terminal_ClearScreen()
{
    std::cout << "\033[H\033[J" << std::flush;
}
static inline void Terminal_MoveCursor(int col, int row)
{
    std::cout << "\033[" << row << ";" << col<<"H";
}

std::string GetFuncUniqueName(const char *prefix, void *ptr)
{
    static int ___num = 0;
    static std::mutex __getfunctuniquename__mtx;
    std::lock_guard<std::mutex> lock(__getfunctuniquename__mtx);

    std::ostringstream name;
    name << prefix << "_" << ptr << "_" << ++___num;

    return name.str();
}


#endif