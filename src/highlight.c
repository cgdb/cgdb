/* highlight.c:
 * ------------
 * 
 * Syntax highlighting routines.
 *
 */

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

/* System Includes */
#if HAVE_CTYPE_H
#include <ctype.h>
#endif /* HAVE_CTYPE_H */

#if HAVE_LIMITS_H
#include <limits.h>
#endif /* HAVE_LIMITS_H */

#if HAVE_STRING_H
#include <string.h>
#endif /* HAVE_STRING_H */

#if HAVE_STDLIB_H 
#include <stdlib.h>
#endif  /* HAVE_STDLIB_H */

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif /* HAVE_SYS_TYPES_H */

#if HAVE_REGEX_H
#include <regex.h>
#endif /* HAVE_REGEX_H */

/* Local Includes */
#include "highlight.h"
#include "sources.h"
#include "cgdb.h"

/* ----------- */
/* Definitions */
/* ----------- */
 
#define HL_CHAR CHAR_MAX       /* Special marker character */
#define HL_OFFSET 6            /* The number to add to a color to get
                                  Its highlighted color */

#define SET_COLOR(string, color) \
   do { \
       string[pos++] = HL_CHAR; \
       string[pos++] = color; \
       cur_color = color; \
   } while(0)

#define issep(c) (strchr(separators, c) != NULL)

/* --------------- */
/* Data Structures */
/* --------------- */

/* Syntax highlighting colors: */
enum syntax_type {
    SYN_KEYWORD = 1,
    SYN_TYPE,
    SYN_LITERAL,
    SYN_COMMENT,
    SYN_DIRECTIVE,
    SYN_TEXT,
    SYN_SEARCH,
    SYN_LAST
};

/* Note: These next two arrays should probably be loaded from the .tgdbrc file
 *       in the future (so that they are user-configurable). */

/* Colors for the syntax types enumerated above */
static int colors[] = {
    0,                          /* (0) Blank first entry */
    CGDB_COLOR_BLUE,            /* (1) C/C++ Keyword color */
    CGDB_COLOR_GREEN,           /* (2) Typemark color */
    CGDB_COLOR_RED,             /* (3) String or numeric literal color */
    CGDB_COLOR_YELLOW,          /* (4) Comment color */
    CGDB_COLOR_CYAN,            /* (5) Preprocessor directive color */
    CGDB_COLOR_WHITE,           /* (6) Other text color */
    CGDB_COLOR_INVERSE_BLUE,    /* (7) C/C++ Keyword inverse color */
    CGDB_COLOR_INVERSE_GREEN,   /* (8) Typemark inverse color */
    CGDB_COLOR_INVERSE_RED,     /* (9) String or numeric literal inverse color */
    CGDB_COLOR_INVERSE_YELLOW,  /* (a) Comment inverse color */
    CGDB_COLOR_INVERSE_CYAN,    /* (b) Preprocessor directive inverse color */
    CGDB_COLOR_INVERSE_WHITE,   /* (c) Other text inverse color */
};

/* Attributes may be joined with a bitwise OR (|) */
static int attributes[] = {
    0,                     /* Blank first entry */
    A_BOLD,                /* C/C++ keyword attributes */
    A_BOLD,                /* Typemark attributes */
    A_BOLD,                /* String or numeric literal attributes */
    A_NORMAL,              /* Comment attributes */
    A_BOLD,                /* Preprocessor directive attributes */
    A_NORMAL,              /* Other text attributes */
    A_NORMAL,              /* Other text attributes */
    A_NORMAL,              /* Other text attributes */
    A_NORMAL,              /* Other text attributes */
    A_NORMAL,              /* Other text attributes */
    A_NORMAL,              /* Other text attributes */
    A_NORMAL,              /* Other text attributes */
};

/* --------------- */
/* Local Variables */
/* --------------- */

static char *c_extensions[] = { ".c", ".C", ".cc", ".cpp", ".cxx", ".h", ".hpp" };
static char *ada_extensions[] = { ".adb", ".ads", ".ada", 
                                  ".ADB", ".ADS", ".ADA" };

static char *c_types[] = { 
    "auto", "bool", "char", "class", "const", "double", "float", 
    "friend", "explicit", "extern", "enum", "inline", "int", "long", 
    "mutable", "namespace", "register", "short", "signed", "static", 
    "struct", "union", "unsigned", "virtual", "void", "volatile", "wchar_t"
};

static char *c_keywords[] = {
    "and", "and_eq", "asm", "bitand", "bitor", "break", "case", "catch", 
    "compl", "const_cast", "continue", "default", "delete", "do", 
    "dynamic_cast", "else", "export", "false", "for", "goto", "if", "new", 
    "not", "not_eq", "operator", "or", "or_eq", "private", "protected", 
    "public", "reinterpret_cast", "return", "sizeof", "static_cast", 
    "switch", "template", "this", "throw", "true", "try", "typedef", 
    "typeid", "typename", "using", "while", "xor", "xor_eq"
};

static char *c_separators = " \t\n\r+-/*(),^|&~=;:<>!{}%?\\[]";

static char *ada_types[] = {
    "array", "boolean", "character", "constant", "fixed", "float", 
    "integer", "long_float", "long_long_float", "long_integer", 
    "long_long_integer", "natural", "positive", "short_float", 
    "short_integer", "short_short_integer", "string"
};
static char *ada_keywords[] = {
    "abort", "abs", "abstract", "accept", "access", "aliased", "all", "and",
    "at", "begin", "body", "case", "declare", "delay",
    "delta", "digits", "do", "else", "elsif", "end", "entry", "exception", 
    "exit", "for", "function", "generic", "goto", "if", "in", "is", "limited",
    "loop", "mod", "new", "not", "null", "of", "or", "others", "out", "package",
    "pragma", "private", "procedure", "protected", "raise", "range", "record",
    "rem", "renames", "requeue", "return", "reverse", "select", "separate", 
    "subtype", "tagged", "task", "terminate", "then", "type", "until", "use",
    "when", "while", "with", "xor"
};

static char *ada_separators = " \t\n\r+-/*(),^|&~=;:<>!{}%?\\[]'";

/* --------------- */
/* Local Functions */
/* --------------- */

/* highlight_c:  Called by highlight() to highlight the given C/C++ file.
 * ------------
 *
 *   node:  The node containing the file buffer to highlight.
 */
static void highlight_c(struct list_node *node)
{
    enum state_type { NORMAL, COMMENT, QUOTE1, QUOTE2 } state = NORMAL;
    char new_line[MAX_LINE];
    char *separators = c_separators;
    char *line;
    int length;
    int i, j, pos;
    int cur_color = 0;

    /* Note:    This is a kind of ugly state machine for syntax highlighting.
     *        Here's the idea.  Iterate through the file, one line at a time.
     *        Traverse the line, one character at a time.  For comments and
     *        quotes which can carry over to the next line, set a state so
     *        that we remember this for the next line.
     *          To set a color, we insert the HL_CHAR (an unprintable char)
     *        into the new_line (our new copy of the line we're editing),
     *        followed by the numeric from enum syntax_type (see highlight.h).
     *        When we finish with a line, blast the original line from the
     *        buffer, and replace it with our new_line.
     *          I may reimplement this more like a lexical analyzer in the
     *        future.  It seems to work the way it is, but it just feels
     *        awkward. */
       
    for (i = 0; i < node->buf.length; i++){
        line = node->buf.tlines[i];
        length = strlen(line);
        pos = 0;
        switch(state){
            case NORMAL:
                SET_COLOR(new_line, SYN_TEXT);
                break;
            case COMMENT:
                SET_COLOR(new_line, SYN_COMMENT);
                break;
            case QUOTE1:
            case QUOTE2:
                SET_COLOR(new_line, SYN_LITERAL);
                break;
        }
            
        for (j = 0; j < length; j++){
            switch(state){
                case NORMAL:
                    /* Check for comment start */
                    if (line[j] == '/' && j+1 < length){
                        if (line[j+1] == '/'){
                            /* C++ style comment */
                            SET_COLOR(new_line, SYN_COMMENT);
                            strcpy(new_line+pos, line+j);
                            pos += length - j;
                            j = length;
                            break;
                        }
                        if (line[j+1] == '*'){
                            /* C style comment */
                            state = COMMENT;
                            SET_COLOR(new_line, SYN_COMMENT);
                            new_line[pos++] = line[j];
                            break;
                        }
                    }
                    
                    /* Check for quotes */
                    if (line[j] == '\''){
                        /* Single quote */
                        state = QUOTE1;
                        SET_COLOR(new_line, SYN_LITERAL);
                    }
                    if (line[j] == '"'){
                        /* Double quote */
                        state = QUOTE2;
                        SET_COLOR(new_line, SYN_LITERAL);
                    }

                    /* Check for C/C++ keywords */
                    if (j == 0 || issep(line[j-1])){
                        int x;
                        for (x = 0; x < sizeof(c_types)/sizeof(char *); x++){
                            if (strncmp(line + j, 
                                        c_types[x], 
                                        strlen(c_types[x])) == 0
                                && (j + strlen(c_types[x]) == length
                                    || issep(line[j+strlen(c_types[x])])))
                                SET_COLOR(new_line, SYN_TYPE);
                        }
                        for (x = 0; x < sizeof(c_keywords)/sizeof(char *); x++){
                            if (strncmp(line + j,
                                        c_keywords[x],
                                        strlen(c_keywords[x])) == 0
                                && (j + strlen(c_keywords[x]) == length
                                    || issep(line[j+strlen(c_keywords[x])])))
                                SET_COLOR(new_line, SYN_KEYWORD);
                        }
                    }

                    /* Check for preprocessor directives */
                    if (line[j] == '#')
                        SET_COLOR(new_line, SYN_DIRECTIVE);

                    /* Check for integer/numeric literals */
                    if (j == 0 || issep(line[j-1])){
                        if (isdigit(line[j]) || 
                           (line[j] == '.' && j+1<length && isdigit(line[j+1])))
                            SET_COLOR(new_line, SYN_LITERAL);
                    }

                    if (issep(line[j]) && cur_color != SYN_TEXT)
                        SET_COLOR(new_line, SYN_TEXT);
                    
                    new_line[pos++] = line[j];
                    break;
                        
                case COMMENT:
                    new_line[pos++] = line[j];
                    if (j > 0 && line[j-1] == '*' && line[j] == '/'){
                        SET_COLOR(new_line, SYN_TEXT);
                        state = NORMAL;
                    }
                    break;
                    
                case QUOTE1:
                    new_line[pos++] = line[j];
                    if (line[j] == '\'' && ((j > 0 && line[j-1] != '\\') ||
                                            (j > 1 && line[j-2] == '\\'))){
                        SET_COLOR(new_line, SYN_TEXT);
                        state = NORMAL;
                    }
                    break;
                        
                case QUOTE2:
                    new_line[pos++] = line[j];
                    if (line[j] == '"' && ((j > 0 && line[j-1] != '\\') ||
                                           (j > 1 && line[j-2] == '\\'))){
                        SET_COLOR(new_line, SYN_TEXT);
                        state = NORMAL;
                    }
                    break;
            }
        }

        free(line);
        new_line[pos] = 0;
        node->buf.tlines[i] = strdup(new_line);
    }

}

static void highlight_ada(struct list_node *node)
{
    enum state_type { NORMAL, QUOTE1, QUOTE2 } state = NORMAL;
    char new_line[MAX_LINE];
    char *separators = ada_separators;
    char *line;
    int length;
    int i, j, pos;
    int cur_color = 0;

    for (i = 0; i < node->buf.length; i++){
        line = node->buf.tlines[i];
        length = strlen(line);
        pos = 0;
        switch(state){
            case NORMAL:
                SET_COLOR(new_line, SYN_TEXT);
                break;
            case QUOTE1:
            case QUOTE2:
                SET_COLOR(new_line, SYN_LITERAL);
                break;
        }
            
        for (j = 0; j < length; j++){
            switch(state){
                case NORMAL:
                    /* Check for comment start */
                    if (line[j] == '-' && j+1 < length && line[j+1] == '-'){
                        /* Comment */
                        SET_COLOR(new_line, SYN_COMMENT);
                        strcpy(new_line+pos, line+j);
                        pos += length - j;
                        j = length;
                        break;
                    }
                    
                    /* Check for quotes */
                    if (line[j] == '\'' && j + 2 < length && line[j+2] == '\''){
                        /* Single quote */
                        state = QUOTE1;
                        SET_COLOR(new_line, SYN_LITERAL);
                    }
                    if (line[j] == '"'){
                        /* Double quote */
                        state = QUOTE2;
                        SET_COLOR(new_line, SYN_LITERAL);
                    }

                    /* Check for Ada keywords */
                    if (j == 0 || issep(line[j-1])){
                        int x;
                        for (x = 0; x < sizeof(ada_types)/sizeof(char *); x++){
                            if (strncasecmp(line + j, 
                                        ada_types[x], 
                                        strlen(ada_types[x])) == 0
                                && (j + strlen(ada_types[x]) == length
                                || issep(line[j+strlen(ada_types[x])])))
                                SET_COLOR(new_line, SYN_TYPE);
                        }
                        for (x = 0; x<sizeof(ada_keywords)/sizeof(char *); x++){
                            if (strncasecmp(line + j,
                                        ada_keywords[x],
                                        strlen(ada_keywords[x])) == 0
                                && (j + strlen(ada_keywords[x]) == length
                                   || issep(line[j+strlen(ada_keywords[x])])))
                                SET_COLOR(new_line, SYN_KEYWORD);
                        }
                    }

                    /* Check for integer/numeric literals */
                    if (j == 0 || issep(line[j-1])){
                        if (isdigit(line[j]) || 
                           (line[j] == '.' && j+1<length && isdigit(line[j+1])))
                            SET_COLOR(new_line, SYN_LITERAL);
                    }

                    if (issep(line[j]) && cur_color != SYN_TEXT)
                        SET_COLOR(new_line, SYN_TEXT);
                    
                    new_line[pos++] = line[j];
                    break;
                        
                case QUOTE1:
                    new_line[pos++] = line[j];
                    if (line[j] == '\''){
                        SET_COLOR(new_line, SYN_TEXT);
                        state = NORMAL;
                    }
                    break;
                        
                case QUOTE2:
                    new_line[pos++] = line[j];
                    if (line[j] == '"'){
                        SET_COLOR(new_line, SYN_TEXT);
                        state = NORMAL;
                    }
                    break;
            }
        }

        free(line);
        new_line[pos] = 0;
        node->buf.tlines[i] = strdup(new_line);
    }


}

/* --------- */
/* Functions */
/* --------- */

/* See comments in highlight.h for function descriptions. */

void highlight(struct list_node *node)
{
    char *extension = strrchr(node->path, '.');
    int i;

    if ( !extension )
        return;

    for (i = 0; i < sizeof(c_extensions) / sizeof(char *); i++){
        if (strcmp(extension, c_extensions[i]) == 0){
            highlight_c(node);
            return;
        }
    }

    for (i = 0; i < sizeof(ada_extensions) / sizeof(char *); i++){
        if (strcmp(extension, ada_extensions[i]) == 0){
            highlight_ada(node);
            return;
        }
    }
}

/* highlight_line_segment: Creates a new line that is hightlighted.
 * ------------------------
 *
 *  orig:   The line that needs to be highlighted.
 *  start:  The desired starting position of the highlighted portion.
 *          The start index *is* included in the highlighted segment.
 *  end:    The desired ending position of the highlighted portion.
 *          The end index *is not* include in the highlighted segment.
 *
 *  Return Value: Null on error. Or a pointer to a new line that
 *  has highlighting. The new line MUST BE FREED.
 */ 
//#define HIGHLIGHT_DEBUG
static char *highlight_line_segment(const char *orig, int start, int end) {
    char *new_line = NULL;
    int length = strlen(orig), j = 0, pos = 0;
    int syn_search_recieved = 0, cur_color = SYN_TEXT;
#ifdef HIGHLIGHT_DEBUG
    extern struct scroller *gdb_win;
#endif

    /* Cases not possible */
    if ( start > end || orig == NULL || 
         start > length || start < 0 || 
         end > length || end < 0)
        return NULL;
    
    /* The 5 is: color start (2), color end (2) and EOL */
    if ( (new_line = (char *)malloc(sizeof(char)*(length + 5))) == NULL )
        return NULL;

#ifdef HIGHLIGHT_DEBUG
    /*
    for ( j = 0; j < strlen(orig); j++ ) {
       char temp[100];
       sprintf(temp, "(%d:%c)", orig[j], orig[j]);
       scr_add(gdb_win, temp);
    }
    scr_add(gdb_win, "\r\n");
    scr_refresh(gdb_win, 1);
    */
#endif
    
    /* This traverses the input line. It creates a new line with the section
     *     given highlighted.
     * If a highlight symbol is encountered, the end and/or start position 
     *     is incremented because the original match was against only the text,
     *     not against the color embedded text :)
     */
    for( j = 0; j < length; j++) {
        if ( orig[j] == HL_CHAR ) {
            if ( j <= start )
                start += 2;

            if ( j <= end )
                end +=2;

            cur_color = orig[j + 1];
        }

        /* Mark when the search is started and when it ends */
        if ( j == start ) {
            syn_search_recieved = 1;
            new_line[pos++] = HL_CHAR;
            new_line[pos++] = cur_color + HL_OFFSET;
        } else if ( j == end ) {
            syn_search_recieved = 0;
            new_line[pos++] = HL_CHAR;
            new_line[pos++] = cur_color;
        }

        new_line[pos++] = orig[j];

        /* If the search has started, then make all the colors the 
         * highlighted colors by adding HL_OFFSET to the color */
        if ( syn_search_recieved && orig[j] == HL_CHAR )
            new_line[pos++] = ((int)orig[++j]) + HL_OFFSET;
    }

    new_line[pos] = '\0';

#ifdef HIGHLIGHT_DEBUG
    for ( j = 0; j < strlen(new_line); j++ ) {
       char temp[100];
       sprintf(temp, "(%d:%c)", new_line[j], new_line[j]);
       scr_add(gdb_win, temp);
    }
    scr_add(gdb_win, "\r\n");
    scr_refresh(gdb_win, 1);
#endif

    /* Now the line has new information in it.
     * Lets traverse it again and correct the highlighting to be the 
     * reverse.
     */

    return new_line;
}

void hl_wprintw(WINDOW *win, const char *line, int width, int offset)
{
    int length;
    enum syntax_type color = SYN_TEXT;
    int i, j;
    int p;
    
    /* Set default color to text color */
    wattron(win, COLOR_PAIR(colors[color]));
    wattron(win, attributes[color]);
    
    /* Jump ahead to the character at offset (process color commands too) */
    length = strlen(line);
    for (i = 0, j = 0; i < length && j < offset; i++){
        if (line[i] == HL_CHAR && i+1 < length){
            wattroff(win, COLOR_PAIR(colors[color]));
            wattroff(win, attributes[color]);
            color = (int)line[++i];
            wattron(win, COLOR_PAIR(colors[color]));
            wattron(win, attributes[color]);
        }
        else
            j++;
    }
        
    /* Print string 1 char at a time */
    for (p = 0; i < length && p < width; i++){
        if (line[i] == HL_CHAR){
            if (++i < length){
                wattroff(win, COLOR_PAIR(colors[color]));
                wattroff(win, attributes[color]);
                color = (int)line[i];
                wattron(win, COLOR_PAIR(colors[color]));
                wattron(win, attributes[color]);
            }
        }
        else{
            switch (line[i]){
                case '\t':
                    for (j = 0; j < 4 && p < width; j++, p++)
                        wprintw(win, " ");
                    break;
                default:
                    wprintw(win, "%c", line[i]);
                    p++;
            }
        }
    }
    
    /* Shut off color attribute */
    wattroff(win, COLOR_PAIR(colors[color]));
    wattroff(win, attributes[color]);

    for (; p < width; p++)
        wprintw(win, " ");
}

int hl_regex(const char *regex, const char **hl_lines, const char **tlines, 
             const int length, char **cur_line, int *sel_line, 
             int *sel_rline, int *sel_col_rbeg, int *sel_col_rend, 
             int opt, int direction, int icase) {
    regex_t t;                   /* Regular expression */
    regmatch_t pmatch[1];        /* Indexes of matches */
    int i = 0, result = 0;
    char *local_cur_line;
    int success = 0;
    int offset =  0;

    if (tlines == NULL || tlines[0] == NULL ||
        cur_line == NULL || sel_line == NULL || 
        sel_rline == NULL || sel_col_rbeg == NULL || sel_col_rend == NULL )
        return -1; 

    /* Clear last highlighted line */
    if ( *cur_line != NULL ) {
        free ( *cur_line);
        *cur_line=NULL;
    }

    /* If regex is empty, set current line to original line */
    if ( regex == NULL || *regex == '\0' ) {
        *sel_line = *sel_rline;
        return -2;
    }

    /* Compile the regular expression */
    if ( regcomp(&t, regex, REG_EXTENDED & (icase)?REG_ICASE:0) != 0) {
        regfree(&t);
        return -3;
    }

    /* Forward search */
    if ( direction ) {
        offset = *sel_col_rend;
        for ( i = *sel_rline; i < length; i++){
            local_cur_line = (char *)tlines[i];
            
            /* Add the position of the current line's last match */
            if ( i == *sel_rline ) 
                local_cur_line += offset;

            /* Found a match */
            if ( ( result = regexec(&t, local_cur_line, 1, pmatch, 0)) == 0 ) {
                success = 1;
                break;
            }
        }
    } else { /* Reverse search */
        int j, pos;
        offset = *sel_col_rbeg;

        /* Try each line */
        for ( i = *sel_rline; i >= 0; i--){
            local_cur_line = (char *)tlines[i];
            pos = strlen(local_cur_line) - 1;
            
            if ( i == *sel_rline )
                pos = offset - 1; 

            /* Try each line, char by char starting from the end */
            for ( j = pos; j >= 0; j-- ) {
                if ( ( result = regexec(&t, local_cur_line + j, 1, pmatch, 0)) == 0 ) {
                    if ( i == *sel_rline && pmatch[0].rm_so > pos - j)
                        continue;
                    /* Found a match */
                    success = 1;
                    offset = j;
                    break;
                }
            }

            if ( success )
               break;
        }
    }

    if ( success ) {
        /* The offset is 0 if the line was not on the original line */
        if ( direction && *sel_rline != i )
            offset = 0;

        /* If final match ( user hit enter ) make position perminant */
        if ( opt == 2 ) {
            *sel_col_rbeg = pmatch[0].rm_so + offset;
            *sel_col_rend = pmatch[0].rm_eo + offset;
            *sel_rline    = i;
        }

        /* Keep the new line as the selected line */
        *sel_line = i;

        /* If the match is not perminant then give cur_line highlighting */ 
        if ( opt != 2  && pmatch[0].rm_so != -1 && pmatch[0].rm_eo != -1 )
                *cur_line = highlight_line_segment( 
                   hl_lines[i], pmatch[0].rm_so + offset, pmatch[0].rm_eo + offset);
    } else {
        /* On failure, the current line goes to the original line */
        *sel_line = *sel_rline;
    }

    regfree(&t);

    return success;
}

