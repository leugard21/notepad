#pragma once
#include <QMainWindow>
#include <QString>

class EditorWidget;

class MainWindow : public QMainWindow {
  Q_OBJECT
public:
  explicit MainWindow(QWidget *parent = nullptr);

protected:
  void closeEvent(QCloseEvent *event) override;

private slots:
  void newFile();
  void openFile();
  bool saveFile();
  bool saveFileAs();
  void goToLine();
  void find();
  void findNext();
  void findPrev();
  void documentModified();
  void cursorPositionChanged();

private:
  void createMenus();
  void updateStatusBar();
  void highlightSearch(const QString &term);
  bool maybeSave();
  bool saveToPath(const QString &path);
  bool loadFromPath(const QString &path);

  EditorWidget *m_editor = nullptr;
  QString m_currentFile;
  bool m_dirty = false;
  QString m_lastSearch;
};
