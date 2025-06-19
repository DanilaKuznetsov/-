#include "loginform.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include "clientapi.h"

LoginForm::LoginForm(QWidget *parent) : QWidget(parent)
{
    usernameEdit = new QLineEdit(this);
    passwordEdit = new QLineEdit(this);
    loginButton = new QPushButton("Login", this);
    errorLabel = new QLabel(this);
    errorLabel->setStyleSheet("color: red;");

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(new QLabel("Username"));
    layout->addWidget(usernameEdit);
    layout->addWidget(new QLabel("Password"));
    layout->addWidget(passwordEdit);
    layout->addWidget(loginButton);
    layout->addWidget(errorLabel);
    setLayout(layout);

    connect(loginButton, &QPushButton::clicked, this, &LoginForm::authenticateUser);
}

void LoginForm::authenticateUser()
{
    QString username = usernameEdit->text();
    QString password = passwordEdit->text();

    if (username.isEmpty() || password.isEmpty()) {
        errorLabel->setText("Please enter both username and password.");
        return;
    }

    ClientAPI *clientApi = new ClientAPI();
    QByteArray response = clientApi->authenticateUser(username, password);

    if (response == "auth ok") {
        errorLabel->setText("Login successful!");
        emit loginSuccess();  // Отправляем сигнал успешной авторизации
    } else {
        errorLabel->setText("Invalid credentials. Please try again.");
    }
}
