#include "kui_map.h"
#include "kui_term.h"

#include <string.h>

std::shared_ptr<kui_map> kui_map::create(const char *key, const char *value)
{
    /* Validify parameters */
    if (!key || !value)
        return {};

    char *orig_key = strdup(key);
    if (!orig_key) {
        return {};
    }

    char *orig_value = strdup(value);
    if (!orig_value) {
        free(orig_key);
        return {};
    }

    int *lit_key = nullptr;
    if (kui_term_string_to_key_array(orig_key, &lit_key) == -1) {
        free(orig_key);
        free(orig_value);
        free(lit_key);
        return {};
    }

    int *lit_value = nullptr;
    if (kui_term_string_to_key_array(orig_value, &lit_value) == -1) {
        free(orig_key);
        free(orig_value);
        free(lit_key);
        free(lit_value);
        return {};
    }

    struct expose_ctor : public kui_map
    {
        expose_ctor(char *orig_key, int *lit_key,
                    char *orig_val, int *lit_val)
            : kui_map{ orig_key, lit_key, orig_val, lit_val }
        {}
    };
    return std::make_shared<expose_ctor>(orig_key, lit_key, orig_value, lit_value);
}

kui_map::~kui_map()
{
    free(m_original_key);
    free(m_original_value);
    free(m_literal_key);
    free(m_literal_value);
}
