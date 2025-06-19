#ifndef REGISTERFORM_H
#define REGISTERFORM_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>

class RegisterForm : public QWidget
{
    Q_OBJECT

public:
    explicit RegisterForm(QWidget *parent = nullptr);

signals:
    void registerSuccess();  // Сигнал успешной регистрации

private slots:
    void registerUser();  // Слот для регистрации пользователя

private:
    QLineEdit *usernameEdit;
    QLineEdit *passwordEdit;
    QLineEdit *confirmPasswordEdit;
    QPushButton *registerButton;
    QLabel *errorLabel;
};

#endif // REGISTERFORM_H
