#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void browse_source_clicked();
    void browse_target_clicked();
    void backup_clicked();
    void recover_clicked();
    void pack_clicked();
    void auto_clicked();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
