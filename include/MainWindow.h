#pragma once
#include "EditorWidget.h"
#include <QMainWindow>
#include <QString>
#include <QStringList>
#include <QTabWidget>

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
  void openRecentFile();
  void closeCurrentTab();
  void newTab();
  void toggleDarkTheme(bool on);

  void documentModified();
  void cursorPositionChanged();
  void currentTabChanged(int index);

private:
  void createMenus();
  void rebuildRecentFilesMenu();
  void updateStatusBar();
  void highlightSearch(const QString &term);
  bool maybeSave(EditorWidget *ed);
  bool saveToPath(EditorWidget *ed, const QString &path);
  bool loadFromPath(EditorWidget *ed, const QString &path);

  EditorWidget *currentEditor() const;
  void setTabTitle(EditorWidget *ed);

  QTabWidget *m_tabs = nullptr;

  QStringList m_recentFiles;
  QAction *m_recentMenuAction = nullptr;
  QMenu *m_recentMenu = nullptr;

  const int kMaxRecent = 10;

  QString m_currentFile;
  QString m_lastSearch;

  bool m_dirty = false;

  void applyTheme(bool dark);
  bool isDarkTheme() const;
};
