#include <string>
#include <type_traits>
#include <utility>

#include <valijson/internal/optional.hpp>

int main()
{
    using valijson::internal::make_optional;
    using valijson::internal::nullopt;
    using valijson::internal::optional;

    static_assert(std::is_default_constructible<optional<int>>::value,
            "optional must be default-constructible");
    static_assert(std::is_copy_constructible<optional<int>>::value,
            "optional must be copy-constructible");
    static_assert(std::is_move_constructible<optional<int>>::value,
            "optional must be move-constructible");

    optional<int> empty;
    if (empty.has_value() || static_cast<bool>(empty)) {
        return 1;
    }

    optional<int> engaged = make_optional(42);
    if (!engaged.has_value() || *engaged != 42) {
        return 2;
    }

    engaged = nullopt;
    if (engaged) {
        return 3;
    }

    optional<std::string> text(std::string("valijson"));
    if (!text || *text != "valijson" || text->size() != 8) {
        return 4;
    }

    text.reset();
    if (text.has_value()) {
        return 5;
    }

    return 0;
}
