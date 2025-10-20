// stack.c
// Example usage of Dhilly template engine leveraging the stack-first API
// Created by Iván Markov, 2025-10-20

#include "../dhilly.h"
#include <stdio.h>
#include <string.h>

int main() {
    DhillyTemplate example = dhilly_template_create(3);

    /*
    Here we will define the DhillyContext. The DhillyContext is data that the dynamic functions of the template will have
    access to throughout the rendering process. Each function should know the index of the array inside the context
    that gives them the necessary information
    */
    DhillyContext context = dhilly_context_create(1); // The context object will have one slot: [0].
    
    /*
    We have to create string objects manually. Strings can be dynamic (false) or static (true).
    Making a string dynamic gives Dhilly the permission to free it, otherwise if it's static,
    we have ownership of it and thus the caller is responsible for freeing instead.

    Let's make a dynamic string, for the sake of it! This one will be owned by context. Thus, the
    moment we free context, the string will be freed along with it. If this is a problem, mark it
    as static and Dhilly will leave the freeing to you!
    */
    char *heap_str = strdup("mega");
    DhillyString dyn = dhilly_string_create(heap_str, false);
    dhilly_context_set_string(&context, 0, &dyn); // We save the word "mega" on slot [0] of our DhillyContext.

    // We define the shards of the template!
    DhillyString s1 = dhilly_string_create("This should be ", true);
    dhilly_add_shard_to_template(&example, SHARD_TYPE_STRING, &s1, NULL);

    /* We set the function's "bound" to integer zero, because an integer is what dhilly_print_text.
    This integer will serve as an index that it's gonna read out from our DhillyContext. */
    dhilly_add_shard_to_template(&example, SHARD_TYPE_FUNCTION, dhilly_print_text, 0);
    
    DhillyString s2 = dhilly_string_create(" super easy!", true);
    dhilly_add_shard_to_template(&example, SHARD_TYPE_STRING, &s2, NULL);
   
    DhillyStringArray dhilly_result = dhilly_template_to_string_array(&example, &context); // dhilly_print_text will return mega
    
    printf("Zero-allocation printing example:\n");
    for (size_t i = 0; i < dhilly_result.size; i++) {
        printf("%s", dhilly_result.data[i].data);
    }
    
    DhillyString actual_string = dhilly_string_array_to_string(&dhilly_result);

    printf("\n\nConcatenated printing example:\n%s\n", actual_string.data);

    
cleanup:
    dhilly_string_free(&actual_string);
    dhilly_string_array_free(&dhilly_result);
    dhilly_context_free(&context);
    dhilly_template_free(&example);

    return 0;
}