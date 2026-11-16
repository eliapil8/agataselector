#pragma once

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

class Lut {
  protected:
    Lut() = default;
    struct BoardChan {
        unsigned int brd, chan;
        BoardChan(const unsigned int& brd, const unsigned int& chan)
            : brd(brd), chan(chan){};
        bool operator==(const BoardChan& p) const {
            return brd == p.brd && chan == p.chan;
        };
        struct hash_fn {
            std::size_t operator()(const BoardChan& p) const {
                return std::hash<unsigned int>()(p.brd)
                     ^ std::hash<unsigned int>()(p.chan);
            }
        };
    };

  public:
    /// @brief Function that prints the lut to a file
    void PrintLut_toFile(const std::string& filename) {
        std::ofstream file(filename);
        PrintLut(file);
    }

    /// @brief Function that extracts words and white spaces from the string,
    /// starting first with whitespaces. It is very stupid, but works
    /// @param line
    /// @param words
    /// @param white_spaces
    static void
    SplitStringIntoWordsAndSpaces(std::string               line,
                                  std::vector<std::string>& words,
                                  std::vector<std::string>& white_spaces) {
        if(line.empty()) return;
        bool isspace = true;
        white_spaces.push_back("");
        while(!line.empty()) {
            if(line.at(0) == ' ' || line.at(0) == '\t') {
                if(!isspace) {
                    white_spaces.push_back("");
                    isspace = true;
                }
                white_spaces.back() += line.at(0);
                line.erase(0, 1);
            } else {
                if(isspace) {
                    words.push_back("");
                    isspace = false;
                }
                words.back() += line.at(0);
                line.erase(0, 1);
            }
        }
    };

  public:
    virtual void                  ReadLut(const std::string&) = 0;
    static constexpr unsigned int MAXCH{16};
    static unsigned int
    getID(const unsigned int& brd, const unsigned int& ch) {
        return brd * MAXCH + ch;
    };
    virtual void PrintLut(std::ostream& outstream = std::cout) = 0;

  private:
};
