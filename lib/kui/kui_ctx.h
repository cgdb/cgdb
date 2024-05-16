#ifndef __KUI_CTX_H__
#define __KUI_CTX_H__

#include "kui_map_set.h"

#include <list>
#include <memory>

/******************************************************************************/
/**
 * @name General operations on a kui context.
 * These function's are the basic functions used to operate on a kui context
 */
/******************************************************************************/

class kuictx;

/**
 * The callback function that libkui will call when it needs a character 
 * to be read.
 *
 * \param fd
 * The descriptor to read in from. ( if needed )
 *
 * \param ms
 * The The amount of time in milliseconds to wait for input.
 * Pass 0, if you do not want to wait.
 *
 * \param obj
 * A piece of state data to pass along
 *
 * \param key
 * The key read
 *
 * Must return 
 * 1 on success,
 * 0 if no more input, 
 * or -1 on error.
 */
typedef int (*kui_getkey_callback) (const int fd,
                                    const unsigned int ms,
                                    kuictx *obj,
                                    int *key);

/**
 * A Key User Interface context.
 */
class kuictx {

public:

    /**
     * Cretae a "Key User Interface" Context
     *
     * This is capable of reading in any type of single/multibyte sequence and 
     * abstracting the details from any higher level. 
     *
     * \param stdinfd 
     * The descriptor to read from when looking for the next char
     *
     * \param callback
     * The function that libkui calls to have 1 char read.
     *
     * \param ms
     * The number of milliseconds that this context should block while 
     * attempting to match a mapping by waiting for the user to type
     * the next key.
     *
     * \param state_data
     * This is a piece of data that is not looked at by this context. It
     * is passed to the callback.
     *
     * @return
     * A new instance on success, or NULL on error. 
     */
    kuictx(const std::shared_ptr<kui_map_set>& map_set, int fd,
           kui_getkey_callback callback, unsigned long ms, kuictx *state_data)
        : m_map_set{ map_set }
        , m_callback{ callback }
        , m_ms{ ms }
        , m_state_data{ state_data }
        , m_fd { fd }
    {}

    void set_map_set(const std::shared_ptr<kui_map_set>& map_set)
    {
        m_map_set = map_set;
    }

    std::shared_ptr<kui_map_set> get_map_set() const
    {
        return m_map_set;
    }

    /**
     * Determine's if libkui has data ready to read. It has already been
     * read by the file descriptor, or it was buffered through some other
     * means.
     *
     * @return
     * True if can get a key, or false if nothing available.
     */
    bool cangetkey() const
    {
        return !m_buffer.empty();
    }

	/**
	 * Get's the next key for the application to process. 
	 *
	 * In the event that this is called when kui_manager_cangetkey returns 0, 
	 * and the caller didn't check the input file handle to determine if there
	 * was input, this is a non-blocking call. Use kui_manager_getkey_blocking
	 * if you want the semantics to be a blocking call.
	 *
	 * @return
	 * -1 on error, otherwise, a valid key.
	 *  A key can either be a normal ascii key, or a CGDB_KEY_* value.
	 */
    int getkey();

    /**
     * Get the number of milliseconds that the kui should block while waiting to
     * complete a mapping. This value is set with kui_set_blocking_ms.
     *
     * \return
     * The number of milliseconds.
     */
    unsigned long get_blocking_ms() const
    {
        return m_ms;
    }

    /**
     * Tell's the kui context the maximum number of milliseconds that it is allowed
     * to block while waiting to complete a mapping.
     *
     * \param msec
     * The maximum number of milliseconds to block while waiting to complete a map
     * The value 0 causes no waiting.
     * The value -1 causes an indefinate amount of blocking.
     */
    void set_blocking_ms(unsigned long ms)
    {
        m_ms = ms;
    }

    int get_fd()
    {
        return m_fd;
    }

private:

    /**
     * The current map set for this KUI context.
     */
    std::shared_ptr<kui_map_set> m_map_set;

    /**
     * A list of characters, used as a buffer for stdin.
     */
    std::list<int> m_buffer;

    /**
     * A volitale buffer. This is reset upon every call to kui_getkey.
     */
    std::list<int> m_volatile_buffer;

    /**
     * The callback function used to get data read in.
     */
    kui_getkey_callback m_callback;

    /**
     * Milliseconds to block on a read.
     */
    unsigned long m_ms;

    /**
     * state data
     */
    kuictx *m_state_data;

    /**
     * The file descriptor to read from.
     */
    int m_fd;

private:
    int findkey(int& map_found);
    int findchar(int& key);
    int update_map_set(int key, int& map_found);
    bool should_continue_looking() const;
    kui_map *get_found_map() const;
    void update_buffer(kui_map *the_map_found, int *key);
};

#endif
