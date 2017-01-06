#ifndef _HIGHLIGHT_H_
#define _HIGHLIGHT_H_

/**
 * Syntax highlighting routines.
 */

struct hl_regex_info;

/**
 * Do a regex search.
 *
 * @param info
 * The regular expression structure. Pass in the address of a NULL pointer
 * the first time. Afterwards, reuse the same pointer. Call hl_regex_free
 * when done.
 *
 * @param line
 * The line to search.
 *
 * @param regex
 * The regular expression to search the line with.
 *
 * @param icase
 * Non-zero to be case insensitive, otherwise 0 for case sensitivity.
 *
 * @param start
 * If a match is found (this function returns non-zero), the starting
 * character for the match. Otherwise, if no match found, -1.
 * 
 * @param end
 * If a match is found (this function returns non-zero), the ending
 * character for the match. Otherwise, if no match found, -1.
 */
int hl_regex_search(struct hl_regex_info **info, char *line, const char *regex,
    int icase, int *start, int *end);

/**
 * Free the regular expression context.
 *
 * @param info
 * The regular expression context to free.
 */
void hl_regex_free(struct hl_regex_info **info);

/**
 * Highlight the regular expressions found.
 *
 * @param info
 * A regular expression context previously created in hl_regex_search.
 *
 * @param line
 * A line of text to highlight based on the regular expression.
 *
 * @param hlattr
 * The attribute to use for highlighting.
 *
 * @return
 * A list of line attributes corresponding to the positions in the
 * line that matched the regular expression and should be highlighted.
 * Will return NULL if no matches were found.
 */
struct hl_line_attr *hl_regex_highlight(struct hl_regex_info **info,
    char *line, int hlattr);

#endif /* _HIGHLIGHT_H_ */
