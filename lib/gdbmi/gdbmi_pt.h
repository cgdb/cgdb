#ifndef __GDBMI_PT_H__
#define __GDBMI_PT_H__

typedef struct gdbmi_output *gdbmi_output_ptr;
typedef struct gdbmi_oob_record *gdbmi_oob_record_ptr;
typedef struct gdbmi_result_record *gdbmi_result_record_ptr;
typedef long gdbmi_token_t;
typedef struct gdbmi_result *gdbmi_result_ptr;
typedef struct gdbmi_async_record *gdbmi_async_record_ptr;
typedef struct gdbmi_stream_record *gdbmi_stream_record_ptr;
typedef struct gdbmi_value *gdbmi_value_ptr;
typedef struct gdbmi_tuple *gdbmi_tuple_ptr;
typedef struct gdbmi_list *gdbmi_list_ptr;
typedef struct gdbmi_pdata *gdbmi_pdata_ptr;

struct gdbmi_pdata {
    int parsed_one;
    gdbmi_output_ptr tree;
};

/* A choice of result's that GDB is capable of producing  */
enum gdbmi_result_class {
    GDBMI_DONE,
    GDBMI_RUNNING,
    GDBMI_CONNECTED,
    GDBMI_ERROR,
    GDBMI_EXIT
};

/* This is the root of a parsed GDB/MI Output command.  */
struct gdbmi_output {
    /* Every output command has a list of optional oob_record's.  This will be 
       the head of the list, otherwise NULL.  */
    gdbmi_oob_record_ptr oob_record;

    /* Every output command has an optional result_record list, or NULL.  */
    gdbmi_result_record_ptr result_record;

    /* A pointer to the next output  */
    gdbmi_output_ptr next;
};

/* A result record represents the result of a command sent to GDB.  */
struct gdbmi_result_record {
    /* This is the unique identifier that was passed to GDB when asking for a 
       command to be done on behalf of the front end  */
    gdbmi_token_t token;

    /* The choice of result that this class represents  */
    enum gdbmi_result_class result_class;

    /* The results of the command  */
    gdbmi_result_ptr result;
};

/* There are several kinds of output that GDB can send  */
enum gdbmi_oob_record_choice {
    /* GDBMI_ASYNC
       I believe that the asyncronous records contain data that was not asked
       for by the front end. An asyncronous event occured within the inferior
       or GDB and GDB needs to update the front end.

       For instance, I believe this is useful for when breakpoints are modified, 
       instead of having the front end ask if the breakpoints were modified 
       every time.  */
    GDBMI_ASYNC,
    /* GDBMI_STREAM
       This is the result of normal output from the console, target or GDB.  */
    GDBMI_STREAM
};

/* This is an out of band record.  */
struct gdbmi_oob_record {
    /* This is the choice of oob_record  */
    enum gdbmi_oob_record_choice record;
    union {
        /* If it's an GDBMI_ASYNC record  */
        gdbmi_async_record_ptr async_record;
        /* If it's an GDBMI_STREAM record  */
        gdbmi_stream_record_ptr stream_record;
    } option;

    /* A pointer to the next oob_record  */
    gdbmi_oob_record_ptr next;
};

/* This represents each choice of asyncronous record GDB is capable of 
   sending.  */
enum gdbmi_async_record_choice {
    /* Contains on-going status information about the progress of a slow 
       operation. It can be discarded.  */
    GDBMI_STATUS,
    /* Contains asynchronous state change on the target (stopped, started, 
       disappeared).  */
    GDBMI_EXEC,

    /* Contains supplementary information that the client should handle 
       (e.g., a new breakpoint information).  */
    GDBMI_NOTIFY
};

/* This represents each choice of stream record GDB is capable of sending.  */
enum gdbmi_stream_record_choice {
    /* Output that should be displayed as is in the console. It is the textual 
       response to a CLI command.  */
    GDBMI_CONSOLE,
    /* Output produced by the target program.  */
    GDBMI_TARGET,
    /* Output text coming from GDB's internals, for instance messages 
     * that should be displayed as part of an error log.  */
    GDBMI_LOG
};

enum gdbmi_async_class {
    GDBMI_STOPPED
};

/* An asyncronous record  */
struct gdbmi_async_record {
    /* This is the unique identifier that was passed to GDB when asking for a 
       command to be done on behalf of the front end.  */
    gdbmi_token_t token;

    /* The choice of asyncronous record this represents  */
    enum gdbmi_async_record_choice async_record;

    enum gdbmi_async_class async_class;

    gdbmi_result_ptr result;
};

/* The result from GDB. This is a linked list. If the result is a key/value 
   pair, then 'variable' is the key and 'value' is the value.  */
struct gdbmi_result {
    /* Key  */
    char *variable;
    /* Value  */
    gdbmi_value_ptr value;
    /* Pointer to the next result  */
    gdbmi_result_ptr next;
};

enum gdbmi_value_choice {
    GDBMI_CSTRING,
    GDBMI_TUPLE,
    GDBMI_LIST
};

struct gdbmi_value {
    enum gdbmi_value_choice value_choice;

    union {
        char *cstring;
        gdbmi_tuple_ptr tuple;
        gdbmi_list_ptr list;
    } option;

    gdbmi_value_ptr next;
};

struct gdbmi_tuple {
    gdbmi_result_ptr result;
    gdbmi_tuple_ptr next;
};

enum gdbmi_list_choice {
    GDBMI_VALUE,
    GDBMI_RESULT
};

struct gdbmi_list {
    enum gdbmi_list_choice list_choice;

    union {
        gdbmi_value_ptr value;
        gdbmi_result_ptr result;
    } option;

    gdbmi_list_ptr next;
};

struct gdbmi_stream_record {
    enum gdbmi_stream_record_choice stream_record;
    char *cstring;
};

/* Print result class  */
int print_gdbmi_result_class(enum gdbmi_result_class param);

/* Creating and  Destroying */
gdbmi_pdata_ptr create_gdbmi_pdata(void);
int destroy_gdbmi_pdata(gdbmi_pdata_ptr param);

/* Creating, Destroying and printing output  */
gdbmi_output_ptr create_gdbmi_output(void);
int destroy_gdbmi_output(gdbmi_output_ptr param);
gdbmi_output_ptr append_gdbmi_output(gdbmi_output_ptr list,
        gdbmi_output_ptr item);
int print_gdbmi_output(gdbmi_output_ptr param);

/* Creating, Destroying and printing record  */
gdbmi_result_record_ptr create_gdbmi_result_record(void);
int destroy_gdbmi_result_record(gdbmi_result_record_ptr param);
int print_gdbmi_result_record(gdbmi_result_record_ptr param);

/* Creating, Destroying and printing result  */
gdbmi_result_ptr create_gdbmi_result(void);
int destroy_gdbmi_result(gdbmi_result_ptr param);
gdbmi_result_ptr append_gdbmi_result(gdbmi_result_ptr list,
        gdbmi_result_ptr item);
int print_gdbmi_result(gdbmi_result_ptr param);

int print_gdbmi_oob_record_choice(enum gdbmi_oob_record_choice param);

/* Creating, Destroying and printing oob_record  */
gdbmi_oob_record_ptr create_gdbmi_oob_record(void);
int destroy_gdbmi_oob_record(gdbmi_oob_record_ptr param);
gdbmi_oob_record_ptr append_gdbmi_oob_record(gdbmi_oob_record_ptr list,
        gdbmi_oob_record_ptr item);
int print_gdbmi_oob_record(gdbmi_oob_record_ptr param);

int print_gdbmi_async_record_choice(enum gdbmi_async_record_choice param);

int print_gdbmi_stream_record_choice(enum gdbmi_stream_record_choice param);

/* Creating, Destroying and printing async_record  */
gdbmi_async_record_ptr create_gdbmi_async_record(void);
int destroy_gdbmi_async_record(gdbmi_async_record_ptr param);
int print_gdbmi_async_record(gdbmi_async_record_ptr param);

int print_gdbmi_async_class(enum gdbmi_async_class param);

int print_gdbmi_value_choice(enum gdbmi_value_choice param);

/* Creating, Destroying and printing value  */
gdbmi_value_ptr create_gdbmi_value(void);
int destroy_gdbmi_value(gdbmi_value_ptr param);
gdbmi_value_ptr append_gdbmi_value(gdbmi_value_ptr list, gdbmi_value_ptr item);
int print_gdbmi_value(gdbmi_value_ptr param);

/* Creating, Destroying and printing tuple  */
gdbmi_tuple_ptr create_gdbmi_tuple(void);
int destroy_gdbmi_tuple(gdbmi_tuple_ptr param);
int print_gdbmi_tuple(gdbmi_tuple_ptr param);

int print_gdbmi_list_choice(enum gdbmi_list_choice param);

/* Creating, Destroying and printing list  */
gdbmi_list_ptr create_gdbmi_list(void);
int destroy_gdbmi_list(gdbmi_list_ptr param);
gdbmi_list_ptr append_gdbmi_list(gdbmi_list_ptr list, gdbmi_list_ptr item);
int print_gdbmi_list(gdbmi_list_ptr param);

/* Creating, Destroying and printing stream_record  */
gdbmi_stream_record_ptr create_gdbmi_stream_record(void);
int destroy_gdbmi_stream_record(gdbmi_stream_record_ptr param);
int print_gdbmi_stream_record(gdbmi_stream_record_ptr param);

#endif
