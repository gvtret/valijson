#include <valijson/adapters/jsoncpp_adapter.hpp>
#include <valijson/adapters/nlohmann_json_adapter.hpp>
#include <valijson/adapters/rapidjson_adapter.hpp>
#include <valijson/schema.hpp>
#include <valijson/schema_parser.hpp>
#include <valijson/validator.hpp>

#include <json/json.h>
#include <nlohmann/json.hpp>
#include <rapidjson/document.h>

int main()
{
    // RapidJSON
    {
        rapidjson::Document document;
        document.Parse("{\"type\":\"object\"}");
        valijson::adapters::RapidJsonAdapter schemaAdapter(document);
        valijson::Schema schema;
        valijson::SchemaParser parser;
        parser.populateSchema(schemaAdapter, schema);

        rapidjson::Document target;
        target.Parse("{}");
        valijson::adapters::RapidJsonAdapter targetAdapter(target);
        valijson::Validator validator;
        if (!validator.validate(schema, targetAdapter, nullptr)) {
            return 1;
        }
    }

    // jsoncpp
    {
        Json::Value schemaJson;
        Json::Reader reader;
        if (!reader.parse("{\"type\":\"object\"}", schemaJson, false)) {
            return 2;
        }
        valijson::adapters::JsonCppAdapter schemaAdapter(schemaJson);
        valijson::Schema schema;
        valijson::SchemaParser parser;
        parser.populateSchema(schemaAdapter, schema);

        Json::Value targetJson(Json::objectValue);
        valijson::adapters::JsonCppAdapter targetAdapter(targetJson);
        valijson::Validator validator;
        if (!validator.validate(schema, targetAdapter, nullptr)) {
            return 3;
        }
    }

    // nlohmann
    {
        nlohmann::json schemaJson = nlohmann::json::parse("{\"type\":\"object\"}");
        valijson::adapters::NlohmannJsonAdapter schemaAdapter(schemaJson);
        valijson::Schema schema;
        valijson::SchemaParser parser;
        parser.populateSchema(schemaAdapter, schema);

        nlohmann::json targetJson = nlohmann::json::object();
        valijson::adapters::NlohmannJsonAdapter targetAdapter(targetJson);
        valijson::Validator validator;
        if (!validator.validate(schema, targetAdapter, nullptr)) {
            return 4;
        }
    }

    return 0;
}
