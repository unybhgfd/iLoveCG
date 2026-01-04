#include <iostream>
#include <fstream>
#include <optional>
#include <string>
#include <utility>

namespace parser {
namespace parser_base {
// template <typename ItemType>
// concept typename = std::is_copy_assignable_v<ItemType>;

template <typename Derived, typename ItemType>
class ParserIterator;
template <typename Derived, typename ItemType>
class Parser;

template <typename Derived, typename ItemType>
class ParserIterator {
private:
    Parser<Derived, ItemType>* parser_;
    bool is_parse_end_ = false;

public:
    explicit ParserIterator(
        Parser<Derived, ItemType>* parser,
        bool is_parse_end = false
    ) : parser_(parser), is_parse_end_(is_parse_end) {}

    bool operator==(const ParserIterator& other) const {
        if (this->is_parse_end_ && other.is_parse_end_) { return true; }
        if (this->is_parse_end_ != other.is_parse_end_) { return false; }
        return this->parser_ == other.parser_;
    }

    [[nodiscard]] ItemType operator*();

    ParserIterator& operator++();
};

template <typename Derived, typename ItemType>
class Parser {
    friend Derived;

private:
    Parser() = default;
    std::optional<ItemType> result_item_;
    bool is_parse_end_ = false;

public:
    [[nodiscard]] ParserIterator<Derived, ItemType> begin() {
        return ++ParserIterator(this);
    }

    [[nodiscard]] ParserIterator<Derived, ItemType> end() {
        return ParserIterator(this, true);
    }

    [[nodiscard]] std::optional<ItemType>& get_result() {
        return result_item_;
    }

    [[nodiscard]] bool has_reached_end() const {
        return is_parse_end_;
    }
};

template <typename Derived, typename ItemType>
ItemType ParserIterator<Derived, ItemType>::operator*() {
    return parser_->get_result().value();
}

template <typename Derived, typename ItemType>
ParserIterator<Derived, ItemType>& ParserIterator<Derived, ItemType>::operator++() {
    if (parser_->has_reached_end()) {
        is_parse_end_ = true;
    } else {
        static_cast<Derived*>(parser_)->parse_next();
    }
    return *this;
}

}  // namespace parser_base


namespace test {
template<typename T>
concept ifstream_constructable = requires (T val) {
    std::ifstream(val);
};

struct Item {
    std::string data;

    explicit Item(std::string data) :data(std::move(data)) {}
};

class Parser : public parser_base::Parser<Parser, Item> {
private:
    mutable std::ifstream file_stream_;
    std::ifstream::pos_type file_size_;

public:
    Parser(const Parser&) = delete;
    Parser& operator=(const Parser&) = delete;

    void parse_next() {
        if (is_parse_end_) { return; }
        std::string result;
        char chr;
        while (true) {
            chr = this->file_stream_.get();
            if (chr == ' ') {
                break;
            }
            if (chr == std::char_traits<char>::eof()) {
                this->is_parse_end_ = true;
                this->result_item_ = Item(result);
                return;
            }
            result += chr;
        }
        this->result_item_ = Item(result);
    }

    template <ifstream_constructable T>
    explicit Parser(T val) : parser_base::Parser<Parser, Item>() {
        file_stream_ = std::ifstream(std::move(val));
        file_stream_.seekg(0, std::ios_base::end);
        file_size_ = file_stream_.tellg();
        file_stream_.seekg(0, std::ios_base::beg);
    }
};
}  // namespace test
}  // namespace parser
