#include "loginwindow.h"
#include "mainwindow.h"
#include <unistd.h>
#include "ui_loginwindow.h"
#include <QMessageBox>
#include "extensions.hpp"
#include <string>
#include "client.hpp"

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
        Client client(1);
        bool connect_flag = client.connectServer();

        // connect failed
        if (connect_flag == false)
        {
            msgbox.critical(this, "Sorry", "The server is currently unavailable, please try again later");
            return;
        }

        // connected
        unsigned int account = atoll(usernames[ui->AccountEdit->text().toStdString()].c_str());

        int fail_flag = client.login(account, ui->PasswdEdit->text().toStdString().c_str());

        // login successfully
        if (!fail_flag)
        {
            msgbox.information(this, "Login", "Welcome, " + ui->AccountEdit->text() + "!");
            w->set_passwd(ui->PasswdEdit->text().toStdString());
            w->show();
            this->close();
        }
        else
        {
            msgbox.critical(this, "Login failed", "Invalid account or password, please try again");
        }
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
