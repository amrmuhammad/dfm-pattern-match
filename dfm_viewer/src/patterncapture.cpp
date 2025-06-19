#include "patterncapture.h"
#include "Logging.h"
#include <QDebug>
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>

PatternCapture::PatternCapture(QWidget *parent) : QWidget(parent) {
    LOG_FUNCTION();
    settings = new QSettings("MyOrg", "DFMPatternViewer", this);
    QVBoxLayout *layout = new QVBoxLayout(this);
    layoutFileEdit = new QLineEdit("/home/amrmuhammad/dev/dfm_pattern_match4/dfm-pattern-capture-enhanced5/build/spm.gds", this);
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

    process = new QProcess(this);
    connect(browseButton, &QPushButton::clicked, this, &PatternCapture::browseLayoutFile);
    connect(runButton, &QPushButton::clicked, this, &PatternCapture::runPatternCapture);
    connect(cancelButton, &QPushButton::clicked, this, &PatternCapture::cancelProcess);
    connect(process, &QProcess::readyReadStandardOutput, this, &PatternCapture::readProcessOutput);
    connect(process, &QProcess::readyReadStandardError, this, &PatternCapture::readProcessOutput);
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &PatternCapture::processFinished);

    loadSettings();
    LOG_INFO("PatternCapture initialized");
}

void PatternCapture::browseLayoutFile() {
    LOG_FUNCTION();
    QString fileName = QFileDialog::getOpenFileName(this, "Select Layout File", "", "GDSII/OASIS Files (*.gds *.oas)");
    if (!fileName.isEmpty()) {
        layoutFileEdit->setText(fileName);
        LOG_INFO("Selected layout file: " + fileName.toStdString());
    }
}

void PatternCapture::runPatternCapture() {
    LOG_FUNCTION();
    if (!validateInputs()) {
        logText->append("Error: Invalid inputs. Please check all fields.");
        LOG_WARN("Invalid inputs for pattern capture");
        return;
    }
    logText->clear();
    QString program = "/home/amrmuhammad/dev/dfm_pattern_match4/dfm-pattern-capture-enhanced5/build/dfm_pattern_capture";
    QStringList arguments = {
        "-layout_file", layoutFileEdit->text(),
        "-mask_layer_number", maskLayerEdit->text(),
        "-input_layers_numbers", inputLayersEdit->text(),
        "-db_name", dbNameEdit->text()
    };
    saveSettings();
    process->start(program, arguments);
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

void PatternCapture::cancelProcess() {
    LOG_FUNCTION();
    if (process->state() == QProcess::Running) {
        process->kill();
        logText->append("Pattern capture process cancelled.");
        LOG_INFO("Pattern capture process cancelled");
    }
    runButton->setEnabled(true);
    cancelButton->setEnabled(false);
}

void PatternCapture::readProcessOutput() {
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

void PatternCapture::processFinished(int exitCode, QProcess::ExitStatus exitStatus) {
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

bool PatternCapture::validateInputs() {
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

void PatternCapture::loadSettings() {
    LOG_FUNCTION();
    layoutFileEdit->setText(settings->value("layoutFile", "/home/amrmuhammad/dev/dfm_pattern_match4/dfm-pattern-capture-enhanced5/build/spm.gds").toString());
    maskLayerEdit->setText(settings->value("maskLayer", "1").toString());
    inputLayersEdit->setText(settings->value("inputLayers", "2").toString());
    dbNameEdit->setText(settings->value("dbName", "test_db").toString());
    LOG_DEBUG("Loaded PatternCapture settings");
}

void PatternCapture::saveSettings() {
    LOG_FUNCTION();
    settings->setValue("layoutFile", layoutFileEdit->text());
    settings->setValue("maskLayer", maskLayerEdit->text());
    settings->setValue("inputLayers", inputLayersEdit->text());
    settings->setValue("dbName", dbNameEdit->text());
    settings->sync();
    LOG_DEBUG("Saved PatternCapture settings");
}
