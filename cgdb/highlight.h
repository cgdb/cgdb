#ifndef _HIGHLIGHT_H_
#define _HIGHLIGHT_H_

/* highlight.h:
 * ------------
 * 
 * Syntax highlighting routines.
 *
 */

struct hl_regex_info;

int hl_regex_search(struct hl_regex_info **info, char *line, const char *regex, int icase, int *start, int *end);
void hl_regex_free(struct hl_regex_info **info);

struct hl_line_attr *hl_regex_highlight(struct hl_regex_info **info, char *line);

/* Return the regex string (or NULL) */
const char *hl_regex_get(struct hl_regex_info *info);

#endif /* _HIGHLIGHT_H_ */
