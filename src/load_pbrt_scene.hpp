#include <cstdint>
#include <iostream>
#include <fstream>
#include <optional>
#include <string>
#include <utility>

namespace pbrt_parser {
enum struct DecorateChar : uint8_t {
    kNoDecoration,
    kRoundBracket,  // ()
    kSquareBracket, // []
    kBraces,        // {}
    kSingleQuote,   // ''
    kDoubleQuote    // ""
};

namespace base_file_parser {
template<typename T>
concept ifstream_constructable = requires (T val) {
    std::ifstream(val);
};

class ParserIterator;
class Parser;

struct Item {
    std::string data;

    Item(std::string data) :data(std::move(data)) {}
};

class ParserIterator {
private:
    Parser* parser_;
    bool is_parse_end_ = false;

public:
    explicit ParserIterator(
        Parser* parser,
        bool is_parse_end = false
    ) : parser_(parser), is_parse_end_(is_parse_end) {}

    bool operator==(const ParserIterator& other) const {
        if (this->is_parse_end_ && other.is_parse_end_) { return true; }
        if (this->is_parse_end_ != other.is_parse_end_) { return false; }
        return this->parser_ == other.parser_;
    }

    [[nodiscard]] Item operator*();

    ParserIterator& operator++();
};

class Parser {
private:
    mutable std::ifstream file_stream_;
    std::ifstream::pos_type file_size_;
    std::optional<Item> result_item_;
    bool is_parse_end_ = false;

public:
    Parser(const Parser&) = delete;
    Parser& operator=(const Parser&) = delete;

    template <ifstream_constructable T>
    explicit Parser(T val) : file_stream_(std::move(val)) {
        file_stream_.seekg(0, std::ios_base::end);
        file_size_ = file_stream_.tellg();
        file_stream_.seekg(0, std::ios_base::beg);
    }

    [[nodiscard]] ParserIterator begin() {
        return ParserIterator(this);
    }

    [[nodiscard]] ParserIterator end() {
        return ParserIterator(this, true);
    }

    [[nodiscard]] std::optional<Item>& get_result() {
        return result_item_;
    }

    [[nodiscard]] bool has_reached_end() const {
        return is_parse_end_;
    }

    void parse_next() {
        std::string result;
        char chr;
        while (true) {
            chr = this->file_stream_.get();
            if (chr == ' ') {
                break;
            }
            if (chr == std::char_traits<char>::eof()) {
                this->is_parse_end_ = true;
                this->result_item_ = std::nullopt;
                return;
            }
            result += chr;
        }
        this->result_item_ = Item(result);
    }
};

Item ParserIterator::operator*() {
    return parser_->get_result().value_or(Item("ERR"));
}

ParserIterator& ParserIterator::operator++() {
    if (!parser_->has_reached_end()) {
        parser_->parse_next();
        if (parser_->has_reached_end()) {
            is_parse_end_ = true;
        }
    }
    return *this;
}

}  // namespace base_file_parser
}  // namespace pbrt_parser
