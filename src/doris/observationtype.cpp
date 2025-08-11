#include <stdexcept>
#include <cstdio>
#include "obstypes.hpp"


char dso::dobstype_to_char(dso::DorisObservationType o) {
  switch (o) {
    case (DorisObservationType::phase):
      return 'L';
    case (DorisObservationType::pseudorange):
      return 'C';
    case (DorisObservationType::power_level):
      return 'W';
    case (DorisObservationType::frequency_offset):
      return 'F';
    case (DorisObservationType::ground_pressure):
      return 'P';
    case (DorisObservationType::ground_temperature):
      return 'T';
    case (DorisObservationType::ground_humidity):
      return 'H';
    default:
      throw std::runtime_error(
          "[ERROR] Cannot translate DorisObservationType to char");
  }
}

dso::DorisObservationType dso::char_to_dobstype(char c) {
  switch (c) {
    case ('L'):
      return DorisObservationType::phase;
    case ('C'):
      return DorisObservationType::pseudorange;
    case ('W'):
      return DorisObservationType::power_level;
    case ('F'):
      return DorisObservationType::frequency_offset;
    case ('P'):
      return DorisObservationType::ground_pressure;
    case ('T'):
      return DorisObservationType::ground_temperature;
    case ('H'):
      return DorisObservationType::ground_humidity;
    default:
      throw std::runtime_error(
          "[ERROR] Cannot translate char to DorisObservationType");
  }
}

/** The function will check the type DorisObservationType instance, to see if
 *  frequency information is needed for this DorisObservationType. That is,
 *  it will return true if type is any of:
 *  * DorisObservationType::phase, or
 *  * DorisObservationType::pseudorange, or
 *  * DorisObservationType::power_level
 */
bool dso::dobstype_has_frequency(dso::DorisObservationType type) noexcept {
  switch (type) {
    case (DorisObservationType::phase):
    case (DorisObservationType::pseudorange):
    case (DorisObservationType::power_level):
      return true;
    case (DorisObservationType::frequency_offset):
    case (DorisObservationType::ground_pressure):
    case (DorisObservationType::ground_temperature):
    case (DorisObservationType::ground_humidity):
    default:
      return false;
  }
}

/** If the ObservationType is any of phase, pseudorange or power_level, then
 *  the freq must be either 1 or 2. Otherwise (i.e. if type is not in phase,
 *  pseudorange or power_level), the passed in frequency is ignored and set to
 *  0.
 *
 *  If the type is any of phase, pseudorange or power_level, and freq is not
 *  in range [1,2], then the constructor will throw an std::runtime_error.
 */
dso::DorisObservationCode::DorisObservationCode(dso::DorisObservationType type,
                                                int freq)
    : m_type(type), m_freq(freq) {
  if (dso::dobstype_has_frequency(m_type)) {
    if (m_freq < 1 || m_freq > 2) {
      throw std::runtime_error("[ERROR] Invalid DORIS frequency number");
    }
  } else {
    m_freq = 0;
  }
}

char *dso::DorisObservationCode::to_str(char *buffer) const noexcept {
  std::sprintf(buffer, "%c%1d", dobstype_to_char(m_type), (int)m_freq);
  return buffer;
}
