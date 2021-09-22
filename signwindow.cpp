#include "signwindow.h"
#include "ui_signwindow.h"
#include <QMessageBox>
#include "extensions.hpp"
#include <map>
#include <string>

using namespace std;

SignWindow::SignWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SignWindow)
{
    ui->setupUi(this);

    // signup clicked
    connect(ui->SignButton, &QPushButton::clicked, this, &SignWindow::signup_clicked);
    // exit  clicked
    connect(ui->ExitButton, &QPushButton::clicked, this, &SignWindow::exit_clicked);
}

SignWindow::~SignWindow()
{
    delete ui;
}

void SignWindow::signup_clicked()
{
    QMessageBox msgbox;

    // invalid password
    if ((ui->PasswdEdit1->text().length() < MIN_PASSWD_LEN) || (ui->PasswdEdit2->text().length() < MIN_PASSWD_LEN))
    {
        QString num(std::to_string(MIN_PASSWD_LEN).c_str());
        msgbox.critical(this, "Invalid password", "The password should be no shorter than " + num + " characters");
        ui->PasswdEdit1->clear();
        ui->PasswdEdit2->clear();
        ui->AccountLineEdit->clear();
        return;
    }
    else if ((ui->PasswdEdit1->text().length() > MAX_PASSWD_LEN) || (ui->PasswdEdit2->text().length() > MAX_PASSWD_LEN))
    {
        QString num(std::to_string(MAX_PASSWD_LEN).c_str());
        msgbox.critical(this, "Invalid password", "The password should not be longer than " + num + " characters");
        ui->PasswdEdit1->clear();
        ui->PasswdEdit2->clear();
        ui->AccountLineEdit->clear();
        return;
    }
    else if (ui->PasswdEdit1->text() != ui->PasswdEdit2->text())
    {
        msgbox.critical(this, "Invalid password", "Two passwords are not identical");
        ui->PasswdEdit1->clear();
        ui->PasswdEdit2->clear();
        ui->AccountLineEdit->clear();
        return;
    }

    // invalid username
    MapFile users("./accounts");
    map<string, string> usernames = users.load();
    if (usernames.find(ui->AccountLineEdit->text().toStdString()) != usernames.end())
    {
        msgbox.critical(this, "User already exists", "User already exists, please try another user name");
        ui->PasswdEdit1->clear();
        ui->PasswdEdit2->clear();
        ui->AccountLineEdit->clear();
        return;
    }

    // valid username and password
    // todo: get an account from server
    QString account = "114514";
    usernames[ui->AccountLineEdit->text().toStdString()] = account.toStdString();
    users.save(usernames);

    msgbox.information(this, "Success", "Signed up successfully!");
    // ui->AccountLineEdit->setText(account);
    this->close();
}

void SignWindow::exit_clicked()
{
    this->close();
}
