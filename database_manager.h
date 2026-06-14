#pragma once
#include "common.h"
#include "entity.h"
#include "course.h"
#include "section.h"
#include "venue.h"

// ─────────────────────────────────────────────────────────────────────────────
//  Weightage config struct
// ─────────────────────────────────────────────────────────────────────────────
struct Weightage {
    MyString courseType;
    double   exam, assignment, quiz;
};

// ─────────────────────────────────────────────────────────────────────────────
//  AssessmentRecord  —  flat record read from assessments.txt
// ─────────────────────────────────────────────────────────────────────────────
struct AssessmentRecord {
    MyString sectionID, studentID, type;
    double   raw, max;
};

// ─────────────────────────────────────────────────────────────────────────────
//  FeedbackRecord
// ─────────────────────────────────────────────────────────────────────────────
struct FeedbackRecord {
    MyString teacherID, studentID, comment;
    int      rating;
};

// ─────────────────────────────────────────────────────────────────────────────
//  EnrollmentRecord
// ─────────────────────────────────────────────────────────────────────────────
struct EnrollmentRecord {
    MyString studentID, sectionID;
};

// ─────────────────────────────────────────────────────────────────────────────
//  DatabaseManager
// ─────────────────────────────────────────────────────────────────────────────
class DatabaseManager {
public:

    // ── Weightages ────────────────────────────────────────────────────────────
    Weightage* loadWeightages(int& count) const {
        count=0;
        std::ifstream f("data/weightages.txt");
        if(!f.is_open()) return nullptr;
        // count lines first
        MyString line; int cap=8;
        Weightage* arr=new Weightage[cap];
        while(myGetline(f,line)){
            line=line.trim();
            if(line.empty()||line[0]=='#') continue;
            MyArray<MyString> p=splitLine(line);
            if(p.size()<4) continue;
            if(count==cap){ cap*=2; Weightage* nb=new Weightage[cap]; for(int i=0;i<count;i++) nb[i]=arr[i]; delete[] arr; arr=nb; }
            arr[count].courseType=p[0];
            arr[count].exam=p[1].toDouble();
            arr[count].assignment=p[2].toDouble();
            arr[count].quiz=p[3].toDouble();
            count++;
        }
        return arr;
    }

    Weightage getWeightage(const char* courseType) const {
        int n=0;
        Weightage* arr=loadWeightages(n);
        Weightage w; w.exam=50; w.assignment=30; w.quiz=20; // defaults
        // match "Core"<->"CoreCourse" etc.
        char expanded[64];
        snprintf(expanded,sizeof(expanded),"%sCourse",courseType);
        for(int i=0;i<n;i++){
            if(arr[i].courseType==courseType||arr[i].courseType==MyString(expanded)){
                w=arr[i]; break;
            }
        }
        delete[] arr;
        return w;
    }

    // ── Students ──────────────────────────────────────────────────────────────
    Student** loadStudents(int& count) const {
        count=0; int cap=16;
        Student** arr=new Student*[cap];
        std::ifstream f("data/students.txt");
        if(!f.is_open()) return arr;
        MyString line;
        while(myGetline(f,line)){
            line=line.trim();
            if(line.empty()||line[0]=='#') continue;
            MyArray<MyString> p=splitLine(line);
            Student* s=makeStudent(p);
            if(!s) continue;
            if(count==cap){ cap*=2; Student** nb=new Student*[cap]; for(int i=0;i<count;i++) nb[i]=arr[i]; delete[] arr; arr=nb; }
            arr[count++]=s;
        }
        return arr;
    }

    void saveStudents(Student** arr, int count) const {
        std::ofstream f("data/students.txt");
        for(int i=0;i<count;i++) arr[i]->toFile(f);
    }

    void freeStudents(Student** arr, int count) const {
        for(int i=0;i<count;i++) delete arr[i];
        delete[] arr;
    }

    // ── Teachers ──────────────────────────────────────────────────────────────
    Teacher* loadTeachers(int& count) const {
        count=0; int cap=8;
        Teacher* arr=new Teacher[cap];
        std::ifstream f("data/teachers.txt");
        if(!f.is_open()) return arr;
        MyString line;
        while(myGetline(f,line)){
            line=line.trim();
            if(line.empty()||line[0]=='#') continue;
            MyArray<MyString> p=splitLine(line);
            if(p.size()<3) continue;
            if(count==cap){ cap*=2; Teacher* nb=new Teacher[cap]; for(int i=0;i<count;i++) nb[i]=arr[i]; delete[] arr; arr=nb; }
            arr[count++]=Teacher::fromParts(p);
        }
        return arr;
    }

    void saveTeachers(Teacher* arr, int count) const {
        std::ofstream f("data/teachers.txt");
        for(int i=0;i<count;i++) arr[i].toFile(f);
    }

    // ── Courses ───────────────────────────────────────────────────────────────
    Course** loadCourses(int& count) const {
        count=0; int cap=16;
        Course** arr=new Course*[cap];
        std::ifstream f("data/courses.txt");
        if(!f.is_open()) return arr;
        MyString line;
        while(myGetline(f,line)){
            line=line.trim();
            if(line.empty()||line[0]=='#') continue;
            MyArray<MyString> p=splitLine(line);
            Course* c=makeCourseFromParts(p);
            if(!c) continue;
            if(count==cap){ cap*=2; Course** nb=new Course*[cap]; for(int i=0;i<count;i++) nb[i]=arr[i]; delete[] arr; arr=nb; }
            arr[count++]=c;
        }
        return arr;
    }

    void saveCourses(Course** arr, int count) const {
        std::ofstream f("data/courses.txt");
        for(int i=0;i<count;i++) arr[i]->toFile(f);
    }

    void freeCourses(Course** arr, int count) const {
        for(int i=0;i<count;i++) delete arr[i];
        delete[] arr;
    }

    // ── Sections ──────────────────────────────────────────────────────────────
    Section* loadSections(int& count) const {
        count=0; int cap=16;
        Section* arr=new Section[cap];
        std::ifstream f("data/sections.txt");
        if(!f.is_open()) return arr;
        MyString line;
        while(myGetline(f,line)){
            line=line.trim();
            if(line.empty()||line[0]=='#') continue;
            MyArray<MyString> p=splitLine(line);
            if(p.size()<5) continue;
            if(count==cap){ cap*=2; Section* nb=new Section[cap]; for(int i=0;i<count;i++) nb[i]=arr[i]; delete[] arr; arr=nb; }
            arr[count++]=Section::fromParts(p);
        }
        // inject student lists
        int en=0;
        EnrollmentRecord* enr=loadEnrollments(en);
        for(int i=0;i<count;i++){
            for(int j=0;j<en;j++){
                if(enr[j].sectionID==arr[i].sectionID)
                    arr[i].addStudent(enr[j].studentID);
            }
        }
        delete[] enr;
        return arr;
    }

    void saveSections(Section* arr, int count) const {
        std::ofstream f("data/sections.txt");
        for(int i=0;i<count;i++) arr[i].toFile(f);
    }

    // ── Venues ────────────────────────────────────────────────────────────────
    Venue* loadVenues(int& count) const {
        count=0; int cap=8;
        Venue* arr=new Venue[cap];
        std::ifstream f("data/venues.txt");
        if(!f.is_open()) return arr;
        MyString line;
        while(myGetline(f,line)){
            line=line.trim();
            if(line.empty()||line[0]=='#') continue;
            MyArray<MyString> p=splitLine(line);
            if(p.size()<3) continue;
            if(count==cap){ cap*=2; Venue* nb=new Venue[cap]; for(int i=0;i<count;i++) nb[i]=arr[i]; delete[] arr; arr=nb; }
            arr[count++]=Venue::fromParts(p);
        }
        return arr;
    }

    void saveVenues(Venue* arr, int count) const {
        std::ofstream f("data/venues.txt");
        for(int i=0;i<count;i++) arr[i].toFile(f);
    }

    // ── Enrollments ───────────────────────────────────────────────────────────
    // File: studentID|sectionID
    EnrollmentRecord* loadEnrollments(int& count) const {
        count=0; int cap=32;
        EnrollmentRecord* arr=new EnrollmentRecord[cap];
        std::ifstream f("data/enrollments.txt");
        if(!f.is_open()) return arr;
        MyString line;
        while(myGetline(f,line)){
            line=line.trim();
            if(line.empty()) continue;
            MyArray<MyString> p=splitLine(line);
            if(p.size()<2) continue;
            if(count==cap){ cap*=2; EnrollmentRecord* nb=new EnrollmentRecord[cap]; for(int i=0;i<count;i++) nb[i]=arr[i]; delete[] arr; arr=nb; }
            arr[count].studentID=p[0];
            arr[count].sectionID=p[1];
            count++;
        }
        return arr;
    }

    void appendEnrollment(const MyString& stuID, const MyString& secID) const {
        std::ofstream f("data/enrollments.txt", std::ios::app);
        f << stuID << "|" << secID << "\n";
    }

    bool isEnrolled(const MyString& stuID, const MyString& secID) const {
        int n=0;
        EnrollmentRecord* arr=loadEnrollments(n);
        bool found=false;
        for(int i=0;i<n;i++){
            if(arr[i].studentID==stuID && arr[i].sectionID==secID){ found=true; break; }
        }
        delete[] arr;
        return found;
    }

    // sections a student is enrolled in — caller must delete[]
    MyString* getStudentSections(const MyString& stuID, int& count) const {
        count=0; int cap=8;
        MyString* arr=new MyString[cap];
        int n=0;
        EnrollmentRecord* enr=loadEnrollments(n);
        for(int i=0;i<n;i++){
            if(enr[i].studentID==stuID){
                if(count==cap){ cap*=2; MyString* nb=new MyString[cap]; for(int j=0;j<count;j++) nb[j]=arr[j]; delete[] arr; arr=nb; }
                arr[count++]=enr[i].sectionID;
            }
        }
        delete[] enr;
        return arr;
    }

    // ── Assessments ───────────────────────────────────────────────────────────
    // File: sectionID|studentID|type|rawScore|maxScore
    AssessmentRecord* loadAssessments(int& count) const {
        count=0; int cap=32;
        AssessmentRecord* arr=new AssessmentRecord[cap];
        std::ifstream f("data/assessments.txt");
        if(!f.is_open()) return arr;
        MyString line;
        while(myGetline(f,line)){
            line=line.trim();
            if(line.empty()) continue;
            MyArray<MyString> p=splitLine(line);
            if(p.size()<5) continue;
            if(count==cap){ cap*=2; AssessmentRecord* nb=new AssessmentRecord[cap]; for(int i=0;i<count;i++) nb[i]=arr[i]; delete[] arr; arr=nb; }
            arr[count].sectionID=p[0];
            arr[count].studentID=p[1];
            arr[count].type=p[2];
            arr[count].raw=p[3].toDouble();
            arr[count].max=p[4].toDouble();
            count++;
        }
        return arr;
    }

    void appendAssessment(const MyString& secID, const MyString& stuID,
                          const MyString& type, double raw, double mx) const {
        std::ofstream f("data/assessments.txt", std::ios::app);
        f << secID<<"|"<<stuID<<"|"<<type<<"|"<<raw<<"|"<<mx<<"\n";
    }

    // Compute final grade percentage for one student in one section
    double computeGrade(const MyString& secID, const MyString& stuID,
                        const char* courseType) const {
        Weightage w = getWeightage(courseType);

        int n=0;
        AssessmentRecord* recs=loadAssessments(n);

        double eRaw=0,eMax=0, aRaw=0,aMax=0, qRaw=0,qMax=0;
        for(int i=0;i<n;i++){
            if(recs[i].sectionID!=secID||recs[i].studentID!=stuID) continue;
            if(recs[i].type=="Exam")       { eRaw+=recs[i].raw; eMax+=recs[i].max; }
            else if(recs[i].type=="Assignment"){ aRaw+=recs[i].raw; aMax+=recs[i].max; }
            else if(recs[i].type=="Quiz")  { qRaw+=recs[i].raw; qMax+=recs[i].max; }
        }
        delete[] recs;

        double grade=0;
        if(eMax>0) grade += (eRaw/eMax)*w.exam;
        if(aMax>0) grade += (aRaw/aMax)*w.assignment;
        if(qMax>0) grade += (qRaw/qMax)*w.quiz;
        return grade;
    }

    // ── Feedback ──────────────────────────────────────────────────────────────
    // File: teacherID|studentID|rating|comment
    FeedbackRecord* loadFeedback(int& count) const {
        count=0; int cap=16;
        FeedbackRecord* arr=new FeedbackRecord[cap];
        std::ifstream f("data/feedback.txt");
        if(!f.is_open()) return arr;
        MyString line;
        while(myGetline(f,line)){
            line=line.trim();
            if(line.empty()) continue;
            MyArray<MyString> p=splitLine(line);
            if(p.size()<4) continue;
            if(count==cap){ cap*=2; FeedbackRecord* nb=new FeedbackRecord[cap]; for(int i=0;i<count;i++) nb[i]=arr[i]; delete[] arr; arr=nb; }
            arr[count].teacherID=p[0];
            arr[count].studentID=p[1];
            arr[count].rating=p[2].toInt();
            arr[count].comment=p[3];
            count++;
        }
        return arr;
    }

    void appendFeedback(const MyString& tid, const MyString& sid,
                        int rating, const MyString& comment) const {
        std::ofstream f("data/feedback.txt", std::ios::app);
        f << tid<<"|"<<sid<<"|"<<rating<<"|"<<comment<<"\n";
    }

    // Recompute all teacher avg scores from feedback file, then save
    void recalcAndSaveTeacherFeedback() const {
        int tn=0; Teacher* teachers=loadTeachers(tn);
        int fn=0; FeedbackRecord* fb=loadFeedback(fn);
        for(int i=0;i<tn;i++){
            double sum=0; int cnt=0;
            for(int j=0;j<fn;j++)
                if(fb[j].teacherID==teachers[i].ID){ sum+=fb[j].rating; cnt++; }
            teachers[i].avgFeedback = cnt>0 ? sum/cnt : 0.0;
        }
        saveTeachers(teachers,tn);
        delete[] teachers;
        delete[] fb;
    }

    // ── Exam Schedule ─────────────────────────────────────────────────────────
    void saveExamSchedule(const char* content) const {
        std::ofstream f("data/exam_schedule.txt");
        f << content;
    }

    // returns heap-allocated char*, caller must delete[]
    char* loadExamSchedule() const {
        std::ifstream f("data/exam_schedule.txt");
        if(!f.is_open()){ char* r=new char[1]; r[0]='\0'; return r; }
        // read whole file
        f.seekg(0,std::ios::end);
        int sz=(int)f.tellg(); f.seekg(0,std::ios::beg);
        if(sz<=0){ char* r=new char[1]; r[0]='\0'; return r; }
        char* buf=new char[sz+1];
        f.read(buf,sz); buf[sz]='\0';
        return buf;
    }

    // ── Helpers to find by ID ─────────────────────────────────────────────────
    int findSectionIdx(Section* arr, int n, const MyString& sid) const {
        for(int i=0;i<n;i++) if(arr[i].sectionID==sid) return i;
        return -1;
    }
    int findVenueIdx(Venue* arr, int n, const MyString& rid) const {
        for(int i=0;i<n;i++) if(arr[i].roomID==rid) return i;
        return -1;
    }
    Course* findCourse(Course** arr, int n, const MyString& cid) const {
        for(int i=0;i<n;i++) if(arr[i]->courseID==cid) return arr[i];
        return nullptr;
    }
    Student* findStudent(Student** arr, int n, const MyString& id) const {
        for(int i=0;i<n;i++) if(arr[i]->ID==id) return arr[i];
        return nullptr;
    }
};
