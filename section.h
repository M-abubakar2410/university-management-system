#pragma once
#include "common.h"

struct Section {
    MyString sectionID, courseID, teacherID, venueID, timeSlot;
    MyString* studentIDs;    // dynamic array of enrolled student IDs
    int       studentCount;
    int       studentCap;
    int       maxCapacity;   // venue capacity limit (set after load)

    Section()
        : studentIDs(nullptr), studentCount(0), studentCap(0), maxCapacity(40)
    { }

    Section(const char* sid, const char* cid, const char* tid,
            const char* vid, const char* ts, int mc=40)
        : sectionID(sid), courseID(cid), teacherID(tid),
          venueID(vid), timeSlot(ts),
          studentIDs(nullptr), studentCount(0), studentCap(0), maxCapacity(mc)
    { }

    // copy
    Section(const Section& o)
        : sectionID(o.sectionID), courseID(o.courseID),
          teacherID(o.teacherID), venueID(o.venueID), timeSlot(o.timeSlot),
          studentIDs(nullptr), studentCount(0), studentCap(0),
          maxCapacity(o.maxCapacity)
    {
        for(int i=0;i<o.studentCount;i++) addStudent(o.studentIDs[i]);
    }
    Section& operator=(const Section& o){
        if(this!=&o){
            sectionID=o.sectionID; courseID=o.courseID;
            teacherID=o.teacherID; venueID=o.venueID; timeSlot=o.timeSlot;
            maxCapacity=o.maxCapacity;
            delete[] studentIDs; studentIDs=nullptr; studentCount=0; studentCap=0;
            for(int i=0;i<o.studentCount;i++) addStudent(o.studentIDs[i]);
        }
        return *this;
    }
    ~Section() { delete[] studentIDs; }

    bool isFull()  const { return studentCount >= maxCapacity; }
    bool hasStudent(const MyString& sid) const {
        for(int i=0;i<studentCount;i++) if(studentIDs[i]==sid) return true;
        return false;
    }

    void addStudent(const MyString& sid) {
        if(studentCount==studentCap){
            int nc=studentCap==0?4:studentCap*2;
            MyString* nb=new MyString[nc];
            for(int i=0;i<studentCount;i++) nb[i]=studentIDs[i];
            delete[] studentIDs; studentIDs=nb; studentCap=nc;
        }
        studentIDs[studentCount++]=sid;
    }

    void display() const {
        std::cout << "  ";
        printCol(sectionID,10); printCol(courseID,10);
        printCol(venueID,8);    printCol(timeSlot,14);
        std::cout << studentCount << "/" << maxCapacity << "\n";
    }

    void toFile(std::ofstream& f) const {
        f << sectionID<<"|"<<courseID<<"|"<<teacherID<<"|"<<venueID<<"|"<<timeSlot<<"\n";
    }

    static Section fromParts(const MyArray<MyString>& p) {
        if(p.size()<5) return Section();
        return Section(p[0].c_str(),p[1].c_str(),p[2].c_str(),p[3].c_str(),p[4].c_str());
    }
};
