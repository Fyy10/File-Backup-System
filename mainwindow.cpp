#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QMessageBox>
#include <QFileDialog>

#include <string>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // backup clicked
    connect(ui->BackupButton, &QPushButton::clicked, this, &MainWindow::backup_clicked);
    // recover clicked
    connect(ui->RecoverButton, &QPushButton::clicked, this, &MainWindow::recover_clicked);
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

bool MainWindow::build_generator(Generator && g, FileFilter && f, Export && e)
{
    return g.build(f, e);
}

void MainWindow::backup_clicked()
{
    QMessageBox msgbox;
    bool succ_flag = build_generator(LocalGenerator(ui->SourcePath->text().toStdString(), ui->TargetPath->text().toStdString()), FileFilter(), Duplicator());
    if (succ_flag)
    {
        // listen file change
        pthread_t tid;
        struct watch_roots* tmp = new struct watch_roots;
        tmp->source_root = ui->SourcePath->text().toStdString();
        tmp->target_root = ui->TargetPath->text().toStdString();
        pthread_create(&tid, NULL, listen_file_change, (void*)tmp);

        msgbox.information(this, "Success", "Backup done!");
    }
    else
        msgbox.critical(this, "Failure", "An unexpected error occurred!");
}

void MainWindow::recover_clicked()
{
    QMessageBox msgbox;
    bool succ_flag = build_generator(LocalGenerator(ui->TargetPath->text().toStdString(), ui->SourcePath->text().toStdString()), FileFilter(), Duplicator());
    if (succ_flag)
        msgbox.information(this, "Success", "Recover done!");
    else
        msgbox.critical(this, "Failure", "An unexpected error occurred!");
}
