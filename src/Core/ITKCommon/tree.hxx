/*
 For more information, please see: http://software.sci.utah.edu
 
 The MIT License
 
 Copyright (c) 2013 Scientific Computing and Imaging Institute,
 University of Utah.
 
 
 Permission is hereby granted, free of charge, to any person obtaining a
 copy of this software and associated documentation files (the "Software"),
 to deal in the Software without restriction, including without limitation
 the rights to use, copy, modify, merge, publish, distribute, sublicense,
 and/or sell copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included
 in all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 DEALINGS IN THE SOFTWARE.
 */

// File         : tree.hxx
// Author       : Pavel A. Koshevoy
// Created      : 2006/02/09 13:14
// Copyright    : (C) 2004-2008 University of Utah
// Description  : A kd-tree.

#ifndef TREE_HXX_
#define TREE_HXX_

// system includes:
#include <vector>
#include <list>
#include <algorithm>
#include <iostream>
#include <math.h>


//----------------------------------------------------------------
// node_t
// 
template <int kd, typename point_t, typename data_t = double>
class node_t
{
public:
  
  //----------------------------------------------------------------
  // point_sort_predicate
  //
  class point_sort_predicate
  {
  public:
    point_sort_predicate(const unsigned int & sort_on_dimension):
    i_(sort_on_dimension)
    {}
    
    inline bool operator() (const point_t & a, const point_t & b) const
    { return a[i_] < b[i_]; }
    
    // dimension along which the points will be sorted:
    unsigned int i_;
  };
  
  // default constructor:
  node_t():
  i_(0),
  m_(data_t(0)),
  min_(data_t(0)),
  max_(data_t(0)),
  points_(NULL),
  num_pts_(0),
  parent_(NULL),
  a_(NULL),
  b_(NULL)
  {}
  
  // copy constructor:
  node_t(const node_t<kd, point_t, data_t> & node):
  i_(0),
  m_(data_t(0)),
  min_(data_t(0)),
  max_(data_t(0)),
  points_(NULL),
  num_pts_(0),
  parent_(NULL),
  a_(NULL),
  b_(NULL)
  { *this = node; }
  
  // destructor:
  ~node_t()
  {
    delete a_;
    a_ = NULL;
    
    delete b_;
    b_ = NULL;
  }
  
  //----------------------------------------------------------------
  // node_t
  // 
  // given a list of points, where each point is composed of
  // kd elements of type data_t, construct a balanced kd-tree
  // 
  node_t(node_t<kd, point_t, data_t> * parent,
         point_t * points,
         const unsigned int num_pts):
  i_(0),
  m_(data_t(0)),
  min_(data_t(0)),
  max_(data_t(0)),
  points_(NULL),
  num_pts_(0),
  parent_(parent),
  a_(NULL),
  b_(NULL)
  {
    // first, find the mean point value for each dimension:
    data_t mean[kd] = { data_t(0) };
    
    for (unsigned int i = 0; i < num_pts; i++)
    {
      for (unsigned int j = 0; j < kd; j++)
      {
        mean[j] += data_t(points[i][j]);
      }
    }
    
    for (unsigned int j = 0; j < kd; j++)
    {
      mean[j] /= data_t(num_pts);
    }
    
    // next, find the point variance for each dimension:
    data_t variance[kd] = { data_t(0) };
    
    for (unsigned int i = 0; i < num_pts; i++)
    {
      for (unsigned int j = 0; j < kd; j++)
      {
        data_t d = data_t(points[i][j]) - mean[j];
        variance[j] += d * d;
      }
    }
    
    // find the dimension with the highest variance:
    for (unsigned int j = 1; j < kd; j++)
    {
      if (variance[j] > variance[i_])
      {
        i_ = j;
      }
    }
    
    if (variance[i_] != data_t(0))
    {
      // sort the points in the ascending order:
      point_sort_predicate predicate(i_);
      std::sort(&points[0], &points[num_pts], predicate);
      
      // find the median:
      std::vector<data_t> unique(num_pts);
      unsigned int num_unique = 1;
      unique[0] = points[0][i_];
      for (unsigned int i = 0; i < num_pts; i++)
      {
        const data_t & prev = unique[num_unique - 1];
        const data_t   next = points[i][i_];
        if (prev != next)
        {
          unique[num_unique] = next;
          num_unique++;
        }
      }
      
      if (num_unique > 1)
      {
        m_ = unique[num_unique / 2];
        
        // store the range of point values bounded by this node:
        min_ = points[0][i_];
        max_ = points[num_pts - 1][i_];
        
        // find the cut:
        unsigned int num_a = 1;
        for (; num_a < num_pts && data_t(points[num_a][i_]) < m_; num_a++);
        unsigned int num_b = num_pts - num_a;
        
        // FIXME:
        assert(points[num_a - 1][i_] != points[num_a][i_]);
        
        if (num_a != 0)
        {
          a_ = new node_t<kd, point_t, data_t>(this, &(points[0]), num_a);
        }
        
        if (num_b != 0)
        {
          b_ = new node_t<kd, point_t, data_t>(this, &(points[num_a]), num_b);
        }
      }
      else // num_unique == 1
      {
        // this can happen due to numerical imprecision of
        // calculating the mean (an therefore - the variance):
        variance[i_] = data_t(0);
      }
    }
    
    if (variance[i_] == data_t(0))
    {
      // this is a leaf node:
      m_ = points[0][i_];
      min_ = m_;
      max_ = m_;
      points_ = points;
      num_pts_ = num_pts;
    }
  }
  
  // assignment operator (deep copy):
  node_t<kd, point_t, data_t> &
  operator = (const node_t<kd, point_t, data_t> & node)
  {
    i_ = node.i_;
    m_ = node.m_;
    min_ = node.min_;
    max_ = node.max_;
    points_ = node.points_;
    num_pts_ = node.num_pts_;
    parent_ = node.parent_;
    
    delete a_;
    a_ = NULL;
    
    delete b_;
    b_ = NULL;
    
    if (node.a_ != NULL)
    {
      a_ = new node_t<kd, point_t, data_t>(*node.a_);
      a_->parent_ = this;
    }
    
    if (node.b_ != NULL)
    {
      b_ = new node_t<kd, point_t, data_t>(*node.b_);
      b_->parent_ = this;
    }
    
    return *this;
  }
  
  //----------------------------------------------------------------
  // unexplored_branch_t
  // 
  class unexplored_branch_t
  {
  public:
    unexplored_branch_t(const node_t<kd, point_t, data_t> * node,
                        const data_t & distance_to_median):
    node_(node),
    dist_(distance_to_median)
    {}
    
    // this is used to sort the nodes:
    inline bool operator < (const unexplored_branch_t & b) const
    { return dist_ < b.dist_; }
    
    // the root node of the unexplored branch:
    const node_t<kd, point_t, data_t> * node_;
    
    // distance from the query point to the median:
    data_t dist_;
  };
  
  // find a given leaf node:
  const node_t<kd, point_t, data_t> *
  leaf(std::list<unexplored_branch_t> & unexplored,
       const point_t & query,
       const data_t & best_distance) const
  {
    const data_t p = data_t(query[i_]);
    const data_t da = std::min(std::max(min_ - p, 0.0), std::max(p - m_, 0.0));
    const data_t db = std::min(std::max(m_ - p, 0.0), std::max(p - max_, 0.0));
    
    if (da > best_distance && db > best_distance) return NULL;
    if (points_ != NULL) return this;
    
    if (da < db)
    {
      // will traverse branch a, save branch b for later:
      unexplored.push_back(unexplored_branch_t(b_, db));
      return a_->leaf(unexplored, query, best_distance);
    }
    else
    {
      // will traverse branch b, save branch a for later:
      unexplored.push_back(unexplored_branch_t(a_, da));
      return b_->leaf(unexplored, query, best_distance);
    }
  }
  
  // calculate the Euclidian distance between the points stored
  // in the leaf node and the query point:
  data_t
  euclidian_distance(const point_t & query) const
  {
    assert(points_ != NULL);
    
    const point_t & point = points_[0];
    data_t distance = 0.0;
    for (unsigned int j = 0; j < kd; j++)
    {
      data_t d = data_t(query[j]) - data_t(point[j]);
      distance += d * d;
    }
    distance = data_t(sqrt(double(distance)));
    return distance;
  }
  
  // FIXME: this is for debugging:
  void dump(std::ostream & so) const
  {
    if (points_ != NULL)
    {
      for (unsigned int i = 0; i < num_pts_; i++)
      {
#if 0
        for (unsigned int j = 0; j < kd; j++)
        {
          so << ' ' << points_[i][j];
        }
        so << std::endl;
#else
        so << points_[i] << std::endl;
#endif
      }
    }
    else
    {
      if (a_ != NULL) a_->dump(so);
      if (b_ != NULL) b_->dump(so);
    }
  }
  
  // dimension index in the point:
  unsigned int i_;
  
  // median point value along that dimension:
  data_t m_;
  
  // min/max point values along that dimension:
  data_t min_;
  data_t max_;
  
  // the payload:
  point_t * points_;
  unsigned int num_pts_;
  
  // the parent node:
  node_t<kd, point_t, data_t> * parent_;
  
  // branch containing points with value lesser than the median:
  node_t<kd, point_t, data_t> * a_;
  
  // branch containing points with value greater or equal to the median:
  node_t<kd, point_t, data_t> * b_;
};

//----------------------------------------------------------------
// operator <<
//
template <int kd, typename point_t, typename data_t>
inline std::ostream &
operator << (std::ostream & so, const node_t<kd, point_t, data_t> & node)
{
  node.dump(so);
  return so;
}

//----------------------------------------------------------------
// tree_t
// 
template <int kd, typename point_t, typename data_t = double>
class tree_t
{
public:
  //----------------------------------------------------------------
  // unexplored_branch_t
  // 
  typedef typename node_t<kd, point_t, data_t>::unexplored_branch_t
  unexplored_branch_t;
  
  // default constructor:
  tree_t():
  root_(NULL)
  {}
  
  // copy constructor:
  tree_t(const tree_t<kd, point_t, data_t> & tree):
  root_(NULL)
  {
    *this = tree;
  }
  
  // destructor:
  ~tree_t()
  {
    delete root_;
    root_ = NULL;
  }
  
  // assignment operator:
  tree_t<kd, point_t, data_t> &
  operator = (const tree_t<kd, point_t, data_t> & tree)
  {
    delete root_;
    root_ = NULL;
    if (tree.root_ != NULL)
    {
      root_ = new node_t<kd, point_t, data_t>(*(tree.root_));
    }
    
    return *this;
  }
  
  // build a kd-tree from a given set of points (duplicates are allowed):
  void setup(point_t * points, const unsigned int num_pts)
  {
    delete root_;
    root_ = NULL;
    
    if (num_pts != 0)
    {
      root_ = new node_t<kd, point_t, data_t>(NULL, points, num_pts);
    }
  }
  
  //----------------------------------------------------------------
  // nn_t
  // 
  class nn_t
  {
  public:
    nn_t(const node_t<kd, point_t, data_t> * node, const double & dist):
    node_(node),
    dist_(dist)
    {}
    
    inline bool operator < (const nn_t & nn) const
    { return dist_ <  nn.dist_; }
    
    // pointer to the destination node:
    const node_t<kd, point_t, data_t> * node_;
    
    // distance to the destination node:
    double dist_;
  };
  
  // find the node that contains the nearest neighbor(s) of a given point:
  unsigned int
  nn(const point_t & query,
     data_t & best_distance,
     std::list<nn_t> & nn_sorted,
     const unsigned int max_traversals = 200,
     const unsigned int max_nn = 3) const
  {
    if (root_ == NULL) return 0;
    
    // the results:
    const node_t<kd, point_t, data_t> * best_match = NULL;
    best_distance = std::numeric_limits<data_t>::max();
    
    // bootstrap the search by starting at the root:
    std::list<unexplored_branch_t> unexplored;
    unexplored.push_back(unexplored_branch_t(root_, best_distance));
    
    for (unsigned int i = 0; i < max_traversals && !unexplored.empty(); i++)
    {
      // retrieve the next search entry point:
      const node_t<kd, point_t, data_t> * start_here =
      unexplored.front().node_;
      unexplored.pop_front();
      
      // find a matching point in the tree:
      const node_t<kd, point_t, data_t> * match =
      start_here->leaf(unexplored, query, best_distance);
      if (match == NULL) continue;
      
      // find the distance to the matching point:
      data_t distance = match->euclidian_distance(query);
      
      // update the best neighbor estimate:
      if (distance < best_distance)
      {
        best_match = match;
        best_distance = distance;
        
        // NOTE: this does not guarantee that the "correct" match will
        // be added to the list, because it is entirely possible that
        // the "correct" match was never found due to being pruned out.
        nn_sorted.push_front(nn_t(best_match, best_distance));
        
        // FIXME: this should be a 
        if (nn_sorted.size() > max_nn)
        {
          // remove the worst neighbor:
          nn_sorted.pop_back();
        }
      }
      
      // prune some of the branches:
      typename std::list<unexplored_branch_t>::iterator it =
      unexplored.begin();
      
      while (it != unexplored.end())
      {
        const unexplored_branch_t & ub = *it;
        
        if (ub.dist_ > best_distance)
        {
          it = unexplored.erase(it);
        }
        else
        {
          ++it;
        }
      }
      
      // sort the unexplored branches:
      unexplored.sort();
    }
    
    return nn_sorted.size();
  }
  
  // same as above, except ignoring the full list of close neighbors found:
  const node_t<kd, point_t, data_t> *
  nn(const point_t & query,
     data_t & best_distance,
     unsigned int max_traversals = 200) const
  {
    std::list<nn_t> nn_sorted;
    if (nn(query, best_distance, nn_sorted, max_traversals, 1) == 0)
    {
      return NULL;
    }
    
    return nn_sorted.front().node_;
  }
  
  // collect nodes within some radius around a given point:
  unsigned int
  neighbors(const point_t & query,
            const data_t & radius,
            std::list<nn_t> & nn_sorted,
            const unsigned int max_traversals = 200) const
  {
    if (root_ == NULL) return 0;
    
    // bootstrap the search by starting at the root:
    std::list<unexplored_branch_t> unexplored;
    unexplored.push_back(unexplored_branch_t(root_, 0.0));
    
    for (unsigned int i = 0; i < max_traversals && !unexplored.empty(); i++)
    {
      // retrieve the next search entry point:
      const node_t<kd, point_t, data_t> * start_here =
      unexplored.front().node_;
      unexplored.pop_front();
      
      // find a matching point in the tree:
      const node_t<kd, point_t, data_t> * match =
      start_here->leaf(unexplored, query, radius);
      if (match == NULL) continue;
      
      // find the distance to the matching point:
      data_t distance = match->euclidian_distance(query);
      if (distance > radius) continue;
      nn_sorted.push_back(nn_t(match, distance));
      
      // sort the unexplored branches:
      unexplored.sort();
    }
    
    // sort the neighbors from closest to furthest away:
    nn_sorted.sort();
    
    return nn_sorted.size();
  }
  
  // dump the leaf nodes into the stream:
  void dump(std::ostream & so) const
  {
    if (root_ != NULL) root_->dump(so);
    else so << "NULL";
  }
  
  node_t<kd, point_t, data_t> * root_;
};

//----------------------------------------------------------------
// operator <<
// 
template <int kd, typename point_t, typename data_t>
inline std::ostream &
operator << (std::ostream & so, const tree_t<kd, point_t, data_t> & tree)
{
  tree.dump(so);
  return so;
}


#endif // TREE_HXX_
