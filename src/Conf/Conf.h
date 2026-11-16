#pragma once

#include <exception>
#include <functional>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <TVector3.h>

#include "ReactionFragment.h"
#include "Settings.h"

std::istream& operator>>(std::istream& is, TVector3& vec);
std::ostream& operator<<(std::ostream& os, const TVector3& vec);
std::istream& operator>>(std::istream& is, std::vector<double>& v);
std::ostream& operator<<(std::ostream& os, std::vector<double>& v);
std::istream& operator>>(std::istream& is, std::pair<double, double>& v);
std::ostream& operator<<(std::ostream& os, const std::pair<double, double>& v);
std::istream& operator>>(std::istream& is, std::pair<int, int>& v);
std::ostream& operator<<(std::ostream& os, const std::pair<int, int>& v);
std::istream& operator>>(std::istream& is, ReactionFragment::Ion& v);
std::ostream& operator<<(std::ostream& os, const ReactionFragment::Ion& v);
std::istream& operator>>(std::istream& is, std::vector<std::string>& v);
std::ostream& operator<<(std::ostream& os, std::vector<std::string>& v);
std::istream&
operator>>(std::istream& is, std::vector<ReactionFragment::Ion>& v);
std::ostream&
operator<<(std::ostream& os, const std::vector<ReactionFragment::Ion>& v);
std::istream& operator>>(std::istream& is, std::unordered_map<int, int>& i);
std::string   operator*=(std::string& v, const double& d);
std::pair<int, int> operator*=(std::pair<int, int>& v, const double& d);
std::pair<double, double>
operator*=(std::pair<double, double>& v, const double& d);
ReactionFragment::Ion operator*=(ReactionFragment::Ion& v, const double& d);
std::vector<ReactionFragment::Ion>
operator*=(std::vector<ReactionFragment::Ion>& v, const double& d);
std::unordered_map<int, int>
operator*=(std::unordered_map<int, int>& v, const double& d);
// std::vector<std::string>
// operator*=(std::vector<std::string>& v, const double& d);
ReactionFragment::Ion
operator+(const ReactionFragment::Ion& v1, const ReactionFragment::Ion& v2);
ReactionFragment::Ion
operator-(const ReactionFragment::Ion& v1, const ReactionFragment::Ion& v2);

class Conf {
  public:
    Conf() = delete;
    const std::string        marker;
    double*                  getParameterPtr(const std::string&);
    std::vector<std::string> getListOfOptimizableParameters();

  protected:
    virtual void finalize(){};
    explicit Conf(std::string mark) : marker(std::move(mark)) {
        std::lock_guard<std::mutex> guard(lock);
        markers.push_back(marker);
    };

    template <class T> struct Property {
        T             val;
        std::string   unit{"#"};
        std::string   comment{};
        std::istream& operator>>(std::istream& is) {
            is >> val;
            is >> unit;
            val *= UNITS::classify(unit);
            return is;
        };
        std::ostream& operator<<(std::ostream& os) const {
            T tmpval = val;
            tmpval *= (1 / UNITS::classify(unit));
            os << tmpval;
            os << "\t" << unit << "\t\t\t" << comment << std::endl;
            return os;
        };
    };

    std::unordered_map<std::string, Property<int&>>    parI;
    std::unordered_map<std::string, Property<unsigned int&>> parUI;
    std::unordered_map<std::string, Property<long long&>>    parLL;
    std::unordered_map<std::string, Property<unsigned long long&>>    parULL;
    std::unordered_map<std::string, Property<double&>> parD;
    std::unordered_map<std::string, Property<bool&>>   parB;
    std::unordered_map<std::string, Property<std::vector<double>&>>
        parVD; // read whole line of unspecified number of double numbers
    std::unordered_map<std::string, Property<std::vector<int>&>>
        parVI; // read whole line of unspecified number of double int
    std::unordered_map<std::string,
                       Property<std::vector<std::pair<double, double>>&>>
        parVP;
    std::unordered_map<std::string, Property<std::vector<std::pair<int, int>>&>>
                                                                    parVPI;
    std::unordered_map<std::string, Property<std::pair<int, int>&>> parPI;
    std::unordered_map<std::string, Property<std::unordered_map<int, int>&>>
                                                            parMI;
    std::unordered_map<std::string, Property<std::string&>> parS;
    std::unordered_map<std::string, Property<std::string&>>
        parSS; // read whole line as a single string until comment is found, it
               // is unit-less
    std::unordered_map<std::string, Property<std::vector<std::string>&>> parVS;
    std::unordered_map<std::string, Property<TVector3&>>                 parV;
    std::unordered_map<std::string, Property<ReactionFragment::Ion&>> parION;
    std::unordered_map<
        std::string,
        Property<std::unordered_map<ReactionFragment::Ion, bool,
                                    ReactionFragment::Ion::hash_fn>&>>
        parFI;
    std::unordered_map<
        std::string,
        Property<std::unordered_map<std::vector<ReactionFragment::Ion>, bool,
                                    ReactionFragment::Ion::vec_hash_fn>&>>
        parAFI;

  public:
    virtual std::ostream& operator<<(std::ostream& os) const;
    virtual std::istream& operator>>(std::istream& is);

  protected:
    static std::string              ltrim(const std::string& s);
    static std::string              rtrim(const std::string& s);
    static std::string              trim(const std::string& s);
    static std::vector<std::string> markers;

    void                            findMarker(std::istream& is);

  private:
    std::mutex lock;
};
