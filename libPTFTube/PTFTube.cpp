#include "PTFTube.h"

#include <osg/Geometry>
#include <osg/Geode>
#include <osg/StateSet>
#include <osg/LineWidth>
#include <osg/BlendFunc>

PTFTube::PTFTube(const std::vector<osg::Vec3f> &path, float radius, int numProfPts)
    : m_Ps(path)
{
    this->extractTangents();
    this->makeCircleProfile(radius, numProfPts);
}

void PTFTube::build()
{
    m_frames.clear();
    int n = m_Ps.size();
    if (n>=3){
        m_frames.resize(n);
        m_frames[0] = firstFrame(m_Ps[0], m_Ps[1], m_Ps[2]);
        for (int i=1; i<n; ++i){
            osg::Vec3f prevT = m_Ts[i-1];
            osg::Vec3f currT = m_Ts[i];
            m_frames[i] = nextFrame(m_frames[i-1], m_Ps[i-1], m_Ps[i], prevT, currT);
        }
        m_frames[n-1] = lastFrame(m_frames[n-2], m_Ps[n-2], m_Ps[n-1]);
    }
}

osg::Node *PTFTube::generatePath() const
{
    // vertex array
    osg::Vec3Array* vertices = new osg::Vec3Array;
    for (unsigned int i=0; i<m_Ps.size(); ++i)
        vertices->push_back(m_Ps.at(i));

    osg::Vec4Array* colors = new osg::Vec4Array;
    colors->push_back(osg::Vec4f(0.f,1.f,0.f,1.f));

    osg::Geometry* geom = new osg::Geometry;
    geom->setVertexArray(vertices);
    geom->setColorArray(colors, osg::Array::BIND_OVERALL);
    geom->addPrimitiveSet(new osg::DrawArrays(GL_LINE_STRIP, 0, vertices->size()));

    osg::Geode* geode = new osg::Geode;
    geode->addDrawable(geom);
    geode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

    return geode;
}

osg::Node *PTFTube::generateWireFrame() const
{
    if (m_Ps.size() != m_frames.size() || m_frames.size()<3 || m_prof.empty())
        return nullptr;

    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
    osg::Geometry* geom = new osg::Geometry;
    geode->addDrawable(geom);

    osg::Vec4Array* colors = new osg::Vec4Array;
    colors->push_back(osg::Vec4f(0.7f,0,0.7f,1.f));
    geom->setColorArray(colors, osg::Array::BIND_OVERALL);

    osg::Vec3Array* vertices = new osg::Vec3Array;
    for (unsigned int i=0; i<m_Ps.size()-1; ++i){
        if (i==0){
            // profile line if it's the first point
            for (unsigned int ci=0; ci<m_prof.size(); ++ci){
                vertices->push_back(m_prof[ci] * m_frames[i]);
            }
        }

        for (unsigned int ci=0; ci<m_prof.size(); ++ci){
            int idx0 = ci;
            int idx1 = (ci == (m_prof.size() - 1)) ? 0 : ci + 1;
            osg::Vec3f P0 = m_prof[idx0] * m_frames[i];
            osg::Vec3f P1 = m_prof[idx1] * m_frames[i];
            osg::Vec3f P2 = m_prof[idx1] * m_frames[i+1];
            osg::Vec3f P3 = m_prof[idx0] * m_frames[i+1];

            // first triangle
            vertices->push_back(P0);
            vertices->push_back(P3);
            vertices->push_back(P1);

            // second triangle
            vertices->push_back(P1);
            vertices->push_back(P3);
            vertices->push_back(P2);
        }
    }
    geom->setVertexArray(vertices);
    geom->addPrimitiveSet(new osg::DrawArrays(GL_LINE_STRIP, 0, vertices->size()));
    geom->getOrCreateStateSet()->setAttribute(new osg::LineWidth(1.5f), osg::StateAttribute::ON);

    geode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

    return geode.release();
}

osg::Node *PTFTube::generateFrameSlices(float scale) const
{
    if (m_Ps.empty() || m_frames.empty())
        return nullptr;

    osg::ref_ptr<osg::Geode> geode = new osg::Geode;

    osg::Geometry* geomLines = new osg::Geometry;
    osg::Geometry* geomQuads = new osg::Geometry;

    geode->addDrawable(geomLines);
    geode->addDrawable(geomQuads);

    osg::Vec4Array* colorsLines = new osg::Vec4Array;
    colorsLines->push_back(osg::Vec4f(1,1,1,0.75f));
    geomLines->setColorArray(colorsLines, osg::Array::BIND_OVERALL);

    osg::Vec4Array* colorsQuads = new osg::Vec4Array;
    colorsQuads->push_back(osg::Vec4f(1,1,1,0.25f));
    geomQuads->setColorArray(colorsQuads, osg::Array::BIND_OVERALL);

    // bounding lines
    osg::Vec3Array* vertLines = new osg::Vec3Array;
    for (unsigned int i=0; i<m_Ps.size(); ++i){
        vertLines->push_back(osg::Vec3f(-1,1,0) * scale * m_frames[i]);
        vertLines->push_back(osg::Vec3f(1,1,0) * scale * m_frames[i]);

        vertLines->push_back(osg::Vec3f(1,1,0) * scale * m_frames[i]);
        vertLines->push_back(osg::Vec3f(1,-1,0) * scale * m_frames[i]);

        vertLines->push_back(osg::Vec3f(1,-1,0) * scale * m_frames[i]);
        vertLines->push_back(osg::Vec3f(-1,-1,0) * scale * m_frames[i]);

        vertLines->push_back(osg::Vec3f(-1,-1,0) * scale * m_frames[i]);
        vertLines->push_back(osg::Vec3f(-1,1,0) * scale * m_frames[i]);
    }
    geomLines->setVertexArray(vertLines);
    geomLines->addPrimitiveSet(new osg::DrawArrays(GL_LINES, 0, vertLines->size()));

    // quads
    osg::Vec3Array* vertQuads = new osg::Vec3Array;
    for (unsigned int i=0; i<m_Ps.size(); ++i){
        vertQuads->push_back(osg::Vec3f(-1,1,0) * scale * m_frames[i]);
        vertQuads->push_back(osg::Vec3f(1,1,0) * scale * m_frames[i]);
        vertQuads->push_back(osg::Vec3f(1,-1,0) * scale * m_frames[i]);
        vertQuads->push_back(osg::Vec3f(-1,-1,0) * scale * m_frames[i]);
    }

    geomQuads->setVertexArray(vertQuads);
    geomQuads->addPrimitiveSet(new osg::DrawArrays(GL_QUADS, 0, vertQuads->size()));

    geode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
    geode->getOrCreateStateSet()->setAttributeAndModes(new osg::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

    return geode.release();
}

osg::Node *PTFTube::generateTriMesh() const
{
    if (m_Ps.size() != m_frames.size() || m_frames.size()<3 || m_prof.empty())
        return nullptr;

    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
    osg::Geometry* geom = new osg::Geometry;
    geode->addDrawable(geom);

    osg::Vec4Array* colors = new osg::Vec4Array;
    colors->push_back(osg::Vec4f(0.5f,0.5,0.7f,1.f));
    geom->setColorArray(colors, osg::Array::BIND_OVERALL);

    osg::Vec3Array* vertices = new osg::Vec3Array;
    osg::Vec3Array* normals = new osg::Vec3Array;

    // first frame cover
    for (unsigned int ci=1; ci<m_prof.size()-1; ++ci){
        osg::Vec3f P0 = m_prof[0] * m_frames.front();
        osg::Vec3f P1 = m_prof[ci] * m_frames.front();
        osg::Vec3f P2 = m_prof[ci+1] * m_frames.front();

        vertices->push_back(P2);
        vertices->push_back(P0);
        vertices->push_back(P1);

        // normals
        osg::Vec3f n = this->getNormal(P0, P1, P2);
        normals->push_back(n);
        normals->push_back(n);
        normals->push_back(n);
    }

    // frame structure
    // for each point in path
    for (unsigned int i=0; i<m_Ps.size()-1; ++i){
        // for each point in profile
        for (unsigned int ci=0; ci<m_prof.size(); ++ci){
            int idx0 = ci;
            int idx1 = (ci == (m_prof.size() - 1)) ? 0 : ci + 1;
            osg::Vec3f P0 = m_prof[idx0] * m_frames[i];
            osg::Vec3f P1 = m_prof[idx1] * m_frames[i];
            osg::Vec3f P2 = m_prof[idx1] * m_frames[i+1];
            osg::Vec3f P3 = m_prof[idx0] * m_frames[i+1];

            // normal for the given face
            osg::Vec3f n = this->getNormal(P0, P3, P1);

            // first triangle
            vertices->push_back(P0);
            vertices->push_back(P3);
            vertices->push_back(P1);

            normals->push_back(n);
            normals->push_back(n);
            normals->push_back(n);

            // second triangle
            vertices->push_back(P1);
            vertices->push_back(P3);
            vertices->push_back(P2);

            normals->push_back(n);
            normals->push_back(n);
            normals->push_back(n);
        }
    }
    //last frame cover
    for (unsigned int ci=1; ci<m_prof.size()-1; ++ci){
        osg::Vec3f P0 = m_prof[0] * m_frames.back();
        osg::Vec3f P1 = m_prof[ci] * m_frames.back();
        osg::Vec3f P2 = m_prof[ci+1] * m_frames.back();

        vertices->push_back(P1);
        vertices->push_back(P0);
        vertices->push_back(P2);

        // normals
        osg::Vec3f n = this->getNormal(P0, P1, P2);
        normals->push_back(n);
        normals->push_back(n);
        normals->push_back(n);
    }

    geom->setVertexArray(vertices);
    geom->setNormalArray(normals, osg::Array::BIND_PER_VERTEX);
    geom->addPrimitiveSet(new osg::DrawArrays(GL_TRIANGLES, 0, vertices->size()));

    geode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::ON);
    geode->getOrCreateStateSet()->setMode(GL_LIGHT0, osg::StateAttribute::ON);
    geode->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON);

    return geode.release();
}

void PTFTube::extractTangents()
{
    m_Ts.clear();
    for (unsigned int i=0; i<m_Ps.size(); ++i){
        osg::Vec3f t;
        if (i==0)
            t = m_Ps[i+1]-m_Ps[i];
        else if (i<m_Ps.size()-1){
            osg::Vec3f dt = m_Ps[i+1] - m_Ps[i-1];
            t = dt / dt.length();
        }
        else{
            t = m_Ps[i]-m_Ps[i-1];
        }
        t.normalize();

        m_Ts.push_back(t);
    }
}

void PTFTube::makeCircleProfile(float radius, int segments)
{
    m_prof.clear();
    float dt = 2.f*E_PI/(float)segments;
    for (int i=0; i<segments; ++i){
        float t= i*dt;
        m_prof.push_back(osg::Vec3f(std::cos(t)*radius, std::sin(t)*radius, 0.f));
    }
}

osg::Matrix PTFTube::firstFrame(const osg::Vec3f &firstPoint, const osg::Vec3f &secondPoint, const osg::Vec3f &thirdPoint)
{
    osg::Vec3f t = secondPoint - firstPoint;
    t.normalize();

    osg::Vec3f n = t ^ (thirdPoint - firstPoint);
    n.normalize();

    // if two vectors are collinear, an arbitrary twist value is chosen
    if (n.length() == 0.f){
        int i = std::fabs(t[0]) < std::fabs(t[1]) ? 0 : 1;
        if (std::fabs(t[2])<std::fabs(t[i])) i=2;

        osg::Vec3f v;
        v[i] = 1.f;

        n = t^v;
        n.normalize();
    }
    osg::Vec3f b = t^n;

    osg::Matrix M(b[0], b[1], b[2], 0.f,
            n[0], n[1], n[2], 0.f,
            t[0], t[1], t[2], 0.f,
            firstPoint[0], firstPoint[1], firstPoint[2], 1.f);

    return M;
}

osg::Matrix PTFTube::nextFrame(const osg::Matrix &prevMatrix, const osg::Vec3f &prevPoint, const osg::Vec3f &currPoint, osg::Vec3f prevTangent, osg::Vec3f currTangent)
{
    osg::Vec3f axis;
    float r = 0.f;

    if (prevTangent.length() != 0 && currTangent.length() != 0){
        prevTangent.normalize();
        currTangent.normalize();
        float dot = prevTangent * currTangent;
        if (dot>1)
            dot=1;
        else if (dot<-1)
            dot=-1;

        r = std::acos(dot);
        axis = prevTangent ^ currTangent;
    }
    if (axis.length() != 0 && r!=0){
        osg::Matrix R = osg::Matrix::rotate(r, axis);
        osg::Matrix Tj = osg::Matrix::translate(currPoint);
        osg::Matrix Ti = osg::Matrix::translate(-prevPoint);

        return prevMatrix*Ti*R*Tj;
    }
    else{
        osg::Matrix Tr = osg::Matrix::translate(currPoint - prevPoint);
        return prevMatrix*Tr;
    }
}

osg::Matrix PTFTube::lastFrame(const osg::Matrix &prevMatrix, const osg::Vec3f &prevPoint, const osg::Vec3f &lastPoint)
{
    return prevMatrix * osg::Matrix::translate(lastPoint - prevPoint);
}

osg::Vec3f PTFTube::getNormal(const osg::Vec3f &P0, const osg::Vec3f &P1, const osg::Vec3f &P2) const
{
    osg::Vec3f a = P1-P0;
    osg::Vec3f b = P2-P0;
    osg::Vec3f n = a^b;
    n.normalize();
    return n;
}

#include "PTFTube.h"
