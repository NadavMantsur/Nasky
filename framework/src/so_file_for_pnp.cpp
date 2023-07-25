#include <iostream>

class DisplayFunc
{
public:
    explicit DisplayFunc() =default;
    ~DisplayFunc() noexcept =default;

    DisplayFunc(const DisplayFunc&) =delete;
    DisplayFunc& operator=(const  DisplayFunc&) =delete;

    void Display()
    {
        std::cout << "task was added" << std::endl;
    }
}

void __attribute__ ((constructor)) Action();

void Action()
{
   AddTask(&DisplayFunc::Display);   
}