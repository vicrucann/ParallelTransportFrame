#ifndef PTFTUBE_H
#define PTFTUBE_H

#include <vector>
#include <osg/Array>
#include <osg/Matrix>
#include <osg/Node>

class PTFTube
{
public:
    /*! Constructor copies path data, creates profile data and calculates tangents.
     * \param path is a set of 3D points along which to extrude,
     * \param radius is the tube's radius,
     * \param numProfPts defines extrustion shape, e.g. 3 is triangle, 4 is square, and so on. */
    PTFTube(const std::vector<osg::Vec3f>& path, float radius, int numProfPts = 4);

    /*! Compute consequtive frames along the path. */
    void build();

    /*! Build original path geometry. */
    osg::Node* generatePath() const;

    /*! The frame geometry is built using line_strip as follows:
     * 1. Define two consequitive points along profile.
     * 2. Find same-index profile points along the next frame.
     * 3. Connect: P0-P3-P1 and P1-P3-P2.
     * 4. Make sure the first frame's profile is drawn (we always assume the path is not closed).
     *
     * P0     P1  - frame[i]
     * |   /
     * | /
     * P3-----P2  - frame[i+1]
     * \return geode which contains wire geometry of the tube. */
    osg::Node* generateWireFrame() const;

    /*! \return geometry containing each frame's location as rectangles. */
    osg::Node* generateFrameSlices(float scale = 1.f) const;

    /*! The mesh geometry is build using triangules. \sa buildWireFrame(). */
    osg::Node* generateTriMesh() const;

protected:
    /*! Compute first derivatives for the path. */
    void extractTangents();

    /*! Computer a set of points for extrusion profile. */
    void makeCircleProfile(float radius = 0.25f, int segments = 16);

    /*! Compute the first reference frame along a path.
     * \return transformation matrix to the reference frame defined by the three points. */
    osg::Matrix firstFrame(const osg::Vec3f& firstPoint, const osg::Vec3f& secondPoint, const osg::Vec3f& thirdPoint);

    /*! Compute the next reference frame along a path.
     * \return transformation matrix to the next reference frame defined by the previously computed transformation matrix,
     * and the new point and tangent vector along the curve. */
    osg::Matrix nextFrame(const osg::Matrix& prevMatrix, const osg::Vec3f& prevPoint, const osg::Vec3f& currPoint,
                              osg::Vec3f prevTangent, osg::Vec3f currTangent);

    /*! Compute the last reference frame along a path.
     * \return the transformation matrix to the last reference frame defined by the previously computed transformation matrix
     * and the last point along the path. */
    osg::Matrix lastFrame(const osg::Matrix& prevMatrix, const osg::Vec3f& prevPoint, const osg::Vec3f& lastPoint);

    /*! \return normal to the plane given by three input points. */
    osg::Vec3f getNormal(const osg::Vec3f& P0, const osg::Vec3f& P1, const osg::Vec3f& P2) const;

private:
    std::vector<osg::Vec3f>         m_Ps; //!< points
    std::vector<osg::Vec3f>         m_prof; //!< profile shape
    std::vector<osg::Vec3f>         m_Ts; //!< tangents
    std::vector<osg::Matrix>        m_frames; //!< result frames

    const float E_PI = 3.14159265359f;

};

#endif // PTFTUBE_H
