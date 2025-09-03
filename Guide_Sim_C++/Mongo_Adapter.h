#ifndef MONGO_ADAPTER_H
#define MONGO_ADAPTER_H

#include <bson/bson.h>
#include <mongoc/mongoc.h>
#include <iostream>
#include <string>
#include "DataFrame.h"

class Mongo_Adapter {
private:
    mongoc_client_t *client;
    mongoc_collection_t *collection;
    mongoc_cursor_t *cursor;
    const bson_t *doc;
    bson_t *query;
    char *str;

public:
    Mongo_Adapter(const char *, const char *, const char *);

    ~Mongo_Adapter();

    Mongo_Adapter(const Mongo_Adapter &other) = default;

    Mongo_Adapter &operator=(const Mongo_Adapter &other) = default;

    DataFrame *
    get_document(const char *, const char *, const std::vector<std::string>, const std::vector<std::string>, int,
                 bool);

    DataFrame *get_all_documents(const std::vector<std::string> &, const std::vector<std::string> &, int, bool);
};

#endif // !MONGO_ADAPTER_H


