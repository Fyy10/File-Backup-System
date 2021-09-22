#include "loginwindow.h"
#include "mainwindow.h"
#include <unistd.h>
#include "ui_loginwindow.h"
#include <QMessageBox>
#include "extensions.hpp"
#include <string>

using namespace std;

LoginWindow::LoginWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LoginWindow)
{
    ui->setupUi(this);

    // login clicked
    connect(ui->LoginButton, &QPushButton::clicked, this, &LoginWindow::login_clicked);
    // signup clicked
    connect(ui->SignButton, &QPushButton::clicked, this, &LoginWindow::signup_clicked);
}

LoginWindow::~LoginWindow()
{
    delete ui;
}

void LoginWindow::login_clicked()
{
    QMessageBox msgbox;
    MapFile users("./accounts");
    map<string, string> usernames = users.load();

    // check user identity
    if (usernames.find(ui->AccountEdit->text().toStdString()) != usernames.end())
    {
        msgbox.information(this, "Login", "Welcome, " + ui->AccountEdit->text() + "!");
        w->show();
        this->close();
    }
    else
    {
        msgbox.critical(this, "Login failed", "Invalid account or password, please try again");
    }
}

void LoginWindow::signup_clicked()
{
    sign->show();
}

void LoginWindow::set_MainWindow(MainWindow *w)
{
    this->w = w;
}

void LoginWindow::set_SignWindow(SignWindow *sign)
{
    this->sign = sign;
}
