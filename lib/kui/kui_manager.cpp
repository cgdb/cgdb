#include "kui_manager.h"
#include "kui_term.h"
#include "io.h"

static int char_cb(const int fd, const unsigned int ms, kuictx *obj, int *key)
{

    return io_getchar(fd, ms, key);
}

static int kui_cb(const int fd, const unsigned int ms, kuictx *kctx, int *key)
{
    int result;

    if (!key)
        return -1;

    result = kctx->cangetkey();
    if (result == -1)
        return -1;

    if (result == 1) {
        *key = kctx->getkey();
        if (*key == -1)
            return -1;
    }

    /* If there is no data ready, check the I/O */
    if (result == 0) {
        result = io_data_ready(kctx->get_fd(), ms);
        if (result == -1)
            return -1;

        if (result == 1) {
            *key = kctx->getkey();
            if (*key == -1)
                return -1;
        }

        if (result == 0)
            return 0;
    }

    return 1;
}

std::unique_ptr<kui_manager> kui_manager::create(
    int stdinfd,
    unsigned long keycode_timeout,
    unsigned long mapping_timeout)
{
    std::unique_ptr<kui_manager> obj{
        new kui_manager{ stdinfd, keycode_timeout, mapping_timeout } };
    if (kui_term_get_terminal_mappings(*obj->terminal_key_set))
    {
        return obj;
    }
    return std::unique_ptr<kui_manager>{};
}

kui_manager::kui_manager(int stdinfd,
                         unsigned long keycode_timeout,
                         unsigned long mapping_timeout)
    : terminal_key_set{ std::make_shared<kui_map_set>() }
    , terminal_keys{ terminal_key_set, stdinfd, char_cb, keycode_timeout, nullptr }
    , normal_keys  { nullptr, -1, kui_cb, mapping_timeout, &terminal_keys }
{
}

void kui_manager::set_map_set(const std::shared_ptr<kui_map_set>& kui_ms)
{
    normal_keys.set_map_set(kui_ms);
}

void kui_manager::clear_map_set()
{
    normal_keys.set_map_set({});
}

bool kui_manager::cangetkey() const
{
    /* I'm not sure if checking the terminal keys here is the best solution.
     *
     * It might make more sense to flow the extra characters read from during
     * the terminal keys processing up to the normal keys buffer after a 
     * mapping is found.
     *
     * For now this seems to work. Essentially, the next read get's the
     * buffered terminal keys first which is what should happen anyways.
     */
    return terminal_keys.cangetkey() || normal_keys.cangetkey();
}

int kui_manager::getkey()
{
    return normal_keys.getkey();
}

int kui_manager::getkey_blocking()
{
    /* Get the original values */
    unsigned long terminal_keys_msec = terminal_keys.get_blocking_ms();
    unsigned long normal_keys_msec = normal_keys.get_blocking_ms();

    /* Set the values to be blocking */
    terminal_keys.set_blocking_ms(-1);
    normal_keys.set_blocking_ms(-1);

    /* Get the key */
    int val = normal_keys.getkey();

    /* Restore the values */
    terminal_keys.set_blocking_ms(terminal_keys_msec);
    normal_keys.set_blocking_ms(normal_keys_msec);

    return val;
}

void kui_manager::set_terminal_escape_sequence_timeout(unsigned long msec)
{
    terminal_keys.set_blocking_ms(msec);
}

void kui_manager::set_key_mapping_timeout(unsigned long msec)
{
    normal_keys.set_blocking_ms(msec);
}

int kui_manager::get_terminal_keys_kui_map(
    enum cgdb_key key, const std::list<std::string> &keyseq)
{
    const char *keycode_str;

    keycode_str = kui_term_get_keycode_from_cgdb_key(key);
    if (keycode_str == NULL)
        return -1;

    /* The first map set in the terminal_keys */
    auto map_set = terminal_keys.get_map_set();

    for (const auto &data : keyseq) {
        map_set->register_map(data.c_str(), keycode_str);
    }

    return 0;
}

