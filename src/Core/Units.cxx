#include "Units.h"

#include <algorithm>
#include <cctype>
#include <string>

double UNITS::classify(std::string unit) {
    // Length
    //
    unit.erase(std::remove_if(unit.begin(), unit.end(), ::isspace),
               unit.end());
    if(unit.empty())
        throw std::runtime_error("Trying to classify an empty string\n");
    if(unit == "meter" || unit == "m")
        return meter;
    else if(unit == "centimeter" || unit == "cm")
        return centimeter;
    else if(unit == "cm2")
        return cm2;
    else if(unit == "cm3")
        return cm3;
    else if(unit == "millimeter" || unit == "mm")
        return millimeter;
    else if(unit == "mm2")
        return mm2;
    else if(unit == "mm3")
        return mm3;
    else if(unit == "micrometer" || unit == "um")
        return micrometer;
    else if(unit == "nanometer" || unit == "nm")
        return nanometer;
    else if(unit == "picometer" || unit == "pm")
        return picometer;
    else if(unit == "femtometer" || unit == "fm")
        return femtometer;

    // Angles
    else if(unit == "deg")
        return deg;
    else if(unit == "radian" || unit == "rad")
        return rad;

    // Mass
    else if(unit == "kilogram" || unit == "kg")
        return kilogram;
    else if(unit == "gram" || unit == "g")
        return gram;
    else if(unit == "milligram" || unit == "mg")
        return milligram;
    else if(unit == "microgram" || unit == "ug")
        return microgram;
    else if(unit == "nanogram" || unit == "ng")
        return nanogram;

    // Energy
    else if(unit == "elettronvolt" || unit == "eV")
        return eV;
    else if(unit == "kiloelettronvolt" || unit == "keV")
        return keV;
    else if(unit == "megaelettronvolt" || unit == "MeV")
        return MeV;
    else if(unit == "gigaelettronvolt" || unit == "GeV")
        return GeV;
    else if(unit == "teraelettronvolt" || unit == "GTeV")
        return TeV;

    // Time
    else if(unit == "year" || unit == "ys")
        return years;
    else if(unit == "day")
        return days;
    else if(unit == "hour" || unit == "hr")
        return hours;
    else if(unit == "minute" || unit == "min")
        return minutes;
    else if(unit == "second" || unit == "s")
        return second;
    else if(unit == "millisecond" || unit == "ms")
        return millisecond;
    else if(unit == "microsecond" || unit == "us")
        return microsecond;
    else if(unit == "nanosecond" || unit == "ns")
        return nanosecond;
    else if(unit == "picosecond" || unit == "ps")
        return picosecond;
    else if(unit == "femtosecond" || unit == "fs")
        return femtosecond;

    // magnetic field
    else if(unit == "tesla" || unit == "T")
        return tesla;

    // Compound types
    else if(unit.find('/') != std::string::npos) {
        auto pos  = unit.find('/');
        auto test = classify(unit.substr(0, pos))
                  / classify(unit.substr(pos + 1, unit.length() - pos));
        return test;
    } else if(unit.find('*') != std::string::npos) {
        auto pos = unit.find('*');
        return classify(unit.substr(0, pos))
             * classify(unit.substr(pos + 1, unit.length() - pos));
    }

    // Unit number
    else if(unit == "#")
        return 1;

    else { throw std::runtime_error("Unknown unit type : " + unit); }
}
