/**
Copyright (C) 2017 Markku Pulkkinen

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
**/

#include <cmath>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "randutils.hpp"

namespace {
const std::string fnAll{"kotus-sanalista_v1.txt"};
const std::string fnBip0039{"finnish.txt"};
const std::string fnForbidden{"forbidden.txt"};
const std::string fnRejected{"rejected.txt"};

unsigned long strDifference(const std::string &s1, const std::string &s2) {
    auto cnt = std::min(s1.length(), s2.length());
    auto diff{0};
    for (auto i = 0U; i < cnt; i++) {
        if (s1.at(i) != s2.at(i)) diff++;
    }
    return diff + abs(int(s1.length() - s2.length()));
}
}

int main() {
    std::ifstream forbiddenfile(fnForbidden);
    std::ofstream of(fnBip0039);
    std::ofstream rf(fnRejected);
    std::vector<std::string> forbidden;
    for (std::string temp; std::getline(forbiddenfile, temp); /**/)
        forbidden.push_back(temp);

    std::string line;
    std::vector<std::string> words;
    std::ifstream myfile(fnAll);
    if (myfile.is_open()) {
        unsigned int cnt{0};
        std::string rare("bcdfgqwxz");
        std::string rare_first("qxz");
        unsigned int three_same_cnt = 0;
        while (std::getline(myfile, line)) {
            auto isValid{true};
            auto len{line.length()};

            // 1. rule: word length range [5,8]
            if (len < 5 || len > 8) {
                rf << line << std::endl;
                continue;
            }

            // 2. rule: no forbidden words
            if (std::find(forbidden.begin(), forbidden.end(), line) !=
                    forbidden.end())
                continue;

            // 3. rule: word must contain only small alphabets
            for (auto &i : line) {
                if (!std::isalpha(i) || !std::islower(i)) {
                    isValid = false;
                    break;
                }
            }
            if (!isValid) {
                rf << line << std::endl;
                continue;
            }

            // 4. rule: First four (4) letters must be unique
            for (auto &i : words) {
                std::string s1 = i.substr(0, 4);
                std::string s2 = line.substr(0, 4);
                if (s1 == s2) {
                    isValid = false;
                    break;
                }
            }
            if (!isValid) {
                rf << line << std::endl;
                continue;
            }

            // 5. rule: First 3 letters max 3 same
            three_same_cnt = 0;
            for (auto &i : words) {
                std::string s1 = i.substr(0, 3);
                std::string s2 = line.substr(0, 3);
                if (s1 == s2) {
                    if (++three_same_cnt >= 3) {
                        isValid = false;
                        break;
                    }
                }
            }
            if (!isValid) {
                rf << line << std::endl;
                continue;
            }

            // 6. rule: no words that start with rare char
            auto pos = line.find_first_of(rare_first);
            if (pos == 0) {
                rf << line << std::endl;
                continue;
            }

            // 7. rule: no multiple rare chars
            int same{0};
            for (auto &i : line) {
                for (auto &j : rare) {
                    if (i == j) same++;
                }
                if (same > 1) {
                    same = 0;
                    isValid = false;
                    break;
                }
            }
            if (!isValid) {
                rf << line << std::endl;
                continue;
            }

            // 8. rule: no too similar words
            for (auto &i : words) {
                unsigned int diff = strDifference(line, i);
                isValid = (diff >= 3 ? true : false);
                if (!isValid) break;
            }
            if (isValid) {
                words.push_back(line);
                // std::cout << line << std::endl;
                cnt++;
            }
        }

        if (words.size() < 2048) return -1;

        // pick up the 2048 words using random engine
        std::vector<std::string> valid;

        // generate long enough init seed sequence so that no special warm-up time
        // is required for the M-T algorithm engine.
        std::mt19937 engine = std::mt19937(randutils::auto_seed_256{}.base());
        std::uniform_int_distribution<unsigned int> dist(0, words.size() - 1);
        valid.resize(2048);
        cnt = 0;

        // generate valid word list using M-T engine
        do {
            std::string word = words[dist(engine)];
            if (std::find(valid.begin(), valid.end(), word) != valid.end()) continue;
            valid[cnt++] = word;
        } while (cnt < 2048);

        // sort and save valid word list
        std::sort(valid.begin(), valid.end());
        for (auto &it : valid) of << it << std::endl;
        
        // print some statistics, close files
        std::cout << "valid word count: " << cnt << std::endl;
        std::cout << "total word count: " << words.size() << std::endl;
        of.close();
        rf.close();
        myfile.close();
        forbiddenfile.close();
    }
}
