// ABOUTME: ArduinoJson mock implementation for simulator
// ABOUTME: Provides minimal JSON parsing/serialization compatible with firmware

#include "ArduinoJson.h"
#include <regex>
#include <sstream>

// JsonVariant template specializations
template<>
String JsonVariant::as<String>() const {
    return strValue;
}

template<>
const char* JsonVariant::as<const char*>() const {
    return strValue.c_str();
}

template<>
int JsonVariant::as<int>() const {
    return intValue;
}

template<>
bool JsonVariant::as<bool>() const {
    return boolValue;
}

template<>
JsonArray JsonVariant::as<JsonArray>() const {
    if (arrayValue) {
        return *arrayValue;
    }
    return JsonArray();
}

JsonVariant::operator JsonArray() const {
    if (arrayValue) {
        return *arrayValue;
    }
    return JsonArray();
}

JsonVariant JsonVariant::operator[](const char* key) const {
    auto it = objectValues.find(String(key));
    if (it != objectValues.end()) {
        return it->second;
    }
    return JsonVariant();
}

// JsonDocument methods
JsonArray JsonDocument::createNestedArray(const char* key) {
    auto arr = std::make_shared<JsonArray>();
    JsonVariant var;
    var.setArray(arr);
    data[String(key)] = var;
    return *arr;
}

template<>
JsonArray JsonDocument::as<JsonArray>() {
    auto it = data.find(String("items"));
    if (it != data.end()) {
        return it->second.as<JsonArray>();
    }
    return JsonArray();
}

static std::vector<std::string> extractItems(const std::string& payload) {
    std::vector<std::string> items;

    size_t itemsPos = payload.find("\"items\"");
    if (itemsPos == std::string::npos) return items;

    size_t arrayStart = payload.find('[', itemsPos);
    if (arrayStart == std::string::npos) return items;

    int braceDepth = 0;
    bool inObject = false;
    std::string currentItem;

    for (size_t i = arrayStart + 1; i < payload.size(); ++i) {
        char c = payload[i];

        if (c == '{') {
            if (!inObject) {
                currentItem.clear();
                inObject = true;
            }
            braceDepth++;
        }

        if (inObject) {
            currentItem.push_back(c);
        }

        if (c == '}') {
            braceDepth--;
            if (braceDepth == 0 && inObject) {
                items.push_back(currentItem);
                inObject = false;
            }
        }

        if (c == ']' && braceDepth == 0) {
            break;
        }
    }

    return items;
}

DeserializationError deserializeJson(JsonDocument& doc, const String& input) {
    std::string payload = input;
    auto itemsArray = std::make_shared<JsonArray>();

    std::regex summaryRegex("\"summary\"\\s*:\\s*\"([^\"]*)\"");
    std::regex idRegex("\"id\"\\s*:\\s*\"([^\"]*)\"");
    std::regex startDateRegex("\"start\"\\s*:\\s*\\{[^\\}]*\"date\"\\s*:\\s*\"([^\"]*)\"");
    std::regex startDateTimeRegex("\"start\"\\s*:\\s*\\{[^\\}]*\"dateTime\"\\s*:\\s*\"([^\"]*)\"");
    std::regex endDateRegex("\"end\"\\s*:\\s*\\{[^\\}]*\"date\"\\s*:\\s*\"([^\"]*)\"");
    std::regex endDateTimeRegex("\"end\"\\s*:\\s*\\{[^\\}]*\"dateTime\"\\s*:\\s*\"([^\"]*)\"");

    for (const auto& itemString : extractItems(payload)) {
        std::map<String, JsonVariant> fields;
        std::smatch match;

        if (std::regex_search(itemString, match, summaryRegex)) {
            JsonVariant summaryVar;
            summaryVar.setString(match[1].str());
            fields["summary"] = summaryVar;
        }

        if (std::regex_search(itemString, match, idRegex)) {
            JsonVariant idVar;
            idVar.setString(match[1].str());
            fields["id"] = idVar;
        }

        std::map<String, JsonVariant> startFields;
        if (std::regex_search(itemString, match, startDateRegex)) {
            JsonVariant dateVar;
            dateVar.setString(match[1].str());
            startFields["date"] = dateVar;
        }

        if (std::regex_search(itemString, match, startDateTimeRegex)) {
            JsonVariant dateTimeVar;
            dateTimeVar.setString(match[1].str());
            startFields["dateTime"] = dateTimeVar;
        }

        if (!startFields.empty()) {
            JsonVariant startVar;
            startVar.setObject(startFields);
            fields["start"] = startVar;
        }

        std::map<String, JsonVariant> endFields;
        if (std::regex_search(itemString, match, endDateRegex)) {
            JsonVariant dateVar;
            dateVar.setString(match[1].str());
            endFields["date"] = dateVar;
        }

        if (std::regex_search(itemString, match, endDateTimeRegex)) {
            JsonVariant dateTimeVar;
            dateTimeVar.setString(match[1].str());
            endFields["dateTime"] = dateTimeVar;
        }

        if (!endFields.empty()) {
            JsonVariant endVar;
            endVar.setObject(endFields);
            fields["end"] = endVar;
        }

        if (!fields.empty()) {
            JsonVariant obj;
            obj.setObject(fields);
            itemsArray->add(obj);
        }
    }

    JsonVariant itemsVar;
    itemsVar.setArray(itemsArray);
    doc["items"] = itemsVar;

    return DeserializationError(DeserializationError::Ok);
}

void serializeJson(const JsonDocument& doc, String& output) {
    auto it = doc.data.find(String("items"));
    if (it == doc.data.end()) {
        output = "{}";
        return;
    }

    std::ostringstream oss;
    oss << "{\"items\":[";
    bool first = true;
    for (JsonVariant item : it->second.as<JsonArray>()) {
        if (!first) oss << ",";
        first = false;
        oss << "{";

        String summary = item["summary"].as<String>();
        bool hasSummary = !summary.isEmpty();
        bool hasWrittenField = false;

        if (hasSummary) {
            oss << "\"summary\":\"" << summary << "\"";
            hasWrittenField = true;
        }

        String id = item["id"].as<String>();
        if (!id.isEmpty()) {
            if (hasWrittenField) oss << ",";
            oss << "\"id\":\"" << id << "\"";
            hasWrittenField = true;
        }

        JsonVariant start = item["start"];
        String startDate = start["date"].as<String>();
        String startDateTime = start["dateTime"].as<String>();
        if (!startDate.isEmpty() || !startDateTime.isEmpty()) {
            if (hasWrittenField) oss << ",";
            oss << "\"start\":{";
            bool firstStartField = true;
            if (!startDate.isEmpty()) {
                oss << "\"date\":\"" << startDate << "\"";
                firstStartField = false;
            }
            if (!startDateTime.isEmpty()) {
                if (!firstStartField) oss << ",";
                oss << "\"dateTime\":\"" << startDateTime << "\"";
            }
            oss << "}";
            hasWrittenField = true;
        }

        JsonVariant end = item["end"];
        String endDate = end["date"].as<String>();
        String endDateTime = end["dateTime"].as<String>();
        if (!endDate.isEmpty() || !endDateTime.isEmpty()) {
            if (hasWrittenField) oss << ",";
            oss << "\"end\":{";
            bool firstEndField = true;
            if (!endDate.isEmpty()) {
                oss << "\"date\":\"" << endDate << "\"";
                firstEndField = false;
            }
            if (!endDateTime.isEmpty()) {
                if (!firstEndField) oss << ",";
                oss << "\"dateTime\":\"" << endDateTime << "\"";
            }
            oss << "}";
        }

        oss << "}";
    }
    oss << "]}";
    output = String(oss.str());
}
