#include "gdbmi_parse.h"

enum async_record {
	/*
	 * contains on-going status information about the progress 
	 * of a slow operation. It can be discarded. All status output 
	 * is prefixed by `+'.
	 */
	STATUS_ASYNC_OUTPUT,

	/*
	 * contains asynchronous state change on the target 
	 * (stopped, started, disappeared). 
	 * All async output is prefixed by `*'.
	 */
	EXEC_ASYNC_OUTPUT,

	/*
	 * contains supplementary information that the client should handle 
	 * (e.g., a new breakpoint information). All notify output is prefixed 
	 * by `='.
	 */
	NOTIFY_ASYNC_OUTPUT
};

enum stream_record {
	/*
	 * Output that should be displayed as is in the console. 
	 * It is the textual response to a CLI command. 
	 * All the console output is prefixed by `~'
	 */
	CONSOLE_STREAM_OUTPUT,

	/*
	 * Output produced by the target program. 
	 * All the target output is prefixed by `@'.
	 */
	TARGET_STREAM_OUTPUT,

	/*
	 * Output text coming from GDB's internals, for instance messages 
	 * that should be displayed as part of an error log. 
	 * All the log output is prefixed by `&'.
	 */
	LOG_STREAM_OUTPUT
};

int gdbmi_parse(char *data, size_t size, char *gui_data, size_t gui_size, struct queue *q) {
    int i;
    for(i = 0; i < size; ++i)
        gui_data[i] = data[i];
    return size;
}
