#pragma once

#include <algorithm>
#include <assert.h>
#include <iostream>
#include <string>

#include "TMath.h"
#include "TObject.h"
#include "TVector3.h"

namespace gridopti
{

class TPositionOffset : public TObject
{
  private:
    double fDx;
    double fDy;
    double fDz;
    double fDtheta;
    double fDphi;

    bool fIsCartesian;

  public:
    TPositionOffset() : TObject(){};
    /// @brief constructor for carthesian offset, units are in mm
    /// @param dx
    /// @param dy
    /// @param dz
    TPositionOffset(const double &dx, const double &dy, const double &dz)
        : TObject(), fDx(dx), fDy(dy), fDz(dz), fDtheta(0), fDphi(0), fIsCartesian(true){};

    /// @brief constructor for spherical offset, units are in radians
    /// @param dtheta
    /// @param dphi
    TPositionOffset(const double &dtheta, const double &dphi) : TObject(), fDx(0), fDy(0), fDz(0)
    {
        assert(TMath::Abs(dtheta) < TMath::Pi() && "theta offset is larger then 180deg");
        assert(TMath::Abs(dphi) < 2 * TMath::Pi() && "phi offset is larger then 360deg");

        fIsCartesian = false;
        fDtheta      = dtheta;
        fDphi        = dphi;
    };
    TPositionOffset(const TPositionOffset &other) = default;
    TPositionOffset(TPositionOffset &&other)      = default;
    ~TPositionOffset()                            = default;

    TPositionOffset &operator=(const TPositionOffset &other)
    {
        if (this != &other)
        {
            fIsCartesian = other.fIsCartesian;
            fDtheta      = other.fDtheta;
            fDphi        = other.fDphi;
            fDx          = other.fDx;
            fDy          = other.fDy;
            fDz          = other.fDz;
        }
        return *this;
    }

    bool operator==(const TPositionOffset &other) const
    {
        return fIsCartesian == other.fIsCartesian && fDtheta == other.fDtheta && fDphi == other.fDphi && fDx == other.fDx &&
               fDy == other.fDy && fDz == other.fDz;
    }

    std::string GenerateString() const noexcept
    {
        std::string retval;
        if (fIsCartesian) { retval = "dx__" + std::to_string(fDx) + "__dy__" + std::to_string(fDy) + "__dz__" + std::to_string(fDz); }
        else
        {
            retval = "dtheta__" + std::to_string(fDtheta * TMath::RadToDeg()) + "__dphi__" + std::to_string(fDphi * TMath::RadToDeg());
        }

        // we need to sanitze the string to avoid filenames having mathematical operators
        std::replace(retval.begin(), retval.end(), '.', '_');
        std::replace(retval.begin(), retval.end(), '-', 'm');
        return retval;
    }

    /// @brief Offset input vector by the offset stored in this class
    /// @param vec vector on which the offset is applied
    void OffsetVector(TVector3 &vec) const noexcept
    {
        if (fIsCartesian)
        {
            vec.SetX(vec.X() + fDx);
            vec.SetY(vec.Y() + fDy);
            vec.SetZ(vec.Z() + fDz);
        }
        else
        {
            /// theta cannot be negative, we have rotate phi by 180deg if that case
            vec.SetTheta(vec.Theta() + fDtheta);
            vec.SetPhi(vec.Phi() + fDphi);
            // if (vec.Theta() + fDtheta > 0)
            // {
            //     vec.SetTheta(vec.Theta() + fDtheta);
            //     vec.SetPhi(vec.Phi() + fDphi);
            // }
            // else
            // {
            //     vec.SetTheta(TMath::Abs(vec.Theta() + fDtheta));
            //     vec.SetPhi(vec.Phi() + fDphi - 180. * TMath::DegToRad());
            // }
        }
    }

    void Print() const // *MENU
    {
        std::string str;
        if (fIsCartesian) str = "dx_" + std::to_string(fDx) + "_dy_" + std::to_string(fDy) + "_dz_" + std::to_string(fDz);
        else
            str = "dtheta_" + std::to_string(fDtheta * TMath::RadToDeg()) + "_dphi_" + std::to_string(fDphi * TMath::RadToDeg());
        std::cout << str << std::endl;
    }

    double GetThetaOffset() const
    {
        assert(!fIsCartesian);
        return fDtheta;
    };
    double GetPhiOffset() const
    {
        assert(!fIsCartesian);
        return fDphi;
    };
    double GetXOffset() const
    {
        assert(fIsCartesian);
        return fDx;
    };
    double GetYOffset() const
    {
        assert(fIsCartesian);
        return fDy;
    };
    double GetZOffset() const
    {
        assert(fIsCartesian);
        return fDz;
    };
    ClassDefOverride(TPositionOffset, 1);
};
} // namespace gridopti
