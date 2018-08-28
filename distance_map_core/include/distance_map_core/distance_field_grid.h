#ifndef _DISTANCE_MAP_CORE_DISTANCE_FIELD_GRID_H_
#define _DISTANCE_MAP_CORE_DISTANCE_FIELD_GRID_H_

#include <stdexcept>

namespace distmap {

class DistanceFieldGrid
{
public:

  struct Dimension
  {
    Dimension(const std::size_t width, const std::size_t height);
    std::size_t width, height;
  };

  struct Origin
  {
    Origin()  = default;
    ~Origin() = default;

    Origin(const double x, const double y,
           const double yaw);

    double x   = 0,
           y   = 0,
           yaw = 0;
  };

  struct Gradient
  {
    double dx, dy;
  };

  DistanceFieldGrid(const Dimension& dimension,
                    const double resolution,
                    const Origin& origin /*= Origin()*/);

  ~DistanceFieldGrid();

  void resize(const std::size_t row, const std::size_t col);

  bool isCellValid(const std::size_t row, const std::size_t col) const noexcept;
  bool isPositionValid(const double x, const double y) const noexcept;

  void cellToPosition(const std::size_t row, const std::size_t col,
                      double& x, double& y) const;

  void positionToCell(const double x, const double y,
                      std::size_t& row, std::size_t& col) const;

  /**
   * @brief atCell
   * @param row
   * @param col
   * @return The distance to the closest black (0) cell, in cell unit.
   */
  double atCell(const std::size_t row, const std::size_t col) const;

  /**
   * @brief atCellSafe, if cell is out of bound, return 0 (obstacle).
   * @param row
   * @param col
   * @return The distance to the closest black (0) cell, in cell unit.
   */
  double atCellSafe(const std::size_t row, const std::size_t col) const;

  /**
   * @brief atPosition
   * @param x,
   * @param y
   * @return The distance to the closest black (0) cell, in m unit.
   */
  double atPosition(const double x, const double y) const;

  /**
   * @brief atPositionSafe, if position is out of bound, return 0 (obstacle).
   * @param x
   * @param y
   * @return The distance to the closest black (0) cell, in m unit.
   */
  double atPositionSafe(const double x, const double y) const;

  /**
   * @brief gradientAtCell
   * @param row
   * @param col
   * @return
   */
  Gradient gradientAtCell(std::size_t row, std::size_t col) const;

  /**
   * @brief gradientAtPosition
   * @param x
   * @param y
   * @return
   */
  Gradient gradientAtPosition(const double x, const double y) const;

  /**
   * @brief gradientAtCellSafe
   * @param row
   * @param col
   * @return
   */
  Gradient gradientAtCellSafe(std::size_t row, std::size_t col) const;

  /**
   * @brief gradientAtPositionSafe
   * @param x
   * @param y
   * @return
   */
  Gradient gradientAtPositionSafe(const double x, const double y) const;

  // Setter/getter

  double* data();
  const double* data() const;

  //void setDimension(const Dimension& dimension);
  void setOrigin(const Origin& origin);
  void setResolution(const double resolution);

  const Dimension& getDimension()  const noexcept;
  const double&    getResolution() const noexcept;
  const Origin&    getOrigin()     const noexcept;

protected:

  bool initialized_ = false;

  Dimension dimension_; ///< @brief Dimension of the grid in cells.
  double resolution_ = 1;
  Origin origin_; ///< @brief The 2-D pose of the bottom-left pixel in the map.

  double* data_;

  std::size_t getIndex(const std::size_t row, const std::size_t col) const;

  void assertIsValidCell(const std::size_t& row, const std::size_t& col) const;
  void assertIsValidPosition(const double& x, const double& y) const;
};

using DistanceFieldGridPtr = std::shared_ptr<DistanceFieldGrid>;
using DistanceFieldGridConstPtr = std::shared_ptr<const DistanceFieldGrid>;

DistanceFieldGrid::Dimension::Dimension(const std::size_t width, const std::size_t height)
  : width(width), height(height)
{
  if (width == 0)
    throw std::runtime_error("Dimension x can't be zero !");

  if (height == 0)
    throw std::runtime_error("Dimension x can't be zero !");
}

DistanceFieldGrid::Origin::Origin(const double _x, const double _y,
                                  const double _yaw)
  : x(_x), y(_y), yaw(_yaw)
{
  //
}

DistanceFieldGrid::DistanceFieldGrid(const Dimension& dimension,
                                     const double resolution,
                                     const Origin& origin)
  : dimension_(dimension)
  , resolution_(resolution)
  , origin_(origin)
{
  if (resolution_ <= 0)
    throw std::runtime_error("Resolution can't be zero nor negative !");

  data_ = new double[dimension_.width*dimension_.height];
  initialized_ = true;
}

DistanceFieldGrid::~DistanceFieldGrid()
{
  if (initialized_)
    delete data_;
}

void DistanceFieldGrid::resize(const std::size_t row, const std::size_t col)
{
  if (initialized_)
    delete data_;

  dimension_ = Dimension(row, col);
  data_ = new double[dimension_.width*dimension_.height];
  initialized_ = true;
}

bool DistanceFieldGrid::isCellValid(const std::size_t row, const std::size_t col) const noexcept
{
  return (row < dimension_.height) && (col < dimension_.width);
}

bool DistanceFieldGrid::isPositionValid(const double x, const double y) const noexcept
{
  std::size_t row, col;
  positionToCell(x,y,row,col);

  return isCellValid(row, col);
}

void DistanceFieldGrid::cellToPosition(const std::size_t row,
                                       const std::size_t col,
                                       double& x, double& y) const
{
  /*R*/
  const double cos_yaw = std::cos(origin_.yaw);
  const double sin_yaw = std::sin(origin_.yaw);
  const double xt = double(row) * resolution_ + origin_.x;
  const double yt = double(col) * resolution_ + origin_.y;
  x = cos_yaw*xt - sin_yaw*yt;
  y = sin_yaw*xt + cos_yaw*yt;
}

void DistanceFieldGrid::positionToCell(const double x, const double y,
                                       std::size_t& row, std::size_t& col) const
{
  /*R^-1*/
  const double cos_yaw = std::cos(origin_.yaw);
  const double sin_yaw = std::sin(origin_.yaw);
  const double x_diff = x - origin_.x;
  const double y_diff = y - origin_.y;
  col = static_cast<std::size_t>(std::floor(( cos_yaw * x_diff + sin_yaw * y_diff) / resolution_));
  row = static_cast<std::size_t>(std::floor((-sin_yaw * x_diff + cos_yaw * y_diff) / resolution_));
}

double DistanceFieldGrid::atCell(const std::size_t row, const std::size_t col) const
{
  assertIsValidCell(row, col);
  return data_[getIndex(row, col)];
}

double DistanceFieldGrid::atCellSafe(const std::size_t row, const std::size_t col) const
{
  /// @todo return -dist_to_in_bound ?
  return isCellValid(row,col)? data_[getIndex(row, col)] : 0;
}

double DistanceFieldGrid::atPosition(const double x, const double y) const
{
  assertIsValidPosition(x,y);

  const double lx = std::floor(x),
               ly = std::floor(y);
  const double hx = lx + 1.0,
               hy = ly + 1.0;

  std::size_t lxi,lyi,hxi,hyi;
  positionToCell(lx,ly,lxi,lyi);
  positionToCell(hx,hy,hxi,hyi);

  return
      (hx-x)*(hy-y)*data_[getIndex(lxi, lyi)] +
      (x-lx)*(hy-y)*data_[getIndex(hxi, lyi)] +
      (hx-x)*(y-ly)*data_[getIndex(lxi, hyi)] +
      (x-lx)*(y-ly)*data_[getIndex(hxi, hyi)] ;
}

double DistanceFieldGrid::atPositionSafe(const double x, const double y) const
{
  /// @todo return -dist_to_in_bound ?
  double d = 0;
  if (isPositionValid(x,y) && isPositionValid(x+1,y+1))
  {
    d = atPosition(x,y);
  }
  return d;
}

DistanceFieldGrid::Gradient
DistanceFieldGrid::gradientAtCell(std::size_t row, std::size_t col) const
{
  assertIsValidCell(row, col);

  // handle borders
  row = std::max(row, std::size_t(1));
  col = std::max(col, std::size_t(1));

  row = std::min(row, dimension_.height-2);
  col = std::min(col, dimension_.width -2);

  Gradient grad;

  grad.dx = (atCell(row, col-1) - atCell(row, col+1)) / 2.;
  grad.dy = (atCell(row-1, col) - atCell(row+1, col)) / 2.;

  return grad;
}

DistanceFieldGrid::Gradient
DistanceFieldGrid::gradientAtPosition(double x, double y) const
{
  assertIsValidPosition(x,y);
  assertIsValidPosition(x+1,y+1);

  const double lx = std::floor(x),
               ly = std::floor(y);
  const double hx = lx + 1.0,
               hy = ly + 1.0;

  Gradient grad;

  grad.dx = (hy-y) * (atPosition(hx, ly)-atPosition(lx, ly)) +
            (y-ly) * (atPosition(hx, hy)-atPosition(lx, hy));

  grad.dy = (hx-x) * (atPosition(lx, hy)-atPosition(lx, ly)) +
            (x-lx) * (atPosition(hx, hy)-atPosition(hx, ly));

  return grad;
}

DistanceFieldGrid::Gradient
DistanceFieldGrid::gradientAtCellSafe(std::size_t row, std::size_t col) const
{
  Gradient grad;
  if (isCellValid(row,col))
  {
    // handle borders
    row = std::max(row, std::size_t(1));
    col = std::max(col, std::size_t(1));

    row = std::min(row, dimension_.height-2);
    col = std::min(col, dimension_.width -2);

    grad.dx = (atCell(row, col-1) - atCell(row, col+1)) / 2.;
    grad.dy = (atCell(row-1, col) - atCell(row+1, col)) / 2.;
  }
  else
  {
    grad.dx = (row>dimension_.height-2)? row-dimension_.height-2 : 0;
    grad.dy = (col>dimension_.width -2)? col-dimension_.width -2 : 0;
  }

  return grad;
}

DistanceFieldGrid::Gradient
DistanceFieldGrid::gradientAtPositionSafe(const double x, const double y) const
{
  const double lx = std::floor(x),
               ly = std::floor(y);
  const double hx = lx + 1.0,
               hy = ly + 1.0;
  Gradient grad;

  grad.dx = (hy-y) * (atPositionSafe(hx, ly)-atPositionSafe(lx, ly)) +
            (y-ly) * (atPositionSafe(hx, hy)-atPositionSafe(lx, hy));
  grad.dy = (hx-x) * (atPositionSafe(lx, hy)-atPositionSafe(lx, ly)) +
            (x-lx) * (atPositionSafe(hx, hy)-atPositionSafe(hx, ly));

  return grad;
}

// Setter/getter

inline double* DistanceFieldGrid::data()
{
  return data_;
}

inline const double* DistanceFieldGrid::data() const
{
  return data_;
}

inline void DistanceFieldGrid::setOrigin(const Origin& origin)
{
  origin_ = origin;
}

inline void DistanceFieldGrid::setResolution(const double resolution)
{
  if (resolution <= 0)
    throw std::runtime_error("Resolution can't be zero nor negative !");

  resolution_ = resolution;
}

inline const DistanceFieldGrid::Dimension&
DistanceFieldGrid::getDimension() const noexcept
{
  return dimension_;
}

inline const double&
DistanceFieldGrid::getResolution() const noexcept
{
  return resolution_;
}

inline const DistanceFieldGrid::Origin&
DistanceFieldGrid::getOrigin() const noexcept
{
  return origin_;
}

std::size_t DistanceFieldGrid::getIndex(const std::size_t row, const std::size_t col) const
{
  return col + (dimension_.height - row - 1) * dimension_.width;
}

void DistanceFieldGrid::assertIsValidCell(const std::size_t& row, const std::size_t& col) const
{
  if (!isCellValid(row,col))
    throw std::out_of_range("Cell index " + std::to_string(row) +
                            ", " + std::to_string(col) + " is out of range !\n"
                            "Origin ["  + std::to_string(origin_.x) + ","
                                        + std::to_string(origin_.y) + ","
                                        + std::to_string(origin_.yaw) + "], "
                            "Dimension [" + std::to_string(dimension_.width) + ","
                                          + std::to_string(dimension_.height) + "],"
                            "Resolution " + std::to_string(resolution_) + ".");
}

void DistanceFieldGrid::assertIsValidPosition(const double& x, const double& y) const
{
  if (!isPositionValid(x,y))
    throw std::out_of_range("Position " + std::to_string(x) +
                            ", " + std::to_string(y) + " is out of range !\n"
                            "Origin ["  + std::to_string(origin_.x) + ","
                                        + std::to_string(origin_.y) + ","
                                        + std::to_string(origin_.yaw) + "], "
                            "Dimension [" + std::to_string(dimension_.width) + ","
                                          + std::to_string(dimension_.height) + "],"
                            "Resolution " + std::to_string(resolution_) + ".");
}

template <typename Stream>
Stream& operator<<(Stream& s, const DistanceFieldGrid& g)
{
  s << "DistanceFieldGrid:\n"
    << "\tdimension: " << g.getDimension().width << "x" << g.getDimension().height << "\n"
    << "\tresolution: " << g.getResolution() << "\n"
    << "\torigin: " << g.getOrigin().x << ", "
                    << g.getOrigin().y << ", "
                    << g.getOrigin().yaw
    << "\n";

  return s;
}

} /* namespace distmap */

#endif /* _DISTANCE_MAP_CORE_DISTANCE_FIELD_GRID_H_ */
