#ifndef LOGGER_H
#define LOGGER_H

class Logger {
public:
  Logger(TString name="") :
    _isatty(isatty(fileno(stdout))),
    _fhandle(_isatty ? stdout : stderr),
    _name(name),
    _info("INFO", "32m", _fhandle),
    _debug("DEBUG", "36m", _fhandle),
    _warning("WARNING", "91m", stderr),
    _error("ERROR", "41m\033[1;37m", stderr) { }

  void info(const char *title, const char *msg, const char *n="\n") {
    _report(_info, title, msg, n);
  }
  void debug(const char *title, const char *msg, const char *n="\n") {
    _report(_debug, title, msg, n);
  }
  void warning(const char *title, const char *msg, const char *n="\n") {
    _report(_warning, title, msg, n);
  }
  void error(const char *title, const char *msg, const char *n="\n") {
    _report(_error, title, msg, n);
  }

private:
  struct RType {
    RType(TString n, TString c, File& f) : name(n), color(c), fhandle(f) {
      pad = "";
      for (int i = n.Length(); i != 9; ++i)
        pad += " ";
    }
    TString name;
    TString color;
    TString pad;
    File& fhandle;
  };
  void _report(const RType& r, const char *title, const char *msg, const char *n) {
    if (_isatty) {
      fprintf(
        r.fhandle,
        Form("\033[0;%s%s\0330m[%s [%-30s]: %s%s",
             r.color, _name+r.name, r.pad, title, msg, n);
      )
    } else {
      fprintf(
        r.fhandle,
        Form("%s%s [%-30s]: %s%s",
             _name+r.name, r.pad, title, msg, n);
      )
    }
  }
  bool _isatty;
  File _fhandle;
  TString _name;
  RType _name, _info, _debug, _warning, _error;
};

#endif
