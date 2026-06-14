#pragma once
#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <climits>
#include "MyString.h"
#include "MyArray.h"

const char* const DATA_DIR = "data/";

// ── Simple line / header ──────────────────────────────────────────────────────
inline void printLine(char c = '-', int w = 60) {
    for (int i = 0; i < w; i++) std::cout << c;
    std::cout << "\n";
}

inline void printHeader(const char* title) {
    std::cout << "\n";
    printLine('=');
    std::cout << "  " << title << "\n";
    printLine('=');
}

inline void pause() {
    std::cout << "\n  Press ENTER to continue...";
    std::cin.ignore(INT_MAX, '\n');
    std::cin.get();
}

// ── Input helpers ─────────────────────────────────────────────────────────────
inline int getInt(const char* prompt, int lo, int hi) {
    char buf[64];
    int v;
    while (true) {
        std::cout << prompt;
        if (!std::cin.getline(buf, sizeof(buf))) { std::cin.clear(); continue; }
        char* end;
        v = (int)strtol(buf, &end, 10);
        if (end != buf && v >= lo && v <= hi) return v;
        std::cout << "  Invalid input. Enter a number between " << lo << " and " << hi << ".\n";
    }
}

inline MyString getString(const char* prompt) {
    char buf[1024];
    std::cout << prompt;
    std::cin.getline(buf, sizeof(buf));
    MyString s(buf);
    return s.trim();
}

inline double getDouble(const char* prompt, double lo, double hi) {
    char buf[64];
    double v;
    while (true) {
        std::cout << prompt;
        if (!std::cin.getline(buf, sizeof(buf))) { std::cin.clear(); continue; }
        char* end;
        v = strtod(buf, &end);
        if (end != buf && v >= lo && v <= hi) return v;
        std::cout << "  Invalid input. Enter a number between " << lo << " and " << hi << ".\n";
    }
}

// ── File line splitting ───────────────────────────────────────────────────────
inline MyArray<MyString> splitLine(const MyString& line, char delim = '|') {
    MyArray<MyString> parts;
    int start = 0, pos = 0;
    const char* s = line.c_str();
    int len = line.size();
    while (pos <= len) {
        if (pos == len || s[pos] == delim) {
            char tmp[1024];
            int l = pos - start;
            if (l >= 1024) l = 1023;
            memcpy(tmp, s + start, l);
            tmp[l] = '\0';
            MyString tok(tmp);
            parts.push(tok.trim());
            start = pos + 1;
        }
        pos++;
    }
    return parts;
}

// ── Column printing ───────────────────────────────────────────────────────────
inline void printCol(const char* s, int width) {
    int l = (int)strlen(s);
    std::cout << s;
    for (int i = l; i < width; i++) std::cout << ' ';
}
inline void printCol(const MyString& s, int width) { printCol(s.c_str(), width); }

// ── Grade helpers ─────────────────────────────────────────────────────────────
inline const char* gradeFromPct(double pct) {
    if (pct >= 90) return "A+";
    if (pct >= 85) return "A";
    if (pct >= 80) return "A-";
    if (pct >= 75) return "B+";
    if (pct >= 70) return "B";
    if (pct >= 65) return "B-";
    if (pct >= 60) return "C+";
    if (pct >= 55) return "C";
    if (pct >= 50) return "D";
    return "F";
}
inline double gradePoints(const char* g) {
    if (strcmp(g,"A+")==0 || strcmp(g,"A")==0) return 4.0;
    if (strcmp(g,"A-")==0) return 3.7;
    if (strcmp(g,"B+")==0) return 3.3;
    if (strcmp(g,"B")==0)  return 3.0;
    if (strcmp(g,"B-")==0) return 2.7;
    if (strcmp(g,"C+")==0) return 2.3;
    if (strcmp(g,"C")==0)  return 2.0;
    if (strcmp(g,"D")==0)  return 1.0;
    return 0.0;
}
