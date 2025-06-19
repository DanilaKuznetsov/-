#include "registerform.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include "clientapi.h"

RegisterForm::RegisterForm(QWidget *parent) : QWidget(parent)
{
    usernameEdit = new QLineEdit(this);
    passwordEdit = new QLineEdit(this);
    confirmPasswordEdit = new QLineEdit(this);
    registerButton = new QPushButton("Register", this);
    errorLabel = new QLabel(this);
    errorLabel->setStyleSheet("color: red;");

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(new QLabel("Username"));
    layout->addWidget(usernameEdit);
    layout->addWidget(new QLabel("Password"));
    layout->addWidget(passwordEdit);
    layout->addWidget(new QLabel("Confirm Password"));
    layout->addWidget(confirmPasswordEdit);
    layout->addWidget(registerButton);
    layout->addWidget(errorLabel);
    setLayout(layout);

    connect(registerButton, &QPushButton::clicked, this, &RegisterForm::registerUser);
}

void RegisterForm::registerUser()
{
    QString username = usernameEdit->text();
    QString password = passwordEdit->text();
    QString confirmPassword = confirmPasswordEdit->text();

    if (username.isEmpty() || password.isEmpty() || confirmPassword.isEmpty()) {
        errorLabel->setText("Please fill in all fields.");
        return;
    }

    if (password != confirmPassword) {
        errorLabel->setText("Passwords do not match.");
        return;
    }

    ClientAPI *clientApi = new ClientAPI();
    QByteArray response = clientApi->registerUser(username, password);

    if (response == "reg ok") {
        errorLabel->setText("Registration successful!");
        emit registerSuccess();  // Отправляем сигнал успешной регистрации
    } else {
        errorLabel->setText("Registration failed. Please try again.");
    }
}
