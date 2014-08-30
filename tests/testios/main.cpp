#include <QCoreApplication>
#include <string>
#include <iostream>
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    std::string s = "sdfd";
    std::cout<<s;
    return a.exec();
}
