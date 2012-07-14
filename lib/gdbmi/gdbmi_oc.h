#ifndef __GDBMI_OC_H__
#define __GDBMI_OC_H__

#include "gdbmi_pt.h"

struct gdbmi_oc;
typedef struct gdbmi_oc *gdbmi_oc_ptr;

/* The possible/implemented GDBMI commands */
enum gdbmi_input_command {
    /*  24.7 GDB/MI Program control */
    GDBMI_FILE_LIST_EXEC_SOURCE_FILE,
    GDBMI_FILE_LIST_EXEC_SOURCE_FILES,

    /*  24.5 GDB/MI Breakpoint table commands */
    GDBMI_BREAK_LIST,

    GDBMI_LAST
};

/* A cstring linked list for use by the gdbmi output commands */
struct gdbmi_oc_cstring_ll;
typedef struct gdbmi_oc_cstring_ll *gdbmi_oc_cstring_ll_ptr;
struct gdbmi_oc_cstring_ll {
    /* The cstring */
    char *cstring;

    /* A pointer to the next output  */
    gdbmi_oc_cstring_ll_ptr next;
};

/* A file path linked list, for use by the gdbmi output commands */
struct gdbmi_oc_file_path_info;
typedef struct gdbmi_oc_file_path_info *gdbmi_oc_file_path_info_ptr;
struct gdbmi_oc_file_path_info {
    /* The filename, relative path. */
    char *file;

    /* The fullname, absolute path. */
    char *fullname;

    /* A pointer to the next file path.  */
    gdbmi_oc_file_path_info_ptr next;
};

enum breakpoint_type {
    GDBMI_BREAKPOINT,
    GDBMI_WATCHPOINT
};

enum breakpoint_disposition {
    GDBMI_KEEP,
    GDBMI_NOKEEP
};

struct gdbmi_oc_breakpoint;
typedef struct gdbmi_oc_breakpoint *gdbmi_oc_breakpoint_ptr;
struct gdbmi_oc_breakpoint {
    int number;
    enum breakpoint_type type;
    enum breakpoint_disposition disposition;
    /* 1 if enabled, otherwise 0 */
    int enabled;
    char *address;
    char *func;
    char *file;
    char *fullname;
    int line;
    int times;

    gdbmi_oc_breakpoint_ptr next;
};

struct gdbmi_oc {
    /* If this is 1, then the command was asynchronous, otherwise it wasn't */
    int is_asynchronous;

    /* The console output. This is a null terminated list. */
    gdbmi_oc_cstring_ll_ptr console_output;

    /* The GDBMI output command this represents. If set to GDBMI_LAST,
     * then this is an asynchronous command. */
    enum gdbmi_input_command input_command;

    union {

        /*  24.7 GDB/MI Program control */
        struct {
            int line;
            char *file;
            char *fullname;
        } file_list_exec_source_file;

        struct {
            gdbmi_oc_file_path_info_ptr file_name_pair;
        } file_list_exec_source_files;

        /*  24.5 GDB/MI Breakpoint table commands */
        struct {
            gdbmi_oc_breakpoint_ptr breakpoint_ptr;
        } break_list;
    } input_commands;

    /* The next MI output command */
    gdbmi_oc_ptr next;
};

/**
 * This will take in a parse tree and return a list of MI output commands.
 *
 * \param output_ptr
 * The MI parse tree
 *
 * \param mi_input_cmds
 * The next MI input command.
 *
 * \param oc_ptr
 * On return, this will be the MI output commands that were derived from the 
 * parse tree.
 * 
 * \return
 * 0 on success, -1 on error.
 */
int
gdbmi_get_output_commands(gdbmi_output_ptr output_ptr,
        gdbmi_oc_cstring_ll_ptr mi_input_cmds, gdbmi_oc_ptr * oc_ptr);

/* Creating, Destroying and printing MI output commands  */
gdbmi_oc_ptr create_gdbmi_oc(void);
int destroy_gdbmi_oc(gdbmi_oc_ptr param);
gdbmi_oc_ptr append_gdbmi_oc(gdbmi_oc_ptr list, gdbmi_oc_ptr item);
int print_gdbmi_oc(gdbmi_oc_ptr param);

/* Creating, Destroying and printing MI cstring linked lists */
gdbmi_oc_cstring_ll_ptr create_gdbmi_cstring_ll(void);
int destroy_gdbmi_cstring_ll(gdbmi_oc_cstring_ll_ptr param);
gdbmi_oc_cstring_ll_ptr append_gdbmi_cstring_ll(gdbmi_oc_cstring_ll_ptr list,
        gdbmi_oc_cstring_ll_ptr item);
int print_gdbmi_cstring_ll(gdbmi_oc_cstring_ll_ptr param);

/* Creating, Destroying and printing MI file_path linked lists */
gdbmi_oc_file_path_info_ptr create_gdbmi_file_path_info(void);
int destroy_gdbmi_file_path_info(gdbmi_oc_file_path_info_ptr param);
gdbmi_oc_file_path_info_ptr
append_gdbmi_file_path_info(gdbmi_oc_file_path_info_ptr list,
        gdbmi_oc_file_path_info_ptr item);
int print_gdbmi_file_path_info(gdbmi_oc_file_path_info_ptr param);

/* Creating, Destroying and printing MI breakpoint_list linked lists */
gdbmi_oc_breakpoint_ptr create_gdbmi_breakpoint(void);
int destroy_gdbmi_breakpoint(gdbmi_oc_breakpoint_ptr param);
gdbmi_oc_breakpoint_ptr append_gdbmi_breakpoint(gdbmi_oc_breakpoint_ptr list,
        gdbmi_oc_breakpoint_ptr item);
int print_gdbmi_breakpoint(gdbmi_oc_breakpoint_ptr param);

#endif /* __GDBMI_OC_H__ */
