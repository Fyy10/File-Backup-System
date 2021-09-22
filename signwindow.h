#ifndef SIGNWINDOW_H
#define SIGNWINDOW_H

#include <QWidget>
#include <QString>

#define MIN_PASSWD_LEN 6
#define MAX_PASSWD_LEN 32

namespace Ui {
class SignWindow;
}

class SignWindow : public QWidget
{
    Q_OBJECT

public:
    explicit SignWindow(QWidget *parent = nullptr);
    ~SignWindow();

private slots:
    void signup_clicked();
    void exit_clicked();

private:
    Ui::SignWindow *ui;
};

#endif // SIGNWINDOW_H
