#pragma once
#include <QRegularExpression>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>

class Highlighter : public QSyntaxHighlighter {
  Q_OBJECT
public:
  enum class Lang { None, Cpp, Json, Markdown };
  explicit Highlighter(QTextDocument *parent = nullptr);

  void setLanguage(Lang lang);

protected:
  void highlightBlock(const QString &text) override;

private:
  struct Rule {
    QRegularExpression pattern;
    QTextCharFormat format;
  };

  void buildRules();

  Lang m_lang = Lang::None;
  QVector<Rule> m_rules;

  QTextCharFormat m_keywordFmt, m_typeFmt, m_stringFmt, m_numberFmt,
      m_commentFmt, m_funcFmt, m_headerFmt, m_mdItalicFmt, m_mdBoldFmt,
      m_mdCodeFmt;
};
