////////////////////////////////////////////////////////////////////////////
//
// Copyright 2014 Realm Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
////////////////////////////////////////////////////////////////////////////

#include "object_schema.hpp"
#include "object_store.hpp"

using namespace realm;
using namespace std;

ObjectSchema::ObjectSchema(realm::Group *group, std::string name) : name(name) {
    TableRef table = ObjectStore::table_for_object_type(group, name);
    size_t count = table->get_column_count();
    for (size_t col = 0; col < count; col++) {
        Property property;
        property.name = table->get_column_name(col).data();
        property.type = (PropertyType)table->get_column_type(col);
        property.is_indexed = table->has_search_index(col);
        property.is_primary = false;
        property.table_column = col;
        if (property.type == PropertyTypeObject || property.type == PropertyTypeArray) {
            // set link type for objects and arrays
            realm::TableRef linkTable = table->get_link_target(col);
            property.object_type = ObjectStore::object_type_for_table_name(linkTable->get_name().data());
        }
        else {
            property.object_type = "";
        }
        properties.push_back(property);
    }

    primary_key = realm::ObjectStore::get_primary_key_for_object(group, name);
    if (primary_key.length()) {
        auto primary_key_iter = primary_key_property();
        if (!primary_key_iter) {
            std::vector<std::string> errors;
            errors.push_back("No property matching primary key '" + primary_key + "'");
            throw ObjectStoreValidationException(errors, name);
        }
        primary_key_iter->is_primary = true;
    }
}

Property *ObjectSchema::property_for_name(std::string name) {
    for (auto& prop:properties) {
        if (prop.name == name) {
            return &prop;
        }
    }
    return nullptr;
}

std::vector<ObjectSchema> ObjectSchema::object_schema_from_group(realm::Group *group) {
    // generate object schema and class mapping for all tables in the realm
    unsigned long numTables = group->size();
    vector<ObjectSchema> object_schema;

    for (unsigned long i = 0; i < numTables; i++) {
        std::string name = ObjectStore::object_type_for_table_name(group->get_table_name(i).data());
        if (name.length()) {
            object_schema.push_back(ObjectSchema(group, name));
        }
    }

    return object_schema;
}
