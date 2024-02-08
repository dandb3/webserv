#include "HttpResponseModule.hpp"

StatusLine& StatusLine::operator=(const StatusLine& ref)
{
    version = ref.version;
    code = ref.code;
    text = ref.text;
    return *this;
}
