#include "annotations.h"
#include "sys_util.h"

/**
 * From the GDB manual.
 *
 * Annotations start with a newline character, two ‘control-z’ characters,
 * and the name of the annotation.
 *
 * If there is no additional information associated with this annotation,
 * the name of the annotation is followed immediately by a newline.
 *
 * If there is additional information, the name of the annotation is
 * followed by a space, the additional information, and a newline.
 *
 * Annotations are of the form
 * '\n\032\032annotation\n'
 * However, on windows \n gets mapped to \r\n so that makes,
 * '\r+\n\032\032annotation\r+\n'
 */

enum annotation_state {
    /**
     * When in this state, characters recieved from GDB are literal
     * GDB output.
     *
     * State transitions:
     * newline -> ANNOTATION_NEW_LINE
     * other   -> ANNOTATION_GDB_DATA
     */
    ANNOTATION_GDB_DATA,
    
    /**
     * When in this state, GDB is either starting an annotation
     * or simply recieving a newline.
     *
     * State transitions:
     * control-z -> ANNOTATION_CONTROL_Z
     * newline   -> output newline, stay in same state
     * other     -> output newline, transition to ANNOTATION_GDB_DATA
     */
    ANNOTATION_NEW_LINE,

    /**
     * When in this state, GDB is either starting an annotation
     * or has recieved a newline followed by a control-z.
     *
     * State transitions:
     * control-z -> ANNOTATION_TEXT (an annotation has been found)
     * newline   -> output newline, output control z,
     *              transition to ANNOTATION_NEW_LINE
     * other     -> output newline, output control z,
     *              transition to ANNOTATION_GDB_DATA
     */
    ANNOTATION_CONTROL_Z,

    /**
     * When in this state, GDB has recieved an annotation.
     * It is currently collecting the annotation information.
     *
     * State transitions:
     * other    -> collect annotation information
     * new line -> ANNOTATION_GDB_DATA
     */
    ANNOTATION_TEXT
};

struct annotations_parser {
    /** The client parser callbacks */
    annotations_parser_callbacks callbacks;

    /** The state of the annotation parser ( current context ). */
    enum annotation_state state;

    /** The current annotation text being collected */
    std::string annotation_text;

    /** The debugger's current prompt and last prompt. */
    std::string gdb_prompt, gdb_prompt_last;

    /** The error message between error-begin and error or quit */
    std::string error_message;

    /** The console output from GDB */
    std::string console_output;

    /** True if GDB is ready for input and at the prompt, false otherwise */
    bool at_prompt;

    /**
     * True if GDB is at the pre_prompt, false otherwise.
     *
     * GDB normally outputs the prompts as follows,
     *   pre-prompt
     *   (gdb)      <- This will be saved into the gdb_prompt variable above
     *   prompt
     *   b main
     *   post-prompt
     *
     * In this situation, the text between the pre-prompt annotation
     * and the prompt annotation should be used as the new prompt.
     */
    bool at_pre_prompt;

    /**
     * True if GDB is at a miscellaneous prompt, false otherwise.
     *
     * The miscellaneous prompt annotations call the handle_misc_pre_prompt
     * functions. In general, when a miscellaneous prompt is encountered,
     * CGDB should only send user console commands to GDB.
     */
    bool at_misc_prompt;

    /**
     * True if collecting an error message, False otherwise.
     *
     * Should only be true between error-begin and error or quit.
     */
    bool at_error_message;

    /**
     * True if the breakpoints have changed, false otherwise.
     */
    bool breakpoints_changed;

    /**
     * True if the source location has changed, false otherwise.
     */
    bool source_location_changed;
};

annotations_parser *annotations_parser_initialize(
    annotations_parser_callbacks callbacks)
{
    annotations_parser *a = new annotations_parser();
    a->callbacks = callbacks;
    a->state = ANNOTATION_GDB_DATA;
    a->at_prompt = false;
    a->at_pre_prompt = false;
    a->at_misc_prompt = false;
    a->at_error_message = false;
    a->breakpoints_changed = false;
    a->source_location_changed = false;
    return a;
}

void annotations_parser_shutdown(annotations_parser *parser)
{
    delete parser;
}

static void send_available_console_output(annotations_parser *parser)
{
    if (!parser->console_output.empty()) {
        parser->callbacks.console_output_callback(
                parser->callbacks.context, parser->console_output);
        parser->console_output.clear();
    }
}

bool annotations_parser_at_prompt(annotations_parser *parser)
{
    return parser->at_prompt;
}

bool annotations_parser_at_miscellaneous_prompt(annotations_parser *parser)
{
    return parser->at_misc_prompt;
}

static void
update_prompt(struct annotations_parser *parser)
{
    if (parser->gdb_prompt != parser->gdb_prompt_last) {
        parser->callbacks.prompt_changed_callback(
            parser->callbacks.context, parser->gdb_prompt);
        parser->gdb_prompt_last = parser->gdb_prompt;
    }
    parser->gdb_prompt.clear();
}

static void
console_at_prompt(struct annotations_parser *parser)
{
    parser->callbacks.console_at_prompt_callback(parser->callbacks.context);
}

static void
handle_breakpoints_invalid(struct annotations_parser *parser)
{
    parser->breakpoints_changed = true;
}

static void
handle_source(struct annotations_parser *parser)
{
    parser->source_location_changed = true;
}

static void
handle_frame_end(struct annotations_parser *parser)
{
    parser->source_location_changed = true;
}

static void
handle_frames_invalid(struct annotations_parser *parser)
{
    parser->source_location_changed = true;
}

static void handle_misc_pre_prompt(struct annotations_parser *parser)
{
    parser->at_pre_prompt = true;
}

static void handle_misc_prompt(struct annotations_parser *parser)
{
    send_available_console_output(parser);

    parser->at_prompt = true;
    parser->at_misc_prompt = true;
    parser->at_pre_prompt = false;

    update_prompt(parser);
    console_at_prompt(parser);
}

static void handle_misc_post_prompt(struct annotations_parser *parser)
{
    parser->at_prompt = false;
    parser->at_misc_prompt = false;
}

static void handle_pre_prompt(struct annotations_parser *parser)
{
    parser->at_pre_prompt = true;

    if (parser->breakpoints_changed) {
        parser->callbacks.breakpoints_changed_callback(
            parser->callbacks.context);
        parser->breakpoints_changed = false;
    }

    if (parser->source_location_changed) {
        parser->callbacks.source_location_changed_callback(
            parser->callbacks.context);
        parser->source_location_changed = false;
    }
}

static void handle_prompt(struct annotations_parser *parser)
{
    send_available_console_output(parser);

    parser->at_prompt = true;
    parser->at_pre_prompt = false;

    update_prompt(parser);
    console_at_prompt(parser);
}

static void handle_post_prompt(struct annotations_parser *parser)
{
    parser->at_prompt = false;
}

static void handle_error_begin(struct annotations_parser *parser)
{
    send_available_console_output(parser); 

    /* After a signal is sent (^c), the debugger will then output 
     * something like "Quit\n", so that should be displayed to the user.
     *
     * Unfortunately, the debugger ( gdb ) isn't nice enough to return a 
     * post-prompt when a signal is received.
     */
    parser->at_prompt = false;
    parser->at_pre_prompt = false;
    parser->at_misc_prompt = false;
    parser->at_error_message = true;
}

static void handle_error(struct annotations_parser *parser)
{
    send_available_console_output(parser); 

    parser->at_prompt = false;
    parser->at_pre_prompt = false;
    parser->at_misc_prompt = false;
    parser->at_error_message = false;

    parser->callbacks.command_error_callback(
            parser->callbacks.context, parser->error_message);
    parser->error_message.clear();
}

static void handle_quit(struct annotations_parser *parser)
{
    send_available_console_output(parser); 

    parser->at_prompt = false;
    parser->at_pre_prompt = false;
    parser->at_misc_prompt = false;
    parser->at_error_message = false;

    parser->callbacks.command_error_callback(
            parser->callbacks.context, parser->error_message);
    parser->error_message.clear();
}

static void handle_exited(struct annotations_parser *parser)
{
}

/**
 * The main annotation data structure.
 * It represents all of the supported annotataions that can be parsed.
 */
static struct annotation {

    /**
	 * The name of the annotation.
	 */
    const char *data;

    /**
	 * The function to call when the annotation is found.
	 */
    void (*f) (struct annotations_parser *parser);
} annotations[] = {
    {
    "breakpoints-invalid", handle_breakpoints_invalid}, {

    "source", handle_source}, {
    "frame-end", handle_frame_end }, {
    "frames-invalid", handle_frames_invalid }, {

    /**
     * To see the commands annotation appear, set a breakpoint
     * at main and then run 'commands 1'.
     */
    "pre-commands", handle_misc_pre_prompt}, {
    "commands", handle_misc_prompt}, {
    "post-commands", handle_misc_post_prompt}, {

    /**
     * To see the overloaded-choice annotation appear, set the
     * option, set multiple-symbols ask, and then request that gdb
     * set a breakpoint on an overloaded function in c++.
     */
    "pre-overload-choice", handle_misc_pre_prompt}, {
    "overload-choice", handle_misc_prompt}, {
    "post-overload-choice", handle_misc_post_prompt}, {

    /** 
     * The instance-choice annotation comes from the GNAT version of GDB.
     * I don't have steps to reproduce it but I believe it comes up when
     * you want to choose an instance of a package to operate on.
     *
     * CGDB should behave the same way as the overload-choice here.
     */
    "pre-instance-choice", handle_misc_pre_prompt}, {
    "instance-choice", handle_misc_prompt}, {
    "post-instance-choice", handle_misc_post_prompt}, {

    /**
     * To see the query annotations appear, set a breakpoint at main
     * and then run to it. Run again.
     */
    "pre-query", handle_misc_pre_prompt}, {
    "query", handle_misc_prompt}, {
    "post-query", handle_misc_post_prompt}, {

    /**
     * To see the prompt-for-continue annotation appear, set the
     * option, set height 2, then set a breakpoint at main and run to it.
     *
     * This should never come up for CGDB as CGDB sets the height to 0
     * when starting GDB, hopefully preventing this.
     */
    "pre-prompt-for-continue", handle_misc_pre_prompt}, {
    "prompt-for-continue", handle_misc_prompt}, {
    "post-prompt-for-continue", handle_misc_post_prompt}, {

    /**
     * The prompt annotations appear whenever GDB is ready for input.
     */
    "pre-prompt", handle_pre_prompt}, {
    "prompt", handle_prompt}, {
    "post-prompt", handle_post_prompt}, {

    /**
     * The error-begin may appear before an error or quit annotation.
     *
     * Any text between the error-begin and the error or quit annotation
     * is the error message.
     */
    "error-begin", handle_error_begin}, {

    /**
     * An error occurred in the command being executed by GDB.
     *
     * This has no direct error message. It would be between
     * the error-begin annotation and this one.
     */
    "error", handle_error}, {

     /**
      * This occurs when gdb responds to an interupt (ctrl-c).
      *
      * This has no direct error message. It would be between
      * the error-begin annotation and this one.
      */
    "quit", handle_quit}, {

    "exited", handle_exited}, {
    NULL, NULL}
};

static void parse_annotation(struct annotations_parser *parser)
{
    int i;

    std::string annotation_only = parser->annotation_text.substr(0, 
        parser->annotation_text.find(' '));

    for (i = 0; annotations[i].data != NULL; ++i) {
        if (annotation_only == annotations[i].data) {
            annotations[i].f(parser);
        }
    }
}

static void process_output(struct annotations_parser *parser, char c)
{
    if (parser->at_error_message) {
        parser->error_message.push_back(c);
    } else if (parser->at_pre_prompt) {
        parser->gdb_prompt.push_back(c);
    } else {
        parser->console_output.push_back(c);
    }
}

int annotations_parser_io(annotations_parser *parser, char *str, size_t size)
{
    int result = 0;
    size_t i;

    for (i = 0; i < size; ++i) {
        /* Ignore all car returns outputted by gdb */
        if (str[i] == '\r')
            continue;

        switch (parser->state) {
            case ANNOTATION_GDB_DATA:
                if (str[i] == '\n') {
                    parser->state = ANNOTATION_NEW_LINE;
                } else {
                    process_output(parser, str[i]);
                }
                break;
            case ANNOTATION_NEW_LINE:
                if (str[i] == '\032') {
                    parser->state = ANNOTATION_CONTROL_Z;
                }  else {
                    process_output(parser, '\n');
                    if (str[i] == '\n') {
                        // Transition to ANNOTATION_NEW_LINE; nothing to do
                    } else {
                        process_output(parser, str[i]);
                        parser->state = ANNOTATION_GDB_DATA;
                    }
                }
                break;
            case ANNOTATION_CONTROL_Z:
                if (str[i] == '\032') {
                    parser->state = ANNOTATION_TEXT;
                }  else {
                    process_output(parser, '\n');
                    process_output(parser, '\032');

                    if (str[i] == '\n') {
                        parser->state = ANNOTATION_NEW_LINE;
                    } else {
                        process_output(parser, str[i]);
                        parser->state = ANNOTATION_GDB_DATA;
                    }
                }
                break;
            case ANNOTATION_TEXT:
                if (str[i] == '\n') {
                    parse_annotation(parser);
                    parser->state = ANNOTATION_GDB_DATA;
                    parser->annotation_text.clear();

                } else {
                    parser->annotation_text.push_back(str[i]);
                }
                break;
        }
    }

    send_available_console_output(parser);

    return result;
}

