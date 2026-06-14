#pragma once
#include "common.h"
#include "section.h"
#include "venue.h"
#include "course.h"

// ─────────────────────────────────────────────────────────────────────────────
//  BookingRecord  —  tracks venue+slot -> sectionID
// ─────────────────────────────────────────────────────────────────────────────
struct BookingRecord {
    MyString venueID;
    MyString slot;
    MyString sectionID;
};

class Scheduler {
public:
    // Returns heap-allocated char* with the full schedule text.
    // Caller must delete[].
    char* generateExamSchedule(
        Section* sections, int secCount,
        Venue*   venues,   int venCount,
        Course** courses,  int courCount)
    {
        const char* slots[] = {
            "MON-08:00","MON-11:00","MON-14:00",
            "TUE-08:00","TUE-11:00","TUE-14:00",
            "WED-08:00","WED-11:00","WED-14:00",
            "THU-08:00","THU-11:00","THU-14:00",
            "FRI-08:00","FRI-11:00"
        };
        const int SLOT_COUNT = 14;

        // Dynamic booking table
        int       bookCap=32, bookCount=0;
        BookingRecord* bookings=new BookingRecord[bookCap];

        // Output buffer — grow as needed
        int   outCap=4096, outLen=0;
        char* out=new char[outCap];
        out[0]='\0';

        auto append=[&](const char* s){
            int l=(int)strlen(s);
            if(outLen+l+1>=outCap){
                outCap=(outLen+l+1)*2;
                char* nb=new char[outCap];
                memcpy(nb,out,outLen); nb[outLen]='\0';
                delete[] out; out=nb;
            }
            memcpy(out+outLen,s,l+1);
            outLen+=l;
        };
        auto appendFmt=[&](const char* fmt,...){
            char tmp[512]; va_list a; va_start(a,fmt);
            vsnprintf(tmp,sizeof(tmp),fmt,a); va_end(a);
            append(tmp);
        };

        append("============================================================\n");
        append("                     EXAM SCHEDULE\n");
        append("============================================================\n");
        appendFmt("%-12s%-14s%-10s%-16s%s\n","Section","Course","Venue","TimeSlot","Duration");
        append("------------------------------------------------------------\n");

        // Unscheduled list
        int   unsCap=8, unsCount=0;
        MyString* uns=new MyString[unsCap];

        for(int si=0;si<secCount;si++){
            Section& sec=sections[si];

            // Find course
            Course* course=nullptr;
            for(int ci=0;ci<courCount;ci++)
                if(courses[ci]->courseID==sec.courseID){ course=courses[ci]; break; }
            if(!course) continue;

            // Lab courses — no exam
            if(course->getExamDurationMins()==0){
                appendFmt("%-12s%-14s%-10s%-16s%s\n",
                    sec.sectionID.c_str(), sec.courseID.c_str(),
                    "N/A","N/A","No exam (Lab)");
                continue;
            }

            int need=sec.studentCount>0?sec.studentCount:1;
            bool scheduled=false;

            for(int sl=0;sl<SLOT_COUNT&&!scheduled;sl++){
                for(int vi=0;vi<venCount&&!scheduled;vi++){
                    Venue& v=venues[vi];
                    if(course->needsComputers()&&!v.hasComputers) continue;
                    if(v.capacity<need) continue;

                    // Check if this venue+slot is already booked
                    bool taken=false;
                    for(int bi=0;bi<bookCount;bi++){
                        if(bookings[bi].venueID==v.roomID &&
                           bookings[bi].slot==MyString(slots[sl])){ taken=true; break; }
                    }
                    if(taken) continue;

                    // Book it
                    if(bookCount==bookCap){
                        bookCap*=2;
                        BookingRecord* nb=new BookingRecord[bookCap];
                        for(int bi=0;bi<bookCount;bi++) nb[bi]=bookings[bi];
                        delete[] bookings; bookings=nb;
                    }
                    bookings[bookCount].venueID   = v.roomID;
                    bookings[bookCount].slot       = MyString(slots[sl]);
                    bookings[bookCount].sectionID  = sec.sectionID;
                    bookCount++;

                    appendFmt("%-12s%-14s%-10s%-16s%d min\n",
                        sec.sectionID.c_str(), sec.courseID.c_str(),
                        v.roomID.c_str(), slots[sl],
                        course->getExamDurationMins());
                    scheduled=true;
                }
            }

            if(!scheduled){
                appendFmt("%-12s%-14s*** COULD NOT SCHEDULE (no venue/slot available) ***\n",
                    sec.sectionID.c_str(), sec.courseID.c_str());
                if(unsCount==unsCap){
                    unsCap*=2; MyString* nb=new MyString[unsCap];
                    for(int i=0;i<unsCount;i++) nb[i]=uns[i];
                    delete[] uns; uns=nb;
                }
                uns[unsCount++]=sec.sectionID;
            }
        }

        // Conflict solver — scan bookings for duplicate venue+slot (shouldn't happen but verify)
        bool anyConflict=false;
        for(int i=0;i<bookCount;i++){
            for(int j=i+1;j<bookCount;j++){
                if(bookings[i].venueID==bookings[j].venueID &&
                   bookings[i].slot   ==bookings[j].slot){
                    if(!anyConflict){
                        append("\n=== CONFLICT REPORT ===\n");
                        anyConflict=true;
                    }
                    appendFmt("  CONFLICT: %s and %s both at %s %s\n",
                        bookings[i].sectionID.c_str(),
                        bookings[j].sectionID.c_str(),
                        bookings[i].venueID.c_str(),
                        bookings[i].slot.c_str());
                    // Suggest next free slot for the second one
                    const char* nextSlot=nullptr;
                    for(int sl=0;sl<SLOT_COUNT;sl++){
                        bool free=true;
                        for(int bi=0;bi<bookCount;bi++)
                            if(bookings[bi].venueID==bookings[j].venueID &&
                               bookings[bi].slot==MyString(slots[sl])){ free=false; break; }
                        if(free){ nextSlot=slots[sl]; break; }
                    }
                    if(nextSlot)
                        appendFmt("  Suggestion: move %s to %s at %s\n",
                            bookings[j].sectionID.c_str(),
                            bookings[j].venueID.c_str(), nextSlot);
                }
            }
        }

        if(!anyConflict && unsCount==0)
            append("\n  All sections scheduled successfully — no conflicts.\n");

        if(unsCount>0){
            append("\n=== UNSCHEDULED ===\n");
            for(int i=0;i<unsCount;i++){
                append("  "); append(uns[i].c_str()); append("\n");
            }
        }

        delete[] bookings;
        delete[] uns;
        return out; // caller deletes
    }
};
