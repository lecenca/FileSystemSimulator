#ifndef OPTION_H
#define OPTION_H

template<class T>
struct Option{
    bool none;
    T some;

    Option():none(true){}
    Option(T t):none(false),some(t){}
};

#endif // OPTION_H
