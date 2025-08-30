#include "EditorWidget.h"
#include <QTextOption>

EditorWidget::EditorWidget(QWidget *parent) : QPlainTextEdit(parent) {
  setWordWrapMode(QTextOption::NoWrap);
  setTabStopDistance(4 * fontMetrics().horizontalAdvance(' '));
}
