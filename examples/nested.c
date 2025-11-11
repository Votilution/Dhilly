// stack.c
// Example usage of Dhilly template engine leveraging the heap-first API, derived from stack.c
// Created by Iván Markov, 2025-10-20

// This file explains the heap API, you might get a better understanding of the main API in stack.c!

#include "../dhilly.h"
#include <stdio.h>
#include <string.h>

int main() {
    // CREATE NESTED INSTANCE
    DhillyInstance nested_instance;

    {
        DhillyTemplate tpl = dhilly_template_create(2);
        DhillyContext ctx = dhilly_context_create(1);

        nested_instance = dhilly_instance_create(&tpl, &ctx);
    }
    DhillyString s1 = dhilly_string_create("a nested ", DHILLY_STRING_NO_TOUCHY);
    dhilly_add_shard_to_template(nested_instance.template, SHARD_TYPE_STRING, &s1, 0);

    DhillyString dyn = dhilly_string_create("fellow", DHILLY_STRING_NO_TOUCHY);
    dhilly_context_set_string(nested_instance.context, 0, &dyn);
    dhilly_add_shard_to_template(nested_instance.template, SHARD_TYPE_FUNCTION, dhilly_print_text, 0);

    // CREATE MAIN INSTANCE
    DhillyInstance instance;

    {
        DhillyTemplate tpl = dhilly_template_create(3);
        DhillyContext ctx = dhilly_context_create(0);

        instance = dhilly_instance_create(&tpl, &ctx);
    }
    DhillyString s2 = dhilly_string_create("Hello! I am ", DHILLY_STRING_NO_TOUCHY);
    DhillyString s3 = dhilly_string_create(". Nice to meet you!", DHILLY_STRING_NO_TOUCHY);
    dhilly_add_shard_to_template(instance.template, SHARD_TYPE_STRING, &s2, 0);
    dhilly_add_shard_to_template(instance.template, SHARD_TYPE_TEMPLATE, &nested_instance, 0);
    dhilly_add_shard_to_template(instance.template, SHARD_TYPE_STRING, &s3, 0);


    // FLATTEN AND PRINT INSTANCE
    DhillyStringArray dhilly_result = dhilly_template_to_string_array(instance.template, instance.context); // dhilly_print_text will return mega
    DhillyString actual_string = dhilly_string_array_to_string(&dhilly_result);
    
    printf("\n\nConcatenated printing example:\n%s\n", actual_string.data);
    
    
cleanup:
    dhilly_string_array_free(&dhilly_result);
    dhilly_string_free(&actual_string);
    dhilly_instance_free(&instance);

    return 0;
}