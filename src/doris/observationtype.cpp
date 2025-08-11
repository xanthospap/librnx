#include "obstypes.hpp"
#include <stdexcept>

char dso::DorisObservationType_to_char(dso::DorisObservationType o) {
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

dso::DorisObservationType dso::char_to_observationType(char c) {
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

