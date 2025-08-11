#ifndef __DSO_SPACE_GEODESY_OBSERVATION_TYPES_HPP__
#define __DSO_SPACE_GEODESY_OBSERVATION_TYPES_HPP__

namespace dso {

/** @enum DorisObservationType
 *  DORIS Observation Types as defined in RINEX DORIS 3.0 (Issue 1.7)
 */
enum class DorisObservationType : char {
  phase,              ///< L
  pseudorange,        ///< C
  power_level,        ///< W power level received at each frequency, unit dBm
  frequency_offset,   ///< F relative frequency offset of the receiver’s
                      ///< oscillator (f-f0) / f0, unit 10e-11
  ground_pressure,    ///< P ground pressure at the station, unit 100 Pa (mBar)
  ground_temperature, ///< T ground temperature at the station, unit degree
                      ///< Celsius
  ground_humidity,    ///< H ground humidity at the station, unit percent
}; /* enum ObservationType */

/** @brief Translate a DorisObservationType to a character.
 *  
 *  @param[in] obs A DorisObservationType to translate to char
 *  @return A character that corresposnds to the given DorisObservationType.
 *
 *  @throw std::runtime_error if no corresponding char is found for the given
 *         ObservationType.
 */
char dobstype_to_char(DorisObservationType o);

/** @brief Translate a character to a DorisObservationType.
 * 
 * @param[in] c A char corresponging to a DorisObservationType
 * @return A DorisObservationType that corresponds to the given character.
 *
 * @throw std::runtime_error if no corresponding DorisObservationType is 
 *        found for the given char.
 */
DorisObservationType char_to_dobstype(char c);

/** @bried Check if a DorisObservationType goes with a frequuency. 
 *
 * The function will check the type DorisObservationType instance, to see if
 * frequency information is needed for this DorisObservationType. That is,
 * it will return true if type is any of:
 *  * DorisObservationType::phase, or
 *  * DorisObservationType::pseudorange, or
 *  * DorisObservationType::power_level
 *
 * @param[in] type The DorisObservationType considered
 * @return True if type is frequency-dependent, False otherwise.
 */
bool dobstype_has_frequency(DorisObservationType type) noexcept;

/** @brief Observation Code as defined in RINEX DORIS 3.0 (Issue 1.7)
 *
 * An Observation code is actually a colletion of an Observation Type and
 * (if needed) a frequency. Frequency numbers are only relevant for
 * Observation Types of type ObservationType::phase,
 * ObservationType::pseudorange and ObservationType::power_level. In any other
 * case, m_freq is irrelevant and set to 0.a
 *
 * Frequency is defined by an integer, which can be:
 * * 1 to denote the S1 DORIS frequency (on 2 GHz), or
 * * 2 to denote the U2 DORIS frequency (on 400 MHz)
 * 
 * @warning m_freq MUST be set to 0, if m_type is one of:
 *          * frequency_offset,
 *          * ground_pressure,
 *          * ground_temperature,
 *          * ground_humidity
 */
struct DorisObservationCode {
  DorisObservationType m_type;
  signed char m_freq{0};

  /** @brief Constructor; may throw if the frequency is not valid.
   *
   *  @param[in] type The Observation Type
   *  @param[in] freq The corresponding frequency (if any)
   * 
   * @throw std::runtime_error if the passed-in frequency is not valid.
   */
  explicit DorisObservationCode(DorisObservationType type, int freq = 0);

  /* @brief get the ObservationType */
  auto dobstype() const noexcept { return m_type; }

  /* @brief Equality comparisson; checks both ObservationType and frequency. */
  bool operator==(const DorisObservationCode &oc) const noexcept {
    return m_type == oc.m_type && m_freq == oc.m_freq;
  }

  /* @brief InEquality comparisson; checks both ObservationType and frequency. */
  bool operator!=(const DorisObservationCode &oc) const noexcept {
    return !(this->operator==(oc));
  }

  /* @brief Check if the ObservationCode has a corresponding frequency */
  bool has_frequency() const noexcept {return dobstype_has_frequency(m_type);}

  /** @brief Format to string (string length=2+null terminating char)
   *
   *  @param[in] buffer A char array of length >= 3 chars
   *  @return a pointer to the (formatted) buffer string
   */
  char *to_str(char *buffer) const noexcept;
}; /* class ObservationCode */

} /* namespace dso */

#endif
