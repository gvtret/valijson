#include <sstream>
#include <stdexcept>

#include <QComboBox>
#include <QFile>
#include <QFileDialog>
#include <QLabel>
#include <QMenu>
#include <QSplitter>
#include <QStatusBar>
#include <QString>
#include <QTabWidget>
#include <QTextEdit>
#include <QToolBar>
#include <QToolButton>

#include <valijson/adapters/qtjson_adapter.hpp>
#include <valijson/adapters/yaml_cpp_adapter.hpp>
#include <valijson/schema.hpp>
#include <valijson/schema_parser.hpp>
#include <valijson/validation_results.hpp>
#include <valijson/validator.hpp>

#include "highlighter.h"
#include "window.h"

Window::Window(QWidget * parent)
  : QMainWindow(parent)
  , m_formatCombo(nullptr)
  , m_schema(nullptr)
{
    setWindowTitle("JSON / YAML Inspector");

    m_documentEditor = createEditor(false);
    m_schemaEditor = createEditor(false);
    m_errors = createEditor(true);

    auto documentTabWidget = createTabWidget(m_documentEditor, "Document");
    auto schemaTabWidget = createTabWidget(m_schemaEditor, "Schema");
    auto horizontalSplitter = createSplitter(schemaTabWidget, documentTabWidget, true);

    auto errorsTabWidget = createTabWidget(m_errors, "Errors");
    auto verticalSplitter = createSplitter(horizontalSplitter, errorsTabWidget, false);
    verticalSplitter->setStretchFactor(0, 2);
    verticalSplitter->setStretchFactor(1, 1);

    auto toolBar = createToolBar();
    auto statusBar = createStatusBar();

    addToolBar(toolBar);
    setCentralWidget(verticalSplitter);
    setStatusBar(statusBar);

    connect(m_documentEditor, SIGNAL(textChanged()), this, SLOT(refreshJson()));
    connect(m_schemaEditor, SIGNAL(textChanged()), this, SLOT(refreshJson()));

    refreshJson();
}

Window::FormatMode Window::currentFormat() const
{
    if (m_formatCombo && m_formatCombo->currentIndex() == FormatYaml) {
        return FormatYaml;
    }
    return FormatJson;
}

QString Window::openFileFilter() const
{
    if (currentFormat() == FormatYaml) {
        return QString("YAML/JSON (*.yaml *.yml *.json);;All Files (*)");
    }
    return QString("JSON (*.json);;All Files (*)");
}

QTextEdit * Window::createEditor(bool readOnly)
{
    QFont font;
    font.setFamily("Courier");
    font.setFixedPitch(true);
    font.setPointSize(12);

    auto editor = new QTextEdit();
    editor->setFont(font);
    editor->setReadOnly(readOnly);

    auto highlighter = new Highlighter(editor->document());
    Q_UNUSED(highlighter);

    return editor;
}

QSplitter * Window::createSplitter(QWidget * left, QWidget * right, bool horizontal)
{
    auto splitter = new QSplitter(horizontal ? Qt::Horizontal : Qt::Vertical);
    splitter->setChildrenCollapsible(false);
    splitter->insertWidget(0, left);
    splitter->insertWidget(1, right);

    return splitter;
}

QStatusBar * Window::createStatusBar()
{
    return new QStatusBar();
}

QTabWidget * Window::createTabWidget(QWidget * child, const QString & name)
{
    auto tabWidget = new QTabWidget();
    tabWidget->addTab(child, name);
    tabWidget->setDocumentMode(true);

    return tabWidget;
}

QToolBar * Window::createToolBar()
{
    auto toolbar = new QToolBar();
    toolbar->setMovable(false);

    auto openMenu = new QMenu("Open");
    auto openSchemaAction = openMenu->addAction("Open Schema...");
    auto openDocumentAction = openMenu->addAction("Open Document...");

    auto openButton = new QToolButton();
    openButton->setMenu(openMenu);
    openButton->setPopupMode(QToolButton::MenuButtonPopup);
    openButton->setText("Open");
    openButton->setToolButtonStyle(Qt::ToolButtonTextOnly);
    toolbar->addWidget(openButton);

    toolbar->addSeparator();
    toolbar->addWidget(new QLabel(" Format: "));

    m_formatCombo = new QComboBox();
    m_formatCombo->addItem("JSON (QtJsonAdapter)", FormatJson);
    m_formatCombo->addItem("YAML (YamlCppAdapter)", FormatYaml);
    toolbar->addWidget(m_formatCombo);

    connect(openButton, &QToolButton::clicked, openButton, &QToolButton::showMenu);
    connect(openDocumentAction, SIGNAL(triggered()), this, SLOT(showOpenDocumentDialog()));
    connect(openSchemaAction, SIGNAL(triggered()), this, SLOT(showOpenSchemaDialog()));
    connect(m_formatCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(onFormatChanged(int)));

    return toolbar;
}

void Window::onFormatChanged(int)
{
    refreshJson();
}

void Window::refreshJson()
{
    QString errors;
    m_errors->setText("");

    const auto schema = m_schemaEditor->toPlainText().toUtf8();
    const auto doc = m_documentEditor->toPlainText().toUtf8();
    const FormatMode format = currentFormat();

    if (schema.isEmpty()) {
        if (doc.isEmpty()) {
            if (format == FormatYaml) {
                m_errors->setText(
                  "Please provide a schema and a document to be validated.\n\n"
                  "YAML mode uses YamlCppAdapter. JSON documents are also valid YAML.");
            } else {
                m_errors->setText(
                  "Please provide a schema and a document to be validated.\n\n"
                  "JSON mode uses QtJsonAdapter, which does not consider non-array and "
                  "non-object values to be valid JSON documents.");
            }
            return;
        } else {
            errors += "Schema error: must not be empty\n\n";
        }
    } else if (format == FormatJson) {
        QJsonParseError error;
        m_schemaJson = QJsonDocument::fromJson(schema, &error);
        if (m_schemaJson.isNull()) {
            errors += QString("Schema error: ") + error.errorString() + "\n\n";
        }
    } else {
        try {
            m_schemaYaml = YAML::Load(schema.constData());
            if (!m_schemaYaml) {
                errors += "Schema error: empty or null YAML document\n\n";
            }
        } catch (const YAML::Exception & ex) {
            errors += QString("Schema error: ") + ex.what() + "\n\n";
        }
    }

    if (doc.isEmpty()) {
        if (!schema.isEmpty()) {
            errors += "Document error: must not be empty\n\n";
        }
    } else if (format == FormatJson) {
        QJsonParseError error;
        m_documentJson = QJsonDocument::fromJson(doc, &error);
        if (m_documentJson.isNull()) {
            errors += QString("Document error: ") + error.errorString() + "\n\n";
        }
    } else {
        try {
            m_documentYaml = YAML::Load(doc.constData());
            if (!m_documentYaml) {
                errors += "Document error: empty or null YAML document\n\n";
            }
        } catch (const YAML::Exception & ex) {
            errors += QString("Document error: ") + ex.what() + "\n\n";
        }
    }

    if (!errors.isEmpty()) {
      m_errors->setText(errors);
      return;
    }

    try {
        valijson::SchemaParser parser;
        delete m_schema;
        m_schema = new valijson::Schema();
        if (format == FormatJson) {
            valijson::adapters::QtJsonAdapter adapter(m_schemaJson.object());
            parser.populateSchema(adapter, *m_schema);
        } else {
            valijson::adapters::YamlCppAdapter adapter(m_schemaYaml);
            parser.populateSchema(adapter, *m_schema);
        }
        validate();
    } catch (std::runtime_error & error) {
        delete m_schema;
        m_schema = nullptr;
        m_errors->setText(QString("Schema error: ") + error.what());
    }
}

void Window::showOpenDocumentDialog()
{
    const QString fileName = QFileDialog::getOpenFileName(
        this, "Open Document", QString(), openFileFilter());
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (!file.open(QFile::ReadOnly | QFile::Text)) {
            m_errors->setText(QString("Document error: ") + file.errorString());
            return;
        }
        m_documentEditor->setText(file.readAll());
    }
}

void Window::showOpenSchemaDialog()
{
    const QString fileName = QFileDialog::getOpenFileName(
        this, "Open Schema", QString(), openFileFilter());
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (!file.open(QFile::ReadOnly | QFile::Text)) {
            m_errors->setText(QString("Schema error: ") + file.errorString());
            return;
        }
        m_schemaEditor->setText(file.readAll());
    }
}

void Window::validate()
{
    valijson::ValidationResults results;

    bool valid = false;
    if (currentFormat() == FormatJson) {
        // Qt JSON has real number/integer/string types.
        valijson::Validator validator(valijson::Validator::kStrongTypes);
        valijson::adapters::QtJsonAdapter adapter(m_documentJson.object());
        valid = validator.validate(*m_schema, adapter, &results);
    } else {
        // yaml-cpp adapter reports scalars as strings (hasStrictTypes() == false);
        // weak checking is required so integer/number schema types match YAML numbers.
        valijson::Validator validator(valijson::Validator::kWeakTypes);
        valijson::adapters::YamlCppAdapter adapter(m_documentYaml);
        valid = validator.validate(*m_schema, adapter, &results);
    }

    if (valid) {
        m_errors->setText("Document is valid.");
        return;
    }

    valijson::ValidationResults::Error error;
    unsigned int errorNum = 1;
    std::stringstream ss;
    while (results.popError(error)) {
        ss << "Validation error #" << errorNum << std::endl;
        ss << "  reference: #" << error.jsonPointer << std::endl;
        ss << "  description: " << error.description << std::endl << std::endl;
        ++errorNum;
    }

    m_errors->setText(QString::fromStdString(ss.str()));
}
