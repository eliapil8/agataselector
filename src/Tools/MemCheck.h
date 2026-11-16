#pragma once

#include <cstring>
#include <exception>
#include <fstream>
#include <iostream>
#include <sstream>

#ifndef MAC_OSX
#include "sys/sysinfo.h"
#endif

namespace MemCheck
{

inline unsigned long getTotalVirtualMemory()
{
#ifndef MAC_OSX
    struct sysinfo memInfo{};
    sysinfo(&memInfo);
    auto totalVirtualMem = memInfo.totalram;
    // Add other values in next statement to avoid int overflow on right hand
    // side...
    totalVirtualMem += memInfo.totalswap;
    totalVirtualMem *= memInfo.mem_unit;
    return totalVirtualMem;
#else
    return 0;
#endif
}

inline unsigned long getUsedVirtualMemory()
{
#ifndef MAC_OSX
    struct sysinfo memInfo{};
    sysinfo(&memInfo);
    auto virtualMemUsed = memInfo.totalram - memInfo.freeram;
    // Add other values in next statement to avoid int overflow on right hand
    // side...
    virtualMemUsed += memInfo.totalswap - memInfo.freeswap;
    virtualMemUsed *= memInfo.mem_unit;
    return virtualMemUsed;
#else
    return 0;
#endif
}

inline unsigned long getFreeVirtualMemory()
{
#ifndef MAC_OSX
    struct sysinfo memInfo{};
    sysinfo(&memInfo);
    auto virtualMemFree = memInfo.freeram;
    // Add other values in next statement to avoid int overflow on right hand
    // side...
    virtualMemFree += memInfo.freeswap;
    virtualMemFree *= memInfo.mem_unit;
    return virtualMemFree;
#else
    return 0;
#endif
}

inline unsigned long getProcessVirtualMemory()
{
#ifndef MAC_OSX
    FILE         *file   = fopen("/proc/self/status", "r");
    unsigned long result = -1;
    char          line[128];

    while (fgets(line, 128, file) != nullptr)
    {
        if (strncmp(line, "VmSize:", 7) == 0)
        {
            result        = strlen(line);
            const char *p = line;
            while (*p < '0' || *p > '9')
                p++;
            line[result - 3] = '\0';
            result           = atoi(p);
            break;
        }
    }
    fclose(file);
    return result;
#else
    return 0;
#endif
}

inline unsigned long getTotalPhysicalMemory()
{
#ifndef MAC_OSX
    struct sysinfo memInfo{};
    sysinfo(&memInfo);
    auto totalPhysMem = memInfo.totalram;
    // Multiply in next statement to avoid int overflow on right hand side...
    totalPhysMem *= memInfo.mem_unit;
    return totalPhysMem;
#else
    return 0;
#endif
}

inline unsigned long getUsedPhysicalMemory()
{
#ifndef MAC_OSX
    struct sysinfo memInfo{};
    sysinfo(&memInfo);
    auto physMemUsed = memInfo.totalram - memInfo.freeram;
    physMemUsed -= memInfo.bufferram;
    // Multiply in next statement to avoid int overflow on right hand side...
    physMemUsed *= memInfo.mem_unit;
    return physMemUsed;
#else
    return 0;
#endif
}

inline unsigned long getFreePhysicalMemory()
{
#ifndef MAC_OSX
    // struct sysinfo memInfo {};
    // sysinfo(&memInfo);
    // auto physMemFree = memInfo.freeram;
    // physMemFree += memInfo.bufferram;
    //// Multiply in next statement to avoid int overflow on right hand side...
    // physMemFree *= memInfo.mem_unit;
    // return physMemFree;
    std::ifstream meminfoFile("/proc/meminfo");
    if (!meminfoFile.is_open())
    {
        std::cerr << "Error opening /proc/meminfo" << std::endl;
        return -1; // Indicate error
    }

    std::string line;
    long        availableMemoryKB = -1;
    while (std::getline(meminfoFile, line))
    {
        if (line.find("MemAvailable:") == 0)
        {
            // Extract the available memory value
            std::istringstream iss(line);
            std::string        key, unit;
            long               value;
            iss >> key >> value >> unit; // Read "MemAvailable:", the value, and the unit

            if (unit == "kB")
            {                              // Check if the unit is kB
                availableMemoryKB = value; // Store the value in kB
            }
            else
            {
                std::cerr << "Unexpected unit: " << unit << std::endl;
                availableMemoryKB = -1; // Indicate error
            }
            break; // We found what we need
        }
    }
    meminfoFile.close();

    if (availableMemoryKB != -1)
    {
        return availableMemoryKB * 1024; // Convert to bytes
    }
    return -1; // Indicate error if no value found
#else
    return 0;
#endif
}

inline size_t getProcessPhysicalMemory()
{
#ifndef MAC_OSX
    FILE  *file   = fopen("/proc/self/status", "r");
    size_t result = -1;
    char   line[128];

    while (fgets(line, 128, file) != nullptr)
    {
        if (strncmp(line, "VmRSS:", 6) == 0)
        {
            result        = strlen(line);
            const char *p = line;
            while (*p < '0' || *p > '9')
                p++;
            line[result - 3] = '\0';
            result           = std::atoi(p);
            break;
        }
    }
    fclose(file);
    return result;
#else
    return 0;
#endif
}

inline float getPercentFreePhysical()
{
#ifndef MAC_OSX
    return static_cast<float>(getFreePhysicalMemory()) / getTotalPhysicalMemory() * 100;
#else
    return 100;
#endif
}

class OutOfMemoryException : public std::exception
{
  public:
    const char *what() const noexcept override { return "Out of memory error!"; }
};

inline void printStats()
{
    std::cout << "Total Physical memory     : " << getTotalPhysicalMemory() << std::endl;
    std::cout << "Used  Physical memory     : " << getUsedPhysicalMemory() << std::endl;
    std::cout << "Free  Physical memory     : " << getFreePhysicalMemory() << std::endl;
    std::cout << "Total Virtual memory      : " << getTotalVirtualMemory() << std::endl;
    std::cout << "Used  Virtual memory      : " << getUsedVirtualMemory() << std::endl;
    std::cout << "Free  Virtual memory      : " << getFreeVirtualMemory() << std::endl;
}

} // namespace MemCheck
