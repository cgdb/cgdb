#include "annotations.h"
#include "sys_util.h"
#include "gdbwire.h"

struct annotations_parser {
    /** The client parser callbacks */
    annotations_parser_callbacks callbacks;

    /* The gdbwire/Annotation parser callbacks */
    gdbwire_annotation_parser_callbacks parser_callbacks;

    /** The gdbwire/Annotation parser */
    gdbwire_annotation_parser *parser;

    /** The debugger's current prompt and last prompt. */
    std::string gdb_prompt, gdb_prompt_last;

    /** The last annotation recieved from gdb. */
    enum gdbwire_annotation_kind last;

    /**
     * True if the breakpoints have changed, false otherwise.
     */
    bool breakpoints_changed;

    /**
     * True if the source location has changed, false otherwise.
     */
    bool source_location_changed;
};

bool annotations_parser_at_prompt(annotations_parser *parser)
{
    return
            parser->last == GDBWIRE_ANNOTATION_PROMPT ||
            parser->last == GDBWIRE_ANNOTATION_COMMANDS ||
            parser->last == GDBWIRE_ANNOTATION_OVERLOAD_CHOICE ||
            parser->last == GDBWIRE_ANNOTATION_INSTANCE_CHOICE ||
            parser->last == GDBWIRE_ANNOTATION_QUERY ||
            parser->last == GDBWIRE_ANNOTATION_PROMPT_FOR_CONTINUE;
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

static void handle_misc_prompt(struct annotations_parser *parser)
{
    update_prompt(parser);
    console_at_prompt(parser);
}

static void handle_pre_prompt(struct annotations_parser *parser)
{
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
    update_prompt(parser);
    console_at_prompt(parser);
}

int annotations_parser_io(annotations_parser *parser, char *str, size_t size)
{
    enum gdbwire_result result;

    result = gdbwire_annotation_parser_push_data(parser->parser, str, size);

    return result == GDBWIRE_OK;
}

static void annotations_output_callback(void *context,
        struct gdbwire_annotation_output *output)
{
    annotations_parser *parser = (annotations_parser *)context;

    switch (output->kind) {
        case GDBWIRE_ANNOTATION_OUTPUT_CONSOLE_OUTPUT: {
            enum gdbwire_annotation_kind last =
                    output->variant.console_output.last;

            int at_pre_prompt =
                    last == GDBWIRE_ANNOTATION_PRE_PROMPT ||
                    last == GDBWIRE_ANNOTATION_PRE_COMMANDS ||
                    last == GDBWIRE_ANNOTATION_PRE_OVERLOAD_CHOICE ||
                    last == GDBWIRE_ANNOTATION_PRE_INSTANCE_CHOICE ||
                    last == GDBWIRE_ANNOTATION_PRE_QUERY ||
                    last == GDBWIRE_ANNOTATION_PRE_PROMPT_FOR_CONTINUE;

            /**
             * After a signal is sent (^c), the debugger will then output 
             * something like "Quit\n", so that should be displayed to the user.
             */
            int at_error_message = last == GDBWIRE_ANNOTATION_ERROR_BEGIN;

            if (at_error_message) {
                parser->callbacks.command_error_callback(
                        parser->callbacks.context,
                        output->variant.console_output.text);
            } else if (at_pre_prompt) {
                parser->gdb_prompt += output->variant.console_output.text;
            } else {
                parser->callbacks.console_output_callback(
                        parser->callbacks.context,
                        output->variant.console_output.text);
            }
            break;
        }
        case GDBWIRE_ANNOTATION_OUTPUT_ANNOTATION: {
            switch (output->variant.annotation.kind) {
                case GDBWIRE_ANNOTATION_BREAKPOINTS_INVALID:
                    handle_breakpoints_invalid(parser);
                    break;
                case GDBWIRE_ANNOTATION_SOURCE:
                    handle_source(parser);
                    break;
                case GDBWIRE_ANNOTATION_FRAME_END:
                    handle_frame_end(parser);
                    break;
                case GDBWIRE_ANNOTATION_FRAMES_INVALID:
                    handle_frames_invalid(parser);
                    break;
                case GDBWIRE_ANNOTATION_PRE_COMMANDS:
                case GDBWIRE_ANNOTATION_PRE_OVERLOAD_CHOICE:
                case GDBWIRE_ANNOTATION_PRE_INSTANCE_CHOICE:
                case GDBWIRE_ANNOTATION_PRE_QUERY:
                case GDBWIRE_ANNOTATION_PRE_PROMPT_FOR_CONTINUE:
                    break;
                case GDBWIRE_ANNOTATION_COMMANDS:
                case GDBWIRE_ANNOTATION_OVERLOAD_CHOICE:
                case GDBWIRE_ANNOTATION_INSTANCE_CHOICE:
                case GDBWIRE_ANNOTATION_QUERY:
                case GDBWIRE_ANNOTATION_PROMPT_FOR_CONTINUE:
                    handle_misc_prompt(parser);
                    break;
                case GDBWIRE_ANNOTATION_POST_COMMANDS:
                case GDBWIRE_ANNOTATION_POST_OVERLOAD_CHOICE:
                case GDBWIRE_ANNOTATION_POST_INSTANCE_CHOICE:
                case GDBWIRE_ANNOTATION_POST_QUERY:
                case GDBWIRE_ANNOTATION_POST_PROMPT_FOR_CONTINUE:
                    break;
                case GDBWIRE_ANNOTATION_PRE_PROMPT:
                    handle_pre_prompt(parser);
                    break;
                case GDBWIRE_ANNOTATION_PROMPT:
                    handle_prompt(parser);
                    break;
                case GDBWIRE_ANNOTATION_POST_PROMPT:
                    break;
                case GDBWIRE_ANNOTATION_ERROR_BEGIN:
                    break;
                case GDBWIRE_ANNOTATION_ERROR:
                    break;
                case GDBWIRE_ANNOTATION_QUIT:
                    break;
                case GDBWIRE_ANNOTATION_EXITED:
                    break;
                case GDBWIRE_ANNOTATION_UNKNOWN:
                    // Skip unknown annotations for now
                    clog_debug(CLOG_GDBIO, "Skipping unknown annotation [%s]",
                            output->variant.annotation.text);
                    break;
            }

            parser->last = output->variant.annotation.kind;
            break;
        }
    }
}

annotations_parser *annotations_parser_initialize(
    annotations_parser_callbacks callbacks)
{
    annotations_parser *a = new annotations_parser();
    a->callbacks = callbacks;

    a->parser_callbacks.context = a;
    a->parser_callbacks.gdbwire_annotation_output_callback =
            annotations_output_callback;
    a->parser = gdbwire_annotation_parser_create(a->parser_callbacks);
    a->breakpoints_changed = false;
    a->source_location_changed = false;
    return a;
}

void annotations_parser_shutdown(annotations_parser *parser)
{
    delete parser;
}

