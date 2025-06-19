#ifndef WEATHERFORM_H
#define WEATHERFORM_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class WeatherForm : public QWidget
{
    Q_OBJECT

public:
    explicit WeatherForm(QWidget *parent = nullptr);

private slots:
    void fetchWeatherData();
    void onWeatherDataReceived(QNetworkReply *reply);

private:
    QLabel *cityLabel;
    QLabel *temperatureLabel;
    QLabel *humidityLabel;
    QLabel *windSpeedLabel;
    QLineEdit *cityInput;
    QPushButton *fetchButton;
    QNetworkAccessManager *networkManager;
};

#endif // WEATHERFORM_H
