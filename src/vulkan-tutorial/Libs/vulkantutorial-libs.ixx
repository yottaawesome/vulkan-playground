module;
// Workaround for https://www.reddit.com/r/cpp_questions/comments/14qcpme/why_does_the_following_code_emit_error_c3774/
#include <compare>

export module vulkantutorial:libs;
export import :libs_exports; 
export import :libs_formatters;
