#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QMessageBox>
#include <QFileDialog>

#include <string>
#include <time.h>
#include "client.hpp"

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

    listening = false;
}

MainWindow::~MainWindow()
{
    delete ui;
    delete client;
}

int MainWindow::set_passwd(const string passwd)
{
    this->passwd = passwd;
    return 0;
}

int MainWindow::set_client(Client * client)
{
    this->client = client;
    return 0;
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

    // file filter
    string filter;
    if (ui->UserFilterOption->isChecked())
    {
        filter = ".+\\.(" + ui->UserFilterEdit->text().toStdString() + ")$";
    }
    else
    {
        filter = filter_table[ui->FilterBox->currentIndex()];
    }

    // time filter
    time_t time_filter;
    if (ui->DateEdit->isEnabled() && ui->TimeEdit->isEnabled())
    {
        struct tm date_time;
        // date
        // printf("%s\n", ui->DateEdit->text().toStdString().c_str());
        sscanf(ui->DateEdit->text().toStdString().c_str(), "%d/%d/%d", &date_time.tm_year, &date_time.tm_mon, &date_time.tm_mday);
        date_time.tm_year -= 1900;
        date_time.tm_mon -= 1;
        // time
        // printf("%s\n", ui->TimeEdit->text().toStdString().c_str());
        sscanf(ui->TimeEdit->text().toStdString().c_str(), "%d:%d", &date_time.tm_hour, &date_time.tm_min);
        date_time.tm_isdst = 0;
        time_filter = mktime(&date_time);
    }
    else
    {
        // no time filter
        time_filter = 0;
    }

    FileFilter ff(filter, {time_filter, 0});

    LocalGenerator g = LocalGenerator(ui->SourcePath->text().toStdString(), ui->TargetPath->text().toStdString(), passwd, ff);
    // Duplicator e;
    ExportEncodeEncrypt e;

    // remove previous backup files
    string source_root = ui->SourcePath->text().toStdString();
    string target_root = ui->TargetPath->text().toStdString();
    string target_path = target_root + (source_root.c_str() + source_root.rfind('/'));
    g.remove_dir(target_path);

    bool succ_flag = g.build(e);

    if (succ_flag)
    {
        // listen file change
        // cancel previous listen thread if possible
        if (listening)
        {
            pthread_cancel(listen_tid);
        }

        struct watch_roots* tmp = new struct watch_roots;
        tmp->source_root = ui->SourcePath->text().toStdString();
        tmp->target_root = ui->TargetPath->text().toStdString();
        tmp->passwd = passwd;
        tmp->ff = ff;
        tmp->client = client;
        pthread_create(&listen_tid, NULL, listen_file_change, (void*)tmp);

        // remove previous backup on the server
        client->remove_dir(target_path.c_str() + target_path.rfind('/') + 1);
        // upload to the server
        client->backup(target_path.c_str());

        msgbox.information(this, "Success", "Backup done!");
    }
    else
        msgbox.critical(this, "Failure", "An unexpected error occurred!");
}

void MainWindow::recover_clicked()
{
    QMessageBox msgbox;

    LocalGenerator g = LocalGenerator(ui->TargetPath->text().toStdString(), ui->SourcePath->text().toStdString(), passwd);
    // Duplicator e;
    ExportDecodeDecrypt e;
    bool succ_flag = g.build(e);

    if (succ_flag)
        msgbox.information(this, "Success", "Recover done!");
    else
        msgbox.critical(this, "Failure", "An unexpected error occurred!");
}

void MainWindow::verify_clicked()
{
    QMessageBox msgbox;

    // generate local cache
    // file filter
    string filter;
    if (ui->UserFilterOption->isChecked())
    {
        filter = ".+\\.(" + ui->UserFilterEdit->text().toStdString() + ")$";
    }
    else
    {
        filter = filter_table[ui->FilterBox->currentIndex()];
    }

    // time filter
    time_t time_filter;
    if (ui->DateEdit->isEnabled() && ui->TimeEdit->isEnabled())
    {
        struct tm date_time;
        // date
        // printf("%s\n", ui->DateEdit->text().toStdString().c_str());
        sscanf(ui->DateEdit->text().toStdString().c_str(), "%d/%d/%d", &date_time.tm_year, &date_time.tm_mon, &date_time.tm_mday);
        date_time.tm_year -= 1900;
        date_time.tm_mon -= 1;
        // time
        // printf("%s\n", ui->TimeEdit->text().toStdString().c_str());
        sscanf(ui->TimeEdit->text().toStdString().c_str(), "%d:%d", &date_time.tm_hour, &date_time.tm_min);
        date_time.tm_isdst = 0;
        time_filter = mktime(&date_time);
    }
    else
    {
        // no time filter
        time_filter = 0;
    }

    FileFilter ff(filter, {time_filter, 0});

    LocalGenerator g = LocalGenerator(ui->SourcePath->text().toStdString(), ui->TargetPath->text().toStdString(), passwd, ff);
    // Duplicator e;
    ExportEncodeEncrypt e;

    // remove previous backup files
    string source_root = ui->SourcePath->text().toStdString();
    string target_root = ui->TargetPath->text().toStdString();
    string target_path = target_root + (source_root.c_str() + source_root.rfind('/'));
    g.remove_dir(target_path);

    // local cache dir
    g.build(e);

    // verify
    bool verify_flag = client->check_file(target_path);
    if (verify_flag)
    {
        msgbox.information(this, "Success", "Verification passed!");
    }
    else
    {
        msgbox.critical(this, "Sorry", "Verification failed, please backup again");
    }
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
