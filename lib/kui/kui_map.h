#ifndef __KUI_MAP_H__
#define __KUI_MAP_H__

#include <memory>

/* class kui_map {{{ */

/**
 * A kui map structure.
 *
 * This is simply a key/value pair as far as the outside
 * world is concerned. 
 */
class kui_map
{
public:
    /**
    * Create a kui map.
    *
    * \param key_data
    * The map's key data
    *
    * \param value_data
    * The map's value data
    *
    * @return
    * A new instance on success, or an empty shared_ptr on error. 
    */
    static std::shared_ptr<kui_map> create(
        const char *key_data, const char *value_data);
    ~kui_map();

    const int *get_literal_key() const
    {
        return m_literal_key;
    }

    const int *get_literal_value() const
    {
        return m_literal_value;
    }

protected:
    kui_map(char *original_key,   int *literal_key,
            char *original_value, int *literal_value)
        : m_original_key{ original_key }
        , m_literal_key{ literal_key }
        , m_original_value{ original_value }
        , m_literal_value{ literal_value }
    {}

private:

    /**
	 * The user wants this map to be activated when he/she types this sequence.
	 * This data representation is entirly in ascii, since the user has to 
	 * be able to type the sequence on the keyboard.
	 */
    char *m_original_key = nullptr;

    /**
	 * This is the list of keys that must be matched, in order for this map
	 * to be activated. The value of each item is either an ascii char typed 
	 * by the user, or it is a high level terminal escape sequence.
	 *
	 * This is a NULL terminated list.
	 */
    int *m_literal_key = nullptr;

    /**
	 * The value is the substitution data, if the literal_key is typed.
	 */
    char *m_original_value = nullptr;

    /**
	 * This data is passed in place of the key, if the user types the 
	 * literal_key.
	 *
	 * This is a NULL terminated list.
	 */
    int *m_literal_value = nullptr;
};

#endif
