#include <stdio.h>
#include "smloader.h"
#include "MinaCalc/MinaCalc.h"
#include "json.hpp"
#include <string>
#include <iostream>
#include <sstream>

using json = nlohmann::json;

void print(std::string out) {
    std::cout << out << std::endl;
}

struct Ratings {
    float overall;
    float stream;
    float jumpstream;
    float handstream;
    float stamina;
    float jack;
    float chordjack;
    float technical;
};

struct Difficulty {
    int rate;
    Ratings ratings;
};

int main(int argc, char *argv[]) {
    std::string input;

    if (argc < 2) {
        std::string line;

        while (getline(std::cin, line)) {
            input += line;
        }
    } else {
        std::ifstream dataStream;

        dataStream.open(argv[1]);

        if (!dataStream.is_open()) {
            std::cerr << "Could not open file!" << std::endl;
        }

        std::stringstream buf;
        buf << dataStream.rdbuf();
        input = buf.str();
    }

    json song = json::parse(input);

    std::vector<NoteInfo> noteData;
    std::vector<int> lastTracks;

    float prevTime;

    int num = 0;

    for (auto note : song["HitObjects"]) {
        auto time = note["Time"].get<float>();
        auto track = note["Track"].get<float>();

        if (lastTracks.empty() || prevTime == time) {
            lastTracks.push_back(track);
        } else {
            int column_value = 1;
            int notes = 0;

            for (int i = 0; i < 4; i++) {
                bool found = false;

                for (auto trackNum : lastTracks) {
                    if (trackNum == i + 1) {
                        found = true;
                        break;
                    }
                }

                if (found) {
                    notes += column_value;
                }
                
                column_value *= 2;
            }

            noteData.push_back(NoteInfo { (unsigned) notes, prevTime / 1000 });

            notes = 0;
            column_value = 1;

            lastTracks.clear();
            lastTracks.push_back(track);
        }

        prevTime = time;
    }

    Calc* calc = new Calc();

    auto result = MinaSDCalc(noteData, calc);

    json difficulties;

    int index = 0;
    int rate = 70;

    for (auto diffForRate : result) {
        json diff;

        diff["rate"] = rate;
        diff["overall"] = diffForRate[0];
        diff["stream"] = diffForRate[1];
        diff["jumpstream"] = diffForRate[2];
        diff["handstream"] = diffForRate[3];
        diff["stamina"] = diffForRate[4];
        diff["jack"] = diffForRate[5];
        diff["chordjack"] = diffForRate[6];
        diff["technical"] = diffForRate[7];

        difficulties[index] = diff;

        rate += 10;
        index += 1;
    }

    std::cout << difficulties.dump() << std::endl;
}