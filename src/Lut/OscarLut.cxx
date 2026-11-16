#include "OscarLut.h"
#include "Units.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

/*
    the "map" number (ABC) conversion into detector, layer and segment:
       A = detector number (not used but kept for good readability)
       BC / 50 = layer (0 = DeltaE, 1 = Eres)
       BC % 50 = segment

       Delta E:
       -----------------
       | | | | | ... | |
       | | | | | ... | |
       | | | | | ... | |
       | | | | | ... | |
       -----------------
        0 1 2 3  ... 15

       E residual:
       ---------------------
       |  0 |  1 |  2 |  3 |
       ---------------------
       |  4 |  5 |  6 |  7 |
       ---------------------
       |  8 |  9 | 10 | 11 |
       ---------------------
       | 12 | 13 | 14 | 15 |
       ---------------------
*/

OscarLut::OscarLut() : settings(Settings::getInstance()) {}

void OscarLut::ReadLut(const std::string &file_name)
{
    fMap_bch_strip.clear();
    fMap_bch_pad.clear();
    fMap_bch_layer.clear();
    fMap_strippad_telescope.clear();

    std::string   line;
    std::ifstream file(file_name);
    if (!file.is_open())
    {
        std::cerr << "Could not open OSCAR LUT file: " << file_name << std::endl;
        throw std::runtime_error("Oscar Lut not present\n");
    }

    int read_mode{0};
    while (std::getline(file, line))
    {
        if (line.empty() or line.find_first_not_of(" \t") == std::string::npos)
            continue;

        // comment
        if (line.at(line.find_first_not_of(' ')) == '#')
            continue;
        // change read content
        std::stringstream strline(line);
        if (settings.verbosity_level > 0)
            std::cout << line << std::endl;
        if (line.at(line.find_first_not_of(' ')) == '@')
        {
            std::string tmp;
            strline >> tmp;
            if (tmp.compare("@STRIP") == 0)
            {
                read_mode = 1;
                continue;
            }
            if (tmp.compare("@PAD") == 0)
            {
                read_mode = 2;
                continue;
            }
            if (tmp.compare("@TELESCOPE") == 0)
            {
                read_mode = 3;
                continue;
            }
            throw std::runtime_error("Unknown read mode of OSCAR LUT");
        }

        switch (read_mode)
        {
        case 1: {
            Layer l;
            l.type = OscarLayer::DELTA_E;
            int npars;
            strline >> l.board >> l.channel >> l.layerID >> l.name >> l.thr_low >> l.thr_high >>
                l.TSOffset >> npars;
            for (int i = 0; i < npars; i++)
            {
                double par;
                strline >> par;
                l.cal.emplace_back(par);
            }
            l.selector_id                                          = Lut::getID(l.board, l.channel);
            fMap_bch_strip[encodeBoardChannel(l.board, l.channel)] = l;
            fMap_bch_layer[encodeBoardChannel(l.board, l.channel)] = l;
            break;
        }
        case 2: {
            Layer l;
            l.type = OscarLayer::E_RES;
            int npars;
            strline >> l.board >> l.channel >> l.layerID >> l.name >> l.thr_low >> l.thr_high >>
                l.TSOffset >> npars;
            for (int i = 0; i < npars; i++)
            {
                double par;
                strline >> par;
                l.cal.emplace_back(par);
            }
            l.selector_id                                          = Lut::getID(l.board, l.channel);
            fMap_bch_pad[encodeBoardChannel(l.board, l.channel)]   = l;
            fMap_bch_layer[encodeBoardChannel(l.board, l.channel)] = l;

            break;
        }
        case 3: {
            Telescope t;
            short     pad, strip;
            bool      active = true;
            strline >> strip >> pad >> t.telescopeID >> t.name >> active >> t.timeOffset >>
                t.pos_x >> t.pos_y >> t.pos_z >> t.norm_x >> t.norm_y >> t.norm_z >>
                t.thick_dead_dE >> t.thick_active_dE >> t.thick_dead_Eres >> t.thick_active_Eres;

            t.pos_x *= UNITS::mm;
            t.pos_y *= UNITS::mm;
            t.pos_z *= UNITS::mm;
            t.thick_dead_dE *= UNITS::um;
            t.thick_active_dE *= UNITS::um;
            t.thick_dead_Eres *= UNITS::um;
            t.thick_active_Eres *= UNITS::um;
            if (!active)
                continue;
            auto i_strip =
                std::find_if(fMap_bch_strip.begin(), fMap_bch_strip.end(),
                             [strip](const auto &map) { return map.second.layerID == strip; });
            if (i_strip == fMap_bch_strip.end())
                throw std::runtime_error(Form("Strip %i not found for telescope %i in OSCAR LUT\n",
                                              strip, t.telescopeID));
            t.layer_dE = &(i_strip->second);
            auto i_pad = std::find_if(fMap_bch_pad.begin(), fMap_bch_pad.end(),
                                      [pad](const auto &map) { return map.second.layerID == pad; });
            if (i_pad == fMap_bch_pad.end())
                throw std::runtime_error(
                    Form("Pad %i not found for telescope %i in OSCAR LUT\n", pad, t.telescopeID));
            t.layer_Eres = &(i_pad->second);

            int key                      = (strip << 16) | pad;
            fMap_strippad_telescope[key] = t;

            continue;
        }
        default:
            throw std::runtime_error("Unknown read mode of OSCAR LUT");
        }
    }

    if (settings.verbosity_level > 2)
    {
        std::cout << "-------------Printing Oscar lut-------------" << std::endl;
        std::cout << "Strip parameters: " << std::endl;
        std::cout << "Board\tChannel\tID\tName\tThrLow\tThrHigh\tTOff\tnCal\tCalPars" << std::endl;
        for (const auto &it : fMap_bch_strip)
        {
            const auto *i = &(it.second);
            std::cout << i->board << "\t" << i->channel << "\t" << i->layerID << "\t" << i->name
                      << "\t" << i->thr_low << "\t" << i->thr_high << "\t" << i->TSOffset << "\t"
                      << i->cal.size() << "\t";
            for (const auto &c : i->cal)
            {
                std::cout << c << " ";
            }
            std::cout << std::endl;
        }
        std::cout << "\n\n";
        std::cout << "Pad parameters: " << std::endl;
        std::cout << "Board\tChannel\tID\tName\tThrLow\tThrHigh\tTOff\tnCal\tCalPars" << std::endl;
        for (const auto &it : fMap_bch_pad)
        {
            const auto *i = &(it.second);
            std::cout << i->board << "\t" << i->channel << "\t" << i->layerID << "\t" << i->name
                      << "\t" << i->thr_low << "\t" << i->thr_high << "\t" << i->TSOffset << "\t"
                      << i->cal.size() << "\t";
            for (const auto &c : i->cal)
            {
                std::cout << c << " ";
            }
            std::cout << std::endl;
        }

        std::cout << "\n\n";
        std::cout << "Telescope parameters: " << std::endl;
        std::cout << "stripID\tpadID\ttelescopeID\tname\tposX\tposY\tposZ\tnor"
                     "mX\tnormY\tnormZ\tthick_dead_dE\tthick_active_dE\tthick_"
                     "dead_Eres\tthick_active_Eres\ttffset"
                  << std::endl;

        for (const auto &it : fMap_strippad_telescope)
        {
            const auto *t = &(it.second);
            std::cout << t->layer_dE->layerID << "\t" << t->layer_Eres->layerID << "\t"
                      << t->telescopeID << "\t" << t->name << "\t" << t->pos_x << "\t" << t->pos_y
                      << "\t" << t->pos_z << "\t" << t->norm_x << "\t" << t->norm_y << "\t"
                      << t->norm_z << "\t" << t->thick_dead_dE << "\t" << t->thick_active_dE << "\t"
                      << t->thick_dead_Eres << "\t" << t->thick_active_Eres << "\t"
                      << t->timeOffset / (10 * UNITS::ns) << std::endl;
        }
    }
}

void OscarLut::Load_dE_E_Cuts(const std::vector<TCutG *> &cuts)
{
    for (const auto &it : cuts)
    {
        std::string name = it->GetName();
        std::replace(name.begin(), name.end(), '.', '_');
        unsigned int A = static_cast<unsigned int>(
            std::stoi(name.substr(name.find_first_of('a') + 1, name.find_first_of('_') - 1)));
        unsigned int Z = static_cast<unsigned int>(
            std::stoi(name.substr(name.find_first_of('z') + 1, name.find_first_of('_') - 1)));
        std::string id = name.substr(name.find_first_of('_', 3) + 1);
        fpId_EdE.emplace_back(ReactionFragment::Ion(A, Z), it);
    }

    // Look for duplicates
    std::sort(fpId_EdE.begin(), fpId_EdE.end());
    for (auto &it : fpId_EdE)
    {
        bool wasUnique = true;
        for (auto &it2 : fpId_EdE)
            if (it.first == it2.first)
                wasUnique = false;
        if (!wasUnique)
            throw std::runtime_error("Multiple dE-E gates for the same particle and detector\n");
    }

    if (Settings::getInstance().verbosity_level > 0)
    {
        std::cout << "Oscar dE-E cuts " << std::endl;
        for (const auto &it : fpId_EdE)
        {
            std::cout << "Ion A" << it.first.A << " Z" << it.first.Z << " with name "
                      << it.second->GetName() << std::endl;
        }
        std::cout << std::endl;
    }
}

void OscarLut::Load_dE_E_Cuts_TelescopeID(const std::vector<TCutG *> &cuts)
{
    for (const auto &it : cuts)
    {
        std::string name = it->GetName();
        std::replace(name.begin(), name.end(), 'A', 'a');
        std::replace(name.begin(), name.end(), 'Z', 'z');
        unsigned int i_A         = name.find_first_of('a');
        unsigned int i_Z         = name.find_first_of('z');
        unsigned int i_telescope = name.find_first_of('t');

        unsigned int A = static_cast<unsigned int>(
            std::stoi(name.substr(i_A + 1, name.find_first_of('_', i_A) - (i_A + 1))));
        unsigned int Z = static_cast<unsigned int>(
            std::stoi(name.substr(i_Z + 1, name.find_first_of('_', i_Z) - (i_Z + 1))));
        unsigned int telescope = static_cast<unsigned int>(std::stoi(name.substr(
            i_telescope + 1, name.find_first_of('_', i_telescope) - (i_telescope + 1))));

        fMap_telescopeID_pId_EdE[telescope].emplace_back(ReactionFragment::Ion(A, Z), it);
    }

    //     // Look for duplicates
    for (auto &tId : fMap_telescopeID_pId_EdE)
    {
        std::sort(tId.second.begin(), tId.second.end());
        auto itUn      = std::unique(tId.second.begin(), tId.second.end(),
                                     [](const auto &a, const auto &b) { return a.first == b.first; });
        bool wasUnique = (itUn == tId.second.end());
        if (!wasUnique)
            throw std::runtime_error(Form("Multiple gates for the same pad (%i) and strip (%i)\n",
                                          tId.first, tId.first));
    }

    if (Settings::getInstance().verbosity_level > 0)
    {
        std::cout << "Oscar cuts " << std::endl;
        for (const auto &tId : fMap_telescopeID_pId_EdE)
        {
            std::cout << "Telescope ID: " << tId.first << std::endl;
            for (const auto &it : tId.second)
            {
                std::cout << "Ion A" << it.first.A << " Z" << it.first.Z << " with name "
                          << it.second->GetName() << std::endl;
            }
        }
        std::cout << std::endl;
    }
}

void OscarLut::PrintLut(std::ostream &outstream)
{
    // assert(!fLut_file_fullname.empty());
    // std::ifstream old_lut_file(fLut_file_fullname.c_str());

    // if(!old_lut_file.is_open())
    //     throw std::runtime_error("Oscar Lut " + fLut_file_fullname
    //                              + " not present\n");
    // unsigned int board, chan;
    // std::string  chanName;
    // std::string  line;

    // while(std::getline(old_lut_file, line)) {
    //     if(line.empty()
    //        or line.find_first_not_of(" \t") == std::string::npos) {
    //         outstream << line << '\n';
    //         continue;
    //     }
    //     if(line.at(line.find_first_not_of(' ')) == '#') {
    //         outstream << line << '\n';
    //         continue;
    //     }
    //     std::vector<std::string> spaces;
    //     std::vector<std::string> words;

    //     Lut::SplitStringIntoWordsAndSpaces(line, words, spaces);

    //     std::stringstream strline(line);

    //     strline >> board >> chan;
    //     auto         params = prop[board][chan];

    //     // print until theta
    //     unsigned int iter   = 0;

    //     // sanity check
    //     assert(spaces.size() >= 10 + params.cal.size()
    //            && "not enough spaces in the line to print the lut");

    //     outstream << spaces[iter] << board;
    //     ++iter;
    //     outstream << spaces[iter] << chan;
    //     ++iter;
    //     outstream << spaces[iter] << params.map;
    //     ++iter;
    //     outstream << spaces[iter] << params.name;
    //     ++iter;
    //     outstream << spaces[iter] << params.thr.first;
    //     ++iter;
    //     outstream << spaces[iter] << params.thr.second;
    //     ++iter;
    //     // TODO convert set assing X and Y units
    //     outstream << spaces[iter] << params.x;
    //     ++iter;
    //     // TODO convert set assing X and Y units
    //     outstream << spaces[iter] << params.y;
    //     ++iter;
    //     outstream << spaces[iter] << params.timeOffset / (10 * UNITS::ns);
    //     ++iter;
    //     outstream << spaces[iter] << params.cal.size();
    //     ++iter;
    //     for(auto& cal : params.cal) {
    //         outstream << spaces[iter] << cal;
    //         ++iter;
    //     }
    //     // now we should have "iter" number of spaces and "iter" number of
    //     // words printed, but there might be a comment at the end of the
    //     line for(; iter < spaces.size(); iter++) {
    //         outstream << spaces[iter];
    //         if(iter < words.size()) { outstream << words[iter]; }
    //     }
    //     outstream << '\n';
    // }
}
