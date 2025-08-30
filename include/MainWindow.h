#pragma once
#include <QMainWindow>

class EditorWidget;

class MainWindow : public QMainWindow {
  Q_OBJECT
public:
  explicit MainWindow(QWidget *parent = nullptr);

private:
  void createMenus();
  EditorWidget *m_editor = nullptr;
};
