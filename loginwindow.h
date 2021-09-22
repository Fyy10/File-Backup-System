#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QWidget>
#include "mainwindow.h"
#include "signwindow.h"

namespace Ui {
class LoginWindow;
}

class LoginWindow : public QWidget
{
    Q_OBJECT

public:
    explicit LoginWindow(QWidget *parent = nullptr);
    ~LoginWindow();

    void set_MainWindow(MainWindow *w);
    void set_SignWindow(SignWindow *sign);

private slots:
    void login_clicked();
    void signup_clicked();

private:
    Ui::LoginWindow *ui;

    MainWindow *w;
    SignWindow *sign;
};

#endif // LOGINWINDOW_H
