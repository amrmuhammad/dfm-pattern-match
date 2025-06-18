#include "batchpatterncapture.h"
#include "Logging.h"
#include <QDebug>
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>

BatchPatternCapture::BatchPatternCapture(QWidget *parent) : QMainWindow(parent) {
    LOG_FUNCTION();
    settings = new QSettings("MyOrg", "DFMPatternViewer", this);
    centralWidget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(centralWidget);
    layoutFileEdit = new QLineEdit(this);
    browseButton = new QPushButton("Browse", this);
    maskLayerEdit = new QLineEdit("1", this);
    inputLayersEdit = new QLineEdit("2", this);
    dbNameEdit = new QLineEdit("test_db", this);
    runButton = new QPushButton("Run DFM Pattern Capture", this);
    cancelButton = new QPushButton("Cancel", this);
    logText = new QTextEdit(this);
    logText->setReadOnly(true);

    QHBoxLayout *fileLayout = new QHBoxLayout();
    fileLayout->addWidget(new QLabel("Layout File:"));
    fileLayout->addWidget(layoutFileEdit);
    fileLayout->addWidget(browseButton);

    layout->addLayout(fileLayout);
    layout->addWidget(new QLabel("Mask Layer Number:"));
    layout->addWidget(maskLayerEdit);
    layout->addWidget(new QLabel("Input Layers Numbers:"));
    layout->addWidget(inputLayersEdit);
    layout->addWidget(new QLabel("Database Name:"));
    layout->addWidget(dbNameEdit);
    layout->addWidget(runButton);
    layout->addWidget(cancelButton);
    layout->addWidget(logText);

    setCentralWidget(centralWidget);
    process = new QProcess(this);
    connect(browseButton, &QPushButton::clicked, this, &BatchPatternCapture::browseLayoutFile);
    connect(runButton, &QPushButton::clicked, this, &BatchPatternCapture::runPatternCapture);
    connect(cancelButton, &QPushButton::clicked, this, &BatchPatternCapture::cancelProcess);
    connect(process, &QProcess::readyReadStandardOutput, this, &BatchPatternCapture::readProcessOutput);
    connect(process, &QProcess::readyReadStandardError, this, &BatchPatternCapture::readProcessOutput);
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &BatchPatternCapture::processFinished);

    loadSettings();
    LOG_INFO("BatchPatternCapture initialized");
}

void BatchPatternCapture::browseLayoutFile() {
    LOG_FUNCTION();
    QString fileName = QFileDialog::getOpenFileName(this, "Select Layout File", "", "GDSII/OASIS Files (*.gds *.oas)");
    if (!fileName.isEmpty()) {
        layoutFileEdit->setText(fileName);
        LOG_INFO("Selected layout file: " + fileName.toStdString());
    }
}

void BatchPatternCapture::runPatternCapture() {
    LOG_FUNCTION();
    if (!validateInputs()) {
        logText->append("Error: Invalid inputs. Please check all fields.");
        LOG_WARN("Invalid inputs for pattern capture");
        return;
    }
    if (programPath.isEmpty()) {
        programPath = QFileDialog::getOpenFileName(this, "Select dfm_pattern_capture executable", "", "Executable Files (*)");
        if (programPath.isEmpty()) return;
    }
    logText->clear();
    QStringList arguments = {
        "-layout_file", layoutFileEdit->text(),
        "-mask_layer_number", maskLayerEdit->text(),
        "-input_layers_numbers", inputLayersEdit->text(),
        "-db_name", dbNameEdit->text()
    };
    saveSettings();
    process->start(programPath, arguments);
    if (!process->waitForStarted()) {
        logText->append("Error: Failed to start dfm_pattern_capture");
        LOG_ERROR("Failed to start dfm_pattern_capture");
    } else {
        runButton->setEnabled(false);
        cancelButton->setEnabled(true);
        logText->append("Started pattern capture process...");
        LOG_INFO("Started dfm_pattern_capture with args: " + arguments.join(" ").toStdString());
    }
}

void BatchPatternCapture::cancelProcess() {
    LOG_FUNCTION();
    if (process->state() == QProcess::Running) {
        process->kill();
        logText->append("Pattern capture process cancelled.");
        LOG_INFO("Pattern capture process cancelled");
    }
    runButton->setEnabled(true);
    cancelButton->setEnabled(false);
}

void BatchPatternCapture::readProcessOutput() {
    LOG_FUNCTION();
    QString output = QString::fromUtf8(process->readAllStandardOutput());
    QString error = QString::fromUtf8(process->readAllStandardError());
    if (!output.isEmpty()) {
        logText->append(output);
        LOG_DEBUG("Process output: " + output.toStdString());
    }
    if (!error.isEmpty()) {
        logText->append(error);
        LOG_ERROR("Process error: " + error.toStdString());
    }
}

void BatchPatternCapture::processFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    LOG_FUNCTION();
    runButton->setEnabled(true);
    cancelButton->setEnabled(false);
    if (exitStatus == QProcess::NormalExit && exitCode == 0) {
        logText->append("Pattern capture completed successfully.");
        LOG_INFO("Pattern capture completed with exit code 0");
    } else {
        logText->append(QString("Pattern capture failed with exit code %1.").arg(exitCode));
        LOG_ERROR("Pattern capture failed with exit code " + std::to_string(exitCode));
    }
}

bool BatchPatternCapture::validateInputs() {
    LOG_FUNCTION();
    if (layoutFileEdit->text().isEmpty() ||
        maskLayerEdit->text().isEmpty() ||
        inputLayersEdit->text().isEmpty() ||
        dbNameEdit->text().isEmpty()) {
        return false;
    }
    bool ok;
    maskLayerEdit->text().toInt(&ok);
    if (!ok) return false;
    QStringList layers = inputLayersEdit->text().split(",");
    for (const QString &layer : layers) {
        layer.trimmed().toInt(&ok);
        if (!ok) return false;
    }
    QFileInfo file(layoutFileEdit->text());
    if (!file.exists()) {
        return false;
    }
    return true;
}

void BatchPatternCapture::loadSettings() {
    LOG_FUNCTION();
    layoutFileEdit->setText(settings->value("layoutFile", "").toString());
    maskLayerEdit->setText(settings->value("maskLayer", "1").toString());
    inputLayersEdit->setText(settings->value("inputLayers", "2").toString());
    dbNameEdit->setText(settings->value("dbName", "test_db").toString());
    programPath = settings->value("programPath", "").toString();
    LOG_DEBUG("Loaded BatchPatternCapture settings");
}

void BatchPatternCapture::saveSettings() {
    LOG_FUNCTION();
    settings->setValue("layoutFile", layoutFileEdit->text());
    settings->setValue("maskLayer", maskLayerEdit->text());
    settings->setValue("inputLayers", inputLayersEdit->text());
    settings->setValue("dbName", dbNameEdit->text());
    settings->setValue("programPath", programPath);
    settings->sync();
    LOG_DEBUG("Saved BatchPatternCapture settings");
}
