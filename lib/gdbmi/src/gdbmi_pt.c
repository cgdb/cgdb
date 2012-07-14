#include <stdio.h>
#include <stdlib.h>

#include "gdbmi_pt.h"

int print_token(long l)
{
    if (l == -1)
        return 0;
    printf("token(%ld)", l);
    return 0;
}

/* Print result class  */
int print_gdbmi_result_class(enum gdbmi_result_class param)
{
    switch (param) {
        case GDBMI_DONE:
            printf("GDBMI_DONE\n");
            break;
        case GDBMI_RUNNING:
            printf("GDBMI_RUNNING\n");
            break;
        case GDBMI_CONNECTED:
            printf("GDBMI_CONNECTED\n");
            break;
        case GDBMI_ERROR:
            printf("GDBMI_ERROR\n");
            break;
        case GDBMI_EXIT:
            printf("EXIT\n");
            break;
        default:
            return -1;
    };

    return 0;
}

/* Creating and  Destroying */
gdbmi_pdata_ptr create_gdbmi_pdata(void)
{
    return calloc(1, sizeof (struct gdbmi_pdata));
}

int destroy_gdbmi_pdata(gdbmi_pdata_ptr param)
{
    if (!param)
        return 0;

    free(param);

    return 0;
}

/* Creating, Destroying and printing gdbmi_output  */
gdbmi_output_ptr create_gdbmi_output(void)
{
    return calloc(1, sizeof (struct gdbmi_output));
}

int destroy_gdbmi_output(gdbmi_output_ptr param)
{
    if (!param)
        return 0;

    if (destroy_gdbmi_oob_record(param->oob_record) == -1)
        return -1;
    param->oob_record = NULL;

    if (destroy_gdbmi_result_record(param->result_record) == -1)
        return -1;
    param->result_record = NULL;

    if (destroy_gdbmi_output(param->next) == -1)
        return -1;
    param->next = NULL;

    free(param);
    param = NULL;
    return 0;
}

gdbmi_output_ptr
append_gdbmi_output(gdbmi_output_ptr list, gdbmi_output_ptr item)
{
    if (!item)
        return NULL;

    if (!list)
        list = item;
    else {
        gdbmi_output_ptr cur = list;

        while (cur->next)
            cur = cur->next;

        cur->next = item;
    }

    return list;
}

int print_gdbmi_output(gdbmi_output_ptr param)
{
    gdbmi_output_ptr cur = param;
    int result;

    while (cur) {
        result = print_gdbmi_oob_record(cur->oob_record);
        if (result == -1)
            return -1;

        result = print_gdbmi_result_record(cur->result_record);
        if (result == -1)
            return -1;

        cur = cur->next;
    }

    return 0;
}

/* Creating, Destroying and printing record  */
gdbmi_result_record_ptr create_gdbmi_result_record(void)
{
    return calloc(1, sizeof (struct gdbmi_result_record));
}

int destroy_gdbmi_result_record(gdbmi_result_record_ptr param)
{
    if (!param)
        return 0;

    if (destroy_gdbmi_result(param->result) == -1)
        return -1;
    param->result = NULL;

    free(param);
    param = NULL;
    return 0;
}

int print_gdbmi_result_record(gdbmi_result_record_ptr param)
{
    int result;

    if (!param)
        return 0;

    result = print_token(param->token);
    if (result == -1)
        return -1;

    result = print_gdbmi_result_class(param->result_class);
    if (result == -1)
        return -1;

    result = print_gdbmi_result(param->result);
    if (result == -1)
        return -1;

    return 0;
}

/* Creating, Destroying and printing result  */
gdbmi_result_ptr create_gdbmi_result(void)
{
    return calloc(1, sizeof (struct gdbmi_result_record));
}

int destroy_gdbmi_result(gdbmi_result_ptr param)
{
    if (!param)
        return 0;

    if (param->variable) {
        free(param->variable);
        param->variable = NULL;
    }

    if (destroy_gdbmi_value(param->value) == -1)
        return -1;
    param->value = NULL;

    if (destroy_gdbmi_result(param->next) == -1)
        return -1;
    param->next = NULL;

    free(param);
    param = NULL;
    return 0;
}

gdbmi_result_ptr
append_gdbmi_result(gdbmi_result_ptr list, gdbmi_result_ptr item)
{
    if (!item)
        return NULL;

    if (!list)
        list = item;
    else {
        gdbmi_result_ptr cur = list;

        while (cur->next)
            cur = cur->next;

        cur->next = item;
    }

    return list;
}

int print_gdbmi_result(gdbmi_result_ptr param)
{
    gdbmi_result_ptr cur = param;
    int result;

    while (cur) {
        printf("variable->(%s)\n", cur->variable);

        result = print_gdbmi_value(cur->value);
        if (result == -1)
            return -1;

        cur = cur->next;
    }

    return 0;
}

int print_gdbmi_oob_record_choice(enum gdbmi_oob_record_choice param)
{
    switch (param) {
        case GDBMI_ASYNC:
            printf("GDBMI_ASYNC\n");
            break;
        case GDBMI_STREAM:
            printf("GDBMI_STREAM\n");
            break;
        default:
            return -1;
    };

    return 0;
}

/* Creating, Destroying and printing oob_record  */
gdbmi_oob_record_ptr create_gdbmi_oob_record(void)
{
    return calloc(1, sizeof (struct gdbmi_oob_record));
}

int destroy_gdbmi_oob_record(gdbmi_oob_record_ptr param)
{
    if (!param)
        return 0;

    if (param->record == GDBMI_ASYNC) {
        if (destroy_gdbmi_async_record(param->option.async_record) == -1)
            return -1;
        param->option.async_record = NULL;
    } else if (param->record == GDBMI_STREAM) {
        if (destroy_gdbmi_stream_record(param->option.stream_record) == -1)
            return -1;
        param->option.stream_record = NULL;
    } else {
        return -1;
    }

    if (destroy_gdbmi_oob_record(param->next) == -1)
        return -1;
    param->next = NULL;

    free(param);
    param = NULL;
    return 0;
}

gdbmi_oob_record_ptr
append_gdbmi_oob_record(gdbmi_oob_record_ptr list, gdbmi_oob_record_ptr item)
{
    if (!item)
        return NULL;

    if (!list)
        list = item;
    else {
        gdbmi_oob_record_ptr cur = list;

        while (cur->next)
            cur = cur->next;

        cur->next = item;
    }

    return list;
}

int print_gdbmi_oob_record(gdbmi_oob_record_ptr param)
{
    gdbmi_oob_record_ptr cur = param;
    int result;

    while (cur) {
        result = print_gdbmi_oob_record_choice(cur->record);
        if (result == -1)
            return -1;

        if (cur->record == GDBMI_ASYNC) {
            result = print_gdbmi_async_record(cur->option.async_record);
            if (result == -1)
                return -1;
        } else if (cur->record == GDBMI_STREAM) {
            result = print_gdbmi_stream_record(cur->option.stream_record);
            if (result == -1)
                return -1;
        } else
            return -1;

        cur = cur->next;
    }

    return 0;
}

int print_gdbmi_async_record_choice(enum gdbmi_async_record_choice param)
{
    switch (param) {
        case GDBMI_STATUS:
            printf("GDBMI_STATUS\n");
            break;
        case GDBMI_EXEC:
            printf("GDBMI_EXEC\n");
            break;
        case GDBMI_NOTIFY:
            printf("GDBMI_NOTIFY\n");
            break;
        default:
            return -1;
    };

    return 0;
}

int print_gdbmi_stream_record_choice(enum gdbmi_stream_record_choice param)
{
    switch (param) {
        case GDBMI_CONSOLE:
            printf("GDBMI_CONSOLE\n");
            break;
        case GDBMI_TARGET:
            printf("GDBMI_TARGET\n");
            break;
        case GDBMI_LOG:
            printf("GDBMI_LOG\n");
            break;
        default:
            return -1;
    };

    return 0;
}

/* Creating, Destroying and printing async_record  */
gdbmi_async_record_ptr create_gdbmi_async_record(void)
{
    return calloc(1, sizeof (struct gdbmi_async_record));
}

int destroy_gdbmi_async_record(gdbmi_async_record_ptr param)
{
    if (!param)
        return 0;

    if (destroy_gdbmi_result(param->result) == -1)
        return -1;
    param->result = NULL;

    free(param);
    param = NULL;
    return 0;
}

int print_gdbmi_async_record(gdbmi_async_record_ptr param)
{
    int result;

    if (!param)
        return 0;

    result = print_token(param->token);
    if (result == -1)
        return -1;

    result = print_gdbmi_async_record_choice(param->async_record);
    if (result == -1)
        return -1;

    result = print_gdbmi_async_class(param->async_class);
    if (result == -1)
        return -1;

    result = print_gdbmi_result(param->result);
    if (result == -1)
        return -1;

    return 0;
}

int print_gdbmi_async_class(enum gdbmi_async_class param)
{
    switch (param) {
        case GDBMI_STOPPED:
            printf("GDBMI_STOPPED\n");
            break;
        default:
            return -1;
    };

    return 0;
}

int print_gdbmi_value_choice(enum gdbmi_value_choice param)
{
    switch (param) {
        case GDBMI_CSTRING:
            printf("GDBMI_CSTRING\n");
            break;
        case GDBMI_TUPLE:
            printf("GDBMI_TUPLE\n");
            break;
        case GDBMI_LIST:
            printf("GDBMI_LIST\n");
            break;
        default:
            return -1;
    };

    return 0;
}

/* Creating, Destroying and printing value  */
gdbmi_value_ptr create_gdbmi_value(void)
{
    return calloc(1, sizeof (struct gdbmi_value));
}

int destroy_gdbmi_value(gdbmi_value_ptr param)
{
    if (!param)
        return 0;

    if (param->value_choice == GDBMI_CSTRING) {
        if (param->option.cstring) {
            free(param->option.cstring);
            param->option.cstring = NULL;
        }
    } else if (param->value_choice == GDBMI_TUPLE) {
        if (destroy_gdbmi_tuple(param->option.tuple) == -1)
            return -1;
        param->option.tuple = NULL;
    } else if (param->value_choice == GDBMI_LIST) {
        if (destroy_gdbmi_list(param->option.list) == -1)
            return -1;
        param->option.list = NULL;
    } else
        return -1;

    if (destroy_gdbmi_value(param->next) == -1)
        return -1;
    param->next = NULL;

    free(param);
    param = NULL;
    return 0;
}

gdbmi_value_ptr append_gdbmi_value(gdbmi_value_ptr list, gdbmi_value_ptr item)
{
    if (!item)
        return NULL;

    if (!list)
        list = item;
    else {
        gdbmi_value_ptr cur = list;

        while (cur->next)
            cur = cur->next;

        cur->next = item;
    }

    return list;
}

int print_gdbmi_value(gdbmi_value_ptr param)
{
    gdbmi_value_ptr cur = param;
    int result;

    while (cur) {
        result = print_gdbmi_value_choice(cur->value_choice);
        if (result == -1)
            return -1;

        if (cur->value_choice == GDBMI_CSTRING) {
            printf("cstring->(%s)\n", cur->option.cstring);
        } else if (cur->value_choice == GDBMI_TUPLE) {
            result = print_gdbmi_tuple(cur->option.tuple);
            if (result == -1)
                return -1;
        } else if (cur->value_choice == GDBMI_LIST) {
            result = print_gdbmi_list(cur->option.list);
            if (result == -1)
                return -1;
        } else
            return -1;

        cur = cur->next;
    }

    return 0;
}

/* Creating, Destroying and printing tuple  */
gdbmi_tuple_ptr create_gdbmi_tuple(void)
{
    return calloc(1, sizeof (struct gdbmi_tuple));
}

int destroy_gdbmi_tuple(gdbmi_tuple_ptr param)
{
    if (!param)
        return 0;

    if (destroy_gdbmi_result(param->result) == -1)
        return -1;
    param->result = NULL;

    if (destroy_gdbmi_tuple(param->next) == -1)
        return -1;
    param->next = NULL;

    free(param);
    param = NULL;
    return 0;
}

int print_gdbmi_tuple(gdbmi_tuple_ptr param)
{
    gdbmi_tuple_ptr cur = param;
    int result;

    while (cur) {
        result = print_gdbmi_result(cur->result);
        if (result == -1)
            return -1;

        cur = cur->next;
    }

    return 0;
}

int print_gdbmi_list_choice(enum gdbmi_list_choice param)
{
    switch (param) {
        case GDBMI_VALUE:
            printf("GDBMI_VALUE\n");
            break;
        case GDBMI_RESULT:
            printf("GDBMI_RESULT\n");
            break;
        default:
            return -1;
    };

    return 0;
}

/* Creating, Destroying and printing list  */
gdbmi_list_ptr create_gdbmi_list(void)
{
    return calloc(1, sizeof (struct gdbmi_list));
}

int destroy_gdbmi_list(gdbmi_list_ptr param)
{
    if (!param)
        return 0;

    if (param->list_choice == GDBMI_VALUE) {
        if (destroy_gdbmi_value(param->option.value) == -1)
            return -1;
        param->option.value = NULL;
    } else if (param->list_choice == GDBMI_RESULT) {
        if (destroy_gdbmi_result(param->option.result) == -1)
            return -1;
        param->option.result = NULL;
    } else
        return -1;

    if (destroy_gdbmi_list(param->next) == -1)
        return -1;
    param->next = NULL;

    free(param);
    param = NULL;
    return 0;
}

gdbmi_list_ptr append_gdbmi_list(gdbmi_list_ptr list, gdbmi_list_ptr item)
{
    if (!item)
        return NULL;

    if (!list)
        list = item;
    else {
        gdbmi_list_ptr cur = list;

        while (cur->next)
            cur = cur->next;

        cur->next = item;
    }

    return list;
}

int print_gdbmi_list(gdbmi_list_ptr param)
{
    gdbmi_list_ptr cur = param;
    int result;

    while (cur) {
        result = print_gdbmi_list_choice(cur->list_choice);
        if (result == -1)
            return -1;

        if (cur->list_choice == GDBMI_VALUE) {
            result = print_gdbmi_value(cur->option.value);
            if (result == -1)
                return -1;
        } else if (cur->list_choice == GDBMI_RESULT) {
            result = print_gdbmi_result(cur->option.result);
            if (result == -1)
                return -1;
        } else
            return -1;

        cur = cur->next;
    }

    return 0;
}

/* Creating, Destroying and printing stream_record  */
gdbmi_stream_record_ptr create_gdbmi_stream_record(void)
{
    return calloc(1, sizeof (struct gdbmi_stream_record));
}

int destroy_gdbmi_stream_record(gdbmi_stream_record_ptr param)
{
    if (!param)
        return 0;

    if (param->cstring) {
        free(param->cstring);
        param->cstring = NULL;
    }

    free(param);
    param = NULL;
    return 0;
}

int print_gdbmi_stream_record(gdbmi_stream_record_ptr param)
{
    int result;

    result = print_gdbmi_stream_record_choice(param->stream_record);
    if (result == -1)
        return -1;
    printf("cstring->(%s)\n", param->cstring);

    return 0;
}
