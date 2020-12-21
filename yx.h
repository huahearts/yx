#ifndef YX_H_
#define YX_H_
#include <iostream>
#include <string>
class Unit 
{
public:
    Unit() = default;
    ~Unit() = default;
    Unit(int id_,int maxHp_,int curHp, std::string name);
    virtual void Attack() = 0;
    virtual void Run() = 0;
private:
    int m_iId;
    std::string m_sName;
    int m_iMaxHp;
    int m_iCurHp;
};

class Monster : public Unit {
public:
    Monster() = default;
    ~Monster() = default;
};

#endif
