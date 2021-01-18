#ifndef EXAMPLE_MAIN
#define EXAMPLE_MAIN
#define _ACTIVE_EXAMPLES_COUNT (defined(EMBEDDING_TEST) + defined(EXAMPLE_PUBSUB) + defined(EXAMPLE_REQREPACK) + defined(EXAMPLE_QUERY))
#if _ACTIVE_EXAMPLES_COUNT > 1
#error "Only one example can be active at a time!"
#elif _ACTIVE_EXAMPLES_COUNT == 1

/**
 * @brief The main entry point for all examples.
 * 
 * The actual example source needs to implement this function.
 * All example source files are bound to a different preprocessor directive,
 * and only one preprocessor directive should be active. The directive should be set
 * in the appropriate place of your build system.
 * 
 * @return int 0 if the example was successful, an error code otherwise
 */
int example_main();
#endif
#endif /* EXAMPLE_MAIN */
