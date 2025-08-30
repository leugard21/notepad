#pragma once
#include <QPlainTextEdit>
#include <QString>

class EditorWidget : public QPlainTextEdit {
  Q_OBJECT
public:
  explicit EditorWidget(QWidget *parent = nullptr);

  void setFilePath(const QString &path) { m_filePath = path; }
  const QString &filePath() const { return m_filePath; }

private:
  QString m_filePath;
};
