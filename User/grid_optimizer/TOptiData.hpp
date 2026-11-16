#pragma once

#include "TNamed.h"
#include "TPositionOffset.hpp"
#include <array>

namespace gridopti
{
class TOptiData : public TNamed
{
  private:
    TPositionOffset       pos_offset;
    std::array<double, 5> fit_res{0, 0, 0, 0, 0};
    bool                  is_valid{false};

  public:
    TOptiData() : TNamed(){};
    TOptiData(const TPositionOffset &pos_offset, const std::array<double, 5> &fit_res, const bool valid_fit)
        : TNamed(), pos_offset(pos_offset), fit_res(fit_res), is_valid(valid_fit){};

    TOptiData(const TOptiData &other) = default;
    TOptiData(TOptiData &&other)      = default;
    ~TOptiData()                      = default;

    void SetData(const std::string &name, const TPositionOffset &pos_offset, const std::array<double, 5> &fit_res,
                 const bool valid_fit) noexcept
    {
        this->pos_offset = pos_offset;
        this->fit_res    = fit_res;
        this->is_valid   = valid_fit;
        this->SetName(name.c_str());
        this->SetTitle(name.c_str());
    }
    inline const TPositionOffset &      GetPosOffset() const noexcept { return pos_offset; }
    inline const std::array<double, 5> &GetFitResult() const noexcept { return fit_res; }
    inline bool                         IsValid() const noexcept { return is_valid; }
    void                                Print(Option_t *option = "") const override
    {
        TNamed::Print(option);
        std::cout << "TOptiData: " << this->GetName() << " " << this->GetTitle() << std::endl;
        pos_offset.Print();
        std::cout << "fit_res:    " << std::endl;
        std::cout << "           volume: " << fit_res[0] << std::endl;
        std::cout << "           mean  : " << fit_res[1] << std::endl;
        std::cout << "           width : " << fit_res[2] << std::endl;
        std::cout << "           par3  : " << fit_res[3] << std::endl;
        std::cout << "           par4  : " << fit_res[4] << std::endl;
        std::cout << "is_valid:   " << is_valid << std::endl;
    }
    ClassDefOverride(TOptiData, 2);
};

} // namespace gridopti