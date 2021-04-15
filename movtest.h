#ifndef MOVTEST_H
#define MOVTEST_H

#include <QObject>

#include "mov.h"

class MovTest : public QObject
{
    Q_OBJECT
public:
    explicit MovTest(QObject *parent = nullptr);

public slots:
    void slotTest();
    // void slotGetMov(Mov&& m); <-- rvalues don't work, fails moc
    //void slotGetMov(const Mov& m);
    void slotGetMov(Mov& m);
    //void slotGetMov(Mov m);

signals:

};

#endif // MOVTEST_H
