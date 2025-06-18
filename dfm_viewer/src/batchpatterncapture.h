#ifndef BATCHPATTERNCAPTURE_H
#define BATCHPATTERNCAPTURE_H

#include <QMainWindow>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QProcess>
#include <QSettings>

class BatchPatternCapture : public QMainWindow {
    Q_OBJECT
public:
    BatchPatternCapture(QWidget *parent = nullptr);

private slots:
    void browseLayoutFile();
    void runPatternCapture();
    void cancelProcess();
    void readProcessOutput();
    void processFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    bool validateInputs();
    void loadSettings();
    void saveSettings();

    QLineEdit *layoutFileEdit;
    QLineEdit *maskLayerEdit;
    QLineEdit *inputLayersEdit;
    QLineEdit *dbNameEdit;
    QPushButton *runButton;
    QPushButton *cancelButton;
    QPushButton *browseButton;
    QTextEdit *logText;
    QProcess *process;
    QSettings *settings;
    QString programPath;
    QWidget *centralWidget;
};

#endif
