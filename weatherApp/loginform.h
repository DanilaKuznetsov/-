#ifndef LOGINFORM_H
#define LOGINFORM_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>

class LoginForm : public QWidget
{
    Q_OBJECT

public:
    explicit LoginForm(QWidget *parent = nullptr);

signals:
    void loginSuccess();  // Сигнал успешной авторизации

private slots:
    void authenticateUser();  // Слот для авторизации

private:
    QLineEdit *usernameEdit;
    QLineEdit *passwordEdit;
    QPushButton *loginButton;
    QLabel *errorLabel;
};

#endif // LOGINFORM_H
