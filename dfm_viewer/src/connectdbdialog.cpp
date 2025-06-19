#include "connectdbdialog.h"
#include "Logging.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

ConnectDbDialog::ConnectDbDialog(QWidget *parent, DatabaseViewer *viewer)
    : QDialog(parent), dbViewer(viewer) {
    LOG_FUNCTION();
    setWindowTitle("Connect to Database");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Database name input
    QHBoxLayout *dbLayout = new QHBoxLayout();
    dbLayout->addWidget(new QLabel("Enter DB name: "));
    dbNameEdit = new QLineEdit("Enter database name here", this);
    dbLayout->addWidget(dbNameEdit);
    mainLayout->addLayout(dbLayout);

    // Error label
    errorLabel = new QLabel("", this);
    errorLabel->setStyleSheet("color: red;");
    mainLayout->addWidget(errorLabel);

    // Connect button
    connectButton = new QPushButton("Connect", this);
    mainLayout->addWidget(connectButton);

    connect(connectButton, &QPushButton::clicked, this, &ConnectDbDialog::tryConnect);

    setFixedSize(400, 200);
    LOG_INFO("ConnectDbDialog initialized");
}

void ConnectDbDialog::tryConnect() {
    LOG_FUNCTION();
    QString dbName = dbNameEdit->text().trimmed();
    if (dbName.isEmpty() || dbName == "Enter database name here") {
        errorLabel->setText("Error: Please enter a valid database name.");
        LOG_WARN("Invalid database name provided");
        return;
    }

    if (dbViewer->connectToDatabase(dbName)) {
        accept();
    } else {
        errorLabel->setText("Error: Invalid database or schema.");
        LOG_ERROR("Failed to connect to database: " + dbName.toStdString());
    }
}
