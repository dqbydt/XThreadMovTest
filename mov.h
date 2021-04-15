#ifndef MOV_H
#define MOV_H

#include <QDebug>

#ifdef Q_OS_WIN
#include <windows.h>
#include <processthreadsapi.h>
#endif

#include "AddrClassifier.h"

// Various strings for debug printout of ID/addresses.
// NOTE: Had initially made these as private members, but that does not work!
// qPrintable produces a char*, and you cannot return a char* from a function
// unless it has been allocated on the heap (and must therefore be freed by the
// caller)!
#define objStr()    qPrintable(mObjIDString)
#define selfStr()   qPrintable(AddrClassifier::classifyFull(mSelfAddr))
#define allocStr()  qPrintable(AddrClassifier::classifyFull(mAllocAddr))

class Mov
{
private:
    uint32_t*   mpAllocMem;
    uint32_t    mObjID;         // ID of this object
    QString     mObjIDString;   // H/S classifier + ID of this object, derived from its addresss
    uintptr_t   mSelfAddr;
    uintptr_t   mAllocAddr;
    uint32_t    mTID;

    static constexpr size_t ALLOC_SIZE = 64;

    void init();        // Get addr of this object
    void initLite();    // Init w/o mem alloc -- used in move functions

public:
    Mov();

    // Movable type that manages a resource: Rule of Five applies
    // https://en.cppreference.com/w/cpp/language/rule_of_three
    // Unlike Rule of Three, failing to provide MC and MAO is
    // usually not an error, but a missed optimization opportunity.
    ~Mov();                                 // I.   Dtor
    Mov(const Mov& o);                      // II.  Copy ctor
    Mov(Mov&& o) noexcept;                  // III. Move ctor*
    Mov& operator=(const Mov& that);        // IV.  Copy Assignment Operator
    Mov& operator=(Mov&& that) noexcept;    // V.   Move Assignment Operator*

    // *for noexcept, see discussion in book ch 7. Tl;dr: needed to enable
    // moves instead of copies in std::vector reallocation. noexcept specifier needed
    // on both declaration (.h) and defn (.cpp).

    // Retval is const to prevent it from being used as an lvalue. e.g.
    // cannot say (mv1 + mv2) = mv3.
    // UPDATE! THIS IS NOW AN ANTI-PATTERN! If the ret obj is const, then
    // that hinders move optimization in
    //      coll.push_back(mv+mv)
    // SO: for move semantics purposes, operator+ must **NOT** return const!
    Mov operator+(const Mov& rhs) const;

    void id() const { qDebug("Mov %s id  : Obj addr = %s; Alloc addr = %s, contents %03x",
                        objStr(), selfStr(), allocStr(), mpAllocMem[0]); }

};

#endif // MOV_H
