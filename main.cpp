#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <climits>
#include <cstdarg>

#include "MyString.h"
#include "MyArray.h"
#include "common.h"
#include "assessment.h"
#include "venue.h"
#include "entity.h"
#include "course.h"
#include "section.h"
#include "database_manager.h"
#include "scheduler.h"

DatabaseManager db;

// ── Forward declarations ──────────────────────────────────────────────────────
void mainMenu();
void studentLogin();
void teacherLogin();
void adminMenu();
void studentMenu(Student* s);
void teacherMenu(Teacher& t);

// ═════════════════════════════════════════════════════════════════════════════
//  MAIN MENU
// ═════════════════════════════════════════════════════════════════════════════
void mainMenu() {
    while (true) {
        printHeader("FAST University -- Academic Portal");
        std::cout << "  1. Student Login\n"
                  << "  2. Teacher Login\n"
                  << "  3. Admin Panel\n"
                  << "  0. Exit\n\n";

        int ch = getInt("  Choice: ", 0, 3);
        switch (ch) {
            case 1: studentLogin(); break;
            case 2: teacherLogin(); break;
            case 3: adminMenu();    break;
            case 0:
                std::cout << "\n  Goodbye!\n\n";
                return;
        }
    }
}

// ═════════════════════════════════════════════════════════════════════════════
//  LOGIN
// ═════════════════════════════════════════════════════════════════════════════
void studentLogin() {
    printHeader("Student Login");
    MyString id = getString("  Student ID: ");

    int n = 0;
    Student** students = db.loadStudents(n);
    Student* found = db.findStudent(students, n, id);

    if (!found) {
        std::cout << "  Error: Student ID not found.\n";
        db.freeStudents(students, n);
        pause();
        return;
    }
    studentMenu(found);
    db.freeStudents(students, n);
}

void teacherLogin() {
    printHeader("Teacher Login");
    MyString id = getString("  Teacher ID: ");

    int n = 0;
    Teacher* teachers = db.loadTeachers(n);

    int idx = -1;
    for (int i = 0; i < n; i++) if (teachers[i].ID == id) { idx = i; break; }

    if (idx < 0) {
        std::cout << "  Error: Teacher ID not found.\n";
        delete[] teachers;
        pause();
        return;
    }

    // Inject feedback
    int fn = 0;
    FeedbackRecord* fb = db.loadFeedback(fn);
    for (int i = 0; i < fn; i++) {
        if (fb[i].teacherID == teachers[idx].ID) {
            FeedbackEntry fe;
            fe.rating    = fb[i].rating;
            fe.comment   = fb[i].comment;
            fe.studentID = fb[i].studentID;
            teachers[idx].addFeedback(fe);
        }
    }
    delete[] fb;

    teacherMenu(teachers[idx]);
    delete[] teachers;
}

// ═════════════════════════════════════════════════════════════════════════════
//  STUDENT MENU
// ═════════════════════════════════════════════════════════════════════════════
void studentMenu(Student* stu) {
    while (true) {
        printHeader("Student Portal");
        std::cout << "  Logged in as: " << stu->name
                  << "  (" << stu->studentType << ")\n\n";

        std::cout << "  1. View My Profile\n"
                  << "  2. Browse Available Sections\n"
                  << "  3. Register for a Section\n"
                  << "  4. View My Enrolled Sections\n"
                  << "  5. View Transcript and GPA\n"
                  << "  6. View Grades by Section\n"
                  << "  7. Give Teacher Feedback\n"
                  << "  0. Logout\n\n";

        int ch = getInt("  Choice: ", 0, 7);
        if (ch == 0) return;

        // ── 1. Profile ────────────────────────────────────────────────────────
        if (ch == 1) {
            printHeader("My Profile");
            stu->displayProfile();
            pause();
        }

        // ── 2. Browse sections ────────────────────────────────────────────────
        else if (ch == 2) {
            printHeader("Available Sections");

            int sn = 0; Section* sections = db.loadSections(sn);
            int cn = 0; Course** courses  = db.loadCourses(cn);
            int vn = 0; Venue*   venues   = db.loadVenues(vn);

            std::cout << "  ";
            printCol("ID",      10);
            printCol("Course",  10);
            printCol("Title",   28);
            printCol("Time",    14);
            printCol("Venue",   8);
            printCol("Seats",   8);
            std::cout << "Type\n";
            printLine();

            for (int i = 0; i < sn; i++) {
                Course* c  = db.findCourse(courses, cn, sections[i].courseID);
                int     vi = db.findVenueIdx(venues, vn, sections[i].venueID);
                int     cap = vi >= 0 ? venues[vi].capacity : 40;

                char seats[16];
                snprintf(seats, sizeof(seats), "%d/%d", sections[i].studentCount, cap);

                std::cout << "  ";
                printCol(sections[i].sectionID,   10);
                printCol(sections[i].courseID,    10);
                printCol(c ? c->title.c_str() : "?", 28);
                printCol(sections[i].timeSlot,    14);
                printCol(sections[i].venueID,     8);
                printCol(seats,                   8);
                std::cout << (c ? c->getType() : "?") << "\n";
            }

            delete[] sections;
            db.freeCourses(courses, cn);
            delete[] venues;
            pause();
        }

        // ── 3. Register ───────────────────────────────────────────────────────
        else if (ch == 3) {
            printHeader("Register for a Section");

            int sn = 0; Section* sections = db.loadSections(sn);
            int vn = 0; Venue*   venues   = db.loadVenues(vn);

            MyString secID = getString("  Section ID: ");
            int si = db.findSectionIdx(sections, sn, secID);

            if (si < 0) {
                std::cout << "  Error: Section not found.\n";
                delete[] sections; delete[] venues;
                pause(); continue;
            }
            if (db.isEnrolled(stu->ID, secID)) {
                std::cout << "  You are already enrolled in " << secID << ".\n";
                delete[] sections; delete[] venues;
                pause(); continue;
            }

            int vi  = db.findVenueIdx(venues, vn, sections[si].venueID);
            int cap = vi >= 0 ? venues[vi].capacity : 40;
            if (sections[si].studentCount >= cap) {
                std::cout << "  Section is full (capacity: " << cap << ").\n";
                delete[] sections; delete[] venues;
                pause(); continue;
            }

            // Time conflict check
            int mySn = 0;
            MyString* mySecs = db.getStudentSections(stu->ID, mySn);
            bool conflict = false;
            for (int j = 0; j < mySn && !conflict; j++) {
                int mi = db.findSectionIdx(sections, sn, mySecs[j]);
                if (mi >= 0 && sections[mi].timeSlot == sections[si].timeSlot) {
                    std::cout << "  Time conflict: you already have a class at "
                              << sections[si].timeSlot
                              << " (section " << mySecs[j] << ").\n";
                    conflict = true;
                }
            }
            delete[] mySecs;

            if (!conflict) {
                db.appendEnrollment(stu->ID, secID);
                std::cout << "  Registered for " << secID << " successfully.\n";
            }

            delete[] sections; delete[] venues;
            pause();
        }

        // ── 4. My enrollments ─────────────────────────────────────────────────
        else if (ch == 4) {
            printHeader("My Enrolled Sections");

            int mySn = 0;
            MyString* mySecs = db.getStudentSections(stu->ID, mySn);
            int sn = 0; Section* sections = db.loadSections(sn);
            int cn = 0; Course** courses  = db.loadCourses(cn);

            if (mySn == 0) {
                std::cout << "  No enrollments found.\n";
            } else {
                std::cout << "  ";
                printCol("Section", 12);
                printCol("Course",  10);
                printCol("Title",   28);
                std::cout << "Time\n";
                printLine();

                for (int i = 0; i < mySn; i++) {
                    int si = db.findSectionIdx(sections, sn, mySecs[i]);
                    if (si < 0) continue;
                    Course* c = db.findCourse(courses, cn, sections[si].courseID);
                    std::cout << "  ";
                    printCol(sections[si].sectionID,        12);
                    printCol(sections[si].courseID,         10);
                    printCol(c ? c->title.c_str() : "?",   28);
                    std::cout << sections[si].timeSlot << "\n";
                }
            }

            delete[] mySecs;
            delete[] sections;
            db.freeCourses(courses, cn);
            pause();
        }

        // ── 5. Transcript ─────────────────────────────────────────────────────
        else if (ch == 5) {
            printHeader("Transcript");

            int mySn = 0;
            MyString* mySecs = db.getStudentSections(stu->ID, mySn);
            int sn = 0; Section* sections = db.loadSections(sn);
            int cn = 0; Course** courses  = db.loadCourses(cn);

            stu->gradeCount = 0;
            for (int i = 0; i < mySn; i++) {
                int si = db.findSectionIdx(sections, sn, mySecs[i]);
                if (si < 0) continue;
                Course* c = db.findCourse(courses, cn, sections[si].courseID);
                double g  = db.computeGrade(mySecs[i], stu->ID, c ? c->getType() : "Core");
                stu->addGrade(mySecs[i], g);
            }

            stu->viewTranscript();

            if (stu->studentType == "Scholarship") {
                ScholarshipStudent* ss = (ScholarshipStudent*)stu;
                ss->checkStatus();
                if (ss->status == "Probation")
                    std::cout << "\n  WARNING: GPA is below the minimum threshold ("
                              << ss->minGPA << "). Status set to Probation.\n";
            }

            delete[] mySecs;
            delete[] sections;
            db.freeCourses(courses, cn);
            pause();
        }

        // ── 6. Grades by section ──────────────────────────────────────────────
        else if (ch == 6) {
            printHeader("My Grades");

            int mySn = 0;
            MyString* mySecs = db.getStudentSections(stu->ID, mySn);
            int sn = 0; Section* sections = db.loadSections(sn);
            int cn = 0; Course** courses  = db.loadCourses(cn);

            if (mySn == 0) {
                std::cout << "  No enrollments found.\n";
            } else {
                std::cout << "  ";
                printCol("Section", 12);
                printCol("Title",   30);
                printCol("Grade",   10);
                std::cout << "Letter\n";
                printLine();

                for (int i = 0; i < mySn; i++) {
                    int si = db.findSectionIdx(sections, sn, mySecs[i]);
                    if (si < 0) continue;
                    Course* c   = db.findCourse(courses, cn, sections[si].courseID);
                    double  g   = db.computeGrade(mySecs[i], stu->ID, c ? c->getType() : "Core");
                    char    tmp[16];
                    snprintf(tmp, sizeof(tmp), "%.1f%%", g);
                    std::cout << "  ";
                    printCol(mySecs[i],                    12);
                    printCol(c ? c->title.c_str() : "?",  30);
                    printCol(tmp,                          10);
                    std::cout << stu->letterGrade(g) << "\n";
                }
            }

            delete[] mySecs;
            delete[] sections;
            db.freeCourses(courses, cn);
            pause();
        }

        // ── 7. Feedback ───────────────────────────────────────────────────────
        else if (ch == 7) {
            printHeader("Give Teacher Feedback");

            int mySn = 0;
            MyString* mySecs = db.getStudentSections(stu->ID, mySn);

            if (mySn == 0) {
                std::cout << "  You are not enrolled in any section.\n";
                delete[] mySecs; pause(); continue;
            }

            int sn = 0; Section* sections = db.loadSections(sn);
            int tn = 0; Teacher*  teachers = db.loadTeachers(tn);

            // Collect unique teacher IDs from student's sections
            MyString* tids = new MyString[mySn];
            int tidCount = 0;
            for (int i = 0; i < mySn; i++) {
                int si = db.findSectionIdx(sections, sn, mySecs[i]);
                if (si < 0) continue;
                MyString tid = sections[si].teacherID;
                bool already = false;
                for (int j = 0; j < tidCount; j++) if (tids[j] == tid) { already = true; break; }
                if (!already) tids[tidCount++] = tid;
            }

            std::cout << "  Teachers in your sections:\n\n";
            for (int i = 0; i < tidCount; i++) {
                for (int j = 0; j < tn; j++) {
                    if (teachers[j].ID == tids[i])
                        std::cout << "    " << teachers[j].ID
                                  << "  --  " << teachers[j].name << "\n";
                }
            }

            MyString tid = getString("\n  Enter Teacher ID to rate: ");

            bool valid = false;
            for (int i = 0; i < tidCount; i++) if (tids[i] == tid) { valid = true; break; }

            if (!valid) {
                std::cout << "  That teacher is not in any of your sections.\n";
            } else {
                int rating      = getInt("  Rating (1-5): ", 1, 5);
                MyString comment = getString("  Comment: ");
                db.appendFeedback(tid, stu->ID, rating, comment);
                db.recalcAndSaveTeacherFeedback();
                std::cout << "  Feedback submitted. Thank you.\n";
            }

            delete[] tids;
            delete[] mySecs;
            delete[] sections;
            delete[] teachers;
            pause();
        }
    }
}

// ═════════════════════════════════════════════════════════════════════════════
//  TEACHER MENU
// ═════════════════════════════════════════════════════════════════════════════
void teacherMenu(Teacher& teacher) {
    while (true) {
        printHeader("Teacher Portal");
        std::cout << "  Logged in as: " << teacher.name << "\n\n";

        std::cout << "  1. View Profile and Feedback\n"
                  << "  2. View My Sections\n"
                  << "  3. Enter Assessment Marks\n"
                  << "  4. View Section Roster\n"
                  << "  5. Grade Summary by Section\n"
                  << "  0. Logout\n\n";

        int ch = getInt("  Choice: ", 0, 5);
        if (ch == 0) return;

        // ── 1. Profile ────────────────────────────────────────────────────────
        if (ch == 1) {
            printHeader("My Profile");
            teacher.displayProfile();
            pause();
        }

        // ── 2. My sections ────────────────────────────────────────────────────
        else if (ch == 2) {
            printHeader("My Sections");

            int sn = 0; Section* sections = db.loadSections(sn);
            int cn = 0; Course** courses  = db.loadCourses(cn);

            std::cout << "  ";
            printCol("Section", 12);
            printCol("Course",  10);
            printCol("Title",   28);
            printCol("Time",    14);
            std::cout << "Students\n";
            printLine();

            bool any = false;
            for (int i = 0; i < sn; i++) {
                if (sections[i].teacherID != teacher.ID) continue;
                Course* c = db.findCourse(courses, cn, sections[i].courseID);
                std::cout << "  ";
                printCol(sections[i].sectionID,        12);
                printCol(sections[i].courseID,         10);
                printCol(c ? c->title.c_str() : "?",  28);
                printCol(sections[i].timeSlot,         14);
                std::cout << sections[i].studentCount << "\n";
                any = true;
            }
            if (!any) std::cout << "  No sections assigned.\n";

            delete[] sections;
            db.freeCourses(courses, cn);
            pause();
        }

        // ── 3. Enter marks ────────────────────────────────────────────────────
        else if (ch == 3) {
            printHeader("Enter Assessment Marks");

            int sn = 0; Section* sections = db.loadSections(sn);

            std::cout << "  Your sections:\n";
            for (int i = 0; i < sn; i++)
                if (sections[i].teacherID == teacher.ID)
                    std::cout << "    " << sections[i].sectionID
                              << "  (" << sections[i].courseID << ")\n";

            MyString secID = getString("\n  Section ID: ");
            int si = db.findSectionIdx(sections, sn, secID);

            if (si < 0 || sections[si].teacherID != teacher.ID) {
                std::cout << "  Error: Invalid section or not your section.\n";
                delete[] sections; pause(); continue;
            }
            if (sections[si].studentCount == 0) {
                std::cout << "  No students enrolled in this section.\n";
                delete[] sections; pause(); continue;
            }

            std::cout << "\n  Assessment type:\n"
                      << "    1. Exam\n"
                      << "    2. Assignment\n"
                      << "    3. Quiz\n";
            int tc = getInt("  Choice: ", 1, 3);
            const char* atype = (tc == 1 ? "Exam" : (tc == 2 ? "Assignment" : "Quiz"));

            double mx = getDouble("  Max possible score: ", 1, 1000);

            std::cout << "\n  Enter marks for each student:\n";
            printLine();

            int stuN = 0;
            Student** students = db.loadStudents(stuN);

            for (int i = 0; i < sections[si].studentCount; i++) {
                MyString  sid   = sections[si].studentIDs[i];
                Student*  s     = db.findStudent(students, stuN, sid);
                char      prompt[128];
                snprintf(prompt, sizeof(prompt), "  %s  (%s)  score [0 - %.0f]: ",
                         sid.c_str(), s ? s->name.c_str() : "?", mx);
                double score = getDouble(prompt, 0, mx);
                db.appendAssessment(secID, sid, MyString(atype), score, mx);
                std::cout << "    Saved.\n";
            }

            std::cout << "\n  All marks entered and saved.\n";
            db.freeStudents(students, stuN);
            delete[] sections;
            pause();
        }

        // ── 4. Roster ─────────────────────────────────────────────────────────
        else if (ch == 4) {
            printHeader("Section Roster");

            int sn = 0; Section* sections = db.loadSections(sn);

            std::cout << "  Your sections:\n";
            for (int i = 0; i < sn; i++)
                if (sections[i].teacherID == teacher.ID)
                    std::cout << "    " << sections[i].sectionID
                              << "  (" << sections[i].courseID << ")\n";

            MyString secID = getString("\n  Section ID: ");
            int si = db.findSectionIdx(sections, sn, secID);

            if (si < 0 || sections[si].teacherID != teacher.ID) {
                std::cout << "  Error: Invalid section.\n";
                delete[] sections; pause(); continue;
            }
            if (sections[si].studentCount == 0) {
                std::cout << "  No students enrolled.\n";
                delete[] sections; pause(); continue;
            }

            int stuN = 0;
            Student** students = db.loadStudents(stuN);

            std::cout << "\n  ";
            printCol("#",   5);
            printCol("ID",  12);
            printCol("Name",24);
            std::cout << "Type\n";
            printLine();

            for (int i = 0; i < sections[si].studentCount; i++) {
                MyString sid = sections[si].studentIDs[i];
                Student* s   = db.findStudent(students, stuN, sid);
                char     num[16];
                snprintf(num, sizeof(num), "%d.", i + 1);
                std::cout << "  ";
                printCol(num,                           5);
                printCol(sid,                           12);
                printCol(s ? s->name.c_str() : "?",    24);
                std::cout << (s ? s->studentType.c_str() : "?") << "\n";
            }

            db.freeStudents(students, stuN);
            delete[] sections;
            pause();
        }

        // ── 5. Grade summary ──────────────────────────────────────────────────
        else if (ch == 5) {
            printHeader("Grade Summary");

            int sn = 0; Section* sections = db.loadSections(sn);
            int cn = 0; Course** courses  = db.loadCourses(cn);

            std::cout << "  Your sections:\n";
            for (int i = 0; i < sn; i++)
                if (sections[i].teacherID == teacher.ID)
                    std::cout << "    " << sections[i].sectionID
                              << "  (" << sections[i].courseID << ")\n";

            MyString secID = getString("\n  Section ID: ");
            int si = db.findSectionIdx(sections, sn, secID);

            if (si < 0 || sections[si].teacherID != teacher.ID) {
                std::cout << "  Error: Invalid section.\n";
                delete[] sections; db.freeCourses(courses, cn); pause(); continue;
            }

            Course*     course = db.findCourse(courses, cn, sections[si].courseID);
            const char* ct     = course ? course->getType() : "Core";

            int stuN = 0;
            Student** students = db.loadStudents(stuN);

            std::cout << "\n  ";
            printCol("ID",   12);
            printCol("Name", 24);
            printCol("Grade",10);
            std::cout << "Letter\n";
            printLine();

            double classSum = 0;
            int    cnt      = 0;

            for (int i = 0; i < sections[si].studentCount; i++) {
                MyString    sid    = sections[si].studentIDs[i];
                double      g      = db.computeGrade(secID, sid, ct);
                Student*    s      = db.findStudent(students, stuN, sid);
                const char* letter = s ? s->letterGrade(g) : gradeFromPct(g);
                char        tmp[16];
                snprintf(tmp, sizeof(tmp), "%.1f%%", g);
                classSum += g; cnt++;

                std::cout << "  ";
                printCol(sid,                          12);
                printCol(s ? s->name.c_str() : "?",   24);
                printCol(tmp,                          10);
                std::cout << letter << "\n";
            }

            if (cnt > 0) {
                printLine();
                printf("  Class average: %.1f%%\n", classSum / cnt);
            } else {
                std::cout << "  No students or no marks entered yet.\n";
            }

            db.freeStudents(students, stuN);
            delete[] sections;
            db.freeCourses(courses, cn);
            pause();
        }
    }
}

// ═════════════════════════════════════════════════════════════════════════════
//  ADMIN MENU
// ═════════════════════════════════════════════════════════════════════════════
void adminMenu() {
    while (true) {
        printHeader("Admin Panel");

        std::cout << "  1.  Add Student\n"
                  << "  2.  Add Teacher\n"
                  << "  3.  Add Course\n"
                  << "  4.  Add Section\n"
                  << "  5.  Add Venue\n"
                  << "  6.  View All Students\n"
                  << "  7.  View All Teachers\n"
                  << "  8.  View All Courses\n"
                  << "  9.  View All Venues\n"
                  << "  10. Generate Exam Schedule\n"
                  << "  11. View Exam Schedule\n"
                  << "  0.  Back to Main Menu\n\n";

        int ch = getInt("  Choice: ", 0, 11);
        if (ch == 0) return;

        // ── 1. Add student ────────────────────────────────────────────────────
        if (ch == 1) {
            printHeader("Add Student");
            MyString id    = getString("  Student ID : ");
            MyString name  = getString("  Name       : ");
            MyString email = getString("  Email      : ");

            std::cout << "  Type:\n"
                      << "    1. Regular\n"
                      << "    2. Scholarship\n"
                      << "    3. Exchange\n";
            int tc = getInt("  Choice: ", 1, 3);

            int n = 0;
            Student** students = db.loadStudents(n);

            bool dup = false;
            for (int i = 0; i < n; i++) if (students[i]->ID == id) { dup = true; break; }
            if (dup) {
                std::cout << "  Error: Student ID already exists.\n";
                db.freeStudents(students, n); pause(); continue;
            }

            Student* ns = nullptr;
            if (tc == 1) {
                ns = new RegularStudent(id.c_str(), name.c_str(), email.c_str());
            } else if (tc == 2) {
                double mg = getDouble("  Minimum GPA required (e.g. 2.5): ", 0.0, 4.0);
                ns = new ScholarshipStudent(id.c_str(), name.c_str(), email.c_str(), mg);
            } else {
                ns = new ExchangeStudent(id.c_str(), name.c_str(), email.c_str());
            }

            Student** newArr = new Student*[n + 1];
            for (int i = 0; i < n; i++) newArr[i] = students[i];
            newArr[n] = ns;
            db.saveStudents(newArr, n + 1);

            delete[] students;
            db.freeStudents(newArr, n + 1);
            std::cout << "  Student added.\n";
            pause();
        }

        // ── 2. Add teacher ────────────────────────────────────────────────────
        else if (ch == 2) {
            printHeader("Add Teacher");
            MyString id    = getString("  Teacher ID : ");
            MyString name  = getString("  Name       : ");
            MyString email = getString("  Email      : ");

            int n = 0;
            Teacher* teachers = db.loadTeachers(n);

            bool dup = false;
            for (int i = 0; i < n; i++) if (teachers[i].ID == id) { dup = true; break; }
            if (dup) {
                std::cout << "  Error: Teacher ID already exists.\n";
                delete[] teachers; pause(); continue;
            }

            Teacher* newArr = new Teacher[n + 1];
            for (int i = 0; i < n; i++) newArr[i] = teachers[i];
            newArr[n] = Teacher(id.c_str(), name.c_str(), email.c_str(), 0.0);
            db.saveTeachers(newArr, n + 1);

            delete[] teachers;
            delete[] newArr;
            std::cout << "  Teacher added.\n";
            pause();
        }

        // ── 3. Add course ─────────────────────────────────────────────────────
        else if (ch == 3) {
            printHeader("Add Course");
            MyString cid   = getString("  Course ID  : ");
            MyString title = getString("  Title      : ");
            MyString tid   = getString("  Teacher ID : ");

            std::cout << "  Type:\n"
                      << "    1. Core\n"
                      << "    2. Elective\n"
                      << "    3. Lab\n";
            int tc = getInt("  Choice: ", 1, 3);
            const char* type = (tc == 1 ? "Core" : (tc == 2 ? "Elective" : "Lab"));

            int n = 0;
            Course** courses = db.loadCourses(n);
            Course** newArr  = new Course*[n + 1];
            for (int i = 0; i < n; i++) newArr[i] = courses[i];
            newArr[n] = makeCourse(cid.c_str(), title.c_str(), tid.c_str(), type);
            db.saveCourses(newArr, n + 1);

            delete[] courses;
            db.freeCourses(newArr, n + 1);
            std::cout << "  Course added.\n";
            pause();
        }

        // ── 4. Add section ────────────────────────────────────────────────────
        else if (ch == 4) {
            printHeader("Add Section");
            MyString sid = getString("  Section ID : ");
            MyString cid = getString("  Course ID  : ");
            MyString tid = getString("  Teacher ID : ");
            MyString vid = getString("  Venue ID   : ");
            MyString ts  = getString("  Time Slot (e.g. MON-09:00): ");

            int n = 0;
            Section* sections = db.loadSections(n);
            Section* newArr   = new Section[n + 1];
            for (int i = 0; i < n; i++) newArr[i] = sections[i];
            newArr[n] = Section(sid.c_str(), cid.c_str(), tid.c_str(), vid.c_str(), ts.c_str());
            db.saveSections(newArr, n + 1);

            delete[] sections;
            delete[] newArr;
            std::cout << "  Section added.\n";
            pause();
        }

        // ── 5. Add venue ──────────────────────────────────────────────────────
        else if (ch == 5) {
            printHeader("Add Venue");
            MyString rid = getString("  Room ID  : ");
            int      cap = getInt(   "  Capacity : ", 1, 1000);
            int      hc  = getInt(   "  Has computers? (1=Yes, 0=No): ", 0, 1);

            int    n      = 0;
            Venue* venues = db.loadVenues(n);
            Venue* newArr = new Venue[n + 1];
            for (int i = 0; i < n; i++) newArr[i] = venues[i];
            newArr[n] = Venue(rid.c_str(), cap, hc == 1);
            db.saveVenues(newArr, n + 1);

            delete[] venues;
            delete[] newArr;
            std::cout << "  Venue added.\n";
            pause();
        }

        // ── 6. All students ───────────────────────────────────────────────────
        else if (ch == 6) {
            printHeader("All Students");

            int n = 0;
            Student** students = db.loadStudents(n);

            std::cout << "  ";
            printCol("ID",     10);
            printCol("Name",   22);
            printCol("Type",   14);
            std::cout << "Status\n";
            printLine();

            for (int i = 0; i < n; i++) {
                std::cout << "  ";
                printCol(students[i]->ID,         10);
                printCol(students[i]->name,        22);
                printCol(students[i]->studentType, 14);
                std::cout << students[i]->status << "\n";
            }
            std::cout << "\n  Total: " << n << " students.\n";
            db.freeStudents(students, n);
            pause();
        }

        // ── 7. All teachers ───────────────────────────────────────────────────
        else if (ch == 7) {
            printHeader("All Teachers");

            int n = 0;
            Teacher* teachers = db.loadTeachers(n);

            std::cout << "  ";
            printCol("ID",   10);
            printCol("Name", 26);
            std::cout << "Avg Score\n";
            printLine();

            for (int i = 0; i < n; i++) {
                std::cout << "  ";
                printCol(teachers[i].ID,   10);
                printCol(teachers[i].name, 26);
                printf("%.2f / 5.0\n", teachers[i].avgFeedback);
            }

            delete[] teachers;
            pause();
        }

        // ── 8. All courses ────────────────────────────────────────────────────
        else if (ch == 8) {
            printHeader("All Courses");

            int n = 0;
            Course** courses = db.loadCourses(n);

            std::cout << "  ";
            printCol("ID",      10);
            printCol("Title",   30);
            printCol("Type",    12);
            std::cout << "Teacher\n";
            printLine();

            for (int i = 0; i < n; i++) {
                std::cout << "  ";
                printCol(courses[i]->courseID, 10);
                printCol(courses[i]->title,    30);
                printCol(courses[i]->getType(),12);
                std::cout << courses[i]->teacherID << "\n";
            }

            db.freeCourses(courses, n);
            pause();
        }

        // ── 9. All venues ─────────────────────────────────────────────────────
        else if (ch == 9) {
            printHeader("All Venues");

            int    n      = 0;
            Venue* venues = db.loadVenues(n);

            std::cout << "  ";
            printCol("Room", 10);
            printCol("Capacity", 12);
            std::cout << "Computers\n";
            printLine();

            for (int i = 0; i < n; i++) {
                char tmp[8];
                snprintf(tmp, sizeof(tmp), "%d", venues[i].capacity);
                std::cout << "  ";
                printCol(venues[i].roomID, 10);
                printCol(tmp,              12);
                std::cout << (venues[i].hasComputers ? "Yes" : "No") << "\n";
            }

            delete[] venues;
            pause();
        }

        // ── 10. Generate exam schedule ────────────────────────────────────────
        else if (ch == 10) {
            printHeader("Generate Exam Schedule");

            int sn = 0; Section* sections = db.loadSections(sn);
            int vn = 0; Venue*   venues   = db.loadVenues(vn);
            int cn = 0; Course** courses  = db.loadCourses(cn);

            Scheduler sched;
            char* schedule = sched.generateExamSchedule(
                sections, sn, venues, vn, courses, cn);

            db.saveExamSchedule(schedule);
            std::cout << "  Schedule generated and saved.\n\n";
            std::cout << schedule;

            delete[] schedule;
            delete[] sections;
            delete[] venues;
            db.freeCourses(courses, cn);
            pause();
        }

        // ── 11. View exam schedule ────────────────────────────────────────────
        else if (ch == 11) {
            printHeader("Exam Schedule");
            char* sch = db.loadExamSchedule();
            if (sch[0] == '\0')
                std::cout << "  No schedule generated yet. Use option 10 first.\n";
            else
                std::cout << sch;
            delete[] sch;
            pause();
        }
    }
}

// ═════════════════════════════════════════════════════════════════════════════
//  Entry point
// ═════════════════════════════════════════════════════════════════════════════
int main() {
    system("mkdir -p data 2>/dev/null");
    mainMenu();
    return 0;
}
