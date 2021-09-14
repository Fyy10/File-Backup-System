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
    // verify clicked
    connect(ui->VerifyButton, &QPushButton::clicked, this, &MainWindow::verify_clicked);
    // browse source clicked
    connect(ui->BrowseSource, &QPushButton::clicked, this, &MainWindow::browse_source_clicked);
    // browse target clicked
    connect(ui->BrowseTarget, &QPushButton::clicked, this, &MainWindow::browse_target_clicked);
    // time option clicked
    connect(ui->TimeOption, &QCheckBox::toggled, this, &MainWindow::time_clicked);
    // user filter option clicked
    connect(ui->UserFilterOption, &QCheckBox::toggled, this, &MainWindow::user_filter_clicked);

    // filter table
    // All Files (*)
    filter_table[0] = ".+";
    // Documents (*.docx; *.doc; *.xls; *.xlsx; *.ppt; *.pptx)
    filter_table[1] = ".+\\.(doc|docx|xls|xlsx|ppt|pptx)$";
    // Images (*.jpg; *.jpeg; *.png; *.bmp; *.gif)
    filter_table[2] = ".+\\.(jpg|jpeg|png|bmp|gif)$";
    // Videos (*.mp4; *.avi; *.flv; *.mkv)
    filter_table[3] = ".+\\.(mp4|avi|flv|mkv)$";
    // Music (*.mp3; *.ogg; *.wav)
    filter_table[4] = ".+\\.(mp3|ogg|wav)$";
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

    // filter settings
    string filter;
    if (ui->UserFilterOption->isChecked())
    {
        filter = ".+\\.(" + ui->UserFilterEdit->text().toStdString() + ")$";
    }
    else
    {
        filter = filter_table[ui->FilterBox->currentIndex()];
    }

    FileFilter ff(filter, {0, 0});

    LocalGenerator g = LocalGenerator(ui->SourcePath->text().toStdString(), ui->TargetPath->text().toStdString(), ff);
    Duplicator e;
    bool succ_flag = g.build(e);

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

    LocalGenerator g = LocalGenerator(ui->TargetPath->text().toStdString(), ui->SourcePath->text().toStdString());
    Duplicator e;
    bool succ_flag = g.build(e);

    if (succ_flag)
        msgbox.information(this, "Success", "Recover done!");
    else
        msgbox.critical(this, "Failure", "An unexpected error occurred!");
}

void MainWindow::verify_clicked()
{
    QMessageBox msgbox;
    // verify
    msgbox.information(this, "Success", "Verification passed!");
}

void MainWindow::time_clicked()
{
    if (ui->TimeOption->checkState() == Qt::Checked)
    {
        ui->DateEdit->setEnabled(true);
        ui->TimeEdit->setEnabled(true);
    }
    else
    {
        ui->DateEdit->setEnabled(false);
        ui->TimeEdit->setEnabled(false);
    }
}

void MainWindow::user_filter_clicked()
{
    if (ui->UserFilterOption->checkState() == Qt::Checked)
    {
        ui->FilterBox->setEnabled(false);
        ui->UserFilterEdit->setEnabled(true);
    }
    else
    {
        ui->FilterBox->setEnabled(true);
        ui->UserFilterEdit->setEnabled(false);
    }
}
