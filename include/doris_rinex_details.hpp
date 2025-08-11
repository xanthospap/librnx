#ifndef __DSO_DORIS_RINEX_DETAILS_PR_HPP__
#define __DSO_DORIS_RINEX_DETAILS_PR_HPP__

#include <limits>
#include "datetime/calendar.hpp"

namespace dso {

namespace doris_rnx {

/** In DORIS RINEX files, the receiver clock offset may be missing for
 *  some/all epochs; this value signifies a missing epoch Receiver clock
 *  offset value.
 */
static constexpr double RECEIVER_CLOCK_OFFSET_MISSING =
    std::numeric_limits<double>::min();

/** In DORIS RINEX files, the observation value may be missing for some/all
 * epochs; this value signifies a missing observation value.
 */
static constexpr double OBSERVATION_VALUE_MISSING =
    std::numeric_limits<double>::min();

/* @brief A station (aka beacon) as defined in RINEX DORIS 3.0 (Issue 1.7) */
struct Beacon {

  /* Example: 'D31  DIOB DIONYSOS                      12602S012  4'
   *          #0123456789012345678901234567890123456789012345679801'
   *                     10        20        30        40        50
   * where all whitespace character are stored as '\0'.
   */
  char mpool[5+5+30+10+2] = {'\0'};

  /* Internal number used in data records */
  static constexpr const int m_internal_code_at = 0;
  const char *code() const noexcept {return mpool+m_internal_code_at;}
  char *code() noexcept {return mpool+m_internal_code_at;}

  /* 4-character station code */
  static constexpr const int m_station_id_at = 5;
  const char *id() const noexcept {return mpool+m_station_id_at;}
  char *id() noexcept {return mpool+m_station_id_at;}

  /* Station name */
  static constexpr const int m_station_name_at = 10;
  const char *name() const noexcept {return mpool+m_station_name_at;}
  char *name() noexcept {return mpool+m_station_name_at;}

  /* DOMES number */
  static constexpr const int m_station_domes_at = 40;
  const char *domes() const noexcept {return mpool+m_station_domes_at;}
  char *domes() noexcept {return mpool+m_station_domes_at;}

  
  /* Type, 1 for beacon 1.0, 2 for beacon 2.0 or 3 for beacon 3.0 */
  static constexpr const int m_type_at = 51;
  [[nodiscard]] int type() const noexcept {return (int)(mpool[m_type_at]);}
  
  /* Frequency shift factor K (signed) */
  int m_shift_factor;

  /* @return the antenna type (see enum GroundAntennaType) */
  // GroundAntennaType type() const;

  /** @brief Compute iono-free phase center for this antenna, w.r.t the
   *         Antenna's reference point.
   *         
   *  @return iono-free phase center w.r.t Antenna's reference point [m]
   *         such that:
   *         Iono-Free PC = Antenna RP + iono_free_phase_center()
   *  @see Lemoine etal, 2016, "Precise orbit determination and station
   *         position estimation using DORIS RINEX data", section 2.5.7
   */
  //Eigen::Matrix<double, 3, 1> iono_free_phase_center() const noexcept {
  //  // the 2GHz Phase center in [mm]
  //  const auto r_2Ghz = AntennaOffset<GroundAntennaType::Alcatel, 1>::pco() *
  //                          (type() == GroundAntennaType::Alcatel) +
  //                      AntennaOffset<GroundAntennaType::Starec_B, 1>::pco() *
  //                          (type() == GroundAntennaType::Starec_B ||
  //                           type() == GroundAntennaType::Starec_C);

  //  // the 400MHz Phase center in [mm]
  //  const auto r_400MHz = AntennaOffset<GroundAntennaType::Alcatel, 2>::pco() *
  //                            (type() == GroundAntennaType::Alcatel) +
  //                        AntennaOffset<GroundAntennaType::Starec_B, 2>::pco() *
  //                            (type() == GroundAntennaType::Starec_B ||
  //                             type() == GroundAntennaType::Starec_C);

  //  // iono-free phase center in [m] w.r.t Antenna Reference Point
  //  return (r_2Ghz + (r_2Ghz - r_400MHz) / (GAMMA_FACTOR - 1e0)) * 1e-3;
  //}

  /// @brief Concatenate Beacon information to a string
  /// @param[in] buffer A char array of length > 51 chars
  /// @return a pointer to the (formatted) buffer string
  // char *to_str(char *buffer) const noexcept;

  /** @brief Set instance's member as resolved from a DORIS RINEX record.
   *
   *  @param[in] line A RINEX 'STATION REFERENCE' record line
   *  @return Anything other than 0 denotes an error.
   *
   *  @see RINEX DORIS 3.0 (Issue 1.7)
   */
  int set_from_rinex_line(const char *line) noexcept;
}; /* struct Beacon */

/** @class TimeReferenceStation
 *
 *  This class holds time reference stations (beacons) recorded in a DORIS
 *  RINEX files. These stations are marked with the 'TIME REF STATION' in the
 *  RINEX header.
 *
 *  @note The m_station_code variable, aka the internal number used in data
 *        records, must correspond to a station in the 'STATION REFERENCE'
 *        field.
 */
struct TimeReferenceStation {
  /* Internal number used in data records */
  char m_station_code[4] = {'\0'};
  /* Bias of the time beacon reference VS TAI reference time, unit 1
   * microsecond
   */
  double m_bias;
  /* Time beacon reference shift unit 1e-14 second/second */
  double m_shift;

  const char *code() const noexcept {return m_station_code;}
}; /* struct TimeReferenceStation */

/** @class RinexDataRecordHeader
 *  This class holds fields of a data record header line as in DORIS RINEX
 *  files.
 *  @see RINEX DORIS 3.0 (Issue 1.7)
 */
struct RinexDataRecordHeader {
  /** Reference date record epoch (note that this time tag refers to the L1
   * sampling, for L2 you have to apply the 'L2 / L1 DATE OFFSET')
   */
  Datetime<dso::nanoseconds> m_epoch;
  /* Receiver clock offset in seconds (optional) */
  double m_clock_offset{RECEIVER_CLOCK_OFFSET_MISSING};
  /* Number of stations observed in current epoch */
  signed char m_num_stations;
  /* Epoch flag */
  signed char m_flag;
  /* Receiver clock offset flag, 1 if extrapolated, 0 otherwise */
  signed char m_clock_flag;

  /* @brief Apply the recorded (in RINEX) clock offset to time m_epoch
   *
   * Apply the correction (if any) and return the corrected time stamp. Note
   * that the actual instance's m_epoch is left as is.
   */
  Datetime<dso::nanoseconds> apply_clock_offset() const noexcept {
    dso::nanoseconds dt =
        (m_clock_offset == RECEIVER_CLOCK_OFFSET_MISSING)
            ? dso::nanoseconds(0)
            : dso::nanoseconds(static_cast<dso::nanoseconds::underlying_type>(
                  (m_clock_offset * dso::nanoseconds::sec_factor<double>())));
    return m_epoch.add_seconds(dt);
  }
}; /* class RinexDataRecordHeader */

struct RinexObservationValue {
  RinexObservationValue(double v, char f1, char f2) noexcept
      : m_value(v), m_flag1(f1), m_flag2(f2) {};
  /* The actual value parsed from the corresponding RINEX field */
  double m_value;
  /* The m1 and m2 flags */
  char m_flag1, m_flag2;
}; /* struct RinexObservationValue */

struct BeaconObservations {
  /* the observations made from the beacon at a selected epoch */
  std::vector<RinexObservationValue> m_values;
  /* internal beacon id (refernced in RINEX) */
  char m_beacon_id[4] = {'\0'};

  explicit BeaconObservations(int size_hint = 10) noexcept {
    m_values.reserve(size_hint);
  }

  const char *id() const noexcept { return m_beacon_id; }
}; /* struct BeaconObservations */

struct DataBlock {
  /* the block header */
  RinexDataRecordHeader mheader;
  /**/
  std::vector<BeaconObservations> mbeacon_obs;
}; /* struct DorisObsRinexDataBlock */


} /* namespace doris_rnx */
} /* namespace dso */
#endif
