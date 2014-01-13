
#ifndef APPLICATION_IMAGEREGISTRATIONTOOLS_UTILS_LOCALMAPPING_H
#define APPLICATION_IMAGEREGISTRATIONTOOLS_UTILS_LOCALMAPPING_H

//----------------------------------------------------------------
// local_mapping_t
// 
class local_mapping_t
{
public:
  local_mapping_t(const unsigned int & from = UINT_MAX,
                  const unsigned int & to = UINT_MAX,
                  const double & dist = std::numeric_limits<double>::max()):
  from_(from),
  to_(to),
  distance_(dist)
  {}
  
  inline bool operator < (const local_mapping_t & m) const
  {
    if (to_ == m.to_) return distance_ < m.distance_;
    return to_ < m.to_;
  }
  
  inline bool operator == (const local_mapping_t & m) const
  { return to_ == m.to_; }
  
  unsigned int from_;
  unsigned int to_;
  double distance_;
};

#endif