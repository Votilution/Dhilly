// dhilly.c
// A minimal and fast C-first templating engine.
// Created by Votilution

// Potential contributor, take into consideration: LESS IS MORE!
// Stack-first structure, heap allocation only when necessary, zero-copy generation preferred.

#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "dhilly.h"

DhillyString dhilly_string_create(const char *input, bool is_static) {
    DhillyString str;
    str.data = input;
    str.length = strlen(input);
    str.is_static = is_static;

    return str;
}

void dhilly_string_free(DhillyString* string) {
    if (!string) return; 
    if (string->is_static) return;

    if (string && string->data) {
        free((void*)string->data);
        string->data = NULL;
    }

    string->length = 0;
}

void dhilly_shard_free(DhillyShard* shard) {
    if (shard->type == SHARD_TYPE_STRING) dhilly_string_free(&shard->str);
}

DhillyTemplate dhilly_template_create(size_t capacity) {
    DhillyTemplate template;

    template.shards = (DhillyShard *)malloc(sizeof(DhillyShard) * capacity);
    template.shard_count = 0;
    template.shard_capacity = capacity;

    return template;
}

void dhilly_template_free(DhillyTemplate *template) {
    for (size_t i = 0; i < template->shard_count; i++) {
        DhillyShard* shard = &template->shards[i];

        dhilly_shard_free(shard);
    }

    free(template->shards);

    template->shards = NULL;
    template->shard_count = 0;
    template->shard_capacity = 0;
}

void dhilly_set_shard_in_template(DhillyTemplate *template, size_t index, DhillyShardType type, void* contents, void* bound) {
    template->shards[index].type = type;
    if (type == SHARD_TYPE_STRING)
    {
        template->shards[index].str = *(DhillyString*)contents;
    }
    else
    {
        template->shards[index].callable.generate = (DhillyString (*)(void*, void*))contents;
        template->shards[index].callable.bound_arg = bound;
    }
}

void dhilly_add_shard_to_template(DhillyTemplate *template, DhillyShardType type, void* contents, void* bound) {
    dhilly_set_shard_in_template(template, template->shard_count, type, contents, bound);
    template->shard_count++;
}

DhillyContext dhilly_context_create(size_t slots) {
    DhillyContext result;
    
    result.data = malloc(sizeof(DhillyString*) * slots);
    result.slots = slots;

    return result;
}

DhillyStringArray dhilly_template_to_string_array(DhillyTemplate* template, DhillyContext* context) {
    DhillyShard* shards = template->shards;
    DhillyString* array_data = malloc(sizeof(DhillyString) * template->shard_count);
    size_t array_size = template->shard_count;
    size_t array_total_size = 0;

    size_t str_length;
    for (size_t i = 0; i < template->shard_count; i++)
    {
        char* result_str = NULL;
        DhillyString string;

        switch (shards[i].type) {
        case SHARD_TYPE_STRING:
            string = shards[i].str;
            break;
        case SHARD_TYPE_FUNCTION:
            string = shards[i].callable.generate(context, NULL);
            break;
        }

        if (string.data && string.length > 0)
        {
            array_data[i] = string;
            array_total_size += string.length;
        }
        else
        {
            array_data[i] = dhilly_string_create("", true);
        }
    }

    DhillyStringArray result = {array_data, array_size, array_total_size};

    return result;
}

void dhilly_string_array_free(DhillyStringArray* string_array) {
    for (size_t i = 0; i < string_array->size; i++) {
        DhillyString* emt = &string_array->data[i];

        dhilly_string_free(emt);
    }

    free(string_array->data);

    string_array->data = NULL;
    string_array->size = 0;
    string_array->total_size = 0;
}

DhillyString dhilly_string_array_to_string(DhillyStringArray* input) {
    DhillyString result;
    char* result_str = malloc(input->total_size + 1);

    if (result_str == NULL) {
        return (DhillyString){ NULL, 0, true };  // Returning an empty string
    }

    size_t current_pos = 0;

    for (size_t i=0; i < input->size; i++)
    {
        DhillyString str = input->data[i];
        memcpy(result_str + current_pos, str.data, str.length);
        current_pos += str.length;
    }

    result_str[current_pos] = '\0';

    result.data = result_str;
    result.is_static = false;
    result.length = current_pos;

    return result;
}

void dhilly_context_set_string(DhillyContext* context, size_t index, DhillyString* str) {
    if (index < context->slots) {
        context->data[index] = str;
    }
}

void dhilly_context_free(DhillyContext* context) {
    for (size_t i = 0; i < context->slots; i++) {
        DhillyString* emt = context->data[i];

        dhilly_string_free(emt);
    }

    free(context->data);

    context->data = NULL;
    context->slots = 0;
}

DhillyString dhilly_print_text(DhillyContext* ctx, size_t bound) {
    DhillyString* str = ctx->data[bound];
    size_t len = str->length;

    char *copy = malloc(len + 1); // +1 for Null terminator

    if (copy) {
        memcpy(copy, str->data, len);
        copy[len] = '\0';
    }

    return dhilly_string_create(copy, false);
}

/* Here we get into the nitty gritty! DhillyInstance is a wrapper struct for DhillyTemplate and DhillyContext that allows
the user to store them in the heap rather than the stack. This allows it to be passed around more safely! */

DhillyInstance dhilly_instance_create(DhillyTemplate* tpl, DhillyContext* ctx) {
    DhillyInstance instance;

    instance.template = malloc(sizeof(DhillyTemplate));
    memcpy(instance.template, tpl, sizeof(DhillyTemplate));

    instance.context = malloc(sizeof(DhillyContext));
    memcpy(instance.context, ctx, sizeof(DhillyContext));

    instance.result = NULL;

    return instance;
}

void dhilly_instance_free(DhillyInstance* instance) {
    if (!instance) return;

    if (instance->template) {
        dhilly_template_free(instance->template);
        free(instance->template);
        instance->template = NULL;
    }

    if (instance->context) {
        dhilly_context_free(instance->context);
        free(instance->context);
        instance->context = NULL;
    }

    if (instance->result) {
        dhilly_string_array_free(instance->result);
        free(instance->result);
        instance->result = NULL;
    }
}