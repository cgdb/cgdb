#ifndef __GDBMI_IL_H__
#define __GDBMI_IL_H__

typedef struct output *output_ptr;
typedef struct oob_record *oob_record_ptr;
typedef struct result_record *result_record_ptr;
typedef long token_t;
typedef struct result *result_ptr;
typedef struct async_record *async_record_ptr;
typedef struct stream_record *stream_record_ptr;
typedef struct value *value_ptr;
typedef struct tuple *tuple_ptr;
typedef struct list *list_ptr;

/**
 * A kind of result's that GDB is capable of producing.
 */
enum result_class {
	GDBMI_DONE,
	GDBMI_RUNNING,
	GDBMI_CONNECTED,
	GDBMI_ERROR,
	GDBMI_EXIT
};

/**
 * This is the root of a parsed GDB/MI Output command.
 */
struct output {
	/**
	 * Every output command has a list of optional oob_record's.
	 * This will be the head of the list, otherwise NULL.
	 */
	oob_record_ptr oob_record;

	/**
	 * Every output command has an optional result_record.
	 * list, otherwise NULL.
	 */
	result_record_ptr result_record;

	/** A pointer to the next output */
	struct output *next;
};

/**
 * A result record represents the result of a command sent to GDB.
 */
struct result_record {
	/**
	 * This is the unique identifier that was passed to GDB when
	 * asking for a command to be done on behalf of the front end.
	 */
	token_t token;

	/** The kind of result that this class represents */
	enum result_class result_class;

	/** The results of the command. */
	result_ptr result;
};

/**
 * There are several kinds of output that GDB can send.
 *
 * \GDBMI_ASYNC
 * I believe that the asyncronous records contain data that was not asked
 * for by the front end. An asyncronous event occured within the inferior
 * or GDB and GDB needs to update the front end.
 *
 * For instance, I believe this is useful for when breakpoints are modified, 
 * instead of having the front end ask if the breakpoints were modified 
 * every time.
 *
 * \GDBMI_STREAM
 * This is the result of normal output from the console, target or GDB.
 */
enum oob_record_kind {
	GDBMI_ASYNC,
	GDBMI_STREAM
};

/**
 * This is an out of band record.
 */
struct oob_record {
	/**
	 * This is the kind of oob_record
	 */
	enum oob_record_kind record;
	union {
		 /** If it's an GDBMI_ASYNC record */
		async_record_ptr async_record;
		 /** If it's an GDBMI_STREAM record */
		stream_record_ptr stream_record;
	} variant;

	/** A pointer to the next oob_record */
	struct oob_record *next;
};

/**
 * This represents each kind of asyncronous record GDB is capable of sending. 
 */
enum async_record_kind {
	/** 
	 * contains on-going status information about the progress 
	 * of a slow operation. It can be discarded. 
	 */
	GDBMI_STATUS,
	/** 
	 * contains asynchronous state change on the target 
	 * (stopped, started, disappeared). 
	 */
	GDBMI_EXEC,
	
	/** 
	 * contains supplementary information that the client should handle 
	 * (e.g., a new breakpoint information). 
	 */
	GDBMI_NOTIFY
};

/**
 * This represents each kind of stream record GDB is capable of sending. 
 */
enum stream_record_kind {
	/** 
	 * Output that should be displayed as is in the console. 
	 * It is the textual response to a CLI command. 
	 */
	GDBMI_CONSOLE,
	/** 
	 * Output produced by the target program. 
	 */
	GDBMI_TARGET,
	/** 
	 * Output text coming from GDB's internals, for instance messages 
	 * that should be displayed as part of an error log. 
	 */
	GDBMI_LOG
};

enum async_class {
	GDBMI_STOPPED
};

/**
 * An asyncronous record
 */
struct async_record {
	/**
	 * This is the unique identifier that was passed to GDB when
	 * asking for a command to be done on behalf of the front end.
	 */
	token_t token;

	/** The kind of asyncronous record this represents */
	enum async_record_kind async_record;

	enum async_class async_class;

	result_ptr result;
};

/**
 * The result from GDB. This is a linked list.
 * If the result is a key/value pair, then 'variable' is the key
 * and 'value' is the value.
 */
struct result {
	/** Key */
	char *variable;
	/** Value */
	value_ptr value;
	/** Pointer to the next result */
	struct result *next;
};

enum value_kind {
	GDBMI_CSTRING,
	GDBMI_TUPLE,
	GDBMI_LIST
};

struct value {
	enum value_kind value_kind;

	union {
		char *cstring;
		tuple_ptr tuple;
		list_ptr list;
	} variant;

	struct value *next;
};

struct tuple {
	result_ptr result;
	struct tuple *next;
};

enum list_kind {
	GDBMI_VALUE,
	GDBMI_RESULT
};


struct list {
	enum list_kind list_kind;
	
	union {
		value_ptr value;
		result_ptr result;
	} variant;

	struct list *next;
};

struct stream_record {
	enum stream_record_kind stream_record;
	char *cstring;
};

/* Print result class */
int print_result_class ( enum result_class param );

/* Creating, Destroying and printing output */
output_ptr create_output ( void );
int destroy_output ( output_ptr param );
output_ptr append_output ( output_ptr list, output_ptr item );
int print_output ( output_ptr param );

/* Creating, Destroying and printing record */
result_record_ptr create_result_record ( void );
int destroy_result_record ( result_record_ptr param );
int print_result_record ( result_record_ptr param );

/* Creating, Destroying and printing result */
result_ptr create_result ( void );
int destroy_result ( result_ptr param );
result_ptr append_result ( result_ptr list, result_ptr item );
int print_result ( result_ptr param );

int print_oob_record_kind ( enum oob_record_kind param );

/* Creating, Destroying and printing oob_record */
oob_record_ptr create_oob_record ( void );
int destroy_oob_record ( oob_record_ptr param );
oob_record_ptr append_oob_record ( oob_record_ptr list, oob_record_ptr item );
int print_oob_record ( oob_record_ptr param );

int print_async_record_kind ( enum async_record_kind param );

int print_stream_record_kind ( enum stream_record_kind param );

/* Creating, Destroying and printing async_record */
async_record_ptr create_async_record ( void );
int destroy_async_record ( async_record_ptr param );
int print_async_record ( async_record_ptr param );

int print_async_class ( enum async_class param );

int print_value_kind ( enum value_kind param );

/* Creating, Destroying and printing value */
value_ptr create_value ( void );
int destroy_value ( value_ptr param );
value_ptr append_value ( value_ptr list, value_ptr item );
int print_value ( value_ptr param );

/* Creating, Destroying and printing tuple */
tuple_ptr create_tuple ( void );
int destroy_tuple ( tuple_ptr param );
int print_tuple ( tuple_ptr param );

/* Creating, Destroying and printing list */
list_ptr create_list ( void );
int destroy_list ( list_ptr param );
list_ptr append_list ( list_ptr list, list_ptr item );
int print_list ( list_ptr param );

/* Creating, Destroying and printing stream_record */
stream_record_ptr create_stream_record ( void );
int destroy_stream_record ( stream_record_ptr param );
int print_stream_record ( stream_record_ptr param );

#endif
