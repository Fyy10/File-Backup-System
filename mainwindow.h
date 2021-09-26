#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "core.hpp"
#include "extensions.hpp"
#include <map>
#include <string>

using namespace std;

class Client;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    int set_passwd(const string);
    int set_client(Client *);

private slots:
    void browse_source_clicked();
    void browse_target_clicked();
    void backup_clicked();
    void recover_clicked();
    void verify_clicked();

    // user settings
    void time_clicked();
    void user_filter_clicked();

private:
    Ui::MainWindow *ui;

    map<int, string> filter_table;
    string passwd;
    Client * client;
    bool listening;
    pthread_t listen_tid;
};
#endif // MAINWINDOW_H
