/**
 * This is an extension to Tcl that allows a GDB/MI command to be parsed.
 *
 * The initial implementation is around to allow an MI command to get parsed
 * and return to the TCL if the command was successfully parsed or not.
 * This checks the syntax of every MI output command to ensure they are correct.
 *
 * Future enhancements will somehow allow the Tcl to directly access the parse
 * tree and allow for semantic analysis of the MI output command.
 */

#include <tcl.h>
#include "gdbmi_parser.h"

static void Gdbmi_tclext_Parse_Destroy(ClientData data)
{
    gdbmi_parser_ptr parser_ptr = (gdbmi_parser_ptr) data;

    gdbmi_parser_destroy(parser_ptr);
    parser_ptr = NULL;
}

static int Gdbmi_tclext_Parse_Cmd(ClientData cdata,
        Tcl_Interp * interp, int objc, Tcl_Obj * CONST objv[])
{
    int val = -1;
    char *mi_output_command;
    gdbmi_output_ptr output_ptr;
    int parse_failed;

    gdbmi_parser_ptr parser_ptr = (gdbmi_parser_ptr) cdata;

    /* Verify that the arguments are OK */
    if (objc != 2) {
        Tcl_AppendResult(interp, "bad #arg: ", objv[0], "GDB/MI Output Command",
                (char *) NULL);
        return TCL_ERROR;
    }

    /* Get the MI output command to parse */
    mi_output_command = Tcl_GetString(objv[1]);

    /* Parse the MI output command */
    val = gdbmi_parser_parse_string(parser_ptr, mi_output_command, &output_ptr,
            &parse_failed);
    if (val == -1) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj("parser error", -1));
        return TCL_ERROR;
    }

    if (parse_failed == 1)
        Tcl_SetObjResult(interp, Tcl_NewStringObj("syntax error", -1));
    else
        Tcl_SetObjResult(interp, Tcl_NewStringObj("correct syntax", -1));

    return TCL_OK;
}

 /* Called when Tcl [load]'s your extension. */
int Gdbmi_tclext_Init(Tcl_Interp * interp)
{
    /** The parser object capable of parsing GDBMI output commands */
    gdbmi_parser_ptr parser_ptr;

#ifdef USE_TCL_STUBS
    if (Tcl_InitStubs(interp, "8.4", 0) == 0L)
        return TCL_ERROR;
#endif

    /* Allocate the new gdbmi_parser_ctx */
    parser_ptr = gdbmi_parser_create();
    if (!parser_ptr) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj("parser create error", -1));
        return TCL_ERROR;
    }

    Tcl_CreateObjCommand(interp,
            "gdbmi_parse",
            Gdbmi_tclext_Parse_Cmd,
            (ClientData *) parser_ptr, Gdbmi_tclext_Parse_Destroy);

    return TCL_OK;
}
