#ifndef __GDBMI_IL_H__
#define __GDBMI_IL_H__

typedef struct output *output_ptr;
typedef struct oob_record *oob_record_ptr;
typedef struct result_record *result_record_ptr;
typedef long token_t;
enum result_class;
typedef struct result *result_ptr;
typedef struct async_record *async_record_ptr;
typedef struct stream_record *stream_record_ptr;
enum async_class;
typedef struct async_output *async_output_ptr;
typedef struct value *value_ptr;
typedef struct tuple *tuple_ptr;
typedef struct list *list_ptr;

enum result_class {
	GDBMI_DONE,
	GDBMI_RUNNING,
	GDBMI_CONNECTED,
	GDBMI_ERROR,
	GDBMI_EXIT
};

struct output {
	oob_record_ptr oob_record;
	result_record_ptr result_record;
};

struct result_record {
	token_t token;
	enum result_class result_class;
	result_ptr result;
};

enum oob_record_kind {
	GDBMI_ASYNC,
	GDBMI_STREAM
};

struct oob_record {
	enum oob_record_kind record;
	union {
		async_record_ptr async_record;
		stream_record_ptr stream_record;
	} variant;
};

enum async_record_kind {
	GDBMI_STATUS,
	GDBMI_EXEC,
	GDBMI_NOTIFY
};

enum stream_record_kind {
	GDBMI_CONSOLE,
	GDBMI_TARGET,
	GDBMI_LOG
};

struct async_record {
	token_t token;
	enum async_record_kind async_record;
	async_output_ptr async_output;
};

enum async_class {
	GDBMI_STOPPED
};

struct async_output {
	enum async_class async_class;
	result_ptr result_ptr;
};

struct result {
	const char *variable;
	value_ptr value;
};

enum value_kind {
	GDBMI_CSTRING,
	GDBMI_TUPLE,
	GDBMI_LIST
};

struct value {
	enum value_kind value_kind;
	union {
		const char *cstring;
		tuple_ptr tuple;
		list_ptr list;
	} variant;
};

struct tuple {
	result_ptr result;
};

enum list_kind {
	GDBMI_VALUE,
	GDBMI_RESULT
};


struct list {
	enum list_kind list;
	
	union {
		value_ptr value;
		result_ptr result;
	} variant;
};

struct stream_record {
	enum stream_record_kind stream_record;
	const char *cstring;
};

void* append_to_list ( void *list, void*item );

#endif
