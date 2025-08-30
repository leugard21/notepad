#include "MainWindow.h"
#include "EditorWidget.h"
#include "Highlighter.h"

#include <QApplication>
#include <QCloseEvent>
#include <QFile>
#include <QFileDialog>
#include <QInputDialog>
#include <QMenuBar>
#include <QMessageBox>
#include <QSettings>
#include <QStatusBar>
#include <QStyle>
#include <QTabWidget>
#include <QTextBlock>
#include <QTextStream>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
  setWindowTitle("Notepad");
  resize(900, 600);

  m_tabs = new QTabWidget(this);
  m_tabs->setDocumentMode(true);
  m_tabs->setTabsClosable(true);
  setCentralWidget(m_tabs);

  connect(m_tabs, &QTabWidget::currentChanged, this,
          &MainWindow::currentTabChanged);
  connect(m_tabs, &QTabWidget::tabCloseRequested, this, [this](int idx) {
    m_tabs->setCurrentIndex(idx);
    closeCurrentTab();
  });

  createMenus();

  applyTheme(isDarkTheme());

  QSettings s;
  m_recentFiles = s.value("recentFiles").toStringList();
  rebuildRecentFilesMenu();

  newTab();
  statusBar()->showMessage("Ready");
  updateStatusBar();
}

static Highlighter::Lang langForPath(const QString &path) {
  const QString ext = QFileInfo(path).suffix().toLower();
  if (ext == "cpp" || ext == "cc" || ext == "cxx" || ext == "h" || ext == "hpp")
    return Highlighter::Lang::Cpp;
  if (ext == "json")
    return Highlighter::Lang::Json;
  if (ext == "md" || ext == "markdown")
    return Highlighter::Lang::Markdown;
  return Highlighter::Lang::None;
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

  m_recentMenu = fileMenu->addMenu("Recent Files");
  m_recentMenuAction = m_recentMenu->menuAction();

  fileMenu->addSeparator();
  fileMenu->addAction("Close Tab", this, &MainWindow::closeCurrentTab,
                      QKeySequence("Ctrl+W"));
  fileMenu->addAction("Exit", this, &QWidget::close);

  // Edit
  QMenu *editMenu = menuBar()->addMenu("&Edit");
  editMenu->addAction(
      "Undo",
      [this] {
        if (currentEditor())
          currentEditor()->undo();
      },
      QKeySequence::Undo);
  editMenu->addAction(
      "Redo",
      [this] {
        if (currentEditor())
          currentEditor()->redo();
      },
      QKeySequence::Redo);
  editMenu->addSeparator();
  editMenu->addAction(
      "Cut",
      [this] {
        if (currentEditor())
          currentEditor()->cut();
      },
      QKeySequence::Cut);
  editMenu->addAction(
      "Copy",
      [this] {
        if (currentEditor())
          currentEditor()->copy();
      },
      QKeySequence::Copy);
  editMenu->addAction(
      "Paste",
      [this] {
        if (currentEditor())
          currentEditor()->paste();
      },
      QKeySequence::Paste);
  editMenu->addSeparator();
  editMenu->addAction(
      "Select All",
      [this] {
        if (currentEditor())
          currentEditor()->selectAll();
      },
      QKeySequence::SelectAll);
  editMenu->addSeparator();
  editMenu->addAction("Go to Line...", this, &MainWindow::goToLine,
                      QKeySequence("Ctrl+G"));
  editMenu->addAction("Find...", this, &MainWindow::find, QKeySequence::Find);
  editMenu->addAction("Find Next", this, &MainWindow::findNext,
                      QKeySequence::FindNext);
  editMenu->addAction("Find Previous", this, &MainWindow::findPrev,
                      QKeySequence::FindPrevious);

  // View
  QMenu *viewMenu = menuBar()->addMenu("&View");
  auto *wrap = viewMenu->addAction("Toggle Word Wrap", [this] {
    auto ed = currentEditor();
    if (!ed)
      return;
    auto mode = ed->wordWrapMode() == QTextOption::NoWrap
                    ? QTextOption::WordWrap
                    : QTextOption::NoWrap;
    ed->setWordWrapMode(mode);
  });
  wrap->setCheckable(true);
  wrap->setChecked(false);

  viewMenu->addSeparator();
  auto *darkAct =
      viewMenu->addAction("Dark Theme", this, &MainWindow::toggleDarkTheme);
  darkAct->setCheckable(true);
  darkAct->setChecked(isDarkTheme());

  // Help
  QMenu *helpMenu = menuBar()->addMenu("&Help");
  helpMenu->addAction("About Notepad", [this] {
    QMessageBox::about(this, "About Notepad",
                       "Notepad\nA minimal Qt text editor.");
  });
}

EditorWidget *MainWindow::currentEditor() const {
  return qobject_cast<EditorWidget *>(m_tabs->currentWidget());
}

void MainWindow::setTabTitle(EditorWidget *ed) {
  const bool modified = ed->document()->isModified();
  QString name = ed->filePath().isEmpty()
                     ? "Untitled"
                     : QFileInfo(ed->filePath()).fileName();
  if (modified)
    name.prepend("*");
  int idx = m_tabs->indexOf(ed);
  if (idx >= 0)
    m_tabs->setTabText(idx, name);
  setWindowTitle(QString("NotepadX - %1").arg(name));
}

void MainWindow::newTab() {
  auto *ed = new EditorWidget(this);
  ed->setFilePath(QString());
  ed->document()->setModified(false);

  auto *hl = new Highlighter(ed->document());
  hl->setLanguage(Highlighter::Lang::None);

  connect(ed, &QPlainTextEdit::modificationChanged, this,
          &MainWindow::documentModified);
  connect(ed, &QPlainTextEdit::cursorPositionChanged, this,
          &MainWindow::cursorPositionChanged);

  int idx = m_tabs->addTab(ed, "Untitled");
  m_tabs->setCurrentIndex(idx);
  setTabTitle(ed);
}

void MainWindow::closeCurrentTab() {
  auto *ed = currentEditor();
  if (!ed)
    return;
  if (!maybeSave(ed))
    return;

  int idx = m_tabs->indexOf(ed);
  m_tabs->removeTab(idx);
  ed->deleteLater();

  if (m_tabs->count() == 0) {
    newTab();
  }
}

void MainWindow::currentTabChanged(int) {
  // Update title and status when switching
  auto *ed = currentEditor();
  if (!ed)
    return;
  setTabTitle(ed);
  updateStatusBar();
}

void MainWindow::newFile() { newTab(); }

void MainWindow::openFile() {
  QString path = QFileDialog::getOpenFileName(this, "Open File");
  if (path.isEmpty())
    return;

  auto *ed = currentEditor();
  if (!ed || !ed->toPlainText().isEmpty() || !ed->filePath().isEmpty()) {
    newTab();
    ed = currentEditor();
  }
  if (loadFromPath(ed, path)) {
    // recent list
    m_recentFiles.removeAll(path);
    m_recentFiles.prepend(path);
    while (m_recentFiles.size() > kMaxRecent)
      m_recentFiles.removeLast();
    rebuildRecentFilesMenu();
    QSettings s;
    s.setValue("recentFiles", m_recentFiles);
  }
}

bool MainWindow::saveFile() {
  auto *ed = currentEditor();
  if (!ed)
    return false;
  if (ed->filePath().isEmpty())
    return saveFileAs();
  return saveToPath(ed, ed->filePath());
}

bool MainWindow::saveFileAs() {
  auto *ed = currentEditor();
  if (!ed)
    return false;
  QString path = QFileDialog::getSaveFileName(
      this, "Save File As",
      ed->filePath().isEmpty() ? QString() : ed->filePath());
  if (path.isEmpty())
    return false;
  return saveToPath(ed, path);
}

bool MainWindow::saveToPath(EditorWidget *ed, const QString &path) {
  QFile file(path);
  if (!file.open(QFile::WriteOnly | QFile::Text)) {
    QMessageBox::warning(this, "Error", "Cannot save file.");
    return false;
  }
  QTextStream out(&file);
  out << ed->toPlainText();
  ed->document()->setModified(false);
  ed->setFilePath(path);
  setTabTitle(ed);
  statusBar()->showMessage("Saved", 2000);

  if (auto *hl = qobject_cast<Highlighter *>(
          ed->document()->documentLayout()->parent())) {
    // Not reliable; better: find child of document
  }

  for (QObject *child : ed->document()->children()) {
    if (auto *hl = qobject_cast<Highlighter *>(child)) {
      hl->setLanguage(langForPath(path));
      break;
    }
  }

  m_recentFiles.removeAll(path);
  m_recentFiles.prepend(path);
  while (m_recentFiles.size() > kMaxRecent)
    m_recentFiles.removeLast();
  rebuildRecentFilesMenu();
  QSettings s;
  s.setValue("recentFiles", m_recentFiles);

  return true;
}

bool MainWindow::loadFromPath(EditorWidget *ed, const QString &path) {
  QFile file(path);
  if (!file.open(QFile::ReadOnly | QFile::Text)) {
    QMessageBox::warning(this, "Error", "Cannot open file.");
    return false;
  }
  QTextStream in(&file);
  ed->setPlainText(in.readAll());
  ed->document()->setModified(false);
  ed->setFilePath(path);

  if (auto *hl = qobject_cast<Highlighter *>(
          ed->document()->documentLayout()->parent())) {
    // Not reliable; better: find child of document
  }

  for (QObject *child : ed->document()->children()) {
    if (auto *hl = qobject_cast<Highlighter *>(child)) {
      hl->setLanguage(langForPath(path));
      break;
    }
  }

  setTabTitle(ed);
  statusBar()->showMessage("Opened", 2000);
  return true;
}

bool MainWindow::maybeSave(EditorWidget *ed) {
  if (!ed || !ed->document()->isModified())
    return true;

  auto ret = QMessageBox::warning(
      this, "NotepadX",
      "The document has been modified.\nDo you want to save your changes?",
      QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

  if (ret == QMessageBox::Save)
    return saveToPath(ed, ed->filePath().isEmpty()
                              ? QFileDialog::getSaveFileName(this, "Save File")
                              : ed->filePath());
  if (ret == QMessageBox::Cancel)
    return false;
  return true;
}

void MainWindow::rebuildRecentFilesMenu() {
  m_recentMenu->clear();
  if (m_recentFiles.isEmpty()) {
    QAction *none = m_recentMenu->addAction("(empty)");
    none->setEnabled(false);
    return;
  }
  for (const QString &path : m_recentFiles) {
    QAction *a = m_recentMenu->addAction(path);
    connect(a, &QAction::triggered, this, &MainWindow::openRecentFile);
  }
}

void MainWindow::openRecentFile() {
  QAction *a = qobject_cast<QAction *>(sender());
  if (!a)
    return;
  const QString path = a->text();
  newTab();
  auto *ed = currentEditor();
  if (loadFromPath(ed, path)) {
    m_recentFiles.removeAll(path);
    m_recentFiles.prepend(path);
    while (m_recentFiles.size() > kMaxRecent)
      m_recentFiles.removeLast();
    rebuildRecentFilesMenu();
    QSettings s;
    s.setValue("recentFiles", m_recentFiles);
  }
}

void MainWindow::goToLine() {
  bool ok;
  int maxLine = currentEditor()->document()->blockCount();
  int line = QInputDialog::getInt(this, "Go to Line",
                                  QString("Line number (1-%1):").arg(maxLine),
                                  1, 1, maxLine, 1, &ok);
  if (ok) {
    QTextCursor cursor(
        currentEditor()->document()->findBlockByLineNumber(line - 1));
    currentEditor()->setTextCursor(cursor);
    currentEditor()->centerCursor();
  }
}

void MainWindow::find() {
  bool ok;
  QString term = QInputDialog::getText(
      this, "Find", "Text to find:", QLineEdit::Normal, m_lastSearch, &ok);

  if (ok && !term.isEmpty()) {
    m_lastSearch = term;
    findNext();
    highlightSearch(term);
  }
}

void MainWindow::findNext() {
  if (m_lastSearch.isEmpty())
    return;
  if (!currentEditor()->find(m_lastSearch)) {
    currentEditor()->moveCursor(QTextCursor::Start);
    currentEditor()->find(m_lastSearch);
  }
}

void MainWindow::findPrev() {
  if (m_lastSearch.isEmpty())
    return;
  if (!currentEditor()->find(m_lastSearch, QTextDocument::FindBackward)) {
    currentEditor()->moveCursor(QTextCursor::End);
    currentEditor()->find(m_lastSearch, QTextDocument::FindBackward);
  }
}

void MainWindow::highlightSearch(const QString &term) {
  QList<QTextEdit::ExtraSelection> extraSelections;
  if (!term.isEmpty()) {
    QTextCursor cursor(currentEditor()->document());
    QTextCharFormat fmt;
    fmt.setBackground(QBrush(Qt::blue).color());
    while (!cursor.isNull() && !cursor.atEnd()) {
      cursor = currentEditor()->document()->find(term, cursor);
      if (!cursor.isNull()) {
        QTextEdit::ExtraSelection sel;
        sel.cursor = cursor;
        sel.format = fmt;
        extraSelections.append(sel);
      }
    }
  }
  currentEditor()->setExtraSelections(extraSelections);
}

void MainWindow::closeEvent(QCloseEvent *event) {
  // Check all tabs
  for (int i = 0; i < m_tabs->count(); ++i) {
    auto *ed = qobject_cast<EditorWidget *>(m_tabs->widget(i));
    if (!maybeSave(ed)) {
      event->ignore();
      return;
    }
  }
  event->accept();
}

void MainWindow::documentModified() {
  auto *ed = qobject_cast<EditorWidget *>(sender());
  if (!ed)
    ed = currentEditor();
  m_dirty = ed && ed->document()->isModified();
  if (ed)
    setTabTitle(ed);
}

void MainWindow::cursorPositionChanged() { updateStatusBar(); }

void MainWindow::updateStatusBar() {
  auto *ed = currentEditor();
  if (!ed) {
    statusBar()->clearMessage();
    return;
  }
  auto cursor = ed->textCursor();
  int line = cursor.blockNumber() + 1;
  int col = cursor.columnNumber() + 1;
  statusBar()->showMessage(
      QString("Ln %1, Col %2 | UTF-8 | LF").arg(line).arg(col));
}

bool MainWindow::isDarkTheme() const {
  QSettings s;
  return s.value("ui/darkTheme", false).toBool();
}

void MainWindow::applyTheme(bool dark) {
  QPalette p;
  if (dark) {
    p = QPalette();
    p.setColor(QPalette::Window, QColor(30, 30, 30));
    p.setColor(QPalette::WindowText, Qt::white);
    p.setColor(QPalette::Base, QColor(23, 23, 23));
    p.setColor(QPalette::Text, Qt::white);
    p.setColor(QPalette::Highlight, QColor(38, 79, 120));
    p.setColor(QPalette::HighlightedText, Qt::white);
  } else {
    p = QApplication::style()->standardPalette();
  }
  QApplication::setPalette(p);

  // Adjust current editors to follow palette (fonts/wrap remain)
  for (int i = 0; i < m_tabs->count(); ++i) {
    if (auto *ed = qobject_cast<EditorWidget *>(m_tabs->widget(i))) {
      ed->setPalette(p);
    }
  }
}

void MainWindow::toggleDarkTheme(bool on) {
  applyTheme(on);
  QSettings s;
  s.setValue("ui/darkTheme", on);
}
