// stack.c
// Example usage of Dhilly template engine leveraging the stack-first API
// Created by Iván Markov, 2025-10-20

#include "../dhilly.h"
#include <stdio.h>
#include <string.h>

int main() {
    DhillyTemplate example = dhilly_template_create(4);

    /*
    Here we will define the DhillyContext. The DhillyContext is data that the dynamic functions of the template will have
    access to throughout the rendering process. Each function should know the index of the array inside the context
    that gives them the necessary information
    */
    DhillyContext context = dhilly_context_create(2); // The context object will have one slot: [0].
    
    /*
    We have to create string objects manually. Strings can be dynamic (false) or static (true).
    Making a string dynamic gives Dhilly the permission to free it, otherwise if it's static,
    we have ownership of it and thus the caller is responsible for freeing instead.

    Let's make a dynamic string, for the sake of it! This one will be owned by context. Thus, the
    moment we free context, the string will be freed along with it. If this is a problem, mark it
    as static and Dhilly will leave the freeing to you!
    */
    DhillyString dyn1 = dhilly_string_create("mega", DHILLY_STRING_NO_TOUCHY);
    DhillyString dyn2 = dhilly_string_create("!", DHILLY_STRING_NO_TOUCHY);
    dhilly_context_set_string(&context, 0, &dyn1); // We save the string "mega" on slot [0] of our DhillyContext.
    dhilly_context_set_string(&context, 1, &dyn2); // We save the string "!" on slot [1] of our DhillyContext.

    // We define the shards of the template!
    DhillyString s1 = dhilly_string_create("This should be ", DHILLY_STRING_NO_TOUCHY);
    dhilly_add_shard_to_template(&example, SHARD_TYPE_STRING, &s1, 0);

    /* We set the function's "bound" to integer zero, because an integer is what dhilly_print_text.
    This integer will serve as an index that it's gonna read out from our DhillyContext. */
    dhilly_add_shard_to_template(&example, SHARD_TYPE_FUNCTION, dhilly_print_text, 0);
    
    DhillyString s2 = dhilly_string_create(" super easy", true);
    dhilly_add_shard_to_template(&example, SHARD_TYPE_STRING, &s2, 0);

    dhilly_add_shard_to_template(&example, SHARD_TYPE_FUNCTION, dhilly_print_text, 1);

    // We initialize ram n shi.
    dhilly_template_init(&example);

    DhillyStringArray dhilly_result = dhilly_template_to_string_array(&example, &context); // dhilly_print_text will return mega
    
    printf("Zero-allocation printing example:\n");
    for (size_t i = 0; i < dhilly_result.size; i++) {
        printf("%s", dhilly_result.data[i].data);
    }

    printf("\n\nArena contents:\n");

    fwrite(example.arena.ptr, 1, example.arena.offset, stdout);

    printf(" - Plus %d unused bytes", example.arena.size + example.arena.size - example.arena.offset);
    
cleanup:
    dhilly_string_array_free(&dhilly_result);
    dhilly_context_free(&context);
    dhilly_template_free(&example);

    return 0;
}