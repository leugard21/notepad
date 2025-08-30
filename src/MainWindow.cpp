#include "MainWindow.h"
#include "EditorWidget.h"

#include <QMenuBar>
#include <QStatusBar>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
  setWindowTitle("Notepad");
  resize(900, 600);

  m_editor = new EditorWidget(this);
  setCentralWidget(m_editor);

  createMenus();
  statusBar()->showMessage("Ready");
}

void MainWindow::createMenus() {
  // File
  QMenu *fileMenu = menuBar()->addMenu("&File");
  fileMenu->addAction("New");
  fileMenu->addAction("Open...");
  fileMenu->addSeparator();
  fileMenu->addAction("Save");
  fileMenu->addAction("Save As...");
  fileMenu->addSeparator();
  fileMenu->addAction("Exit");

  // Edit
  QMenu *editMenu = menuBar()->addMenu("&Edit");
  editMenu->addAction("Undo");
  editMenu->addAction("Redo");
  editMenu->addSeparator();
  editMenu->addAction("Cut");
  editMenu->addAction("Copy");
  editMenu->addAction("Paste");
  editMenu->addSeparator();
  editMenu->addAction("Select All");

  // View
  QMenu *viewMenu = menuBar()->addMenu("&View");
  viewMenu->addAction("Toggle Word Wrap");

  // Help
  QMenu *helpMenu = menuBar()->addMenu("&Help");
  helpMenu->addAction("About Notepad");
}
