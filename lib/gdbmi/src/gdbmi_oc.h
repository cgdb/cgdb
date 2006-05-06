#ifndef __GDBMI_OC_H__
#define __GDBMI_OC_H__

#include "gdbmi_pt.h"

struct gdbmi_oc;
typedef struct gdbmi_oc *gdbmi_oc_ptr;

/**
 * This will take in a parse tree and return a list of MI output commands.
 *
 * \param output_ptr
 * The MI parse tree
 *
 * \param oc_ptr
 * On return, this will be the MI output commands that were derived from the 
 * parse tree.
 *
 * \return
 * 0 on success, -1 on error.
 */
int
gdbmi_get_output_commands (gdbmi_output_ptr output_ptr, gdbmi_oc_ptr *oc_ptr);

#endif /* __GDBMI_OC_H__ */
