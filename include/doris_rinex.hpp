#ifndef __DSO_DORIS_RINEX_V3_HPP__
#define __DSO_DORIS_RINEX_V3_HPP__

#include "obstypes.hpp"
#include "doris_rinex_details.hpp"
#include <fstream>
#include <vector>

namespace dso {

/** @class DorisObsRinex
 *  @brief A class to hold DORIS Observation RINEX files for reading.
 *  @see RINEX DORIS 3.0 (Issue 1.7),
 *       ftp://ftp.ids-doris.org/pub/ids/data/RINEX_DORIS.pdf
 */
class DorisObsRinex {
public:
  /* Let's not write this more than once. */
  typedef std::ifstream::pos_type pos_type;

  /* No header line can have more than 80 chars. */
  static constexpr int MAX_HEADER_CHARS{81};

  /* No record line can have more than 3+5*16=83 chars */
  static constexpr int MAX_RECORD_CHARS{124};

private:
  /* The name of the file */
  std::string m_filename;
  /* The infput (file) stream; open at construction */
  std::ifstream m_stream;
  /* RINEX version */
  float m_version;

  char m_char_pool[256];
  /* Satellite name */
  static constexpr int m_satellite_name_at = 0; // [60]
  /* COSPAR number */
  static constexpr int m_cospar_number_at = 60; // [20]
  /* DORIS chain used (chain1 or chain2), exp. “CHAIN1” */
  static constexpr int m_rec_chain_at = 80; // [20]
  /* DORIS instrument type; exp. “DGXX” */
  static constexpr int m_rec_type_at = 100; // [20]
  /* The software version used on board DORIS/DIODE, exp. “1.00” */
  static constexpr int m_rec_version_at = 120; // [20]
  /* The antenna type is “STAREC” */
  static constexpr int m_antenna_type_at = 140; // [20]
  /* The antenna number is “DORIS” */
  static constexpr int m_antenna_number_at = 160; // [20]
  
  /* Position of 2 GHz phase center, in the platform reference frame (Units:
   * Meters, System: ITRS recommended)
   */
  float m_approx_position[3];
  
  /** The center of mass of the vehicle (for space borne receivers):
   *  CENTER OF MASS: XYZ, defined at the beginning of the mission.
   */
  float m_center_mass[3];
  
  /* A vector of ObservationCode contained in the RINEX file */
  std::vector<DorisObservationCode> m_obs_codes;
  
  /** A vector of scale factors corresponding to m_obs_codes (aka they have
   *  the same size with a one-to-one correspondance
   */
  std::vector<int> m_obs_scale_factors;
  
  /* Datetime of first observation in RINEX */
  Datetime<nanoseconds> m_time_of_first_obs;
  
  /** This date corresponds to the day of the first measurement performed on
   *  the first time reference beacon in the DORIS RINEX product, at
   *  00h 00mn 00s.
   */
  Datetime<nanoseconds> m_time_ref_stat;
  
  /** Constant shift between the date of the 400MHz phase measurement and the
   *  date of the 2GHz phase measurement in microseconds. Positive if the
   *  measurement of phase 400 MHz is performed after the measurement of phase
   *  2 GHz
   */
  double m_l12_date_offset;
  
  /** Epoch, code, and phase are corrected by applying the realtime-derived
   *  receiver clock offset: 1=yes, 0=no; default: 0=no
   */
  bool rcv_clock_offs_appl{false};
  
  /* List of stations/beacons recorded in file */
  std::vector<doris_rnx::Beacon> m_stations;
  
  /* List of time-reference stations in file (also included in m_stations) */
  std::vector<doris_rnx::TimeReferenceStation> m_ref_stations;
  
  /* Mark the 'END OF HEADER' field (next line is record line) */
  pos_type m_end_of_head;
  
  /* Record lines for each beacon (in data record blocks) */
  int m_lines_per_beacon;

  /** Depending on the number of observables, compute the number of lines 
   * needed to hold a full data record. Each data line can hold up to 5
   * observable values.
   */
  int lines_per_beacon() const noexcept {
    const int obs = m_obs_codes.size();
    return 1 + (!(obs % 5) ? (obs / 5 - 1) : (obs / 5));
  }

  /**/
  int read_header() noexcept;
  
  char *satellite_name() noexcept {return m_char_pool + m_satellite_name_at;}
  char *cospar_number()  noexcept {return m_char_pool + m_cospar_number_at;}
  char *rec_chain()      noexcept {return m_char_pool + m_rec_chain_at;}
  char *rec_type()       noexcept {return m_char_pool + m_rec_type_at;}
  char *rec_version()    noexcept {return m_char_pool + m_rec_version_at;}
  char *antenna_type()   noexcept {return m_char_pool + m_antenna_type_at;}
  char *antenna_number() noexcept {return m_char_pool + m_antenna_number_at;}
  
public:
  const char *satellite_name() const noexcept {return m_char_pool + m_satellite_name_at;}
  const char *cospar_number() const noexcept {return m_char_pool + m_cospar_number_at;}
  const char *rec_chain() const noexcept {return m_char_pool + m_rec_chain_at;}
  const char *rec_type() const noexcept {return m_char_pool + m_rec_type_at;}
  const char *rec_version() const noexcept {return m_char_pool + m_rec_version_at;}
  const char *antenna_type() const noexcept {return m_char_pool + m_antenna_type_at;}
  const char *antenna_number() const noexcept {return m_char_pool + m_antenna_number_at;}

  /* @brief Constructor from filename 
   *
   * The c'tor will open the and call read_header(), which will parse through 
   * the RINEXE's header all get all info.
   * If it fails, an exception will be thrown.
   */
  explicit DorisObsRinex(const char *);

  /* @brief Destructor */
  ~DorisObsRinex() noexcept; // = default;

  /* @brief Copy not allowed ! */
  DorisObsRinex(const DorisObsRinex &) = delete;

  /* @brief Assignment not allowed ! */
  DorisObsRinex &operator=(const DorisObsRinex &) = delete;

  /* @brief Move Constructor. */
  DorisObsRinex(DorisObsRinex &&a) noexcept(
      std::is_nothrow_move_constructible<std::ifstream>::value) = default;

  /* @brief Move assignment operator. */
  DorisObsRinex &operator=(DorisObsRinex &&a) noexcept(
      std::is_nothrow_move_assignable<std::ifstream>::value) = default;

  struct iterator {
        using value_type = doris_rnx::DataBlock;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::input_iterator_tag;
        using pointer = const doris_rnx::DataBlock*;
        using reference = const doris_rnx::DataBlock&;

        iterator() = default; /* end iterator */
        explicit iterator(std::shared_ptr<std::ifstream> in)
            : in_(std::move(in)) { ++(*this); }  // prime first record

        reference operator*()  const { return *current_; }
        pointer   operator->() const { return &*current_; }

        // pre-increment: read next record
        iterator& operator++() {
            current_.reset();
            if (!in_) return *this;

            std::string line;
            while (std::getline(*in_, line)) {
                if (line.empty()) continue; // skip blanks
                current_ = parse_line(line);
                break;
            }
            if (!current_) in_.reset(); // reached EOF -> become end()
            return *this;
        }

        iterator operator++(int) { auto tmp = *this; ++(*this); return tmp; }

        // Equality only guaranteed vs end() for input iterators.
        friend bool operator==(const iterator& a, const iterator& b) {
            // Both end if both have no stream.
            return (!a.in_ && !b.in_);
        }

        friend bool operator!=(const iterator& a, const iterator& b) {
            return !(a == b);
        }

    private:
        static doris_rnx::Block parse_line(const char *line) {
            return Block{ parse_datetime(datetime), std::move(obs) };
        }

        std::ifstream *m_in;
        doris_rnx::DataBlock m_current;
    };
}; /* class DorisRinex */

} /* namespace dso */

#endif
