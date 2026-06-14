#pragma once
#include "common.h"
#include "assessment.h"

// ─────────────────────────────────────────────────────────────────────────────
//  Course  (abstract)
// ─────────────────────────────────────────────────────────────────────────────
class Course {
public:
    MyString courseID, title, teacherID, courseType;
    MyString file_path;

    Course() { file_path="data/courses.txt"; }
    Course(const char* cid, const char* t, const char* tid, const char* ct)
        : courseID(cid), title(t), teacherID(tid), courseType(ct)
    { file_path="data/courses.txt"; }

    virtual ~Course() {}

    // Calculates final % from a pointer-array of Assessment*
    virtual double calculateFinalGrade(Assessment** arr, int count) const = 0;
    virtual int    getExamDurationMins()  const = 0;
    virtual const char* getType()         const = 0;
    virtual bool   needsComputers()       const { return false; }

    void display() const {
        printCol(courseID,10);
        printCol(title,32);
        printCol(getType(),12);
        std::cout<<"T:"<<teacherID<<"\n";
    }

    void toFile(std::ofstream& f) const {
        f << courseID<<"|"<<title<<"|"<<teacherID<<"|"<<courseType<<"\n";
    }
};

// ─────────────────────────────────────────────────────────────────────────────
//  CoreCourse  — Exam 50%, Assignment 30%, Quiz 20%  (3-hour exam)
// ─────────────────────────────────────────────────────────────────────────────
class CoreCourse : public Course {
public:
    CoreCourse() { courseType="Core"; }
    CoreCourse(const char* cid, const char* t, const char* tid)
        : Course(cid,t,tid,"Core") {}

    double calculateFinalGrade(Assessment** arr, int n) const override {
        double total=0;
        for(int i=0;i<n;i++) total+=arr[i]->getWeightedScore();
        return total;
    }
    int getExamDurationMins()  const override { return 180; }
    const char* getType()      const override { return "Core"; }
};

// ─────────────────────────────────────────────────────────────────────────────
//  ElectiveCourse  — Assignment 50%, Exam 30%, Quiz 20%  (2-hour exam)
// ─────────────────────────────────────────────────────────────────────────────
class ElectiveCourse : public Course {
public:
    ElectiveCourse() { courseType="Elective"; }
    ElectiveCourse(const char* cid, const char* t, const char* tid)
        : Course(cid,t,tid,"Elective") {}

    double calculateFinalGrade(Assessment** arr, int n) const override {
        double total=0;
        for(int i=0;i<n;i++) total+=arr[i]->getWeightedScore();
        return total;
    }
    int getExamDurationMins()  const override { return 120; }
    const char* getType()      const override { return "Elective"; }
};

// ─────────────────────────────────────────────────────────────────────────────
//  LabCourse  — 100% continuous (no final exam)
// ─────────────────────────────────────────────────────────────────────────────
class LabCourse : public Course {
public:
    LabCourse() { courseType="Lab"; }
    LabCourse(const char* cid, const char* t, const char* tid)
        : Course(cid,t,tid,"Lab") {}

    double calculateFinalGrade(Assessment** arr, int n) const override {
        double total=0;
        for(int i=0;i<n;i++)
            if(strcmp(arr[i]->getType(),"Exam")!=0) total+=arr[i]->getWeightedScore();
        return total;
    }
    int getExamDurationMins()  const override { return 0; }
    const char* getType()      const override { return "Lab"; }
    bool needsComputers()      const override { return true; }
};

// ─── Factory ──────────────────────────────────────────────────────────────────
inline Course* makeCourse(const char* cid, const char* title,
                          const char* tid, const char* type) {
    if(strcmp(type,"Core")==0)     return new CoreCourse(cid,title,tid);
    if(strcmp(type,"Elective")==0) return new ElectiveCourse(cid,title,tid);
    if(strcmp(type,"Lab")==0)      return new LabCourse(cid,title,tid);
    return new CoreCourse(cid,title,tid);
}
inline Course* makeCourseFromParts(const MyArray<MyString>& p) {
    if(p.size()<4) return nullptr;
    return makeCourse(p[0].c_str(),p[1].c_str(),p[2].c_str(),p[3].c_str());
}
