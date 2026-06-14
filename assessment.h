#pragma once
#include "common.h"

// ─────────────────────────────────────────────────────────────────────────────
//  Assessment  (abstract)
// ─────────────────────────────────────────────────────────────────────────────
class Assessment {
public:
    MyString type;       // "Exam" | "Assignment" | "Quiz"
    double   rawScore;
    double   maxScore;
    double   weightage;  // e.g. 50.0 means 50%

    Assessment(const char* t, double raw, double mx, double w)
        : type(t), rawScore(raw), maxScore(mx), weightage(w) {}
    virtual ~Assessment() {}

    // contribution to final percentage
    double getWeightedScore() const {
        if(maxScore <= 0.0) return 0.0;
        return (rawScore / maxScore) * weightage;
    }

    virtual const char* getType() const = 0;
};

class Exam : public Assessment {
public:
    Exam(double raw, double mx, double w) : Assessment("Exam",raw,mx,w) {}
    const char* getType() const override { return "Exam"; }
};
class Assignment : public Assessment {
public:
    Assignment(double raw, double mx, double w) : Assessment("Assignment",raw,mx,w) {}
    const char* getType() const override { return "Assignment"; }
};
class Quiz : public Assessment {
public:
    Quiz(double raw, double mx, double w) : Assessment("Quiz",raw,mx,w) {}
    const char* getType() const override { return "Quiz"; }
};
