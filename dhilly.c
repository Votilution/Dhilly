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
#include <stdint.h>

DhillyString dhilly_string_create(const char *input, DhillyStringCleanupStrategy cleanup_strategy) {
    DhillyString str;
    str.data = input;
    str.length = strlen(input);
    str.cleanup_strategy = cleanup_strategy;

    return str;
}

void dhilly_string_free(DhillyString* string) {
    if (!string) return; 
    if (string->cleanup_strategy == DHILLY_STRING_NO_TOUCHY) return;

    if (string && string->data) {
        free((void*)string->data);
        string->data = NULL;
    }

    string->length = 0;
}

void dhilly_shard_free(DhillyShard* shard) {
    switch (shard->type) {
    case SHARD_TYPE_STRING:
        dhilly_string_free(&shard->str);
        break;
    case SHARD_TYPE_TEMPLATE:
        dhilly_instance_free(shard->template.instance_ptr);
    };
}

DhillyTemplate dhilly_template_create(size_t capacity) {
    DhillyTemplate template;

    template.shards = (DhillyShard *)malloc(sizeof(DhillyShard) * capacity);
    template.shard_count = 0;
    template.shard_capacity = capacity;

    DhillyArena arena;

    arena.offset = 0;
    arena.size = sizeof(char*) * 1000;
    arena.ptr = malloc(arena.size);

    template.arena = arena;

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

    free(template->arena.ptr);
    
    template->arena.ptr = NULL;
    template->arena.offset = 0;
    template->arena.size = 0;
}

void dhilly_set_shard_in_template(DhillyTemplate *template, size_t index, DhillyShardType type, void* contents, uintptr_t bound) {
    DhillyShard *target = &template->shards[index];

    target->type = type;
    switch (type)
    {
    case SHARD_TYPE_STRING:
        target->str = *(DhillyString*)contents;
        break;
    
    case SHARD_TYPE_FUNCTION:
        target->callable.generate = (DhillyString (*)(DhillyArena*, DhillyContext*, uintptr_t))contents;
        target->callable.bound_arg = bound;
        break;
    
    case SHARD_TYPE_TEMPLATE:
        target->template.instance_ptr = contents;
        break;

    default:
        break;
    }
}

void dhilly_add_shard_to_template(DhillyTemplate *template, DhillyShardType type, void* contents, uintptr_t bound) {
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

    template->arena.offset = 0;

    for (size_t i = 0; i < template->shard_count; i++)
    {
        char* result_str = NULL;
        DhillyString string;
        
        string.data = NULL;
        string.length = 0;
        string.cleanup_strategy = DHILLY_STRING_NO_TOUCHY;

        switch (shards[i].type) {
        case SHARD_TYPE_STRING:
            string = shards[i].str;
            break;

        case SHARD_TYPE_FUNCTION:
            string = shards[i].callable.generate(&template->arena, context, shards[i].callable.bound_arg);
            break;

        case SHARD_TYPE_TEMPLATE:
            DhillyInstance* nested_instance = shards[i].template.instance_ptr;

            DhillyStringArray res = dhilly_template_to_string_array(nested_instance->template, nested_instance->context);
            string = dhilly_string_array_to_string(&res);
            dhilly_string_array_free(&res);

        default:
            break;
        }

        if (string.data && string.length > 0)
        {
            array_data[i] = string;
            array_total_size += string.length;
        }
        else
        {
            array_data[i] = dhilly_string_create("", DHILLY_STRING_NO_TOUCHY);
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
    result.cleanup_strategy = DHILLY_STRING_FREE;
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

DhillyString dhilly_print_text(DhillyArena* arena, DhillyContext* ctx, uintptr_t bound) {
    DhillyString* str = ctx->data[bound];
    printf("%dq", bound);
    size_t len = str->length;

    char *copy = arena->ptr + arena->offset; // +1 for Null terminator

    if (copy) {
        memcpy(copy, str->data, len);
        copy[len] = '\0';
    }

    printf("%d\n", arena->offset);
    arena->offset += len + 1;
    printf("%d\n", arena->offset);
    printf("%s\n", copy);

    return dhilly_string_create(copy, DHILLY_STRING_NO_TOUCHY);
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