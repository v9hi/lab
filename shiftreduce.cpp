#include <iostream>
using namespace std;

int main()
{
    cout << "Stack\t\tinput buffer\tAction\n";
    cout << "----------------------------------------------\n";
    cout << "i\t\ti+i*i$\tShift\n";
    cout << "F\t\t+i*i$\tReduce by F -> id\n";
    cout << "T\t\t+i*i$\tReduce by T -> F\n";
    cout << "E\t\t+i*i$\tReduce by E -> T\n";
    cout << "E+\t\ti*i$\tShift\n";
    cout << "E+i\t\t*i$\tShift\n";
    cout << "E+F\t\t*i$\tReduce by F -> id\n";
    cout << "E+T\t\t*i$\tReduce by T -> F\n";
    cout << "E+T*\t\ti$\tShift\n";
    cout << "E+T*i\t\t$\tShift\n";
    cout << "E+T*F\t\t$\tReduce by F -> id\n";
    cout << "E+T\t\t$\tReduce by T -> T*F\n";
    cout << "E\t\t$\tReduce by E -> E+T\n";
    cout << "E\t\t$\tAccept\n";
    return 0;
}
