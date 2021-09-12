#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "core.hpp"
#include "extensions.hpp"

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

private:
    Ui::MainWindow *ui;

    bool build_generator(Generator && g, FileFilter && f, Export && e);
};
#endif // MAINWINDOW_H
