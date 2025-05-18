enum GRAMMAR {
    BEGIN_ARRAY = '[',
    BEGIN_OBJECT = '{',
    END_ARRAY = ']',
    END_OBJECT = '}',
    NAME_SEPARATOR = ':',
    VALUE_SEPARATOR = ',',
    LITERAL_TRUE,
    LITERAL_FALSE,
    LITERAL_NULL,
    WHITE_SPACE,
};

static char is_whitespace(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

/**
 * A JSON value must be an Object, an Array, Number, String, 'true', 'false', or 'null' 
 */
static char parse_value() {

}