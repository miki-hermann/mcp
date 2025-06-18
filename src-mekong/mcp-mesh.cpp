/**************************************************************************
 *                                                                        *
 *                                                                        *
 *        Multiple Characterization Problem (MCP)                         *
 *                                                                        *
 * Author:   Miki Hermann                                                 *
 * e-mail:   hermann@lix.polytechnique.fr                                 *
 * Address:  LIX (CNRS UMR 7161), Ecole Polytechnique, France             *
 *                                                                        *
 * Author:   Gernot Salzer                                                *
 * e-mail:   gernot.salzer@tuwien.ac.at                                   *
 * Address:  Technische Universitaet Wien, Vienna, Austria                *
 *                                                                        *
 * Author:   César Sagaert                                                *
 * e-mail:   cesar.sagaert@ensta-paris.fr                                 *
 * Address:  ENSTA Paris, Palaiseau, France                               *
 *                                                                        *
 * Version: all                                                           *
 *     File:    src-mekong/mcp-mesh.cpp                                   *
 *                                                                        *
 *      Copyright (c) 2019 - 2025                                         *
 *                                                                        *
 **************************************************************************/

#include "mcp-mesh.hpp"
#include "mcp-matrix+formula.hpp"

using namespace std;
using namespace bucket;
namespace mesh {

  constexpr bucket::Clause empty_clause = {
    .lsign = lnone,
    .rsign = lnone,
    .lval = 0,
    .rval = 0,
    .lcoord = 0,
    .rcoord = 0,
  };

  bucket::Clause isolation(const Point &p, const std::array<size_t, 2> &xy,
			   const mesh::Direction quadrant) {
    bucket::Clause c = empty_clause;
    switch (quadrant) {
    case NW:
      if (p[X] < headlines[xy[X]].DMAX) {
	c.lsign = lpos;
	c.lcoord = xy[X];
	c.lval = p[X] + 1;
      }
      if (p[Y] > 0) {
	c.rsign = lneg;
	c.rcoord = xy[Y];
	c.rval = p[Y] - 1;
      }
      break;
    case NE:
      if (p[X] > 0) {
	c.lsign = lneg;
	c.lcoord = xy[X];
	c.lval = p[X] - 1;
      }
      if (p[Y] > 0) {
	c.rsign = lneg;
	c.rcoord = xy[Y];
	c.rval = p[Y] - 1;
      }
      break;
    case SE:
      if (p[X] > 0) {
	c.lsign = lneg;
	c.lcoord = xy[X];
	c.lval = p[X] - 1;
      }
      if (p[Y] < headlines[xy[Y]].DMAX) {
	c.rsign = lpos;
	c.rcoord = xy[Y];
	c.rval = p[Y] + 1;
      }
      break;
    case SW:
      if (p[X] < headlines[xy[X]].DMAX) {
	c.lsign = lpos;
	c.lcoord = xy[X];
	c.lval = p[X] + 1;
      }
      if (p[Y] < headlines[xy[Y]].DMAX) {
	c.rsign = lpos;
	c.rcoord = xy[Y];
	c.rval = p[Y] + 1;
      }
      break;
    default:
      throw runtime_error("+++ isolation: quadrant " + to_string(quadrant) +
			  " is not an ordinal direction");
    }
    return c;
  }

  void ColTree::insert(const Point &point) {
    if (this->empty())
      this->root = make_shared<ColNode>(point);
    else {
      shared_ptr<ColNode> colnode = this->root;
      shared_ptr<ColNode> colparent;
      while (colnode != nullptr) {
	colparent = colnode;
	if (point[Y] == colnode->value_j) {
	  // point is already in the tree
	  return;
	} else
	  colnode = point[Y] < colnode->value_j ? colnode->south : colnode->north;
      }
      // y-axis does not exist and point[Y] != colparent->value_j
      if (point[Y] < colparent->value_j)
	colparent->south = make_shared<ColNode>(point);
      else
	colparent->north = make_shared<ColNode>(point);
    }
  }

  void RowTree::insert(const Point &point) {
    if (this->empty())
      this->root = make_shared<RowNode>(point);
    else {
      shared_ptr<RowNode> rownode = this->root;
      shared_ptr<RowNode> rowparent;
      while (rownode != nullptr) {
	rowparent = rownode;
	if (point[X] == rownode->value_i) {
	  // x-axis exists, insert on it
	  rownode->column->insert(point);
	  return;
	} else
	  rownode = point[X] < rownode->value_i ? rownode->west : rownode->east;
      }
      // x-axis does not exist and point[X] != rowparent->value
      if (point[X] < rowparent->value_i)
	rowparent->west = make_shared<RowNode>(point);
      else
	rowparent->east = make_shared<RowNode>(point);
    }
  }

  void RowTree::build(const vector<Point> &pts, const size_t low,
		      const size_t high) {
    if (low <= high) {
      size_t med = (low + high) / 2;
      this->insert(pts[med]);
      this->build(pts, low, med - 1);
      this->build(pts, med + 1, high);
    }
  }

  void RowTree::insert(const vector<Point> &pts) {
    this->build(pts, 0, pts.size() - 1);
  }

  //------------------------------------------------------------------------------

  bool ColNode::isolated(const Point &point, const Direction dir) const {
    // checks if the point is isolated in the quadrant dir
    // for all columns in the tree
    // isolated = no other points in the quadrant
    if ((dir == NE || dir == NW) // looking NORTH = larger
	&& this->value_j >= point[Y])
      return false;
    if ((dir == SE || dir == SW) // looking SOUTH = smaller
	&& this->value_j <= point[Y])
      return false;
    if (this->north != nullptr && !this->north->isolated(point, dir))
      return false;
    if (this->south != nullptr && !this->south->isolated(point, dir))
      return false;
    return true;
  }

  bool ColTree::isolated(const Point &point, const Direction dir) const {
    // checks if the point is isolated in the quadrant dir
    // isolated = no other points in the quadrant

    shared_ptr<ColNode> colptr = this->root;
    if (dir == NE || dir == NW) // looking NORTH = larger
      while (colptr != nullptr) {
	if (colptr->value_j >= point[Y])
	  return colptr->isolated(point, dir);
	colptr = colptr->north;
      }
    else if (dir == SE || dir == SW) // looking SOUTH = smaller
      while (colptr != nullptr) {
	if (colptr->value_j <= point[Y])
	  return colptr->isolated(point, dir);
	colptr = colptr->south;
      }
    return true;
  }

  bool RowNode::isolated(const Point &point, const Direction dir) const {
    // checks if the point is isolated in the quadrant dir
    // for all rows in the tree
    // isolated = no other points in the quadrant
    if ((dir == NE || dir == SE) // looking EAST = larger
	&& this->value_i >= point[X] && !this->column->isolated(point, dir))
      return false;
    if ((dir == NW || dir == SW) // looking WEST = smaller
	&& this->value_i <= point[X] && !this->column->isolated(point, dir))
      return false;
    if (this->west != nullptr && !this->west->isolated(point, dir))
      return false;
    if (this->east != nullptr && !this->east->isolated(point, dir))
      return false;
    return true;
  }

  bool RowTree::isolated(const Point &point, const Direction dir) const {
    // checks if the point is isolated in the quadrant dir
    // isolated = no other points in the quadrant

    shared_ptr<RowNode> rowptr = this->root;
    if (dir == NE || dir == SE) // looking EAST = larger
      while (rowptr != nullptr) {
	if (rowptr->value_i >= point[X])
	  return rowptr->isolated(point, dir);
	rowptr = rowptr->east;
      }
    else if (dir == NW || dir == SW) // looking WEST = smaller
      while (rowptr != nullptr) {
	if (rowptr->value_i <= point[X])
	  return rowptr->isolated(point, dir);
	rowptr = rowptr->west;
      }
    else
      throw runtime_error(to_string(dir) + " is not an ordinal direction");
    return true;
  }

  void init(Mesh &mesh, size_t arity) {
    vector<RowTree> tmp4(arity, RowTree());
    for (unsigned j = 0; j < arity; ++j)
      mesh.push_back(tmp4);
  }

  void init(Strip &strip, size_t arity) {
    vector<unordered_set<integer>> tmp4(arity, unordered_set<integer>());
    strip = tmp4;
  }

  // populate strip and mesh with positive samples
  void populate(const Matrix &positiveT,
		Strip &strip, Mesh &mesh, size_t arity) {
    for (size_t k = 0; k < positiveT.num_rows(); ++k) {
      const Row &m = positiveT[k];
      for (size_t i = 0; i < arity; ++i) {
	strip[i].insert(m[i]);
	for (size_t j = i+1; j < arity; ++j)
	  mesh[i][j].insert({m[i], m[j]});
      }
    }
  }

} // namespace mesh

ostream &operator<<(ostream &output, const Point &p) {
  // overloading ostream to print a point
  output << "[" << p[bucket::X] << "," << p[bucket::Y] << "]";
  return output;
}

ostream &operator<<(ostream &output, const mesh::ColNode &cn) {
  if (cn.south != nullptr)
    output << *(cn.south);
  output << " " << cn.value_j;
  if (cn.north != nullptr)
    output << *(cn.north);
  return output;
}

ostream &operator<<(ostream &output, const mesh::ColTree &ct) {
  if (!ct.empty())
    output << *(ct.tree());
  return output;
}

ostream &operator<<(ostream &output, const mesh::RowNode &rn) {
  if (rn.west != nullptr)
    output << *(rn.west);
  output << "... row " << rn.value_i << ": " << *(rn.column) << "." << endl;
  if (rn.east != nullptr)
    output << *(rn.east);
  return output;
}

ostream &operator<<(ostream &output, const mesh::RowTree &rt) {
  if (!rt.empty())
    output << *(rt.tree());
  return output;
}
