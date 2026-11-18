#pragma once

#include <QJsonDocument>
#include <QMainWindow>

#include <yaml-cpp/yaml.h>

class QComboBox;
class QSplitter;
class QStatusBar;
class QTabWidget;
class QTextEdit;
class QToolBar;

namespace valijson {
    class Schema;
}

class Window : public QMainWindow
{
    Q_OBJECT

public:
    enum FormatMode {
        FormatJson = 0,
        FormatYaml = 1
    };

    explicit Window(QWidget * parent = 0);

public slots:
    void refreshJson();
    void onFormatChanged(int index);

    void showOpenDocumentDialog();
    void showOpenSchemaDialog();

private:
    QTextEdit * createEditor(bool readOnly);
    QSplitter * createSplitter(QWidget * left, QWidget * right, bool horizontal);
    QStatusBar * createStatusBar();
    QTabWidget * createTabWidget(QWidget * child, const QString & name);
    QToolBar * createToolBar();

    void validate();
    FormatMode currentFormat() const;
    QString openFileFilter() const;

    QTextEdit * m_documentEditor;
    QTextEdit * m_schemaEditor;

    QTextEdit * m_errors;
    QComboBox * m_formatCombo;

    QJsonDocument m_documentJson;
    QJsonDocument m_schemaJson;

    YAML::Node m_documentYaml;
    YAML::Node m_schemaYaml;

    valijson::Schema * m_schema;
};
