#pragma once
#include <QPlainTextEdit>

class EditorWidget : public QPlainTextEdit {
  Q_OBJECT
public:
  explicit EditorWidget(QWidget *parent = nullptr);
};
