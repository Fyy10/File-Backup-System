#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "QMessageBox"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // backup clicked
    connect(ui->BackupButton, &QPushButton::clicked, this, &MainWindow::backup_clicked);
    // recover clicked
    connect(ui->RecoverButton, &QPushButton::clicked, this, &MainWindow::recover_clicked);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::backup_clicked()
{
    QMessageBox msgbox;
    msgbox.information(this, "Success", "Backup done!");
}

void MainWindow::recover_clicked()
{
    QMessageBox msgbox;
    msgbox.information(this, "Success", "Recover done!");
}
