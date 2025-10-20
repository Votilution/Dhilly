> WARNING: Dhilly is currently in the pre-alpha stage. Here be dragons.

# Dhilly
The templating engine that will knock your socks off.

Stack-first structure, heap allocation only when necessary, less is more philosophy, minimal-copy generation preferred.

Dhilly provides two main methods for interacting with and making templates, giving the developer freedom over how they want to structure their program:

- Stack API: minimal allocations, zero-copy, ephemeral templates. Ideal for one-offs or fast generation.
- Heap API (DhillyInstance): an extension of the Stack API: it's safe, reusable, sharable, with centralized memory management. Perfect for long-lived templates or more complex workflows.

Dhilly isn't strictly a web templating engine. Rather, it's a library that can act as one of the building blocks for one.