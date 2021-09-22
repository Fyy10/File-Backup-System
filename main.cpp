#include "loginwindow.h"
#include "mainwindow.h"
#include "signwindow.h"
#include <iostream>

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    LoginWindow *login = new LoginWindow;
    MainWindow *w = new MainWindow;
    SignWindow *sign = new SignWindow;

    login->set_MainWindow(w);
    login->set_SignWindow(sign);

    login->show();
    return a.exec();
}
