/** @copyright 2025 Sean Kasun */

#include "json.h"
#include <algorithm>
#include <memory>
#include <string>

enum Token {
  TokenNULL,
  TokenTRUE,
  TokenFALSE,
  TokenString,
  TokenNumber,
  TokenObject,
  TokenArray,
  TokenObjectClose,
  TokenArrayClose,
  TokenKeySeparator,
  TokenValueSeparator,
};

class JSONHelper {
  public:
    JSONHelper(const std::string &data) : data(data) {
      pos = 0;
      len = data.length();
    }
    Token nextToken() {
      // eat leading spaces
      while (pos < len && isspace(data.at(pos))) {
        pos++;
      }

      if (pos == len) {
        throw JSONParseException("Unexpected EOF", location());
      }

      char c = data.at(pos++);
      if (isalpha(c)) {  // must be a keyword like null/true/false
        int start = pos - 1;
        // find end of keyword
        while (pos < len && isalpha(data.at(pos))) {
          pos++;
        }
        auto ref = data.substr(start, pos - start);
        std::transform(ref.begin(), ref.end(), ref.begin(), [](unsigned char c){ return std::tolower(c); });
        if (ref == "null") {
          return TokenNULL;
        }
        if (ref == "true") {
          return TokenTRUE;
        }
        if (ref == "false") {
          return TokenFALSE;
        }
        throw JSONParseException("Unquoted string", location());
      }
      if (isdigit(c) || c == '-' || c == '+') {
        pos--;
        return TokenNumber;
      }

      switch (c) {
        case '"':
          return TokenString;
        case '{':
          return TokenObject;
        case '}':
          return TokenObjectClose;
        case '[':
          return TokenArray;
        case ']':
          return TokenArrayClose;
        case ':':
          return TokenKeySeparator;
        case ',':
          return TokenValueSeparator;
        default:
          throw JSONParseException("Unexpected character", location());
      }
    }

    std::string readString() {
      std::string r;
      while (pos < len && data.at(pos) != '"') {
        if (data.at(pos) == '\\') {
          pos++;
          if (pos == len) {
            return "";
          }
          switch (data.at(pos++)) {
            case '"':
              r += '"';
              break;
            case '\\':
              r += '\\';
              break;
            case '/':
              r += '/';
              break;
            case 'b':
              r += '\b';
              break;
            case 'f':
              r += '\f';
              break;
            case 'n':
              r += '\n';
              break;
            case 'r':
              r += '\r';
              break;
            case 't':
              r += '\t';
              break;
            case 'u':  // hex
              {
                int num = 0;
                for (int i = 0; i < 4; i++) {
                  if (pos == len) {
                    throw JSONParseException("Unexpected EOF", location());
                  }
                  num <<= 4;
                  char c = data.at(pos++);
                  if (c >= '0' && c <= '9') {
                    num |= c - '0';
                  } else if (c >= 'a' && c <= 'f') {
                    num |= c - 'a' + 10;
                  } else if (c >= 'A' && c <= 'F') {
                    num |= c - 'A' + 10;
                  } else {
                    throw JSONParseException("Invalid hex code", location());
                  }
                }
                r += num;
              }
              break;
            default:
              throw JSONParseException("Unknown escape sequence", location());
          }
        } else {
          r += data.at(pos++);
        }
      }
      pos++;
      return r;
    }

    double readDouble() {
      double sign = 1.0;
      if (data.at(pos) == '-') {
        sign = -1.0;
        pos++;
      } else if (data.at(pos) == '+') {
        pos++;
      }
      if (pos == len) {
        throw JSONParseException("Unxpected EOF", location());
      }
      double value = 0.0;
      while (pos < len && isdigit(data.at(pos))) {
        value *= 10.0;
        value += data.at(pos++) - '0';
      }
      if (pos == len) {
        throw JSONParseException("Unexpected EOF", location());
      }
      if (data.at(pos) == '.') {
        double pow10 = 10.0;
        pos++;
        while (pos < len && isdigit(data.at(pos))) {
          value += (data.at(pos++) - '0') / pow10;
          pow10 *= 10.0;
        }
      }
      if (pos == len) {
        throw JSONParseException("Unexpected EOF", location());
      }
      double scale = 1.0;
      bool frac = false;
      if (data.at(pos) == 'e' || data.at(pos) == 'E') {
        pos++;
        if (pos == len) {
          throw JSONParseException("Unexpected EOF", location());
        }
        if (data.at(pos) == '-') {
          frac = true;
          pos++;
        } else if (data.at(pos) == '+') {
          pos++;
        }
        unsigned int expon = 0;
        while (pos < len && isdigit(data.at(pos))) {
          expon *= 10.0;
          expon += data.at(pos++) - '0';
        }
        if (expon > 308) {
          expon = 308;
        }
        while (expon >= 50) {
          scale *= 1e50;
          expon -= 50;
        }
        while (expon >= 8) {
          scale *= 1e8;
          expon -= 8;
        }
        while (expon > 0) {
          scale *= 10.0;
          expon -= 1;
        }
      }
      return sign * (frac ? (value / scale) : (value * scale));
    }

    std::string location() {
      int line = 1;
      int col = 0;
      int cpos = pos;
      bool doneCol = false;
      while (cpos >= 0) {
        if (data.at(cpos) == '\n') {
          doneCol = true;
          line++;
        }
        if (!doneCol) col++;
        cpos--;
      }
      return "Line: " + std::to_string(line) + " Offset: " + std::to_string(col);
    }

  private:
    int pos, len;
    std::string data;
    std::string error;
};

std::shared_ptr<JSONData> JSON::parse(const std::string &data) {
  JSONHelper reader(data);
  auto type = reader.nextToken();
  switch (type) {
    case TokenObject:
      return std::make_shared<JSONObject>(reader);
    case TokenArray:
      return std::make_shared<JSONArray>(reader);
    default:
      throw JSONParseException("Object or array expected", reader.location());
  }
  return nullptr;
}

static std::shared_ptr<JSONData> Null = std::make_shared<JSONData>();

JSONData::JSONData() {}

JSONData::~JSONData() {}

bool JSONData::has(const std::string &) {
  return false;
}

std::shared_ptr<JSONData> JSONData::at(const std::string &) {
  return Null;
}

std::shared_ptr<JSONData> JSONData::at(int) {
  return Null;
}

int JSONData::length() {
  return 0;
}

std::string JSONData::asString() {
  return "";
}

double JSONData::asNumber(double def) {
  return def;
}

int16_t JSONData::asInt(int16_t def) {
  return def;
}

bool JSONData::asBool() {
  return false;
}

JSONBool::JSONBool(bool val) : data(val) {}

bool JSONBool::asBool() {
  return data;
}

JSONString::JSONString(const std::string &val) : data(val) {}

std::string JSONString::asString() {
  return data;
}

JSONNumber::JSONNumber(double val) : data(val) {}

double JSONNumber::asNumber(double) {
  return data;
}

int16_t JSONNumber::asInt(int16_t) {
  return static_cast<int16_t>(data);
}

std::string JSONNumber::asString() {
  return std::to_string(static_cast<int16_t>(data));
}

JSONObject::JSONObject(JSONHelper &reader) {
  Token type;

  while ((type = reader.nextToken()) == TokenString) {
    std::string key = reader.readString();
    if (key.length() == 0) {
      throw JSONParseException("Empty key", reader.location());
    }
    if (reader.nextToken() != TokenKeySeparator) {
      throw JSONParseException("Expected ':'", reader.location());
    }


    std::shared_ptr<JSONData> value = Null;
    auto token = reader.nextToken();

    switch (token) {
      case TokenNULL:
        break;
      case TokenTRUE:
        value = std::make_shared<JSONBool>(true);
        break;
      case TokenFALSE:
        value = std::make_shared<JSONBool>(false);
        break;
      case TokenString:
        value = std::make_shared<JSONString>(reader.readString());
        break;
      case TokenNumber:
        value = std::make_shared<JSONNumber>(reader.readDouble());
        break;
      case TokenObject:
        value = std::make_shared<JSONObject>(reader);
        break;
      case TokenArray:
        value = std::make_shared<JSONArray>(reader);
        break;
      default:
        throw JSONParseException("Expected value", reader.location());
    }
    children[key] = value;
    type = reader.nextToken();
    if (type == TokenObjectClose) {
      break;
    }
    if (type != TokenValueSeparator) {
      throw JSONParseException("Expected ',' or '}'", reader.location());
    }
  }
  if (type != TokenObjectClose) {
    throw JSONParseException("Expected '}' or '\"'", reader.location());
  }
}

bool JSONObject::has(const std::string &key) {
  return children.find(key) != children.end();
}

std::shared_ptr<JSONData> JSONObject::at(const std::string &key) {
  if (auto child = children.find(key); child != children.end()) {
    return child->second;
  }
  return Null;
}

JSONArray::JSONArray(JSONHelper &reader) {
  Token type;

  while ((type = reader.nextToken()) != TokenArrayClose) {
    std::shared_ptr<JSONData> value = nullptr;
    switch (type) {
      case TokenNULL:
        break;
      case TokenTRUE:
        value = std::make_shared<JSONBool>(true);
        break;
      case TokenFALSE:
        value = std::make_shared<JSONBool>(false);
        break;
      case TokenString:
        value = std::make_shared<JSONString>(reader.readString());
        break;
      case TokenNumber:
        value = std::make_shared<JSONNumber>(reader.readDouble());
        break;
      case TokenObject:
        value = std::make_shared<JSONObject>(reader);
        break;
      case TokenArray:
        value = std::make_shared<JSONArray>(reader);
        break;
      default:
        throw JSONParseException("Expected value", reader.location());
    }

    data.push_back(value);
    type = reader.nextToken();
    if (type == TokenArrayClose) {
      break;
    }
    if (type != TokenValueSeparator) {
      throw JSONParseException("Expected ',' or ']'", reader.location());
    }
  }
}

int JSONArray::length() {
  return data.size();
}

std::shared_ptr<JSONData> JSONArray::at(int index) {
  if (index < data.size()) {
    return data[index];
  }
  return Null;
}
