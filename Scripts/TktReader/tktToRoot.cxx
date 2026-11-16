#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <vector>

#include <TFile.h>
#include <TH1D.h>

enum DataType
{
    A,
    AB,
    AX,
    C,
    UC,
    S,
    US,
    I,
    UI,
    L,
    UL,
    F,
    D,
    UA,
    UT
};

std::pair<std::vector<unsigned int>, std::string> fileNameParser(std::string fileName);
enum DataType getType(const std::string &type);
TH1D *getHistogram(const std::string &fileName, std::vector<unsigned int> indexes);

template <typename T>
TH1D *readData(const std::string &fileName, std::vector<unsigned int> dimensions, std::vector<unsigned int> indexes);

TH1D *tktToRoot(const std::string &fileName = "Post__5-40-16384-UI__Ener.spec",
                std::vector<unsigned int> indexes = std::vector<unsigned int>{0, 0}, unsigned int nSpec = 1,
                double xcal = 0.25)
{
    indexes.push_back(nSpec);

    TH1D *histo = getHistogram(fileName, indexes);
    histo->GetXaxis()->SetLimits(0, histo->GetXaxis()->GetXmax() * xcal);
    return histo;

    // histo->Draw();
}

// Functions///////////////////////////////////////////////////////////////////////////////////////////////////////
template <typename T>
TH1D *readData(const std::string &fileName, std::vector<unsigned int> dimensions, std::vector<unsigned int> indexes)
{

    unsigned int size{dimensions.back()};
    dimensions.pop_back();

    unsigned int nspec{indexes.back()};
    indexes.pop_back();

    T data[size * nspec];

    std::ifstream spectra(fileName, std::ios::in | std::ios::binary);
    if (!spectra.is_open())
        throw std::runtime_error("Unable to open file " + fileName + "\n");

    // std::reverse(dimensions.begin(), dimensions.end());

    unsigned int pos{0};
    for (unsigned int i{0}; i < dimensions.size(); ++i)
    {
        int tmp = indexes[i];
        // std::cout << "idx" << i << ": " << indexes[i] << std::endl;
        for (unsigned int j{(unsigned int)dimensions.size() - 1}; j > i; --j)
        {
            // std::cout << "dimension" << j << " " << dimensions[j] << std::endl;
            tmp *= dimensions[j];
        }
        pos += tmp;
    }
    pos *= sizeof(T) * size;

    // std::cout << "pos: " << pos << std::endl;

    spectra.seekg(pos);
    spectra.read(reinterpret_cast<char *>(&data), (sizeof data));

    std::string hname = "spec_";
    for (auto i : indexes)
        hname += std::to_string(i) + "-";
    hname += std::to_string(nspec);

    TH1D *spec = new TH1D(hname.c_str(), hname.c_str(), size * nspec, 0, size * nspec);
    for (int i{0}; i < size * nspec; ++i)
    {
        spec->SetBinContent(spec->FindBin(i), data[i]);
    }
    return spec;
}

std::pair<std::vector<unsigned int>, std::string> fileNameParser(std::string fileName)
{
    fileName = fileName.substr(fileName.find_last_of("/") + 1);
    fileName = fileName.substr(0, fileName.find_last_of("_") - 1);
    fileName = fileName.substr(fileName.find_first_of("_") + 2);
    // std::cout << fileName << std::endl;

    auto const re = std::regex{R"(-)"};
    auto const vec = std::vector<std::string>(std::sregex_token_iterator{begin(fileName), end(fileName), re, -1},
                                              std::sregex_token_iterator{});

    std::pair<std::vector<unsigned int>, std::string> output;
    for (const auto &it : vec)
    {
        if (it != vec.back())
            output.first.push_back(std::stoi(it));
        else
            output.second = it;
    }
    return output;
}

TH1D *getHistogram(const std::string &fileName, std::vector<unsigned int> indexes)
{
    auto dimensions = fileNameParser(fileName);
    TH1D *histogram{nullptr};
    // std::cout << "type" << getType(dimensions.second) << std::endl;
    switch (getType(dimensions.second))
    {
    case A:
        break;
    case AB:
        break;
    case AX:
        break;
    case C:
        break;
    case UC:
        break;
    case S:
        histogram = readData<short>(fileName, dimensions.first, indexes);
        break;
    case US:
        histogram = readData<unsigned short>(fileName, dimensions.first, indexes);
        break;
    case I:
        histogram = readData<int32_t>(fileName, dimensions.first, indexes);
        break;
    case UI:
        histogram = readData<uint32_t>(fileName, dimensions.first, indexes);
        break;
    case L:
        histogram = readData<long>(fileName, dimensions.first, indexes);
        break;
    case UL:
        histogram = readData<unsigned long>(fileName, dimensions.first, indexes);
        break;
    case F:
        histogram = readData<float>(fileName, dimensions.first, indexes);
        break;
    case D:
        histogram = readData<double>(fileName, dimensions.first, indexes);
        break;
    case UA:
        break;
    case UT:
        break;
    default:
        break;
    }
    if (histogram != nullptr)
        return histogram;
    else
        throw std::runtime_error("shomething wrong\n");
}

TH1D *changeHistogramRange(TH1D *histo, double energy_offset, double energy_scale)
{
    histo->SetDirectory(nullptr);
    TH1D *new_histo = new TH1D(histo->GetName(), histo->GetTitle(), histo->GetNbinsX(),
                               histo->GetXaxis()->GetXmin() * energy_scale + energy_offset,
                               histo->GetXaxis()->GetXmax() * energy_scale + energy_offset);
    for (int i = 0; i < histo->GetNbinsX(); ++i)
    {
        new_histo->SetBinContent(i, histo->GetBinContent(i));
    }
    // delete histo;
    return new_histo;
}

void saveTktToRoot(const std::string &fileName, const double energy_offset = 0., double energy_scale = 1.)
{
    std::string fname = fileName.substr(0, fileName.find_last_of('.'));
    fname = fname + ".root";
    TFile *file = new TFile(fname.c_str(), "RECREATE");
    file->cd();
    auto dimensions = fileNameParser(fileName);
    for (auto i : dimensions.first)
        std::cout << i << std::endl;
    auto indexes = dimensions.first;
    indexes.pop_back(); // remove binning

    // std::cout << "type" << getType(dimensions.second) << std::endl;

    int recursion_level = 0;

    auto recursive_fcn = [&](auto &&self, std::vector<unsigned int> _in,
                             std::vector<unsigned int> creation_indexes) -> void {
        const auto max_index = _in.front();
        int rl = recursion_level++;
        _in.erase(_in.begin());
        TH1D *histogram{nullptr};

        for (int i = 0; i < max_index; i++)
        {
            creation_indexes.push_back(i);

            if (_in.size() > 0)
            {
                gDirectory->mkdir(std::to_string(i).c_str());
                gDirectory->cd(std::to_string(i).c_str());
                self(self, _in, creation_indexes);
                gDirectory->cd("..");
                creation_indexes.pop_back();
            }
            else
            {
                for (auto i : creation_indexes)
                    std::cout << i << " ";
                std::cout << "       \r" << std::flush;
                creation_indexes.push_back(1);
                switch (getType(dimensions.second))
                {
                case A:
                    break;
                case AB:
                    break;
                case AX:
                    break;
                case C:
                    break;
                case UC:
                    break;
                case S:
                    histogram = readData<short>(fileName, dimensions.first, creation_indexes);
                    break;
                case US:
                    histogram = readData<unsigned short>(fileName, dimensions.first, creation_indexes);
                    break;
                case I:
                    histogram = readData<int32_t>(fileName, dimensions.first, creation_indexes);
                    break;
                case UI:
                    histogram = readData<uint32_t>(fileName, dimensions.first, creation_indexes);
                    break;
                case L:
                    histogram = readData<long>(fileName, dimensions.first, creation_indexes);
                    break;
                case UL:
                    histogram = readData<unsigned long>(fileName, dimensions.first, creation_indexes);
                    break;
                case F:
                    histogram = readData<float>(fileName, dimensions.first, creation_indexes);
                    break;
                case D:
                    histogram = readData<double>(fileName, dimensions.first, creation_indexes);
                    break;
                case UA:
                    break;
                case UT:
                    break;
                default:
                    break;
                }
                if (histogram != nullptr)
                {
                    auto _h = changeHistogramRange(histogram, energy_offset, energy_scale);
                    // _h->SetDirectory(gDirectory);
                    _h->Write();
                }
                else
                    throw std::runtime_error("shomething wrong\n");
                creation_indexes.pop_back();
                creation_indexes.pop_back();
            }
        }
    };
    // auto levels = dimensions.first;
    // levels.pop_back();
    recursive_fcn(recursive_fcn, indexes, {});
}

enum DataType getType(const std::string &type)
{
    if (type == "A")
        return A;
    if (type == "AB")
        return AB;
    if (type == "AX")
        return AX;
    if (type == "C")
        return C;
    if (type == "UC")
        return UC;
    if (type == "S")
        return S;
    if (type == "US")
        return US;
    if (type == "I")
        return I;
    if (type == "UI")
        return UI;
    if (type == "L")
        return L;
    if (type == "UL")
        return UL;
    if (type == "F")
        return F;
    if (type == "D")
        return D;
    if (type == "UA")
        return UA;
    if (type == "UT")
        return UT;
    else
        throw std::runtime_error("Unknown data type");
}