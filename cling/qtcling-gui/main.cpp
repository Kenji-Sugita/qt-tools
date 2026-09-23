#include "cling/Interpreter/Interpreter.h"
#include "cling/MetaProcessor/MetaProcessor.h"

#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/raw_ostream.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>

#include <cstdlib>
#include <memory>
#include <string>
#include <vector>

extern "C" void cling_set_periodic_callback(void (*)(), int) {}
extern "C" void cling_clear_periodic_callback() {}

namespace {

std::string getenvOrDefault(const char* name, const char* fallback)
{
  const char* value = std::getenv(name);
  if (value && value[0] != '\0')
    return value;
  return fallback;
}

std::vector<std::string> splitWords(const std::string& text)
{
  std::vector<std::string> words;
  std::string current;
  for (char ch : text) {
    if (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r') {
      if (!current.empty()) {
        words.push_back(current);
        current.clear();
      }
      continue;
    }
    current.push_back(ch);
  }
  if (!current.empty())
    words.push_back(current);
  return words;
}

std::string qtFrameworkPath(const std::string& qtRoot)
{
  return qtRoot + "/lib";
}

std::string qtFrameworkName(const std::string& module)
{
  return "Qt" + module;
}

std::string qtFrameworkDirectory(const std::string& qtRoot,
                                 const std::string& module)
{
  return qtFrameworkPath(qtRoot) + "/" + qtFrameworkName(module) + ".framework";
}

std::string qtFrameworkHeaders(const std::string& qtRoot,
                               const std::string& module)
{
  return qtFrameworkDirectory(qtRoot, module) + "/Headers";
}

std::string qtFrameworkLibrary(const std::string& qtRoot,
                               const std::string& module)
{
  return qtFrameworkDirectory(qtRoot, module) + "/Versions/A/" +
         qtFrameworkName(module);
}

std::vector<std::string> makeInterpreterArguments(const char* programName)
{
  const std::string qtRoot =
      getenvOrDefault("QTCLING_QT_ROOT", QTCLING_DEFAULT_QT_ROOT);
  const std::string modulesText =
      getenvOrDefault("QTCLING_MODULES", "Core Gui Widgets");
  const std::string resourceDir =
      getenvOrDefault("QTCLING_RESOURCE_DIR", QTCLING_DEFAULT_RESOURCE_DIR);
  const std::vector<std::string> modules = splitWords(modulesText);

  std::vector<std::string> args;
  args.emplace_back(programName);
  args.emplace_back("-std=c++2c");
  args.emplace_back("-resource-dir");
  args.emplace_back(resourceDir);
  args.emplace_back("-F");
  args.emplace_back(qtFrameworkPath(qtRoot));

  for (const std::string& module : modules) {
    args.emplace_back("-I");
    args.emplace_back(qtFrameworkHeaders(qtRoot, module));
  }

  return args;
}

class QtClingWindow : public QMainWindow {
public:
  QtClingWindow(int argc, char** argv)
  {
    auto* central = new QWidget;
    auto* layout = new QVBoxLayout(central);

    output_ = new QPlainTextEdit;
    output_->setReadOnly(true);
    output_->setLineWrapMode(QPlainTextEdit::NoWrap);

    input_ = new QLineEdit;
    input_->setPlaceholderText("C++ / Cling input");

    auto* runButton = new QPushButton("Run");
    auto* inputRow = new QHBoxLayout;
    inputRow->addWidget(new QLabel("[cling]$"));
    inputRow->addWidget(input_, 1);
    inputRow->addWidget(runButton);

    layout->addWidget(output_, 1);
    layout->addLayout(inputRow);
    setCentralWidget(central);
    resize(900, 600);
    setWindowTitle("qtcling-gui prototype");

    initializeInterpreter(argc, argv);

    connect(input_, &QLineEdit::returnPressed, this, [this] { evaluateInput(); });
    connect(runButton, &QPushButton::clicked, this, [this] { evaluateInput(); });
  }

private:
  void initializeInterpreter(int argc, char** argv)
  {
    (void)argc;
    const std::string clingRoot =
        getenvOrDefault("QTCLING_CLING_ROOT", QTCLING_DEFAULT_CLING_ROOT);
    std::vector<std::string> arguments = makeInterpreterArguments(argv[0]);
    interpreterArgStorage_ = arguments;
    interpreterArgPointers_.clear();
    for (const std::string& argument : interpreterArgStorage_)
      interpreterArgPointers_.push_back(argument.c_str());

    interpreter_ = std::make_unique<cling::Interpreter>(
        static_cast<int>(interpreterArgPointers_.size()),
        interpreterArgPointers_.data(), clingRoot.c_str());

    if (!interpreter_->isValid()) {
      appendOutput("failed to create Cling interpreter");
      return;
    }

    const std::string qtRoot =
        getenvOrDefault("QTCLING_QT_ROOT", QTCLING_DEFAULT_QT_ROOT);
    const std::string modulesText =
        getenvOrDefault("QTCLING_MODULES", "Core Gui Widgets");

    for (const std::string& module : splitWords(modulesText))
      interpreter_->loadFile(qtFrameworkLibrary(qtRoot, module), true);

    interpreter_->AddIncludePath(".");
    outputStream_ = std::make_unique<llvm::raw_string_ostream>(outputBuffer_);
    metaProcessor_ =
        std::make_unique<cling::MetaProcessor>(*interpreter_, *outputStream_);

    processStartup();
    appendOutput("qtcling-gui ready");
  }

  void processStartup()
  {
    const char* startupFile = std::getenv("QTCLING_STARTUP_FILE");
    if (!startupFile || startupFile[0] == '\0')
      return;

    cling::Interpreter::CompilationResult result;
    metaProcessor_->process(std::string(".L ") + startupFile, result);
    appendCapturedOutput();
  }

  void evaluateInput()
  {
    const QString input = input_->text();
    if (input.trimmed().isEmpty())
      return;

    input_->clear();
    appendOutput("[cling]$ " + input);

    if (!metaProcessor_) {
      appendOutput("interpreter is not available");
      return;
    }

    cling::Interpreter::CompilationResult result;
    metaProcessor_->process(input.toStdString(), result);
    appendCapturedOutput();
  }

  void appendCapturedOutput()
  {
    outputStream_->flush();
    std::string output;
    output.swap(outputBuffer_);
    if (!output.empty())
      appendOutput(QString::fromStdString(output).trimmed());
  }

  void appendOutput(const QString& text) { output_->appendPlainText(text); }

  QPlainTextEdit* output_ = nullptr;
  QLineEdit* input_ = nullptr;
  std::string outputBuffer_;
  std::unique_ptr<llvm::raw_string_ostream> outputStream_;
  std::vector<std::string> interpreterArgStorage_;
  std::vector<const char*> interpreterArgPointers_;
  std::unique_ptr<cling::Interpreter> interpreter_;
  std::unique_ptr<cling::MetaProcessor> metaProcessor_;
};

} // namespace

int main(int argc, char** argv)
{
  llvm::llvm_shutdown_obj shutdownTrigger;
  QApplication application(argc, argv);
  application.setQuitOnLastWindowClosed(true);

  QtClingWindow window(argc, argv);
  window.show();

  return application.exec();
}
