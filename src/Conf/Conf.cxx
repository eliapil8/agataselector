#include "Conf.h"

std::vector<std::string> Conf::markers{};

std::string              Conf::ltrim(const std::string& s) {
    std::string whitespace = " \n\r\t\f\v";
    size_t      start      = s.find_first_not_of(whitespace);
    return (start == std::string::npos) ? "" : s.substr(start);
}

std::string Conf::rtrim(const std::string& s) {
    std::string whitespace = " \n\r\t\f\v";
    size_t      end        = s.find_last_not_of(whitespace);
    return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}
std::string   Conf::trim(const std::string& s) { return rtrim(ltrim(s)); }

std::ostream& Conf::operator<<(std::ostream& os) const {
    os << "#-----------------------------------------------------------"
       << std::endl;
    os << std::endl;
    os << "#-----------------------------------------------------------"
       << std::endl;
    os << marker << std::endl;
    for(const auto& it : parI) {
        os << it.first << "\t\t\t";
        it.second << os;
    }
    for(const auto& it : parD) {
        os << it.first << "\t\t\t";
        it.second << os;
    }
    for(const auto& it : parUI) {
        os << it.first << "\t\t\t";
        it.second << os;
    }
    for(const auto& it : parB) {
        os << it.first << "\t\t\t";
        if(it.second.val) {
            os << "YES"
               << "\t"
               << "#"
               << "\t\t" << it.second.comment << std::endl;
        } else {
            os << "NO"
               << "\t"
               << "#"
               << "\t\t" << it.second.comment << std::endl;
        }
    }
    for(const auto& it : parPI) {
        os << it.first << "\t\t\t";
        it.second << os;
    }
    for(const auto& it : parMI) {
        for(const auto& it2 : it.second.val) {
            os << it.first << "\t\t\t" << it2.first << " " << it2.second
               << "\t"
               << "#"
               << "\t\t" << it.second.comment << std::endl;
        }
    }
    for(const auto& it : parVD) {
        os << it.first << "\t\t\t";
        for(const auto& it2 : it.second.val) { os << it2; }
        os << "\t" << it.second.unit << "\t" << it.second.comment << std::endl;
    }
    for(const auto& it : parVI) {
        os << it.first << "\t\t\t";
        for(const auto& it2 : it.second.val) { os << it2; }
        os << "\t" << it.second.unit << "\t" << it.second.comment << std::endl;
    }
    for(const auto& it : parVP) {
        //        if(it.second.val.empty())
        //          throw std::runtime_error("Should not be initialized
        //          empty\n");
        for(const auto& it2 : it.second.val) {
            // Property<decltype(it.second.val)> tmp{it2, it.second.unit,
            // it.second.comment};
            Property<std::pair<double, double>> tmp{it2, it.second.unit,
                                                    it.second.comment};
            os << it.first << "\t\t\t";
            tmp << os;
        }
    }
    for(const auto& it : parVPI) {
        //    if(it.second.val.empty())
        //        throw std::runtime_error("Should not be initialized empty\n");
        for(const auto& it2 : it.second.val) {
            // Property<decltype(it.second.val)> tmp{it2, it.second.unit,
            // it.second.comment};
            Property<std::pair<int, int>> tmp{it2, it.second.unit,
                                              it.second.comment};
            os << it.first << "\t\t\t";
            tmp << os;
        }
    }

    for(const auto& it : parSS) {
        os << it.first << "\t\t\t";
        it.second << os;
    }

    for(const auto& it : parVS) {
        //    if(it.second.val.empty())
        //        throw std::runtime_error("Should not be initialized empty\n");
        for(const auto& it2 : it.second.val) {
            // Property<decltype(it.second.val)> tmp{it2, it.second.unit,
            // it.second.comment};
            Property<std::string> tmp{it2, it.second.unit, it.second.comment};
            os << it.first << "\t\t\t";
            tmp << os;
        }
    }
    for(const auto& it : parS) {
        os << it.first << "\t\t\t";
        os << it.second.val;
        os << "\t"
           << "#"
           << "\t\t" << it.second.comment << std::endl;
    }

    for(const auto& it : parVS) {
        //   if(it.second.val.empty())
        //     throw std::runtime_error("Should not be initialized empty!\n");
        for(const auto& it2 : it.second.val) {
            Property<std::string> tmp{it2, it.second.unit, it.second.comment};
            os << it.first << "\t\t\t";
            tmp << os;
        }
    }

    for(const auto& it : parV) {
        os << it.first << "\t\t\t";
        it.second << os;
    }
    for(const auto& it : parION) {
        os << it.first << "\t\t\t";
        it.second << os;
    }
    for(const auto& it : parFI) {
        //   if(it.second.val.empty())
        //     throw std::runtime_error("Should not be initialized empty\n");
        for(const auto& it2 : it.second.val) {
            // Property<decltype(it.second.val)> tmp{it2, it.second.unit,
            // it.second.comment};
            Property<ReactionFragment::Ion> tmp{it2.first, it.second.unit,
                                                it.second.comment};
            os << it.first << "\t\t\t";
            tmp << os;
        }
    }
    for(const auto& it : parAFI) {
        // if (it.second.val.empty()) throw std::runtime_error("Should not be
        // initialized empty\n");
        for(const auto& it2 : it.second.val) {
            // Property<decltype(it.second.val)> tmp{it2, it.second.unit,
            // it.second.comment};
            Property<std::vector<ReactionFragment::Ion>> tmp{
                it2.first, it.second.unit, it.second.comment};
            os << it.first << "\t\t\t";
            tmp << os;
        }
    }
    return os;
}

std::istream& Conf::operator>>(std::istream& is) {
    findMarker(is);
    std::string line;

    // Erase previouse vectors/maps
    if(!parVP.empty()) parVP.begin()->second.val.clear();
    if(!parVPI.empty()) parVPI.begin()->second.val.clear();
    if(!parVS.empty()) parVS.begin()->second.val.clear();
    if(!parFI.empty()) parFI.begin()->second.val.clear();
    if(!parVD.empty()) parVD.begin()->second.val.clear();
    if(!parVI.empty()) parVI.begin()->second.val.clear();
    if(!parAFI.empty()) parAFI.begin()->second.val.clear();

    while(std::getline(is, line)) {
        line = trim(line);
        if(line.find('#') != std::string::npos) line.erase(line.find('#'));
        if(std::find(markers.begin(), markers.end(), line) != markers.end())
            break; // Next marker found

        std::istringstream sstr(line);
        std::string        name;
        sstr >> name;
        std::transform(name.begin(), name.end(), name.begin(),
                       (std::function<int(unsigned char)>)::toupper);

        { // Int type parameters
            auto it = parI.find(name);
            if(it != parI.end()) {
                it->second >> sstr;
                continue;
            }
        }
        { // Unsigned Int type parameters
            auto it = parUI.find(name);
            if(it != parUI.end()) {
                it->second >> sstr;
                continue;
            }
        }
        { // Long long type parameters
            auto it = parLL.find(name);
            if(it != parLL.end()) {
                it->second >> sstr;
                continue;
            }
        }
        { //unsigned Long long type parameters
            auto it = parULL.find(name);
            if(it != parULL.end()) {
                it->second >> sstr;
                continue;
            }
        }
	{ // Double type parameters
            auto it = parD.find(name);
            if(it != parD.end()) {
                it->second >> sstr;
                continue;
            }
        }
        { // Bool type parameters
            auto it = parB.find(name);
            if(it != parB.end()) {
                std::string val;
                sstr >> val;
                if(val == "YES")
                    it->second.val = true;
                else if(val == "NO")
                    it->second.val = false;
                else
                    throw std::runtime_error(
                        "Unknown value parsing conf file\n");
                continue;
            }
        }
        { // Pair of ints type parameters
            auto it = parPI.find(name);
            if(it != parPI.end()) {
                it->second >> sstr;
                continue;
            }
        }
        { // Map of ints type parameters
            auto it = parMI.find(name);
            if(it != parMI.end()) {
                it->second >> sstr;
                continue;
            }
        }
        { // vector of doubles type parameters
            auto it = parVD.find(name);
            if(it != parVD.end()) {
                std::vector<double>& tmparr = it->second.val;
                double               tempval;
                while(sstr >> tempval) { tmparr.push_back(tempval); }
                Property<std::vector<double>> tmp;
                tmp.val = tmparr;
                continue;
            }
        }
        { // vector of doubles type parameters
            auto it = parVI.find(name);
            if(it != parVI.end()) {
                std::vector<int>& tmparr = it->second.val;
                int               tempval;
                while(sstr >> tempval) { tmparr.push_back(tempval); }
                Property<std::vector<int>> tmp;
                tmp.val = tmparr;
                continue;
            }
        }
        { // pair type parameters
            auto it = parVP.find(name);
            if(it != parVP.end()) {
                Property<std::pair<double, double>> tmp;
                tmp >> sstr;
                it->second.val.emplace_back(tmp.val);
                continue;
            }
        }
        { // pair type parameters
            auto it = parVPI.find(name);
            if(it != parVPI.end()) {
                Property<std::pair<int, int>> tmp;
                tmp >> sstr;
                it->second.val.emplace_back(tmp.val);
                //std::cout<<"parVPI "<<tmp.val<<" "<<std::endl;
           
                continue;
            }
        }
        { // pair type parameters
            auto it = parVS.find(name);
            if(it != parVS.end()) {
                Property<std::string> tmp;
                tmp >> sstr;
                it->second.val.emplace_back(tmp.val);
                continue;
            }
        }
        { // Bool type parameters
            auto it = parS.find(name);
            if(it != parS.end()) {
                sstr >> it->second.val;
                continue;
            }
        }
        { // while-line single-string type parameters
            auto it = parSS.find(name);
            if(it != parSS.end()) {
                std::string temp;
                std::getline(sstr, temp, '#');
                // remove spaces and tabs before
                size_t pos = temp.find_first_not_of(' ')
                                   > temp.find_first_not_of('\t')
                               ? temp.find_first_not_of(' ')
                               : temp.find_first_not_of('\t');
                while(pos) {
                    temp.erase(0, pos);
                    pos = temp.find_first_not_of(' ')
                                > temp.find_first_not_of('\t')
                            ? temp.find_first_not_of(' ')
                            : temp.find_first_not_of('\t');
                }
                it->second.val = temp;
                continue;
            }
        }
        { // Bool type parameters
            auto it = parV.find(name);
            if(it != parV.end()) {
                it->second >> sstr;
                continue;
            }
        }
        { // Ion type parameters
            auto it = parION.find(name);
            if(it != parION.end()) {
                it->second >> sstr;
                continue;
            }
        }
        { // Fragment of interest type parameters
            auto it = parFI.find(name);
            if(it != parFI.end()) {
                Property<ReactionFragment::Ion> tmp;
                tmp >> sstr;
                it->second.val.emplace(tmp.val, true);
                continue;
            }
        }
        { // Fragment of interest type parameters
            auto it = parAFI.find(name);
            if(it != parAFI.end()) {
                Property<std::vector<ReactionFragment::Ion>> tmp;
                tmp >> sstr;
                it->second.val.emplace(tmp.val, true);
                continue;
            }
        }
    }
    if(Settings::getInstance().verbosity_level > 1)
        std::cout << "Finished reading :" << marker << std::endl;
    return is;
}

void Conf::findMarker(std::istream& is) {
    if(is.eof()) is.clear(std::ios::eofbit);
    is.seekg(0L, std::ios::beg);
    std::string line;
    if(!is.good()) {}
    while(std::getline(is, line)) {
        line = trim(line);
        if(line[0] == '#') continue; // Comment
        if(line != marker) continue; // Nothing of interest

        if(line == marker) return; // Found marker!
    }
    throw std::runtime_error("Unable to find " + marker + " in conf file\n");
}

double* Conf::getParameterPtr(const std::string& parName) {
    auto it = parD.find(parName);
    if(it != parD.end())
        return &(it->second.val);
    else
        throw std::runtime_error("Parameter requested but not found\n");
}

std::vector<std::string> Conf::getListOfOptimizableParameters() {
    std::vector<std::string> pars;
    pars.reserve(parD.size());
    for(const auto& it : parD) { pars.emplace_back(it.first); }
    return pars;
}

std::istream& operator>>(std::istream& is, TVector3& vec) {
    double x, y, z;
    is >> x >> y >> z;
    vec.SetXYZ(x, y, z);
    return is;
}
std::ostream& operator<<(std::ostream& os, const TVector3& vec) {
    os << vec.x() << " " << vec.y() << " " << vec.z();
    return os;
}
std::istream& operator>>(std::istream& is, std::pair<double, double>& v) {
    is >> v.first >> v.second;
    return is;
}
std::ostream&
operator<<(std::ostream& os, const std::pair<double, double>& v) {
    os << v.first << " " << v.second;
    return os;
}
std::istream& operator>>(std::istream& is, std::pair<int, int>& v) {
    is >> v.first >> v.second;
    return is;
}
std::ostream& operator<<(std::ostream& os, const std::pair<int, int>& v) {
    os << v.first << " " << v.second;
    return os;
}
std::istream& operator>>(std::istream& is, ReactionFragment::Ion& v) {
    is >> v.A >> v.Z;
    return is;
}
std::ostream& operator<<(std::ostream& os, const ReactionFragment::Ion& v) {
    os << v.A << " " << v.Z;
    return os;
}
std::istream&
operator>>(std::istream& is, std::vector<ReactionFragment::Ion>& v) {
    std::string str;
    while(is >> str) {
        if(str == "END") return is;

        int A, Z;
        A = std::stoi(str);
        is >> Z;
        v.emplace_back(A, Z);
    }
    std::sort(v.begin(), v.end());
    throw std::runtime_error(
        "You need to add END keyword after a list of ions\n");
}
std::ostream&
operator<<(std::ostream& os, const std::vector<ReactionFragment::Ion>& v) {
    for(const auto& it : v) { os << it.A << " " << it.Z << " "; }
    os << "END";
    return os;
}
std::istream& operator>>(std::istream& is, std::unordered_map<int, int>& m) {
    std::string str;
    int         i1, i2;
    is >> i1 >> i2;
    m[i1] = i2;
    return is;
}

std::ostream& operator<<(std::ostream& os, const std::vector<std::string>& v) {
    for(const auto& it : v) { os << it; }
    return os;
}

std::istream& operator>>(std::istream& is, std::vector<std::string>& v) {
    std::string str;
    while(is >> str) { v.emplace_back(str); }
    return is;
}

std::ostream& operator<<(std::ostream& os, const std::vector<double>& v) {
    for(const auto& it : v) { os << it; }
    return os;
}

std::istream& operator>>(std::istream& is, std::vector<double>& v) {
    double str;
    while(is >> str) { v.emplace_back(str); }
    return is;
}

std::string operator*=(std::string& v, const double& d) {
    (void)d;
    return v;
}

std::pair<int, int> operator*=(std::pair<int, int>& v, const double& d) {
    v.first *= static_cast<int>(d);
    v.second *= static_cast<int>(d);
    return v;
}
std::pair<double, double>
operator*=(std::pair<double, double>& v, const double& d) {
    v.first *= d;
    v.second *= d;
    return v;
}
ReactionFragment::Ion operator*=(ReactionFragment::Ion& v, const double& d) {
    // v.A *= static_cast<int>(d);
    // v.Z *= static_cast<int>(d);
    (void)d;
    return v;
}
std::vector<ReactionFragment::Ion>
operator*=(std::vector<ReactionFragment::Ion>& v, const double& d) {
    // v.A *= static_cast<int>(d);
    // v.Z *= static_cast<int>(d);
    (void)d;
    return v;
}

/*std::vector<std::string>
operator*=(std::vector<std::string>& v, const double& d) {
  (void) d;
  return v;
  }*/

std::unordered_map<int, int>
operator*=(std::unordered_map<int, int>& m, const double& d) {
    // v.A *= static_cast<int>(d);
    // v.Z *= static_cast<int>(d);
    (void)d;
    return m;
}

std::vector<double> operator*=(std::vector<double>& v, const double& d) {
    std::transform(v.begin(), v.end(), v.begin(),
                   [&d](auto& val) { return val * d; });
    return v;
}

ReactionFragment::Ion
operator+(const ReactionFragment::Ion& v1, const ReactionFragment::Ion& v2) {
    ReactionFragment::Ion res;
    res.A = v1.A + v2.A;
    res.Z = v1.Z + v2.Z;
    return res;
}
ReactionFragment::Ion
operator-(const ReactionFragment::Ion& v1, const ReactionFragment::Ion& v2) {
    ReactionFragment::Ion res;
    if(v1.A > v2.A)
        res.A = v1.A - v2.A;
    else
        res.A = v2.A - v1.A;
    if(v1.Z > v2.Z)
        res.Z = v1.Z - v2.Z;
    else
        res.Z = v2.Z - v1.Z;
    return res;
}
