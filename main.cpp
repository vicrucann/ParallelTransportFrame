#include <iostream>
#include <stdio.h>

#ifdef _WIN32
#include <Windows.h>
#endif

#include <osgViewer/Viewer>
#include <osg/Node>
#include <osg/ShapeDrawable>
#include <osgDB/WriteFile>
#include <osgGA/EventHandler>
#include <osg/Switch>

#include "libPTFTube/PTFTube.h"

const int OSG_WIDTH = 900;
const int OSG_HEIGHT = 900;

osg::Node* createReferenceShape()
{
    osg::Cylinder* cylinder = new osg::Cylinder(osg::Vec3( 0.f, 0.f, 0.f ), 0.5f, 2.f);
    osg::ShapeDrawable* sd = new osg::ShapeDrawable( cylinder );
    sd->setColor( osg::Vec4( 0.8f, 0.5f, 0.2f, 1.f ) );
    osg::Geode* geode = new osg::Geode;
    geode->addDrawable(sd);
    geode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::ON);

    return geode;
}

std::vector<osg::Vec3f> createLinePoints3d()
{
    std::vector<osg::Vec3f> points3d;
    int angle = 0;
    float z=0.f;
    float pi = 3.141592f;
    for (angle; angle<=360*2; angle+=10, z+=0.3f){
        float rad = angle*pi/180.f;
        float dx=0, dy=0, dz=0;
        dx = std::sin(rad)*2.f;
        dy = std::cos(rad)*4.f;
        dz = z;
        points3d.push_back(osg::Vec3f(dx,dy,dz));
    }
    return points3d;
}

class Switch : public osg::Switch
{
public:
    Switch(const PTFTube& extrusion)
        : osg::Switch()
    {
        osg::ref_ptr<osg::Node> path = extrusion.generatePath();
        osg::ref_ptr<osg::Node> mesh = extrusion.generateTriMesh();
        osg::ref_ptr<osg::Node> slices = extrusion.generateFrameSlices(1.f);
        osg::ref_ptr<osg::Node> wire = extrusion.generateWireFrame();

        this->addChild(path.get(), false);
        this->addChild(wire.get(), false);
        this->addChild(slices.get(), false);
        this->addChild(mesh.get(), true);
    }
};

class EventHandler : public osgGA::GUIEventHandler
{
public:
    EventHandler(Switch* all)
        : osgGA::GUIEventHandler()
        , m_root(all)
    {
    }

    virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
    {
        unsigned int index = 3;
        switch (ea.getEventType()){
        case (osgGA::GUIEventAdapter::KEYUP):
            switch (ea.getKey()){
            case '0':
                std::cout << "Original path display.\n";
                index = 0;
                break;
            case '1':
                std::cout << "Wire frame display.\n";
                index = 1;
                break;
            case '2':
                std::cout << "Frame slice display.\n";
                index = 2;
                break;
            case '3':
                std::cout << "Triangular mesh display.\n";
                index = 3;
                break;
            default:
                for (unsigned int i=0; i<m_root->getNumChildren(); ++i){
                    if (m_root->getValue(i))
                        index = i;
                }
                break;
            }

            for (unsigned int i=0; i< m_root->getNumChildren(); ++i){
                if (i == index)
                    m_root->setValue(i, true);
                else
                    m_root->setValue(i, false);
            }
        default:
            break;
        }
        return false;
    }

private:
    osg::observer_ptr<Switch> m_root;
};

int main(int, char**)
{
#ifdef _WIN32
    ::SetProcessDPIAware();
#endif

    osgViewer::Viewer viewer;
    viewer.setUpViewInWindow(100,100,OSG_WIDTH, OSG_HEIGHT);

    osg::ref_ptr<osg::Group> root = new osg::Group();
//    root->addChild(createReferenceShape());

    PTFTube extrusion(createLinePoints3d(), 0.5, 5);
    extrusion.build();

    Switch* geometries = new Switch(extrusion);
    root->addChild(geometries);

    viewer.setSceneData(root.get());

    EventHandler* EH = new EventHandler(geometries);
    viewer.addEventHandler(EH);

    return viewer.run();
}
