#pragma once

#include <unistd.h>
#include <stdbool.h>

typedef enum {
    DHILLY_STRING_FREE,      // Please clean it up for me
    DHILLY_STRING_NO_TOUCHY, // Hands off! I’m managing this one
    DHILLY_STRING_CALLBACK   // Call my custom cleanup function when freeing (UNIMPLEMENTED)
} DhillyStringCleanupStrategy;

typedef struct {
    const char *data;  // Pointer to the string data
    size_t length; // Length of the string EXCLUDING the null terminator
    DhillyStringCleanupStrategy cleanup_strategy;
} DhillyString;

typedef struct {
    DhillyString* data;
    size_t size;
    size_t total_size;
} DhillyStringArray;

typedef struct {
    DhillyString** data; // The context is composed of an array of strings. To access these strings, an index must be leveraged.
    size_t slots; // Although this is optional, it is used to avoid overflows. Although it shouldn't happen under normal circumstances, it is important for it to be the correct size.
} DhillyContext;

typedef enum {
    SHARD_TYPE_STRING,
    SHARD_TYPE_FUNCTION,
    SHARD_TYPE_TEMPLATE,
} DhillyShardType;

typedef struct {
    DhillyString (*generate)(void* context, void* bound_arg);
    void* bound_arg;
} DhillyShardCallable;

typedef struct {
    void* instance_ptr;
    DhillyStringArray result_str_array;
} DhillyShardTemplate;

typedef struct {
    DhillyShardType type;
    union {
        DhillyString str;               // For literal strings
        DhillyShardCallable callable;   // Function pointer for dynamic content
        DhillyShardTemplate template;
    };
} DhillyShard;

typedef struct {
    DhillyShard* shards;
    size_t shard_count;
    size_t shard_capacity;
} DhillyTemplate;

typedef struct {
    DhillyTemplate* template;
    DhillyContext* context;
    DhillyStringArray* result;
} DhillyInstance;


DhillyString dhilly_string_create(const char *input, DhillyStringCleanupStrategy cleanup_strategy);

void dhilly_string_free(DhillyString* string);

void dhilly_shard_free(DhillyShard* shard);

DhillyTemplate dhilly_template_create(size_t capacity);

void dhilly_template_free(DhillyTemplate *template);

void dhilly_set_shard_in_template(DhillyTemplate *template, size_t index, DhillyShardType type, void* contents, void* bound);

void dhilly_add_shard_to_template(DhillyTemplate *template, DhillyShardType type, void* contents, void* bound);

DhillyContext dhilly_context_create(size_t slots);

DhillyStringArray dhilly_template_to_string_array(DhillyTemplate* template, DhillyContext* context);

void dhilly_string_array_free(DhillyStringArray* string_array);

DhillyString dhilly_string_array_to_string(DhillyStringArray* input);

void dhilly_context_set_string(DhillyContext* context, size_t index, DhillyString* str);

void dhilly_context_free(DhillyContext* context);

DhillyInstance dhilly_instance_create(DhillyTemplate* tpl, DhillyContext* ctx);

void dhilly_instance_free(DhillyInstance* instance);

DhillyString dhilly_print_text(DhillyContext* ctx, size_t bound);