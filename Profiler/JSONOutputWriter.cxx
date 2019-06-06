#include "JSONOutputWriter.hxx"
#include "Document/Document.h"
#include <iterator>
#include <unordered_map>

using namespace OCRCorrection;

static std::wostream &writeString(std::wostream &out, const std::wstring &str,
                                  bool tolower = false) {
  if (not tolower) {
    out << '"' << str << '"';
    return out;
  }
  out << '"';
  std::transform(str.begin(), str.end(),
                 std::ostream_iterator<wchar_t, wchar_t>(out), ::towlower);
  out << '"';
  return out;
}

static std::wostream &writeKeyVal(std::wostream &out, const std::wstring &key,
                                  double val) {
  writeString(out, key);
  out << ": " << val;
  return out;
}

static std::wostream &writeKeyVal(std::wostream &out, const std::wstring &key,
                                  size_t val) {
  writeString(out, key);
  out << ": " << val;
  return out;
}

static std::wostream &writeKeyVal(std::wostream &out, const std::wstring &key,
                                  const std::wstring &val,
                                  bool tolower = false) {
  writeString(out, key);
  out << ": ";
  writeString(out, val, tolower);
  return out;
}

void JSONOutputWriter::write() const {
  std::unordered_map<std::wstring, std::pair<size_t, const Token *>> types;
  for (auto i = doc_.begin(); i != doc_.end(); ++i) {
    auto wocr = i->getWOCR();
    std::transform(wocr.begin(), wocr.end(), wocr.begin(), ::towlower);
    types[wocr].first++;
    types[wocr].second = i.operator->();
  }
  out_ << "{";
  wchar_t pre = L'\n';
  for (const auto &t : types) {
    if (t.second.second->isNormal()) {
      writeNormalToken(pre, *t.second.second, t.second.first);
      pre = L',';
    }
  }
  out_ << "}\n";
}

void JSONOutputWriter::writeNormalToken(wchar_t pre, const Token &token,
                                        size_t n) const {
  out_ << pre;
  writeString(out_, token.getWOCR(), true) << ": {\n";
  writeKeyVal(out_, L"OCR", token.getWOCR(), true) << ",\n";
  writeKeyVal(out_, L"N", n) << ",\n";
  pre = L'\n';
  writeString(out_, L"Candidates") << ": [";
  for (auto i = token.candidatesBegin(); i != token.candidatesEnd(); ++i) {
    writeCandidate(pre, *i);
    pre = L',';
  }
  out_ << "]\n}";
}

void JSONOutputWriter::writeCandidate(wchar_t pre,
                                      const Candidate &candidate) const {
  out_ << pre << "{\n";
  writeKeyVal(out_, L"Suggestion", candidate.getString(), true) << ",\n";
  writeKeyVal(out_, L"Modern", candidate.getBaseWord(), true) << ",\n";
  writeKeyVal(out_, L"Dict", candidate.getDictModule().getName()) << ",\n";
  writeString(out_, L"HistPatterns") << ": [";
  pre = L'\n';
  for (const auto &histp : candidate.getHistInstruction()) {
    writeInstruction(pre, histp);
    pre = L',';
  }
  out_ << "],\n";
  pre = L'\n';
  writeString(out_, L"OCRPatterns") << ": [";
  for (const auto &ocrp : candidate.getOCRTrace()) {
    writeInstruction(pre, ocrp);
    pre = L',';
  }
  out_ << "],\n";
  writeKeyVal(out_, L"Distance", candidate.getDlev()) << ",\n";
  writeKeyVal(out_, L"Weight", candidate.getVoteWeight()) << "\n";
  out_ << "}";
}

void JSONOutputWriter::writeInstruction(wchar_t pre,
                                        const csl::PosPattern &instr) const {
  out_ << pre << "{";
  writeKeyVal(out_, L"Left", instr.getLeft(), true) << ",";
  writeKeyVal(out_, L"Right", instr.getRight(), true) << ",";
  writeKeyVal(out_, L"Pos", instr.getPosition()) << "}\n";
}
