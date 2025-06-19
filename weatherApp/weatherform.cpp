#include "weatherform.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QNetworkReply>

WeatherForm::WeatherForm(QWidget *parent) : QWidget(parent)
{
    cityLabel = new QLabel("Enter city:", this);
    cityInput = new QLineEdit(this);
    temperatureLabel = new QLabel("Temperature: ", this);
    humidityLabel = new QLabel("Humidity: ", this);
    windSpeedLabel = new QLabel("Wind Speed: ", this);
    fetchButton = new QPushButton("Fetch Weather", this);

    networkManager = new QNetworkAccessManager(this);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(cityLabel);
    layout->addWidget(cityInput);
    layout->addWidget(fetchButton);
    layout->addWidget(temperatureLabel);
    layout->addWidget(humidityLabel);
    layout->addWidget(windSpeedLabel);
    setLayout(layout);

    connect(fetchButton, &QPushButton::clicked, this, &WeatherForm::fetchWeatherData);
    connect(networkManager, &QNetworkAccessManager::finished, this, &WeatherForm::onWeatherDataReceived);
}

void WeatherForm::fetchWeatherData()
{
    QString city = cityInput->text();
    if (!city.isEmpty()) {
        QUrl url("http://api.weatherapi.com/v1/current.json?key=YOUR_API_KEY&q=" + city);
        QNetworkRequest request(url);
        networkManager->get(request);
    }
}

void WeatherForm::onWeatherDataReceived(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll();
        QString temperature = "25°C";
        QString humidity = "65%";
        QString windSpeed = "15 km/h";
        
        temperatureLabel->setText("Temperature: " + temperature);
        humidityLabel->setText("Humidity: " + humidity);
        windSpeedLabel->setText("Wind Speed: " + windSpeed);
    } else {
        // Handle error
    }
    reply->deleteLater();
}
