#pragma once

#include <TObject.h>

class Container : public TObject
{
  protected:
    Container() = default;
    virtual void resize(const unsigned int &mult) { Time.resize(mult); };

  public:
    virtual void clear()
    {
        TS = 0;
        Time.clear();
    };

    std::size_t size() const { return Time.size(); };

  public:
    unsigned long long  TS{0}; // TSTamp of the first event
    std::vector<double> Time;  // Cfd time difference with respect to TS
    ClassDef(Container, 1);
};