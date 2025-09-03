#ifndef MONGO_ADAPTER_CPP
#define MONGO_ADAPTER_CPP

#include "Mongo_Adapter.h"

Mongo_Adapter::Mongo_Adapter(const char *server, const char *mydb, const char *mycoll) {
    mongoc_init();
    //"mongodb://localhost:27017/"
    client = mongoc_client_new(server);
    collection = mongoc_client_get_collection(client, mydb, mycoll);

    cursor = nullptr;
    doc = nullptr;
    query = nullptr;
    str = nullptr;
}

Mongo_Adapter::~Mongo_Adapter() {
    bson_destroy(query);
    mongoc_cursor_destroy(cursor);
    mongoc_collection_destroy(collection);
    mongoc_client_destroy(client);
    mongoc_cleanup();
}

DataFrame *Mongo_Adapter::get_document(const char *field, const char *value, const std::vector<std::string> column_name,
                                       const std::vector<std::string> data_types, int data_size, bool ignore_first) {
    query = bson_new();
    BSON_APPEND_UTF8(query, field, value);
    cursor = mongoc_collection_find_with_opts(collection, query, NULL, NULL);
    DataFrame *D = new DataFrame(column_name, data_types);

    while (mongoc_cursor_next(cursor, &doc)) {
        str = bson_as_relaxed_extended_json(doc, NULL);
        D->insert(str, data_size, ignore_first);
        bson_free(str);
    }
    return D;
}

DataFrame *Mongo_Adapter::get_all_documents(const std::vector<std::string> &column_name,
                                            const std::vector<std::string> &data_types, int data_size,
                                            bool ignore_first) {
    query = bson_new();
    cursor = mongoc_collection_find_with_opts(collection, query, NULL, NULL);
    DataFrame *D = new DataFrame(column_name, data_types);

    while (mongoc_cursor_next(cursor, &doc)) {
        str = bson_as_relaxed_extended_json(doc, NULL);
        D->insert(str, data_size, ignore_first);
        bson_free(str);
    }
    return D;
}

#endif

