#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QMessageBox>
#include <QFileDialog>

#include "core.hpp"
#include "extensions.hpp"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // backup clicked
    connect(ui->BackupButton, &QPushButton::clicked, this, &MainWindow::backup_clicked);
    // recover clicked
    connect(ui->RecoverButton, &QPushButton::clicked, this, &MainWindow::recover_clicked);
    // select pack
    connect(ui->PackOption, &QCheckBox::clicked, this, &MainWindow::pack_clicked);
    // select auto
    connect(ui->AutoOption, &QCheckBox::clicked, this, &MainWindow::auto_clicked);
    // browse source clicked
    connect(ui->BrowseSource, &QPushButton::clicked, this, &MainWindow::browse_source_clicked);
    // browse target clicked
    connect(ui->BrowseTarget, &QPushButton::clicked, this, &MainWindow::browse_target_clicked);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::browse_source_clicked()
{
    QFileDialog file_diag;
    QString source_path = file_diag.getExistingDirectory(this, "Choose the folder to backup", "./");
    if (!source_path.isEmpty()) ui->SourcePath->setText(source_path);
}

void MainWindow::browse_target_clicked()
{
    QFileDialog file_diag;
    QString target_path = file_diag.getExistingDirectory(this, "Choose the folder to store the backup", "./");
    if (!target_path.isEmpty()) ui->TargetPath->setText(target_path);
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

void MainWindow::pack_clicked()
{
    if (ui->PackOption->checkState() == Qt::Checked)
    {
        ui->AutoOption->setCheckState(Qt::Unchecked);
    }
    else
    {
        // uncheck
    }
}

void MainWindow::auto_clicked()
{
    if (ui->AutoOption->checkState() == Qt::Checked)
    {
        ui->PackOption->setCheckState(Qt::Unchecked);
    }
    else
    {
        // uncheck
    }
}
