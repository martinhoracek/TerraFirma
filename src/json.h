/** @copyright 2025 Sean Kasun */

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <cstdint>

class JSONHelper;

class JSONData {
  public:
    JSONData();
    virtual ~JSONData();
    virtual bool has(const std::string &key);
    virtual std::shared_ptr<JSONData> at(const std::string &key);
    virtual std::shared_ptr<JSONData> at(int index);
    virtual int length();
    virtual std::string asString();
    virtual double asNumber(double def = 0.0);
    virtual int16_t asInt(int16_t def = 0);
    virtual bool asBool();
};

class JSONBool : public JSONData {
  public:
    JSONBool(bool val);
    bool asBool();
  private:
    bool data;
};

class JSONString : public JSONData {
  public:
    JSONString(const std::string &val);
    std::string asString();
  private:
    std::string data;
};

class JSONNumber : public JSONData {
  public:
    JSONNumber(double val);
    double asNumber(double);
    int16_t asInt(int16_t);
    std::string asString();
  private:
    double data;
};

class JSONObject : public JSONData {
  public:
    JSONObject(JSONHelper &);
    bool has(const std::string &key);
    std::shared_ptr<JSONData> at(const std::string &key);
  private:
    std::unordered_map<std::string, std::shared_ptr<JSONData>> children;
};

class JSONArray : public JSONData {
  public:
    JSONArray(JSONHelper &);
    std::shared_ptr<JSONData> at(int index);
    int length();
  private:
    std::vector<std::shared_ptr<JSONData>> data;
};

class JSONParseException {
  public:
    JSONParseException(std::string reason, std::string at) : reason(reason + " at " + at) {}
    std::string reason;
};

class JSON {
  public:
    static std::shared_ptr<JSONData> parse(const std::string &data);

};
