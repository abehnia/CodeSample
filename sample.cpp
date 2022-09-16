bool keysExist(const List &list, const std::string &key) {
  auto it = list | ranges::views::transform(
                       [&key](const auto &x) { return x.at(key); });
  return all_of(it.begin(), it.end(),
                [](const auto &x) { return x.has_value(); });
}

struct Discard {
  size_t count;
  explicit Discard(size_t count) noexcept : count{count} {};

  auto operator()(List &list) noexcept {
    if (count == 0) {
      return;
    }
    count = (count >= list.size()) ? list.size() : count;
    list.erase(list.end() - count, list.end());
  }
  auto operator()(auto &&discard) noexcept { return; }
};

struct Deal {
  size_t count;
  explicit Deal(size_t count) noexcept : count{count} {};

  auto operator()(List &to, List &from) noexcept {
    if (count == 0) {
      return;
    }
    count = (count >= from.size()) ? from.size() : count;
    move(from.end() - count, from.end(), to.end());
    from.erase(from.end() - count, from.end());
  }
  auto operator()(auto &&x, auto &&y) noexcept { return; }
};

struct Print {
  ostream &os;
  explicit Print(ostream &os) : os{os} {}

  auto operator()(const Map &map) -> ostream & {
    os << "{";
    join(map.begin(), map.end(), os, ", ", [](const auto &x) {
      stringstream ss;
      ss << x.first << ": " << x.second;
      return ss.str();
    });
    os << "}";
    return os;
  }
  auto operator()(const List &list) -> ostream & {
    os << "[";
    join(list.begin(), list.end(), os, ", ", [](const auto &x) { return x; });
    os << "]";
    return os;
  }
  auto operator()(monostate x) -> ostream & {
    os << "nil";
    return os;
  }
  auto operator()(const string &x) -> ostream & {
    os << "\"" << x << "\"";
    return os;
  }
  auto operator()(const auto &x) -> ostream & {
    os << x;
    return os;
  }
};

void discard(DSLValue &x, size_t count) noexcept {
  return x.unaryOperation(Discard{count});
}
void deal(DSLValue &from, DSLValue &&to, size_t count) noexcept {
  return from.binaryOperation(to, Deal{count});
}
void deal(DSLValue &from, DSLValue &to, size_t count) noexcept {
  return from.binaryOperation(to, Deal{count});
}
ostream &operator<<(ostream &os, const DSLValue &x) noexcept {
  return x.unaryOperation(Print{os});
}

class DSLValue {
public:
  template <UnaryDSLOperation F> decltype(auto) unaryOperation(F &&f) {
    return std::visit(f, value);
  }

  template <UnaryDSLOperation F> decltype(auto) unaryOperation(F &&f) const {
    return std::visit(f, value);
  }

  template <DSL U, BinaryDSLOperation F>
  decltype(auto) binaryOperation(U &&other, F &&f) {
    return std::visit(f, value, other.value);
  }

  template <DSL U, BinaryDSLOperation F>
  decltype(auto) binaryOperation(const U &other, F &&f) const {
    return std::visit(f, value, other.value);
  }
}
