/**
 * @file
 *
 * @brief Demonstrates schema validation using the yaml-cpp adapter.
 *
 * Schema and target documents are loaded as YAML (JSON is valid YAML, so
 * either format works).
 */

#include <iostream>
#include <stdexcept>

#include <valijson/adapters/yaml_cpp_adapter.hpp>
#include <valijson/utils/yaml_cpp_utils.hpp>
#include <valijson/schema.hpp>
#include <valijson/schema_parser.hpp>
#include <valijson/validation_results.hpp>
#include <valijson/validator.hpp>

using std::cerr;
using std::endl;

using valijson::Schema;
using valijson::SchemaParser;
using valijson::Validator;
using valijson::ValidationResults;
using valijson::adapters::YamlCppAdapter;

int main(int argc, char *argv[])
{
    if (argc != 3) {
        cerr << "Usage: " << argv[0]
             << " <schema document> <test/target document>" << endl;
        return 1;
    }

    YAML::Node schemaDocument;
    if (!valijson::utils::loadDocument(argv[1], schemaDocument)) {
        cerr << "Failed to load schema document." << endl;
        return 1;
    }

    YAML::Node targetDocument;
    if (!valijson::utils::loadDocument(argv[2], targetDocument)) {
        cerr << "Failed to load target document." << endl;
        return 1;
    }

    Schema schema;
    SchemaParser parser;
    YamlCppAdapter schemaDocumentAdapter(schemaDocument);
#if VALIJSON_USE_EXCEPTIONS
    try {
        parser.populateSchema(schemaDocumentAdapter, schema);
    } catch (std::exception &e) {
        cerr << "Failed to parse schema: " << e.what() << endl;
        return 1;
    }
#else
    parser.populateSchema(schemaDocumentAdapter, schema);
#endif

    Validator validator(Validator::kWeakTypes);
    ValidationResults results;
    YamlCppAdapter targetDocumentAdapter(targetDocument);
    if (validator.validate(schema, targetDocumentAdapter, &results)) {
        cerr << "Validation succeeded." << endl;
        return 0;
    }

    cerr << "Validation failed." << endl;
    ValidationResults::Error error;
    unsigned int errorNum = 1;
    while (results.popError(error)) {
        cerr << "Error #" << errorNum << endl;
        cerr << " @ " << error.jsonPointer << endl;
        cerr << " - " << error.description << endl;
        ++errorNum;
    }

    return 1;
}
