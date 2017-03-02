#ifndef ANNOTATIONS_H
#define ANNOTATIONS_H

#include <string>

/**
 * Responsible for parsing the output of GDB when annotations are enabled.
 *
 * When the --annotate= option is provided to GDB it mixes annotations
 * with it's regular output. This parser can filter the annotation output
 * from regular output.
 *
 * CGDB uses a hybrid approach with GDB, it communicates both with GDB/MI
 * and annotations. So in general, CGDB only depends on the annotations to:
 * - Determine when the user is at the prompt (or miscellaneous prompt)
 * - Determine when GDB updates the source location
 * - Determine when the GDB prompt changes
 */

struct annotations_parser;

struct annotations_parser_callbacks {
    /**
     * An arbitrary pointer to associate with the callbacks.
     */
    void *context;

    /**
     * The breakpoint informatoin has changed.
     *
     * When this callback is triggered, the front end should request
     * the new breakpoint information from the debugger to display to the user.
     *
     * @param context
     * The context pointer
     */
    void (*breakpoints_changed_callback)(void *context);

    /**
     * The source location has changed.
     *
     * When this callback is triggered, the front end should request
     * the new source location from the debugger to display to the user.
     *
     * @param context
     * The context pointer
     */
    void (*source_location_changed_callback)(void *context);

    /**
     * The prmopt has changed.
     *
     * @param context
     * The context pointer
     *
     * @param prompt
     * The new prompt
     */
    void (*prompt_changed_callback)(void *context, const std::string &prompt);

    /**
     * Output is available for the console.
     *
     * @param context
     * The context pointer
     *
     * @param str
     * The console output string
     */
    void (*console_output_callback)(void *context, const std::string &str);

    /**
     * GDB reported an error for the command being executed.
     *
     * @param context
     * The context pointer
     *
     * @param msg
     * The error message
     */
    void (*command_error_callback)(void *context, const std::string &msg);

    /**
     * GDB is at a prompt.
     *
     * @param context
     * The context pointer
     */
    void (*console_at_prompt_callback)(void *context);
};

/**
 * Create an annotations parser instance.
 *
 * @param callbacks
 * The callback functions to invoke upon discovery of certain annotations.
 *
 * @return
 * An annotations parser instance.
 */
annotations_parser *annotations_parser_initialize(
        annotations_parser_callbacks callbacks);

/**
 * Destroy an annotations parser instance.
 *
 * @param parser
 * The instance to destroy.
 */
void annotations_parser_shutdown(annotations_parser *parser);

/**
 * Separates the annotations from the regular output of GDB.
 *
 * @param parser
 * The annotations parser instance
 *
 * @param str
 * The gdb annotations string to parse
 *
 * @param size
 * The size of the gdb annotations strings to parse
 *
 * @return
 * 0 on success, otherwise error.
 */
int annotations_parser_io(annotations_parser *parser, char *str, size_t size);

/**
 * Determine if GDB is at a prompt or not.
 *
 * This is useful because if GDB is at a prompt, that means it is
 * ready to recieve another command. This function returns true
 * for miscellaneous prompts as well, which must be taken into account.
 *
 * @param parser
 * The annotations parser instance
 *
 * @return
 * True if ready for input, false otherwise.
 */
bool annotations_parser_at_prompt(annotations_parser *parser);

/**
 * Determine if GDB is at a miscellaneous prompt or not.
 *
 * When at a miscellaneous prompt, CGDB can only send GDB commands
 * that the user has sent, to answer the miscellaneous prompt question.
 *
 * Note, when this function returns true, 
 * annotations_parser_is_ready_for_input will also return true.
 *
 * @param parser
 * The annotations parser instance
 *
 * @return
 * True if ready for input at a miscellaneous prompt, false otherwise.
 */
bool annotations_parser_at_miscellaneous_prompt(annotations_parser *parser);

#endif
