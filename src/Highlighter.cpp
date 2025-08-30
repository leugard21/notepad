#include "Highlighter.h"
#include <QColor>

Highlighter::Highlighter(QTextDocument *parent) : QSyntaxHighlighter(parent) {
  buildRules();
}

void Highlighter::setLanguage(Lang lang) {
  if (m_lang == lang)
    return;
  m_lang = lang;
  rehighlight();
}

void Highlighter::buildRules() {
  // Common formats (colors adjust with theme palette; we just pick roles)
  m_keywordFmt.setForeground(QColor(Qt::blue));
  m_typeFmt.setForeground(QColor(0, 120, 170));
  m_stringFmt.setForeground(QColor(0, 130, 0));
  m_numberFmt.setForeground(QColor(160, 40, 0));
  m_commentFmt.setForeground(QColor(120, 120, 120));
  m_funcFmt.setForeground(QColor(150, 0, 150));
  m_headerFmt.setForeground(QColor(150, 0, 0));
  m_mdItalicFmt.setFontItalic(true);
  m_mdBoldFmt.setFontWeight(QFont::Bold);
  m_mdCodeFmt.setForeground(QColor(120, 80, 0));
}

void Highlighter::highlightBlock(const QString &text) {
  m_rules.clear();

  if (m_lang == Lang::Cpp) {
    static const char *kw[] = {"alignas",
                               "alignof",
                               "and",
                               "and_eq",
                               "asm",
                               "auto",
                               "bitand",
                               "bitor",
                               "break",
                               "case",
                               "catch",
                               "class",
                               "compl",
                               "const",
                               "constexpr",
                               "const_cast",
                               "continue",
                               "decltype",
                               "default",
                               "delete",
                               "do",
                               "dynamic_cast",
                               "else",
                               "enum",
                               "explicit",
                               "export",
                               "extern",
                               "false",
                               "final",
                               "for",
                               "friend",
                               "goto",
                               "if",
                               "inline",
                               "mutable",
                               "namespace",
                               "new",
                               "noexcept",
                               "not",
                               "not_eq",
                               "nullptr",
                               "operator",
                               "or",
                               "or_eq",
                               "override",
                               "private",
                               "protected",
                               "public",
                               "register",
                               "reinterpret_cast",
                               "return",
                               "signed",
                               "sizeof",
                               "static",
                               "static_assert",
                               "static_cast",
                               "struct",
                               "switch",
                               "template",
                               "this",
                               "thread_local",
                               "throw",
                               "true",
                               "try",
                               "typedef",
                               "typeid",
                               "typename",
                               "union",
                               "unsigned",
                               "using",
                               "virtual",
                               "void",
                               "volatile",
                               "while",
                               "xor",
                               "xor_eq"};
    QStringList kwList;
    for (auto word : kw)
      kwList << word;
    QString joined = QString("(?:%1)\\b").arg(kwList.join('|'));

    m_rules.push_back({QRegularExpression(joined), m_keywordFmt});
    m_rules.push_back({QRegularExpression("\\b(?:int|long|short|char|float|"
                                          "double|bool|size_t|std::\\w+)\\b"),
                       m_typeFmt});
    m_rules.push_back({QRegularExpression(R"("([^"\\]|\\.)*")"), m_stringFmt});
    m_rules.push_back({QRegularExpression(R"('(?:\\.|[^\\'])')"), m_stringFmt});
    m_rules.push_back(
        {QRegularExpression("\\b\\d+(?:\\.\\d+)?\\b"), m_numberFmt});
    m_rules.push_back({QRegularExpression("//.*$"), m_commentFmt});
    m_rules.push_back(
        {QRegularExpression("\\b([A-Za-z_][A-Za-z0-9_]*)\\s*(?=\\()"),
         m_funcFmt});

    for (const auto &r : m_rules) {
      auto it = r.pattern.globalMatch(text);
      while (it.hasNext()) {
        auto m = it.next();
        setFormat(m.capturedStart(), m.capturedLength(), r.format);
      }
    }
    // Multi-line /* */ comments
    static QRegularExpression startExp("/\\*");
    static QRegularExpression endExp("\\*/");
    int start = 0;
    if (previousBlockState() != 1)
      start = text.indexOf(startExp);
    else
      start = 0;

    while (start >= 0) {
      int end = text.indexOf(endExp, start);
      int len = (end < 0) ? (text.length() - start) : (end - start + 2);
      setFormat(start, len, m_commentFmt);
      if (end < 0) {
        setCurrentBlockState(1);
        break;
      } else {
        setCurrentBlockState(0);
      }
      start = text.indexOf(startExp, start + len);
    }
    if (start < 0 && previousBlockState() == 1)
      setCurrentBlockState(1);

  } else if (m_lang == Lang::Json) {
    m_rules.push_back(
        {QRegularExpression(R"("([^"\\]|\\.)*"\s*:)"), m_headerFmt}); // keys
    m_rules.push_back({QRegularExpression(R"("([^"\\]|\\.)*")"), m_stringFmt});
    m_rules.push_back(
        {QRegularExpression("\\b\\d+(?:\\.\\d+)?\\b"), m_numberFmt});
    m_rules.push_back(
        {QRegularExpression("\\b(true|false|null)\\b"), m_keywordFmt});

    for (const auto &r : m_rules) {
      auto it = r.pattern.globalMatch(text);
      while (it.hasNext()) {
        auto m = it.next();
        int start = m.capturedStart();
        int len = m.capturedLength();
        setFormat(start, len, r.format);
      }
    }

  } else if (m_lang == Lang::Markdown) {
    m_rules.push_back({QRegularExpression(R"(^\s{0,3}(#{1,6})\s.+)"),
                       m_headerFmt}); // headers
    m_rules.push_back({QRegularExpression(R"(\*\*[^*]+\*\*)"), m_mdBoldFmt});
    m_rules.push_back({QRegularExpression(R"(_[^_]+_)"), m_mdItalicFmt});
    m_rules.push_back({QRegularExpression(R"(`[^`]+`)"), m_mdCodeFmt});

    for (const auto &r : m_rules) {
      auto it = r.pattern.globalMatch(text);
      while (it.hasNext()) {
        auto m = it.next();
        setFormat(m.capturedStart(), m.capturedLength(), r.format);
      }
    }
  } else {
    // Lang::None -> no highlighting
  }
}
