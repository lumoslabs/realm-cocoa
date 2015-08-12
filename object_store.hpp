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

#ifndef __realm__object_store__
#define __realm__object_store__

#include <realm/group.hpp>
#include "object_schema.hpp"

namespace realm {
    class ObjectStore {
    public:
        // Schema version used for uninitialized Realms
        static const uint64_t NotVersioned;

        // get the last set schema version
        static uint64_t get_schema_version(Group *group);

        // checks if a migration is required for a given schema version
        static bool is_migration_required(realm::Group *group, uint64_t new_version);

        // verify a target schema against its table, setting the table_column property on each schema object
        // returns array of validation errors
        static std::vector<std::string> validate_schema_and_update_column_mapping(Group *group, ObjectSchema &target_schema);

        // updates a Realm to a given target schema/version creating tables as necessary
        // returns if any changes were made
        // passed in schema ar updated with the correct column mapping
        // optionally runs migration function/lambda if schema is out of date
        // NOTE: must be performed within a write transaction
        typedef std::function<void()> MigrationFunction;
        typedef std::vector<ObjectSchemaRef> Schema;
        static bool update_realm_with_schema(Group *group,
                                             uint64_t version,
                                             Schema schema,
                                             MigrationFunction migration);

        // get a table for an object type
        static realm::TableRef table_for_object_type(Group *group, StringData object_type);

    private:
        // set a new schema version
        static void set_schema_version(Group *group, uint64_t version);

        // check if the realm already has all metadata tables
        static bool has_metadata_tables(Group *group);

        // create any metadata tables that don't already exist
        // must be in write transaction to set
        // returns true if it actually did anything
        static bool create_metadata_tables(Group *group);

        // set references to tables on targetSchema and create/update any missing or out-of-date tables
        // if update existing is true, updates existing tables, otherwise validates existing tables
        static bool create_tables(realm::Group *group, ObjectStore::Schema target_schema, bool update_existing);

        // get primary key property name for object type
        static StringData get_primary_key_for_object(Group *group, StringData object_type);

        // sets primary key property for object type
        // must be in write transaction to set
        static void set_primary_key_for_object(Group *group, StringData object_type, StringData primary_key);

        static realm::TableRef table_for_object_type_create_if_needed(Group *group, StringData object_type, bool &created);
        static std::string table_name_for_object_type(std::string class_name);
        static std::string object_type_for_table_name(std::string table_name);

        friend ObjectSchema;
    };

    class ObjectStoreException : public std::exception {
    public:
        enum Kind {
            // thrown when calling update_realm_to_schema and the realm version is greater than the given version
            RealmVersionGreaterThanSchemaVersion,
        };
        ObjectStoreException(Kind kind) : m_kind(kind) {}
        ObjectStoreException::Kind kind() { return m_kind; }

    private:
        Kind m_kind;
    };

    class ObjectStoreValidationException : public std::exception {
    public:
        ObjectStoreValidationException(std::vector<std::string> validation_errors, std::string object_type) :
            m_validation_errors(validation_errors), m_object_type(object_type) {}
        std::vector<std::string> validation_errors() { return m_validation_errors; }
        std::string object_type() { return m_object_type; }

    private:
        std::vector<std::string> m_validation_errors;
        std::string m_object_type;
    };
}

#endif /* defined(__realm__object_store__) */

