#pragma once
#include "common.h"

struct Venue {
    MyString roomID;
    int      capacity;
    bool     hasComputers;

    Venue() : capacity(0), hasComputers(false) {}
    Venue(const char* id, int cap, bool hc)
        : roomID(id), capacity(cap), hasComputers(hc) {}

    void display() const {
        std::cout << "  Room: " << roomID
                  << "  Cap: "  << capacity
                  << "  Computers: " << (hasComputers ? "Yes" : "No") << "\n";
    }

    // "V001|60|0"
    void toFile(std::ofstream& f) const {
        f << roomID << "|" << capacity << "|" << (hasComputers?1:0) << "\n";
    }

    static Venue fromParts(const MyArray<MyString>& p) {
        if(p.size()<3) return Venue();
        return Venue(p[0].c_str(), p[1].toInt(), p[2].toInt()==1);
    }
};
