// stack.c
// Example usage of Dhilly template engine leveraging the heap-first API, derived from stack.c
// Created by Iván Markov, 2025-10-20

// This file explains the heap API, you might get a better understanding of the main API in stack.c!

#include "../dhilly.h"
#include <stdio.h>
#include <string.h>

int main() {
    DhillyInstance instance;

    /* We create the template and the context at once, then pass references to them to instance, which will
    be kind enough to copy them to the heap and give us back an awesome pointer! Optionally, do this in brackets,
    that way the original template and context go out of scope and you get a minimal memory save. */
    {
        DhillyTemplate tpl = dhilly_template_create(3);
        DhillyContext ctx = dhilly_context_create(1);
        instance = dhilly_instance_create(&tpl, &ctx);
    }

    // From now on, our context and template are available through the DhillyInstance, which can be freely passed around.
    // How convenient, aye?

    // We perform our magic.
    char *heap_str = strdup("mega");
    DhillyString dyn = dhilly_string_create(heap_str, false);
    dhilly_context_set_string(instance.context, 0, &dyn);

    DhillyString s1 = dhilly_string_create("This should be ", true);
    dhilly_add_shard_to_template(instance.template, SHARD_TYPE_STRING, &s1, NULL);

    dhilly_add_shard_to_template(instance.template, SHARD_TYPE_FUNCTION, dhilly_print_text, 0);
    
    DhillyString s2 = dhilly_string_create(" super easy!", true);
    dhilly_add_shard_to_template(instance.template, SHARD_TYPE_STRING, &s2, NULL);

    // The API is almost the exact same, only we reference DhillyInstance instead.
    DhillyStringArray dhilly_result = dhilly_template_to_string_array(instance.template, instance.context); // dhilly_print_text will return mega
    
    printf("Zero-allocation printing example:\n");
    for (size_t i = 0; i < dhilly_result.size; i++) {
        printf("%s", dhilly_result.data[i].data);
    }
    
    DhillyString actual_string = dhilly_string_array_to_string(&dhilly_result);

    printf("\n\nConcatenated printing example:\n%s\n", actual_string.data);

    
cleanup:
    dhilly_string_free(&actual_string);
    // DhillyInstance makes cleanup easier and safer.
    dhilly_instance_free(&instance);

    return 0;
}