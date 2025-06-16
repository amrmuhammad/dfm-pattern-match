#include "patterncapture.h"
#include <QDebug>
#include <QLabel>
 
PatternCapture::PatternCapture(QWidget *parent) : QWidget(parent) {
    QVBoxLayout *layout = new QVBoxLayout(this);
    layoutFileEdit = new QLineEdit("/home/amrmuhammad/dev/dfm_pattern_match4/dfm-pattern-capture-enhanced5/build/spm.gds", this);
    maskLayerEdit = new QLineEdit("1", this);
    inputLayersEdit = new QLineEdit("2", this);
    dbNameEdit = new QLineEdit("test_db", this);
    runButton = new QPushButton("Run DFM Pattern Capture", this);
    logText = new QTextEdit(this);
    logText->setReadOnly(true);

    layout->addWidget(new QLabel("Layout File:"));
    layout->addWidget(layoutFileEdit);
    layout->addWidget(new QLabel("Mask Layer Number:"));
    layout->addWidget(maskLayerEdit);
    layout->addWidget(new QLabel("Input Layers Numbers:"));
    layout->addWidget(inputLayersEdit);
    layout->addWidget(new QLabel("Database Name:"));
    layout->addWidget(dbNameEdit);
    layout->addWidget(runButton);
    layout->addWidget(logText);

    process = new QProcess(this);
    connect(runButton, &QPushButton::clicked, this, &PatternCapture::runPatternCapture);
    connect(process, &QProcess::readyReadStandardOutput, this, &PatternCapture::readProcessOutput);
    connect(process, &QProcess::readyReadStandardError, this, &PatternCapture::readProcessOutput);
}

void PatternCapture::runPatternCapture() {
    logText->clear();
    QString program = "/home/amrmuhammad/dev/dfm_pattern_match4/dfm-pattern-capture-enhanced5/build/dfm_pattern_capture";
    QStringList arguments = {
        "-layout_file", layoutFileEdit->text(),
        "-mask_layer_number", maskLayerEdit->text(),
        "-input_layers_numbers", inputLayersEdit->text(),
        "-db_name", dbNameEdit->text()
    };
    process->start(program, arguments);
    if (!process->waitForStarted()) {
        logText->append("Error: Failed to start dfm_pattern_capture");
    }
}

void PatternCapture::readProcessOutput() {
    logText->append(process->readAllStandardOutput());
    logText->append(process->readAllStandardError());
}
