#pragma once
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cstdio>

// ─────────────────────────────────────────────────────────────────────────────
//  MyString  —  heap-managed string, no STL
// ─────────────────────────────────────────────────────────────────────────────
class MyString {
    char* buf;
    int   len;

    void alloc(const char* s) {
        len = s ? (int)strlen(s) : 0;
        buf = new char[len + 1];
        if(s) memcpy(buf, s, len + 1);
        else buf[0] = '\0';
    }
public:
    MyString()                    { alloc(""); }
    MyString(const char* s)       { alloc(s ? s : ""); }
    MyString(const MyString& o)   { alloc(o.buf); }
    ~MyString()                   { delete[] buf; }

    MyString& operator=(const MyString& o) {
        if(this!=&o){ delete[] buf; alloc(o.buf); }
        return *this;
    }
    MyString& operator=(const char* s) {
        delete[] buf; alloc(s ? s : "");
        return *this;
    }

    bool operator==(const MyString& o)  const { return strcmp(buf, o.buf)==0; }
    bool operator==(const char* s)      const { return strcmp(buf, s ? s : "")==0; }
    bool operator!=(const MyString& o)  const { return !(*this==o); }
    bool operator!=(const char* s)      const { return !(*this==s); }
    bool operator<(const MyString& o)   const { return strcmp(buf, o.buf)<0; }

    MyString operator+(const MyString& o) const {
        char* tmp = new char[len + o.len + 1];
        memcpy(tmp, buf, len);
        memcpy(tmp+len, o.buf, o.len+1);
        MyString r(tmp); delete[] tmp; return r;
    }
    MyString operator+(const char* s) const {
        MyString t(s); return *this + t;
    }
    MyString& operator+=(const MyString& o) { *this = *this + o; return *this; }
    MyString& operator+=(const char* s)     { *this = *this + s; return *this; }

    const char* c_str() const { return buf; }
    int size()          const { return len; }
    bool empty()        const { return len==0; }

    char operator[](int i) const { return buf[i]; }
    char& operator[](int i)      { return buf[i]; }

    // substring
    MyString substr(int start, int length=-1) const {
        if(start>=len) return MyString("");
        int l = (length<0 || start+length>len) ? len-start : length;
        char* tmp = new char[l+1];
        memcpy(tmp, buf+start, l);
        tmp[l]='\0';
        MyString r(tmp); delete[] tmp; return r;
    }

    // find character, returns -1 if not found
    int find(char c, int from=0) const {
        for(int i=from;i<len;i++) if(buf[i]==c) return i;
        return -1;
    }

    // trim whitespace
    MyString trim() const {
        int s=0, e=len-1;
        while(s<=e && (buf[s]==' '||buf[s]=='\t'||buf[s]=='\r'||buf[s]=='\n')) s++;
        while(e>=s && (buf[e]==' '||buf[e]=='\t'||buf[e]=='\r'||buf[e]=='\n')) e--;
        return substr(s, e-s+1);
    }

    // convert double to MyString
    static MyString fromDouble(double d, int prec=2) {
        char tmp[64];
        snprintf(tmp, sizeof(tmp), "%.*f", prec, d);
        return MyString(tmp);
    }
    static MyString fromInt(int v) {
        char tmp[32]; snprintf(tmp,sizeof(tmp),"%d",v);
        return MyString(tmp);
    }

    double toDouble() const { return atof(buf); }
    int    toInt()    const { return atoi(buf); }

    friend std::ostream& operator<<(std::ostream& os, const MyString& s) {
        os << s.buf; return os;
    }
    friend std::istream& operator>>(std::istream& is, MyString& s) {
        char tmp[4096]; is >> tmp; s = tmp; return is;
    }
};

// ─── getline for MyString ─────────────────────────────────────────────────────
inline bool myGetline(std::istream& is, MyString& s) {
    char tmp[4096]; tmp[0]='\0';
    if(!is.getline(tmp, sizeof(tmp))) { s=""; return false; }
    s = tmp;
    return true;
}
