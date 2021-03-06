namespace simdjson {
namespace SIMDJSON_IMPLEMENTATION {
namespace ondemand {

class document;
class object;
class array;
class value;
class raw_json_string;
class parser;

/**
 * Iterates through JSON tokens, keeping track of depth and string buffer.
 *
 * @private This is not intended for external use.
 */
class json_iterator {
protected:
  token_iterator token{};
  ondemand::parser *parser{};
  /**
   * Next free location in the string buffer.
   *
   * Used by raw_json_string::unescape() to have a place to unescape strings to.
   */
  uint8_t *_string_buf_loc{};
  /**
   * JSON error, if there is one.
   *
   * INCORRECT_TYPE and NO_SUCH_FIELD are *not* stored here, ever.
   *
   * PERF NOTE: we *hope* this will be elided into control flow, as it is only used (a) in the first
   * iteration of the loop, or (b) for the final iteration after a missing comma is found in ++. If
   * this is not elided, we should make sure it's at least not using up a register. Failing that,
   * we should store it in document so there's only one of them.
   */
  error_code error{SUCCESS};
  /**
   * Depth of the current token in the JSON.
   *
   * - 0 = finished with document
   * - 1 = document root value (could be [ or {, not yet known)
   * - 2 = , or } inside root array/object
   * - 3 = key or value inside root array/object.
   */
  depth_t _depth{};

public:
  simdjson_really_inline json_iterator() noexcept = default;
  simdjson_really_inline json_iterator(json_iterator &&other) noexcept;
  simdjson_really_inline json_iterator &operator=(json_iterator &&other) noexcept;
  simdjson_really_inline json_iterator(const json_iterator &other) noexcept = delete;
  simdjson_really_inline json_iterator &operator=(const json_iterator &other) noexcept = delete;

  /**
   * Skips a JSON value, whether it is a scalar, array or object.
   */
  simdjson_warn_unused simdjson_really_inline error_code skip_child(depth_t parent_depth) noexcept;

  /**
   * Tell whether the iterator is still at the start
   */
  simdjson_really_inline bool at_start() const noexcept;

  /**
   * Assert if the iterator is not at the start
   */
  simdjson_really_inline void assert_at_start() const noexcept;

  /**
   * Tell whether the iterator is at the EOF mark
   */
  simdjson_really_inline bool at_eof() const noexcept;

  /**
   * Tell whether the iterator is live (has not been moved).
   */
  simdjson_really_inline bool is_alive() const noexcept;

  /**
   * Abandon this iterator, setting depth to 0 (as if the document is finished).
   */
  simdjson_really_inline void abandon() noexcept;

  /**
   * Advance the current token.
   */
  simdjson_really_inline const uint8_t *advance() noexcept;

  /**
   * Whether we are at the start of an object.
   */

  /**
   * Get the JSON text for a given token (relative).
   *
   * This is not null-terminated; it is a view into the JSON.
   *
   * @param delta The relative position of the token to retrieve. e.g. 0 = next token, -1 = prev token.
   *
   * TODO consider a string_view, assuming the length will get stripped out by the optimizer when
   * it isn't used ...
   */
  simdjson_really_inline const uint8_t *peek(int32_t delta=0) const noexcept;
  /**
   * Get the maximum length of the JSON text for a given token.
   *
   * The length will include any whitespace at the end of the token.
   *
   * @param delta The relative position of the token to retrieve. e.g. 0 = next token, -1 = prev token.
   */
  simdjson_really_inline uint32_t peek_length(int32_t delta=0) const noexcept;

  /**
   * Ascend one level.
   *
   * Validates that the depth - 1 == parent_depth.
   *
   * @param parent_depth the expected parent depth.
   */
  simdjson_really_inline void ascend_to(depth_t parent_depth) noexcept;

  /**
   * Descend one level.
   *
   * Validates that the new depth == child_depth.
   *
   * @param child_depth the expected child depth.
   */
  simdjson_really_inline void descend_to(depth_t parent_depth) noexcept;

  /**
   * Get current depth.
   */
  simdjson_really_inline depth_t depth() const noexcept;

  /**
   * Get current (writeable) location in the string buffer.
   */
  simdjson_really_inline uint8_t *&string_buf_loc() noexcept;

  /**
   * Report an error, preventing further iteration.
   *
   * @param error The error to report. Must not be SUCCESS, UNINITIALIZED, INCORRECT_TYPE, or NO_SUCH_FIELD.
   * @param message An error message to report with the error.
   */
  simdjson_really_inline error_code report_error(error_code error, const char *message) noexcept;

  /**
   * Log error, but don't stop iteration.
   * @param error The error to report. Must be INCORRECT_TYPE, or NO_SUCH_FIELD.
   * @param message An error message to report with the error.
   */
  simdjson_really_inline error_code optional_error(error_code error, const char *message) noexcept;

  template<int N> simdjson_warn_unused simdjson_really_inline bool copy_to_buffer(const uint8_t *json, uint32_t max_len, uint8_t (&tmpbuf)[N]) noexcept;
  template<int N> simdjson_warn_unused simdjson_really_inline bool peek_to_buffer(uint8_t (&tmpbuf)[N]) noexcept;
  template<int N> simdjson_warn_unused simdjson_really_inline bool advance_to_buffer(uint8_t (&tmpbuf)[N]) noexcept;

protected:
  simdjson_really_inline json_iterator(ondemand::parser *parser) noexcept;

  friend class document;
  friend class object;
  friend class array;
  friend class value;
  friend class raw_json_string;
  friend class parser;
  friend class value_iterator;
  friend simdjson_really_inline void logger::log_line(const json_iterator &iter, const char *title_prefix, const char *title, std::string_view detail, int delta, int depth_delta) noexcept;
}; // json_iterator

} // namespace ondemand
} // namespace SIMDJSON_IMPLEMENTATION
} // namespace simdjson

namespace simdjson {

template<>
struct simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::json_iterator> : public SIMDJSON_IMPLEMENTATION::implementation_simdjson_result_base<SIMDJSON_IMPLEMENTATION::ondemand::json_iterator> {
public:
  simdjson_really_inline simdjson_result(SIMDJSON_IMPLEMENTATION::ondemand::json_iterator &&value) noexcept; ///< @private
  simdjson_really_inline simdjson_result(error_code error) noexcept; ///< @private

  simdjson_really_inline simdjson_result() noexcept = default;
  simdjson_really_inline simdjson_result(simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::json_iterator> &&a) noexcept = default;
  simdjson_really_inline ~simdjson_result() noexcept = default; ///< @private
};

} // namespace simdjson
