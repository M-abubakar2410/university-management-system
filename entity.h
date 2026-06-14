#pragma once
#include "common.h"

// ── AcademicEntity (abstract) ─────────────────────────────────────────────────
class AcademicEntity {
public:
    MyString ID, name, email, file_path;

    AcademicEntity() {}
    AcademicEntity(const char* id, const char* n, const char* em)
        : ID(id), name(n), email(em) {}
    virtual ~AcademicEntity() {}

    virtual void displayProfile() const = 0;
};

// ── FeedbackEntry ─────────────────────────────────────────────────────────────
struct FeedbackEntry {
    int      rating;
    MyString comment;
    MyString studentID;
};

// ── Teacher ───────────────────────────────────────────────────────────────────
class Teacher : public AcademicEntity {
public:
    double         avgFeedback;
    FeedbackEntry* feedbacks;
    int            feedbackCount;
    int            feedbackCap;

    Teacher()
        : avgFeedback(0), feedbacks(nullptr), feedbackCount(0), feedbackCap(0)
    { file_path = "data/teachers.txt"; }

    Teacher(const char* id, const char* n, const char* em, double af = 0.0)
        : AcademicEntity(id, n, em), avgFeedback(af),
          feedbacks(nullptr), feedbackCount(0), feedbackCap(0)
    { file_path = "data/teachers.txt"; }

    Teacher(const Teacher& o)
        : AcademicEntity(o.ID.c_str(), o.name.c_str(), o.email.c_str()),
          avgFeedback(o.avgFeedback),
          feedbacks(nullptr), feedbackCount(0), feedbackCap(0)
    {
        file_path = o.file_path;
        for (int i = 0; i < o.feedbackCount; i++) addFeedback(o.feedbacks[i]);
    }

    Teacher& operator=(const Teacher& o) {
        if (this != &o) {
            ID = o.ID; name = o.name; email = o.email;
            file_path = o.file_path; avgFeedback = o.avgFeedback;
            delete[] feedbacks; feedbacks = nullptr;
            feedbackCount = 0; feedbackCap = 0;
            for (int i = 0; i < o.feedbackCount; i++) addFeedback(o.feedbacks[i]);
        }
        return *this;
    }

    ~Teacher() { delete[] feedbacks; }

    void addFeedback(const FeedbackEntry& fe) {
        if (feedbackCount == feedbackCap) {
            int nc = feedbackCap == 0 ? 4 : feedbackCap * 2;
            FeedbackEntry* nb = new FeedbackEntry[nc];
            for (int i = 0; i < feedbackCount; i++) nb[i] = feedbacks[i];
            delete[] feedbacks; feedbacks = nb; feedbackCap = nc;
        }
        feedbacks[feedbackCount++] = fe;
        double sum = 0;
        for (int i = 0; i < feedbackCount; i++) sum += feedbacks[i].rating;
        avgFeedback = sum / feedbackCount;
    }

    void displayProfile() const override {
        printLine();
        std::cout << "  Teacher Profile\n";
        printLine();
        std::cout << "  ID    : " << ID    << "\n"
                  << "  Name  : " << name  << "\n"
                  << "  Email : " << email << "\n";
        printf("  Score : %.2f / 5.0\n", avgFeedback);
        if (feedbackCount > 0) {
            std::cout << "  Feedback (" << feedbackCount << " entries):\n";
            for (int i = 0; i < feedbackCount; i++) {
                std::cout << "    [" << feedbacks[i].rating << "/5] "
                          << feedbacks[i].comment
                          << "  (from " << feedbacks[i].studentID << ")\n";
            }
        } else {
            std::cout << "  No feedback yet.\n";
        }
    }

    void toFile(std::ofstream& f) const {
        f << ID << "|" << name << "|" << email << "|" << avgFeedback << "\n";
    }

    static Teacher fromParts(const MyArray<MyString>& p) {
        if (p.size() < 3) return Teacher();
        double af = p.size() >= 4 ? p[3].toDouble() : 0.0;
        return Teacher(p[0].c_str(), p[1].c_str(), p[2].c_str(), af);
    }
};

// ── GradeRecord ───────────────────────────────────────────────────────────────
struct GradeRecord {
    MyString sectionID;
    double   percentage;
};

// ── Student (abstract) ────────────────────────────────────────────────────────
class Student : public AcademicEntity {
public:
    MyString     studentType;
    MyString     status;
    GradeRecord* grades;
    int          gradeCount;
    int          gradeCap;

    Student()
        : status("Active"), grades(nullptr), gradeCount(0), gradeCap(0)
    { file_path = "data/students.txt"; }

    Student(const char* id, const char* n, const char* em)
        : AcademicEntity(id, n, em), status("Active"),
          grades(nullptr), gradeCount(0), gradeCap(0)
    { file_path = "data/students.txt"; }

    Student(const Student& o)
        : AcademicEntity(o.ID.c_str(), o.name.c_str(), o.email.c_str()),
          studentType(o.studentType), status(o.status),
          grades(nullptr), gradeCount(0), gradeCap(0)
    {
        file_path = o.file_path;
        for (int i = 0; i < o.gradeCount; i++)
            addGrade(o.grades[i].sectionID, o.grades[i].percentage);
    }

    virtual ~Student() { delete[] grades; }

    void addGrade(const MyString& sid, double pct) {
        for (int i = 0; i < gradeCount; i++) {
            if (grades[i].sectionID == sid) { grades[i].percentage = pct; return; }
        }
        if (gradeCount == gradeCap) {
            int nc = gradeCap == 0 ? 4 : gradeCap * 2;
            GradeRecord* nb = new GradeRecord[nc];
            for (int i = 0; i < gradeCount; i++) nb[i] = grades[i];
            delete[] grades; grades = nb; gradeCap = nc;
        }
        grades[gradeCount].sectionID  = sid;
        grades[gradeCount].percentage = pct;
        gradeCount++;
    }

    virtual double      calculateGPA()             const = 0;
    virtual const char* letterGrade(double pct)    const { return gradeFromPct(pct); }
    virtual void        toFile(std::ofstream& f)   const = 0;

    void viewTranscript() const {
        printLine();
        std::cout << "  Transcript  --  " << name << "  (" << ID << ")\n";
        printLine();
        if (gradeCount == 0) { std::cout << "  No grades recorded yet.\n"; return; }

        printCol("Section",    12);
        printCol("Percentage", 14);
        std::cout << "Grade\n";
        printLine('-', 32);

        for (int i = 0; i < gradeCount; i++) {
            char tmp[16];
            snprintf(tmp, sizeof(tmp), "%.1f%%", grades[i].percentage);
            printCol(grades[i].sectionID, 12);
            printCol(tmp, 14);
            std::cout << letterGrade(grades[i].percentage) << "\n";
        }

        double gpa = calculateGPA();
        if (gpa >= 0) {
            printLine('-', 32);
            printf("  GPA: %.2f\n", gpa);
        }
    }
};

// ── RegularStudent ────────────────────────────────────────────────────────────
class RegularStudent : public Student {
public:
    RegularStudent() { studentType = "Regular"; }
    RegularStudent(const char* id, const char* n, const char* em = "")
        : Student(id, n, em) { studentType = "Regular"; }

    double calculateGPA() const override {
        if (gradeCount == 0) return 0.0;
        double sum = 0;
        for (int i = 0; i < gradeCount; i++)
            sum += gradePoints(gradeFromPct(grades[i].percentage));
        return sum / gradeCount;
    }

    void displayProfile() const override {
        printLine();
        std::cout << "  Student Profile  [Regular]\n";
        printLine();
        std::cout << "  ID     : " << ID     << "\n"
                  << "  Name   : " << name   << "\n"
                  << "  Email  : " << email  << "\n"
                  << "  Status : " << status << "\n";
        printf("  GPA    : %.2f\n", calculateGPA());
    }

    void toFile(std::ofstream& f) const override {
        f << ID << "|" << name << "|" << email
          << "|Regular||" << status.c_str() << "|\n";
    }
};

// ── ScholarshipStudent ────────────────────────────────────────────────────────
class ScholarshipStudent : public Student {
public:
    double minGPA;

    ScholarshipStudent() : minGPA(3.0) { studentType = "Scholarship"; }
    ScholarshipStudent(const char* id, const char* n, const char* em = "", double mg = 3.0)
        : Student(id, n, em), minGPA(mg) { studentType = "Scholarship"; }

    double calculateGPA() const override {
        if (gradeCount == 0) return 0.0;
        double sum = 0;
        for (int i = 0; i < gradeCount; i++)
            sum += gradePoints(gradeFromPct(grades[i].percentage));
        return sum / gradeCount;
    }

    void checkStatus() {
        status = (gradeCount > 0 && calculateGPA() < minGPA) ? "Probation" : "Active";
    }

    void displayProfile() const override {
        printLine();
        std::cout << "  Student Profile  [Scholarship]\n";
        printLine();
        std::cout << "  ID      : " << ID     << "\n"
                  << "  Name    : " << name   << "\n"
                  << "  Email   : " << email  << "\n"
                  << "  Status  : " << status << "\n";
        printf("  Min GPA : %.1f\n", minGPA);
        printf("  GPA     : %.2f\n", calculateGPA());
    }

    void toFile(std::ofstream& f) const override {
        f << ID << "|" << name << "|" << email
          << "|Scholarship||" << status.c_str() << "|" << minGPA << "\n";
    }
};

// ── ExchangeStudent ───────────────────────────────────────────────────────────
class ExchangeStudent : public Student {
public:
    ExchangeStudent() { studentType = "Exchange"; }
    ExchangeStudent(const char* id, const char* n, const char* em = "")
        : Student(id, n, em) { studentType = "Exchange"; }

    double calculateGPA() const override { return -1.0; }

    const char* letterGrade(double pct) const override {
        return pct >= 50 ? "Pass" : "Fail";
    }

    void displayProfile() const override {
        printLine();
        std::cout << "  Student Profile  [Exchange]\n";
        printLine();
        std::cout << "  ID      : " << ID     << "\n"
                  << "  Name    : " << name   << "\n"
                  << "  Email   : " << email  << "\n"
                  << "  Grading : Pass / Fail\n"
                  << "  Status  : " << status << "\n";
    }

    void toFile(std::ofstream& f) const override {
        f << ID << "|" << name << "|" << email
          << "|Exchange||" << status.c_str() << "|\n";
    }
};

// ── Factory ───────────────────────────────────────────────────────────────────
inline Student* makeStudent(const MyArray<MyString>& p) {
    if (p.size() < 4) return nullptr;
    MyString id = p[0], name = p[1], email = p[2], type = p[3];
    MyString status = p.size() >= 6 ? p[5] : MyString("Active");
    Student* s = nullptr;
    if (type == "Scholarship") {
        double mg = (p.size() >= 7 && !p[6].empty()) ? p[6].toDouble() : 3.0;
        auto* ss = new ScholarshipStudent(id.c_str(), name.c_str(), email.c_str(), mg);
        ss->status = status; s = ss;
    } else if (type == "Exchange") {
        auto* es = new ExchangeStudent(id.c_str(), name.c_str(), email.c_str());
        es->status = status; s = es;
    } else {
        auto* rs = new RegularStudent(id.c_str(), name.c_str(), email.c_str());
        rs->status = status; s = rs;
    }
    return s;
}
