#pragma once

#include <array>
#include <cassert>
#include <ostream>
#include <type_traits>

namespace FixLengthList {

template <typename T, std::size_t SIZE> class FixLengthList {
private:
  using size_t = std::size_t;
  class Element {
  public:
    T m_data;
    size_t m_pre = INVAILD;
    size_t m_post = INVAILD;
    static constexpr std::size_t INVAILD = static_cast<std::size_t>(-1);
    Element() = default;
    explicit Element(Element const &) = default;
    explicit Element(Element &&) = default;
    Element &operator=(Element const &) = default;
    Element &operator=(Element &&) = default;
    bool isFree() const { return m_pre == INVAILD; }
  };
  using Container = std::array<Element, SIZE>;

public:
  FixLengthList() {
    for (size_t i = 0; i < SIZE - 1; i++) { m_arr[i].m_post = i + 1; }
    m_arr[SIZE - 1].m_post = Element::INVAILD;
  }
  explicit FixLengthList(FixLengthList const &) = default;
  explicit FixLengthList(FixLengthList &&) = default;

  bool empty() const noexcept { return m_head == Element::INVAILD; }
  size_t size() const noexcept { return m_size; }
  constexpr size_t max_size() const noexcept { return SIZE; }

  T *front() {
    if (m_head == Element::INVAILD) { return nullptr; }
    return &m_arr[m_head].m_data;
  }
  T *back() {
    if (m_head == Element::INVAILD) { return nullptr; }
    auto pre = m_arr[m_head].m_pre;
    if (pre == Element::INVAILD) { return nullptr; }
    return &m_arr[pre].m_data;
  }

  template <class... Args> T *emplace(size_t pos, Args &&...args) {
    return create_element(
        [this, pos](size_t newPos) {
          assert(!m_arr[pos].isFree());
          const size_t pre = m_arr[pos].m_pre;
          link(pre, newPos);
          link(newPos, pos);
        },
        std::forward<Args...>(args...));
  }
  template <class... Args> T *emplace_back(Args &&...args) {
    return create_element(
        [this](size_t newPos) {
          if (m_head == Element::INVAILD) {
            assert(m_size == 1);
            m_head = newPos;
            link(newPos, newPos);
          } else {
            assert(m_size > 1);
            const size_t pre = m_arr[m_head].m_pre;
            link(pre, newPos);
            link(newPos, m_head);
          }
        },
        std::forward<Args...>(args...));
  }
  template <class... Args> T *emplace_front(Args &&...args) {
    return create_element(
        [this](size_t newPos) {
          if (m_head == Element::INVAILD) {
            assert(m_size == 1);
            link(newPos, newPos);
          } else {
            assert(m_size > 1);
            size_t pre = m_arr[m_head].m_pre;
            link(newPos, m_head);
            link(pre, newPos);
          }
          m_head = newPos;
        },
        std::forward<Args...>(args...));
  }

  void pop_back() {
    const size_t erasePos = m_arr[m_head].m_pre;
    erase(erasePos);
  }
  void pop_front() {
    const size_t erasePos = m_head;
    m_head = m_arr[m_head].m_post;
    erase(erasePos);
  }

  void erase(size_t pos) {
    assert(m_size > 0);
    Element &element = m_arr[pos];
    assert(!element.isFree());
    T(std::move(element.m_data));
    size_t pre = element.m_pre;
    size_t post = element.m_post;
    if (m_size > 1) {
      link(pre, post);
    } else {
      m_head = Element::INVAILD;
    }
    element.m_pre = Element::INVAILD;
    element.m_post = m_free;
    m_free = pos;
    m_size--;
  }

  friend std::ostream &operator<<(std::ostream &os, FixLengthList const &self) {
    os << "\narray order: ";
    for (Element const &element : self.m_arr) {
      if (element.isFree()) {
        os << "null ";
      } else {
        os << element.m_data << " ";
      }
    }
    os << "\nlist order: ";
    size_t curr = self.m_head;
    for (size_t i = 0; i < self.size(); i++) {
      Element const &currElement = self.m_arr[curr];
      os << currElement.m_data << " -> ";
      curr = currElement.m_post;
    }
    return os;
  }

private:
  void link(size_t pre, size_t post) {
    m_arr[pre].m_post = post;
    m_arr[post].m_pre = pre;
  }

  template <class Fn, class... Args> T *create_element(Fn func, Args &&...args) {
#if __cplusplus >= 201703L
    static_assert(std::is_invocable_r_v<void, Fn, size_t>);
#endif
    if (m_free == Element::INVAILD) { return nullptr; }
    const size_t newPos = m_free;
    Element &newElement = m_arr[newPos];
    // update m_free first
    m_free = newElement.m_post;
    newElement.m_data = T(std::forward<Args...>(args...));
    m_size++;
    func(newPos);
    return &newElement.m_data;
  }

private:
  Container m_arr{};
  size_t m_head = Element::INVAILD;
  size_t m_free = 0;
  size_t m_size = 0;
};
}; // namespace FixLengthList
