#ifndef __KUI_MANAGER_H__
#define __KUI_MANAGER_H__

#include "kui_ctx.h"
#include "kui_cgdb_key.h"

#include <memory>
#include <list>
#include <string>

/**
 * kui manager
 *  - Contains several kui contexts. the normal user mappings and the
 *  - One of the kui contexts represents the current user mappings.
 *    This could be the map or imap mappings depending on cgdb's state.
 *    The other kui context is the terminal mappings.
 *
 * The main kui context.
 * This context is capable of doing all the work associated with the KUI.
 */
class kui_manager {

public:
    /**
     * Initializes the Key User Interface manager
     *
     * \param stdinfd 
     * The descriptor to read from when looking for the next char
     * 
     * \param keycode_timeout
     * The timeout to use while waiting to match a keyboard code.
     *
     * \param mapping_timeout
     * The timeout to use while waiting to match a mapping sequence.
     *
     * @return
     * A new instance on success, an empty pointer on error. 
     */
    static std::unique_ptr<kui_manager> create(int stdinfd,
                                               unsigned long keycode_timeout,
                                               unsigned long mapping_timeout);
    

    /**
     * Set the kui map for the kui manager's kui context.
     *
     * \param kui_ms The new kui map set to use.
     */
    void set_map_set(const std::shared_ptr<kui_map_set>& kui_ms);

    /**
     * Clear the map set, no mappings will be used.
     */
    void clear_map_set();

    /**
     * Determine's if libkui has data ready to read. It has already been
     * read by the file descriptor, or it was buffered through some other
     * means.
     *
     * @return
     * true if can get a key, or false if nothing available.
     */
    bool cangetkey() const;

    /**
     * Get's the next key for the application to process. 
     *
     * In the event that this is called when kui_manager_cangetkey returns 0, 
     * and the caller didn't check the input file handle to determine if there
     * was input, this is a non-blocking call. Use getkey_blocking if you want
     * the semantics to be a blocking call.
     *
     * @return
     * -1 on error, otherwise, a valid key.
     *  A key can either be a normal ascii key, or a CGDB_KEY_* value.
     */
    int getkey();

    /**
     * This is the same as _getkey excpet that if no data is ready to be read,
     * this call will block until data is ready.
     *
     * \return
     * -1 on error, otherwise, a valid key.
     *  A key can either be a normal ascii key, or a CGDB_KEY_* value.
     */
    int getkey_blocking();

    /**
     * Set's the terminal escape sequence time out value.
     * This is used to tell CGDB how long to block when looking to match terminal
     * escape sequences. For instance, in order to get F11, Maybe it's necessary
     * for the characters 27(ESC) 80(P) 81(Q) to get sent. So, if the user types 
     * these within msec each, then CGDB_KEY_F11 get's returned, otherwise the 
     * key's are returned as typed.
     *
     * \param msec
     * The maximum number of milliseconds to block while waiting to complete a 
     * terminal escape sequence.
     */
    void set_terminal_escape_sequence_timeout(unsigned long msec);

    /**
     * Set's the timeout that will be used when matching a mapping.
     *
     * \param msec
     * The maximum number of milliseconds to block while waiting to complete a 
     * mapping sequence.
     */
    void set_key_mapping_timeout(unsigned long msec);

    /**
     * Allow the user to make terminal key bindings.
     *
     * This API doesn't really fit in. It's due to the fact that some
     * of the terminfo entries are incorrect on some machines but
     * readline knows the key sequences because the OS vendors decided
     * to hard code the terminal key sequences into the /etc/inputrc.
     *
     * Therefor, I'm creating an API that is specific to the task, 
     * rather than a general API that allows the user to modify
     * the terminal key mappings. If this is needed in the future
     * it can be added.
     *
     * \param key
     * The key to bind the key sequences too.
     *
     * \param keyseq
     * The list of key sequences to bind to the key. Instead of just
     * making this a 'char*', it's a list, so that multiple sequences
     * can be bound in a single function call.
     *
     * \return
     * 0 on success or -1 on error
     */
    int get_terminal_keys_kui_map(enum cgdb_key key,
                                  const std::list<std::string> &keyseq);

private:
    /* Need a reference to the terminal escape sequence mappings when
     * destroying this context. (a list is populated in the create function)
     */
    std::shared_ptr<kui_map_set> terminal_key_set;

    /* The terminal escape sequence mappings */
    kuictx terminal_keys;
    /* The user defined mappings */
    kuictx normal_keys;

    kui_manager(int stdinfd,
                unsigned long keycode_timeout,
                unsigned long mapping_timeout);
};

#endif
