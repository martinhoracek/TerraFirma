/** @copyright 2025 Sean Kasun */

#include "worldheader.h"
#include "assets.h"
#include "json.h"
#include <SDL3/SDL.h>
#include <cassert>
#include <memory>

WorldHeader::WorldHeader() {
  try {
    const auto json = JSON::parse(header_json);
    for (int i = 0; i < json->length(); i++) {
      fields.push_back(Field(json->at(i)));
    }
    
  } catch (JSONParseException e) {
    SDL_Log("Failed: %s", e.reason.c_str());
    exit(-1);
  }
}

WorldHeader::~WorldHeader() = default;

void WorldHeader::load(std::shared_ptr<Handle> handle, int version) {
  data.clear();

  for (const auto &field : fields) {
    if (version >= field.minVersion && version <= field.maxVersion) {
      auto header = std::make_shared<Header>();
      switch (field.type) {
        case Field::Type::BOOLEAN:
        case Field::Type::BYTE:
          header->setData(handle->r8());
          break;
        case Field::Type::INT16:
          header->setData(handle->r16());
          break;
        case Field::Type::INT32:
          header->setData(handle->r32());
          break;
        case Field::Type::INT64:
          header->setData(handle->r64());
          break;
        case Field::Type::FLOAT32:
          header->setData(handle->rf());
          break;
        case Field::Type::FLOAT64:
          header->setData(handle->rd());
          break;
        case Field::Type::STRING:
          header->setData(handle->rs());
          break;
        case Field::Type::ARRAY_BYTE:
          {
            auto num = getFieldLength(field);
            for (int i = 0; i < num; i++) {
              header->append(handle->r8());
            }
          }
          break;
        case Field::Type::ARRAY_INT32:
          {
            auto num = getFieldLength(field);
            for (int i = 0; i < num; i++) {
              header->append(handle->r32());
            }
          }
          break;
        case Field::Type::ARRAY_STRING:
          {
            auto num = getFieldLength(field);
            for (int i = 0; i < num; i++) {
              header->append(handle->rs());
            }
          }
          break;
      }
      data[field.name] = header;
    }
  }
}

bool WorldHeader::has(const std::string &key) const {
  return data.find(key) != data.end();
}

bool WorldHeader::is(const std::string &key) const {
  if (!data.contains(key)) {
    return false;
  }
  return data.at(key)->toInt();
}

int WorldHeader::treeStyle(int x) const {
  const auto xs = data.at("treeX");
  int i = 0;
  for (; i < xs->length(); i++) {
    if (x <= xs->at(i)->toInt()) {
      break;
    }
  }
  int style = data.at("treeTops")->at(i)->toInt();
  if (style) {
    return style + 5;
  }
  return 0;
}

int WorldHeader::getFieldLength(const Field &f) {
  int len = f.length;
  if (!f.dynamicLength.empty()) {
    if (auto child = data.find(f.dynamicLength); child != data.end()) {
      return child->second->toInt();
    }
  }
  return len;
}

WorldHeader::Field::Field(std::shared_ptr<JSONData> data) {
  name = data->at("name")->asString();
  auto t = data->at("type")->asString();
  if (t.empty() || t == "b") {
    type = Type::BOOLEAN;
  } else if (t == "s") {
    type = (data->has("num") || data->has("relnum")) ? Type::ARRAY_STRING : Type::STRING;
  } else if (t == "u8") {
    type = (data->has("num") || data->has("relnum")) ? Type::ARRAY_BYTE : Type::BYTE;
  } else if (t == "i16") {
    type = Type::INT16;
  } else if (t == "i32") {
    type = (data->has("num") || data->has("relnum")) ? Type::ARRAY_INT32 : Type::INT32;
  } else if (t == "i64") {
    type = Type::INT64;
  } else if (t == "f32") {
    type = Type::FLOAT32;
  } else if (t == "f64") {
    type = Type::FLOAT64;
  } else {
    throw JSONParseException("Invalid header type: " + t + " on " + name, "");
  }
  length = data->at("num")->asInt();
  dynamicLength = data->at("relnum")->asString();
  minVersion = data->at("min")->asInt();
  maxVersion = data->at("max")->asInt();
  if (maxVersion == 0) {
    maxVersion = MaxVersion;
  }
}

std::shared_ptr<WorldHeader::Header> WorldHeader::operator[](const std::string &key) const {
  if (auto child = data.find(key); child != data.end()) {
    return child->second;
  }
  SDL_Log("key: %s", key.c_str());
  assert(false && "Missing key");
}

WorldHeader::Header::Header() : dint(0), ddbl(0.0) {}
WorldHeader::Header::~Header() = default;

std::shared_ptr<WorldHeader::Header> WorldHeader::Header::at(int i) const {
  return darr.at(i);
}

int WorldHeader::Header::length() const {
  return darr.size();
}

int WorldHeader::Header::toInt() const {
  return dint;
}

double WorldHeader::Header::toDouble() const {
  return ddbl;
}

void WorldHeader::Header::setData(uint64_t v) {
  dint = v;
  ddbl = static_cast<double>(v);
}

void WorldHeader::Header::setData(std::string s) {
  dstr = std::move(s);
}

void WorldHeader::Header::append(std::string s) {
  auto h = std::make_shared<Header>();
  h->setData(s);
  darr.push_back(h);
}

void WorldHeader::Header::append(uint64_t v) {
  auto h = std::make_shared<Header>();
  h->setData(v);
  darr.push_back(h);
}

