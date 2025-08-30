#include "MainWindow.h"
#include "EditorWidget.h"

#include <QCloseEvent>
#include <QFile>
#include <QFileDialog>
#include <QMenuBar>
#include <QMessageBox>
#include <QStatusBar>
#include <QTextStream>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
  setWindowTitle("Notepad");
  resize(900, 600);

  m_editor = new EditorWidget(this);
  setCentralWidget(m_editor);

  connect(m_editor, &QPlainTextEdit::modificationChanged, this,
          &MainWindow::documentModified);
  connect(m_editor, &QPlainTextEdit::cursorPositionChanged, this,
          &MainWindow::cursorPositionChanged);

  createMenus();
  statusBar()->showMessage("Ready");
  updateStatusBar();
}

void MainWindow::createMenus() {
  // File
  QMenu *fileMenu = menuBar()->addMenu("&File");
  fileMenu->addAction("New", this, &MainWindow::newFile, QKeySequence::New);
  fileMenu->addAction("Open...", this, &MainWindow::openFile,
                      QKeySequence::Open);
  fileMenu->addSeparator();
  fileMenu->addAction("Save", this, &MainWindow::saveFile, QKeySequence::Save);
  fileMenu->addAction("Save As...", this, &MainWindow::saveFileAs,
                      QKeySequence::SaveAs);
  fileMenu->addSeparator();
  fileMenu->addAction("Exit", this, &QWidget::close);

  // Edit
  QMenu *editMenu = menuBar()->addMenu("&Edit");
  editMenu->addAction("Undo", m_editor, &QPlainTextEdit::undo,
                      QKeySequence::Undo);
  editMenu->addAction("Redo", m_editor, &QPlainTextEdit::redo,
                      QKeySequence::Redo);
  editMenu->addSeparator();
  editMenu->addAction("Cut", m_editor, &QPlainTextEdit::cut, QKeySequence::Cut);
  editMenu->addAction("Copy", m_editor, &QPlainTextEdit::copy,
                      QKeySequence::Copy);
  editMenu->addAction("Paste", m_editor, &QPlainTextEdit::paste,
                      QKeySequence::Paste);
  editMenu->addSeparator();
  editMenu->addAction("Select All", m_editor, &QPlainTextEdit::selectAll,
                      QKeySequence::SelectAll);

  // View
  QMenu *viewMenu = menuBar()->addMenu("&View");
  viewMenu->addAction("Toggle Word Wrap", [this] {
    auto mode = m_editor->wordWrapMode() == QTextOption::NoWrap
                    ? QTextOption::WordWrap
                    : QTextOption::NoWrap;
    m_editor->setWordWrapMode(mode);
  });

  // Help
  QMenu *helpMenu = menuBar()->addMenu("&Help");
  helpMenu->addAction("About Notepad", [this] {
    QMessageBox::about(this, "About Notepad",
                       "Notepad\nA minimal Qt text editor.");
  });
}

void MainWindow::newFile() {
  if (!maybeSave())
    return;
  m_editor->clear();
  m_currentFile.clear();
  m_dirty = false;
  setWindowTitle("Notepad - Untitled");
}

void MainWindow::openFile() {
  if (!maybeSave())
    return;
  QString path = QFileDialog::getOpenFileName(this, "Open File");
  if (!path.isEmpty()) {
    loadFromPath(path);
  }
}

bool MainWindow::saveFile() {
  if (m_currentFile.isEmpty())
    return saveFileAs();
  return saveToPath(m_currentFile);
}

bool MainWindow::saveFileAs() {
  QString path = QFileDialog::getSaveFileName(this, "Save File As");
  if (path.isEmpty())
    return false;
  return saveToPath(path);
}

bool MainWindow::saveToPath(const QString &path) {
  QFile file(path);
  if (!file.open(QFile::WriteOnly | QFile::Text)) {
    QMessageBox::warning(this, "Error", "Cannot save file.");
    return false;
  }
  QTextStream out(&file);
  out << m_editor->toPlainText();
  m_currentFile = path;
  m_editor->document()->setModified(false);
  m_dirty = false;
  setWindowTitle(QString("Notepad - %1").arg(path));
  statusBar()->showMessage("Saved", 2000);
  return true;
}

bool MainWindow::loadFromPath(const QString &path) {
  QFile file(path);
  if (!file.open(QFile::ReadOnly | QFile::Text)) {
    QMessageBox::warning(this, "Error", "Cannot open file.");
    return false;
  }
  QTextStream in(&file);
  m_editor->setPlainText(in.readAll());
  m_currentFile = path;
  m_editor->document()->setModified(false);
  m_dirty = false;
  setWindowTitle(QString("Notepad - %1").arg(path));
  statusBar()->showMessage("Opened", 2000);
  return true;
}

bool MainWindow::maybeSave() {
  if (!m_editor->document()->isModified())
    return true;

  auto ret = QMessageBox::warning(
      this, "Notepad",
      "The document has been modified.\nDo you want to save your changes?",
      QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

  if (ret == QMessageBox::Save)
    return saveFile();
  if (ret == QMessageBox::Cancel)
    return false;
  return true;
}

void MainWindow::closeEvent(QCloseEvent *event) {
  if (maybeSave())
    event->accept();
  else
    event->ignore();
}

void MainWindow::documentModified() {
  m_dirty = m_editor->document()->isModified();
  setWindowTitle(
      QString("Notepad%1 - %2")
          .arg(m_dirty ? "*" : "")
          .arg(m_currentFile.isEmpty() ? "Untitled" : m_currentFile));
}

void MainWindow::cursorPositionChanged() { updateStatusBar(); }

void MainWindow::updateStatusBar() {
  auto cursor = m_editor->textCursor();
  int line = cursor.blockNumber() + 1;
  int col = cursor.columnNumber() + 1;
  statusBar()->showMessage(
      QString("Ln %1, Col %2 | UTF-8 | LF").arg(line).arg(col));
}
