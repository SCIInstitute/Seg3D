/*
  For more information, please see: http://software.sci.utah.edu

  The MIT License

  Copyright (c) 2009 Scientific Computing and Imaging Institute,
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

#include <Utils/Geometry/Transform.h>
#include <Utils/Geometry/Point.h>

namespace Utils {

Transform::Transform()
{
  load_identity();
}

Transform::Transform(const Transform& copy) 
{
  for(size_t i=0;i<4;i++)
  {
    for(size_t j=0;j<4;j++)
    {
      mat_[i][j]=copy.mat_[i][j];
      imat_[i][j]=copy.imat_[i][j];
    }
  }
}

Transform& 
Transform::operator=(const Transform& copy) 
{
  for(size_t i=0;i<4;i++)
  {
    for(size_t j=0;j<4;j++)
    {
      mat_[i][j]=copy.mat_[i][j];
      imat_[i][j]=copy.imat_[i][j];
    }
  }
  
  return (*this);
}

Transform::Transform(const Point& p, const Vector& i, 
                     const Vector& j, const Vector& k)
{
  load_basis(p, i, j, k);
}

void
Transform::load_basis(const Point &p,
                      const Vector &x,
                      const Vector &y,
                      const Vector &z)
{
  load_frame(x,y,z);
  pre_translate(Vector(p));
}

void
Transform::load_frame(const Vector& x, 
                      const Vector& y, 
                      const Vector& z)
{
  mat_[3][3] = imat_[3][3] = 1.0;
  mat_[0][3] = mat_[1][3] = mat_[2][3] = 0.0;
  mat_[3][0] = mat_[3][1] = mat_[3][2] = 0.0;

  mat_[0][0] = x.x();
  mat_[1][0] = x.y();
  mat_[2][0] = x.z();

  mat_[0][1] = y.x();
  mat_[1][1] = y.y();
  mat_[2][1] = y.z();

  mat_[0][2] = z.x();
  mat_[1][2] = z.y();
  mat_[2][2] = z.z();

  compute_imat();
}

void
Transform::post_trans(const Transform& trans)
{
  post_mulmat(trans.mat_);
  compute_imat();
}

void
Transform::pre_trans(const Transform& trans)
{
  pre_mulmat(trans.mat_);
  compute_imat();
}

void
Transform::build_scale(double m[4][4], const Vector& v)
{
  load_identity(m);
  m[0][0]=v.x();
  m[1][1]=v.y();
  m[2][2]=v.z();
}
    
void
Transform::pre_scale(const Vector& v)
{
  double m[4][4];
  build_scale(m,v);
  pre_mulmat(m);
  compute_imat();
}

void
Transform::post_scale(const Vector& v)
{
  double m[4][4];
  build_scale(m,v);
  post_mulmat(m);
  compute_imat();
}

// rotate into a new frame (z=shear-fixed-plane, y=projected shear vector),
//    shear in y (based on value of z), rotate back to original frame
void
Transform::build_shear(double m[4][4], const Vector& s, const Plane& p) 
{    
  load_identity(m);

  Vector sv(p.project(s));      // s projected onto p
  Vector dn(s-sv);      // difference (in normal direction) btwn s and sv
  double d = Dot(dn,p.normal());
  if (Abs(d)<1.0e-8) 
  { // Transform - shear vector lies in shear fixed plane.  Returning identity.
    return;
  }

  double yshear = sv.length()/d; // compute the length of the shear vector,
  // after the normal-to-shear-plane component
  // has been made unit-length.
  Vector svn(sv);
  svn.normalize();      // normalized vector for building orthonormal basis

  Vector su(Cross(p.normal(),svn));
  Transform r;  // the rotation to take the z-axis to the shear normal
  // and the y-axis to the projected shear vector
  r.load_frame(su, svn, p.normal());

  Transform sh;
  double a[16];
  sh.get(a);
  a[6]=yshear;
  a[7]=-yshear*p.distance();  // this last piece is "d" from the plane
  sh.set(a);

  load_identity();
  post_trans(r);
  post_trans(sh);
  r.invert();
  post_trans(r);
}

void
Transform::pre_shear(const Vector& s, const Plane& p)
{
  double m[4][4];
  build_shear(m,s,p);
  pre_mulmat(m);
  compute_imat();
}

void
Transform::post_shear(const Vector& s, const Plane& p)
{
  double m[4][4];
  build_shear(m,s,p);
  post_mulmat(m);
  compute_imat();
}

void
Transform::build_translate(double m[4][4], const Vector& v)
{
  load_identity(m);
  m[0][3]=v.x();
  m[1][3]=v.y();
  m[2][3]=v.z();
}

void
Transform::pre_translate(const Vector& v)
{
  double m[4][4];
  build_translate(m,v);
  pre_mulmat(m);
  compute_imat();
}

void
Transform::post_translate(const Vector& v)
{
  double m[4][4];
  build_translate(m,v);    
  post_mulmat(m);
  compute_imat();
}

void
Transform::build_rotate(double m[4][4], double angle, const Vector& axis)
{
  double sintheta = Sin(angle);
  double costheta = Cos(angle);
  double ux=axis.x();
  double uy=axis.y();
  double uz=axis.z();
  
  m[0][0]=ux*ux+costheta*(1.0-ux*ux);
  m[0][1]=ux*uy*(1.0-costheta)-uz*sintheta;
  m[0][2]=uz*ux*(1.0-costheta)+uy*sintheta;
  m[0][3]=0.0;

  m[1][0]=ux*uy*(1.0-costheta)+uz*sintheta;
  m[1][1]=uy*uy+costheta*(1-uy*uy);
  m[1][2]=uy*uz*(1.0-costheta)-ux*sintheta;
  m[1][3]=0.0;

  m[2][0]=uz*ux*(1.0-costheta)-uy*sintheta;
  m[2][1]=uy*uz*(1.0-costheta)+ux*sintheta;
  m[2][2]=uz*uz+costheta*(1-uz*uz);
  m[2][3]=0.0;

  m[3][0]=0.0;
  m[3][1]=0.0;
  m[3][2]=0.0;
  m[3][3]=1.0;
}

void
Transform::pre_rotate(double angle, const Vector& axis)
{
  double m[4][4];
  build_rotate(m, angle, axis);
  pre_mulmat(m);
  compute_imat();
}       

void
Transform::post_rotate(double angle, const Vector& axis)
{
  double m[4][4];
  build_rotate(m, angle, axis);
  post_mulmat(m);
  compute_imat();
}       

bool
Transform::rotate(const Vector& from, const Vector& to)
{
  Vector t(to); t.normalize();
  Vector f(from); f.normalize();
  Vector axis(Cross(f,t));
  if (axis.length2() < 1.0e-8) 
  {
    // Vectors are too close to each other to get a stable axis of
    // rotation, so return.
    return (false);
  }
  
  double sinth=axis.length();
  double costh=Dot(f,t);
  if(Abs(sinth) < 1.0e-9)
  {
    if(costh > 0.0)
      return (false); // no rotate;
    else 
    {
      // from and to are in opposite directions, find an axis of rotation
      // Try the Z axis first.  This will fail if from is along Z, so try
      // Y next.  Then rotate 180 degrees.
      axis = Cross(from, Vector(0.0,0.0,1.0));
      if(axis.length2() < 1.0e-9)
        axis = Cross(from, Vector(0.0,1.0,0.0));
      axis.normalize();
      pre_rotate(Pi(), axis);
    }
  } 
  else 
  {
    pre_rotate(Atan2(sinth, costh), axis.normal());
  }
  return (true);
}

void
Transform::build_permute(double m[4][4],int xmap, int ymap, int zmap, bool pre)
{
  for (size_t i=0; i<4; i++)
    for (size_t j=0; j<4; j++)
      m[i][j] = 0.0;

  m[3][3]=1.0;

  if (pre) 
  {    // for each row, set the mapped column
    if (xmap<0) m[0][-1-xmap]=-1.0; else m[0][xmap-1]=1.0;
    if (ymap<0) m[1][-1-ymap]=-1.0; else m[1][ymap-1]=1.0;
    if (zmap<0) m[2][-1-zmap]=-1.0; else m[2][zmap-1]=1.0;
  } 
  else 
  {      // for each column, set the mapped row
    if (xmap<0) m[-1-xmap][0]=-1.0; else m[xmap-1][0]=1.0;
    if (ymap<0) m[-1-ymap][1]=-1.0; else m[ymap-1][1]=1.0;
    if (zmap<0) m[-1-zmap][2]=-1.0; else m[zmap-1][2]=1.0;
  }
}

void
Transform::pre_permute(int xmap, int ymap, int zmap)
{
  double m[4][4];
  build_permute(m, xmap, ymap, zmap, true);
  pre_mulmat(m);
  compute_imat();
}

void
Transform::post_permute(int xmap, int ymap, int zmap)
{
  double m[4][4];
  build_permute(m, xmap, ymap, zmap, false);
  post_mulmat(m);
  compute_imat();
}

Point
Transform::project(const Point& p) const
{
  return Point(mat_[0][0]*p.x()+mat_[0][1]*p.y()+mat_[0][2]*p.z()+mat_[0][3],
               mat_[1][0]*p.x()+mat_[1][1]*p.y()+mat_[1][2]*p.z()+mat_[1][3],
               mat_[2][0]*p.x()+mat_[2][1]*p.y()+mat_[2][2]*p.z()+mat_[2][3],
               mat_[3][0]*p.x()+mat_[3][1]*p.y()+mat_[3][2]*p.z()+mat_[3][3]);
}


PointF
Transform::project(const PointF& p) const
{
  return PointF(static_cast<float>(mat_[0][0]*p.x()+mat_[0][1]*p.y()+mat_[0][2]*p.z()+mat_[0][3]),
                static_cast<float>(mat_[1][0]*p.x()+mat_[1][1]*p.y()+mat_[1][2]*p.z()+mat_[1][3]),
                static_cast<float>(mat_[2][0]*p.x()+mat_[2][1]*p.y()+mat_[2][2]*p.z()+mat_[2][3]),
                static_cast<float>(mat_[3][0]*p.x()+mat_[3][1]*p.y()+mat_[3][2]*p.z()+mat_[3][3]));
}

Vector
Transform::project(const Vector& p) const
{
  return Vector(mat_[0][0]*p.x()+mat_[0][1]*p.y()+mat_[0][2]*p.z(),
                mat_[1][0]*p.x()+mat_[1][1]*p.y()+mat_[1][2]*p.z(),
                mat_[2][0]*p.x()+mat_[2][1]*p.y()+mat_[2][2]*p.z());
}

VectorF
Transform::project(const VectorF& p) const
{
  return VectorF(static_cast<float>(mat_[0][0]*p.x()+mat_[0][1]*p.y()+mat_[0][2]*p.z()),
                 static_cast<float>(mat_[1][0]*p.x()+mat_[1][1]*p.y()+mat_[1][2]*p.z()),
                 static_cast<float>(mat_[2][0]*p.x()+mat_[2][1]*p.y()+mat_[2][2]*p.z()));
}

Point
Transform::unproject(const Point& p) const
{
  return Point(imat_[0][0]*p.x()+imat_[0][1]*p.y()+imat_[0][2]*p.z()+imat_[0][3],
               imat_[1][0]*p.x()+imat_[1][1]*p.y()+imat_[1][2]*p.z()+imat_[1][3],
               imat_[2][0]*p.x()+imat_[2][1]*p.y()+imat_[2][2]*p.z()+imat_[2][3],
               imat_[3][0]*p.x()+imat_[3][1]*p.y()+imat_[3][2]*p.z()+imat_[3][3]);
}

Vector
Transform::unproject(const Vector& p) const
{
  return Vector(imat_[0][0]*p.x()+imat_[0][1]*p.y()+imat_[0][2]*p.z(),
                imat_[1][0]*p.x()+imat_[1][1]*p.y()+imat_[1][2]*p.z(),
                imat_[2][0]*p.x()+imat_[2][1]*p.y()+imat_[2][2]*p.z());
}

PointF
Transform::unproject(const PointF& p) const
{
  return PointF(static_cast<float>(imat_[0][0]*p.x()+imat_[0][1]*p.y()+imat_[0][2]*p.z()+imat_[0][3]),
                static_cast<float>(imat_[1][0]*p.x()+imat_[1][1]*p.y()+imat_[1][2]*p.z()+imat_[1][3]),
                static_cast<float>(imat_[2][0]*p.x()+imat_[2][1]*p.y()+imat_[2][2]*p.z()+imat_[2][3]),
                static_cast<float>(imat_[3][0]*p.x()+imat_[3][1]*p.y()+imat_[3][2]*p.z()+imat_[3][3]));
}

VectorF
Transform::unproject(const VectorF& p) const
{
  return VectorF(static_cast<float>(imat_[0][0]*p.x()+imat_[0][1]*p.y()+imat_[0][2]*p.z()),
                 static_cast<float>(imat_[1][0]*p.x()+imat_[1][1]*p.y()+imat_[1][2]*p.z()),
                 static_cast<float>(imat_[2][0]*p.x()+imat_[2][1]*p.y()+imat_[2][2]*p.z()));
}

Vector
Transform::project_normal(const Vector& p) const
{
  return Vector(imat_[0][0]*p.x()+imat_[1][0]*p.y()+imat_[2][0]*p.z(),
                imat_[0][1]*p.x()+imat_[1][1]*p.y()+imat_[2][1]*p.z(),
                imat_[0][2]*p.x()+imat_[1][2]*p.y()+imat_[2][2]*p.z());
}

VectorF
Transform::project_normal(const VectorF& p) const
{
  return VectorF(static_cast<float>(imat_[0][0]*p.x()+imat_[1][0]*p.y()+imat_[2][0]*p.z()),
                 static_cast<float>(imat_[0][1]*p.x()+imat_[1][1]*p.y()+imat_[2][1]*p.z()),
                 static_cast<float>(imat_[0][2]*p.x()+imat_[1][2]*p.y()+imat_[2][2]*p.z()));
}


void
Transform::get(double* data) const
{
  for(size_t i=0;i<4;i++)
    for(size_t j=0;j<4;j++)
    {
      *data=mat_[i][j]; data++;
    }
}

void
Transform::get_trans(double* data) const
{
  for(size_t i=0;i<4;i++)
    for(size_t j=0;j<4;j++)
    {
      *data=mat_[j][i]; data++;
    }
}

void
Transform::set(double* data)
{
  for(size_t i=0;i<4;i++)
    for(size_t j=0;j<4;j++)
    {
      mat_[i][j]= *data; data++;
    }
  
  compute_imat();
}

void
Transform::set_trans(double* data)
{
  for(size_t i=0;i<4;i++)
    for(size_t j=0;j<4;j++)
    {
      mat_[j][i]= *data; data++;
    }
  
  compute_imat();
}

void
Transform::load_identity()
{
  for(size_t i=0;i<4;i++)
    for(size_t j=0;j<4;j++)
    {
      if (i == j) { mat_[i][j] = 1.0; imat_[i][j] = 1.0; }
      else { mat_[i][j] = 0.0; imat_[i][j] = 0.0; }
    }
}

void
Transform::install_mat(double m[4][4])
{
  for(size_t i=0;i<4;i++)
    for(size_t j=0;j<4;j++)
      mat_[i][j] = m[i][j];
}

void
Transform::load_identity(double m[4][4]) 
{
  m[0][0] = 1.0; m[0][1] = 0.0; m[0][2] = 0.0; m[0][3] = 0.0;
  m[1][0] = 0.0; m[1][1] = 1.0; m[1][2] = 0.0; m[1][3] = 0.0;
  m[2][0] = 0.0; m[2][1] = 0.0; m[2][2] = 1.0; m[2][3] = 0.0;
  m[3][0] = 0.0; m[3][1] = 0.0; m[3][2] = 0.0; m[3][3] = 1.0;
}

void
Transform::invert()
{
  for (size_t i=0; i<4; i++)
    for (size_t j=0; j<4; j++) 
    {
      double tmp=mat_[i][j];
      mat_[i][j]=imat_[i][j];
      imat_[i][j]=tmp;
    }
}

void
Transform::compute_imat()
{
  double a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p;
  a=mat_[0][0]; b=mat_[0][1]; c=mat_[0][2]; d=mat_[0][3];
  e=mat_[1][0]; f=mat_[1][1]; g=mat_[1][2]; h=mat_[1][3];
  i=mat_[2][0]; j=mat_[2][1]; k=mat_[2][2]; l=mat_[2][3];
  m=mat_[3][0]; n=mat_[3][1]; o=mat_[3][2]; p=mat_[3][3];

  double q=a*f*k*p - a*f*l*o - a*j*g*p + a*j*h*o + a*n*g*l - a*n*h*k
    - e*b*k*p + e*b*l*o + e*j*c*p - e*j*d*o - e*n*c*l + e*n*d*k
    + i*b*g*p - i*b*h*o - i*f*c*p + i*f*d*o + i*n*c*h - i*n*d*g
    - m*b*g*l + m*b*h*k + m*f*c*l - m*f*d*k - m*j*c*h + m*j*d*g;

  // This test is imperfect. The condition number may be a good indicator,
  // however this is not a perfect indicator neither.
  
  if (Abs(q)==0.0) 
  {
    imat_[0][0]=imat_[1][1]=imat_[2][2]=imat_[3][3]=1.0;
    imat_[1][0]=imat_[1][2]=imat_[1][3]=imat_[0][1]=0.0;
    imat_[2][0]=imat_[2][1]=imat_[2][3]=imat_[0][2]=0.0;
    imat_[3][0]=imat_[3][1]=imat_[3][2]=imat_[0][3]=0.0;
    return;
  }
  
  double iq = 1.0/q;
  imat_[0][0]=(f*k*p - f*l*o - j*g*p + j*h*o + n*g*l - n*h*k)*iq;
  imat_[0][1]=-(b*k*p - b*l*o - j*c*p + j*d*o + n*c*l - n*d*k)*iq;
  imat_[0][2]=(b*g*p - b*h*o - f*c*p + f*d*o + n*c*h - n*d*g)*iq;
  imat_[0][3]=-(b*g*l - b*h*k - f*c*l + f*d*k + j*c*h - j*d*g)*iq;

  imat_[1][0]=-(e*k*p - e*l*o - i*g*p + i*h*o + m*g*l - m*h*k)*iq;
  imat_[1][1]=(a*k*p - a*l*o - i*c*p + i*d*o + m*c*l - m*d*k)*iq;
  imat_[1][2]=-(a*g*p - a*h*o - e*c*p + e*d*o + m*c*h - m*d*g)*iq;
  imat_[1][3]=(a*g*l - a*h*k - e*c*l + e*d*k + i*c*h - i*d*g)*iq;

  imat_[2][0]=(e*j*p - e*l*n - i*f*p + i*h*n + m*f*l - m*h*j)*iq;
  imat_[2][1]=-(a*j*p - a*l*n - i*b*p + i*d*n + m*b*l - m*d*j)*iq;
  imat_[2][2]=(a*f*p - a*h*n - e*b*p + e*d*n + m*b*h - m*d*f)*iq;
  imat_[2][3]=-(a*f*l - a*h*j - e*b*l + e*d*j + i*b*h - i*d*f)*iq;

  imat_[3][0]=-(e*j*o - e*k*n - i*f*o + i*g*n + m*f*k - m*g*j)*iq;
  imat_[3][1]=(a*j*o - a*k*n - i*b*o + i*c*n + m*b*k - m*c*j)*iq;
  imat_[3][2]=-(a*f*o - a*g*n - e*b*o + e*c*n + m*b*g - m*c*f)*iq;
  imat_[3][3]=(a*f*k - a*g*j - e*b*k + e*c*j + i*b*g - i*c*f)*iq;
}

void
Transform::post_mulmat(const double mmat[4][4])
{
  double newmat[4][4];
  for(int i=0;i<4;i++)
  {
    for(int j=0;j<4;j++)
    {
      newmat[i][j]=0.0;
      for(int k=0;k<4;k++)
      {
        newmat[i][j]+=mat_[i][k]*mmat[k][j];
      }
    }
  }
  install_mat(newmat);
}

void
Transform::pre_mulmat(const double mmat[4][4])
{
  double newmat[4][4];
  for(int i=0;i<4;i++)
  {
    for(int j=0;j<4;j++)
    {
      newmat[i][j]=0.0;
      for(int k=0;k<4;k++)
      {
        newmat[i][j]+=mmat[i][k]*mat_[k][j];
      }
    }
  }
  install_mat(newmat);
}

void
Transform::perspective(const Point& eyep, const Point& lookat,
                       const Vector& up, double fov,
                       double znear, double zfar,
                       int xres, int yres)
{
  Vector lookdir(lookat-eyep);
  Vector z(lookdir); z.normalize();
  Vector x(Cross(z, up)); x.normalize();
  Vector y(Cross(x, z));
  double xviewsize=Tan(DegreeToRadian(fov/2.0))*2.0;
  double yviewsize=xviewsize*yres/xres;
  double zscale=-znear;
  double xscale=xviewsize*0.5;
  double yscale=yviewsize*0.5;
  x*=xscale;
  y*=yscale;
  z*=zscale;

  Transform tf(eyep,x,y,z);
  tf.invert();
  pre_trans(tf);
    
  // Perspective...
  double m[4][4];
  m[0][0]=1.0; m[0][1]=0.0; m[0][2]=0.0; m[0][3]=0.0;
  m[1][0]=0.0; m[1][1]=1.0; m[1][2]=0.0; m[1][3]=0.0;
  m[2][0]=0.0; m[2][1]=0.0; m[2][2]=-(zfar-1)/(1+zfar); m[2][3]=-2*zfar/(1+zfar);
  m[3][0]=0.0; m[3][1]=0.0; m[3][2]=-1.0; m[3][3]=0.0;
  pre_mulmat(m);

  pre_scale(Vector(1.0,-1.0,1.0)); // X starts at the top...
  pre_translate(Vector(1.0,1.0,0.0));
  pre_scale(Vector(xres/2.0, yres/2.0, 1.0));     
}


Point
operator*(Transform &t, const Point &d)
{
  double result[4], tmp[4];
  result[0] = result[1] = result[2] = result[3] = 0.0;
  tmp[0] = d.x(); tmp[1] = d.y(); tmp[2] = d.z();  tmp[3] = 1.0;

  for(size_t i=0;i<4;i++) 
    for(size_t j=0;j<4;j++) 
      result[i] += t.mat_[i][j] * tmp[j];

  return Point(result[0], result[1], result[2]);
}

Vector
operator*(Transform &t, const Vector &d)
{
  double result[4], tmp[4];
  result[0] = result[1] = result[2] = result[3] = 0.0;
  tmp[0] = d.x(); tmp[1] = d.y(); tmp[2] = d.z(); tmp[3] = 0.0;

  for(size_t i=0;i<4;i++) 
    for(size_t j=0;j<4;j++) 
      result[i] += t.mat_[i][j] * tmp[j];

  return Vector(result[0], result[1], result[2]);
}

PointF
operator*(Transform &t, const PointF &d)
{
  double tmp[4];
  float  result[4];

  result[0] = result[1] = result[2] = result[3] = 0.0f;
  tmp[0] = static_cast<double>(d.x()); 
  tmp[1] = static_cast<double>(d.y()); 
  tmp[2] = static_cast<double>(d.z());  
  tmp[3] = 1.0;

  for(size_t i=0;i<4;i++) 
    for(size_t j=0;j<4;j++) 
      result[i] += static_cast<float>(t.mat_[i][j] * tmp[j]);

  return PointF(result[0], result[1], result[2]);
}

VectorF
operator*(Transform &t, const VectorF &d)
{
  double tmp[4];
  float  result[4];

  result[0] = result[1] = result[2] = result[3] = 0.0f;
  tmp[0] = static_cast<double>(d.x()); 
  tmp[1] = static_cast<double>(d.y()); 
  tmp[2] = static_cast<double>(d.z()); 
  tmp[3] = 0.0;

  for(size_t i=0;i<4;i++) 
    for(size_t j=0;j<4;j++) 
      result[i] += static_cast<float>(t.mat_[i][j] * tmp[j]);

  return VectorF(result[0], result[1], result[2]);
}


} // namespace Utils
